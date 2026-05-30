/**
 * @file    control_task.c
 * @brief   自动控制逻辑实现
 *
 * 核心策略:
 *   灌溉: 任一土壤传感器湿度 < SOIL_MOISTURE_MIN → 开水泵
 *         全部 > SOIL_MOISTURE_MAX → 关水泵
 *         安全保护: 连续运行 > PUMP_MAX_ON_TIME → 强制停止
 *         间隔保护: 两次灌溉间隔 > PUMP_MIN_INTERVAL
 *   补光: 光照 < LUX_MIN 且非下雨 → 开补光灯 (下雨天不补光)
 *         光照 > LUX_LED_OFF → 关补光灯
 *   通风: 温度 > TEMP_MAX → 开风扇
 *         温度 < TEMP_VENT_STOP → 关风扇
 *   报警: 传感器故障或水泵超时 → 蜂鸣器
 */

#include "control_task.h"

/* 灌溉状态机 */
typedef enum {
    IRRIG_IDLE,         /* 空闲: 等待触发条件 */
    IRRIG_RUNNING,      /* 灌溉中 */
    IRRIG_COOLDOWN      /* 冷却期: 最小间隔保护 */
} irrig_state_t;

static irrig_state_t  irrig_state = IRRIG_IDLE;
static uint32_t       pump_start_tick = 0;
static uint32_t       pump_stop_tick  = 0;

/* 统计灌溉数据 */
static uint32_t       total_irrig_count = 0;
static uint32_t       total_irrig_seconds = 0;

/**
 * @brief  检查是否有任一土壤传感器低于阈值
 */
static uint8_t Soil_NeedWater(sensor_data_t *data)
{
    for (uint8_t i = 0; i < SOIL_ADC_NUM; i++) {
        if (data->soil_moisture[i] >= 0 && data->soil_moisture[i] < SOIL_MOISTURE_MIN) {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief  检查是否所有土壤传感器都高于上限
 */
static uint8_t Soil_IsEnough(sensor_data_t *data)
{
    for (uint8_t i = 0; i < SOIL_ADC_NUM; i++) {
        if (data->soil_moisture[i] >= 0 && data->soil_moisture[i] < SOIL_MOISTURE_MAX) {
            return 0;
        }
    }
    return 1;
}

/**
 * @brief  灌溉控制状态机
 */
static void Irrig_Control(sensor_data_t *data)
{
    uint32_t now = xTaskGetTickCount();
    uint32_t pump_runtime = 0;

    if (Pump_IsOn()) {
        pump_runtime = (now - pump_start_tick) / configTICK_RATE_HZ;
    }

    switch (irrig_state) {
    case IRRIG_IDLE:
        if (Soil_NeedWater(data) && !data->is_raining) {
            /* 检查最小灌溉间隔 */
            uint32_t since_last = (now - pump_stop_tick) / configTICK_RATE_HZ;
            if (pump_stop_tick == 0 || since_last > PUMP_MIN_INTERVAL) {
                Pump_On();
                pump_start_tick = now;
                irrig_state = IRRIG_RUNNING;
                total_irrig_count++;
            }
        }
        break;

    case IRRIG_RUNNING:
        /* 安全保护: 超时强制停止 */
        if (pump_runtime > PUMP_MAX_ON_TIME) {
            Pump_Off();
            pump_stop_tick = now;
            irrig_state = IRRIG_COOLDOWN;
            total_irrig_seconds += pump_runtime;

            /* 告警: 超时说明可能漏水/传感器失灵 */
            Buzzer_Beep(ALARM_BUZZER_MS);
            break;
        }

        /* 正常停止: 湿度达标或下雨 */
        if (Soil_IsEnough(data) || data->is_raining) {
            Pump_Off();
            pump_stop_tick = now;
            irrig_state = IRRIG_COOLDOWN;
            total_irrig_seconds += pump_runtime;
        }
        break;

    case IRRIG_COOLDOWN:
        /* 冷却期: 等待 PUMP_MIN_INTERVAL 后才能再次灌溉 */
        {
            uint32_t since_stop = (now - pump_stop_tick) / configTICK_RATE_HZ;
            if (since_stop > PUMP_MIN_INTERVAL) {
                irrig_state = IRRIG_IDLE;
            }
        }
        break;
    }
}

/**
 * @brief  补光控制 - 考虑下雨天不补光
 */
static void Light_Control(sensor_data_t *data)
{
    if (data->is_raining) {
        if (Led_IsOn()) Led_Off();
        return;
    }

    if (data->lux < LUX_MIN && !Led_IsOn()) {
        Led_On();
    } else if (data->lux > LUX_LED_OFF && Led_IsOn()) {
        Led_Off();
    }
}

/**
 * @brief  通风控制
 */
static void Fan_Control(sensor_data_t *data)
{
    if (data->temperature > TEMP_MAX && !Fan_IsOn()) {
        Fan_On();
    } else if (data->temperature < TEMP_VENT_STOP && Fan_IsOn()) {
        Fan_Off();
    }
}

/**
 * @brief  传感器故障检测
 */
static uint8_t Sensor_FaultCheck(sensor_data_t *data)
{
    if (data->temperature < -50.0f || data->humidity < -50.0f) {
        return 1;  /* DHT22故障 */
    }
    if (data->lux == 0xFFFF) {
        return 1;  /* BH1750故障 */
    }
    for (uint8_t i = 0; i < SOIL_ADC_NUM; i++) {
        if (data->soil_moisture[i] < -0.5f) {
            return 1;  /* 土壤传感器故障 */
        }
    }
    return 0;
}

/**
 * @brief  更新全局执行器状态
 */
static void Update_ActuatorStatus(void)
{
    g_actuator.pump   = Pump_IsOn() ? ACT_ON : ACT_OFF;
    g_actuator.led    = Led_IsOn()  ? ACT_ON : ACT_OFF;
    g_actuator.fan    = Fan_IsOn()  ? ACT_ON : ACT_OFF;
    g_actuator.buzzer = ACT_OFF;

    if (Pump_IsOn()) {
        uint32_t now = xTaskGetTickCount();
        g_actuator.pump_on_seconds = (now - pump_start_tick) / configTICK_RATE_HZ;
    } else {
        g_actuator.pump_on_seconds = 0;
    }
}

void vControlTask(void *argument)
{
    sensor_data_t sensor_data;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    (void)argument;

    while (1) {
        /* 从队列获取最新传感器数据 (非阻塞) */
        if (xQueueReceive(xSensorQueue, &sensor_data, 0) == pdTRUE) {
            /* 仅在自动模式下执行控制逻辑 */
            if (g_sys_status.mode == MODE_AUTO) {
                /* 传感器故障检测 */
                if (Sensor_FaultCheck(&sensor_data)) {
                    /* 有故障: 关闭所有执行器, 触发报警 */
                    Actuator_AllOff();
                    Buzzer_Beep(ALARM_BUZZER_MS);
                } else {
                    Irrig_Control(&sensor_data);
                    Light_Control(&sensor_data);
                    Fan_Control(&sensor_data);
                }
            }
            /* 更新全局数据 */
            g_sensor_data = sensor_data;
        }

        Update_ActuatorStatus();

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
    }
}
