/**
 * @file    comm_task.c
 * @brief   通信任务实现 - 定时上报数据 + 处理远程指令
 */

#include "comm_task.h"
#include "mqtt.h"
#include "actuator.h"
#include <stdio.h>

uint16_t Comm_PackSensorJson(sensor_data_t *data, char *buf, uint16_t len)
{
    return snprintf(buf, len,
        "{"
        "\"dev\":\"" DEVICE_ID "\","
        "\"ts\":%lu,"
        "\"data\":{"
            "\"soil\":[%.1f,%.1f],"
            "\"temp\":%.1f,"
            "\"humi\":%.1f,"
            "\"lux\":%u,"
            "\"rain\":%u"
        "},"
        "\"ctrl\":{"
            "\"pump\":%u,"
            "\"led\":%u,"
            "\"fan\":%u"
        "}"
        "}",
        data->timestamp,
        data->soil_moisture[0], data->soil_moisture[1],
        data->temperature, data->humidity,
        data->lux, data->is_raining,
        g_actuator.pump, g_actuator.led, g_actuator.fan
    );
}

uint8_t Comm_ParseCommand(const char *json_str)
{
    /* 简易JSON解析: 无需cJSON, 直接字符串匹配 */
    /* 完整实现建议引入cJSON库 */

    if (strstr(json_str, "\"pump_on\"")) {
        if (g_sys_status.mode == MODE_MANUAL) {
            Pump_On();
        }
        return 0;
    }
    if (strstr(json_str, "\"pump_off\"")) {
        Pump_Off();
        return 0;
    }
    if (strstr(json_str, "\"led_on\"")) {
        Led_On();
        return 0;
    }
    if (strstr(json_str, "\"led_off\"")) {
        Led_Off();
        return 0;
    }
    if (strstr(json_str, "\"fan_on\"")) {
        Fan_On();
        return 0;
    }
    if (strstr(json_str, "\"fan_off\"")) {
        Fan_Off();
        return 0;
    }
    if (strstr(json_str, "\"auto_mode\"")) {
        g_sys_status.mode = MODE_AUTO;
        return 0;
    }
    if (strstr(json_str, "\"manual_mode\"")) {
        g_sys_status.mode = MODE_MANUAL;
        return 0;
    }
    if (strstr(json_str, "\"reboot\"")) {
        NVIC_SystemReset();
        return 0;
    }

    return 1;
}

void vCommTask(void *argument)
{
    char json_buf[MQTT_BUF_SIZE];
    char cmd_buf[256];
    uint32_t last_ping = 0;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    (void)argument;

    /* 初始化ESP8266并连接WiFi */
    HAL_Delay(2000);  /* 等待ESP8266启动 */
    if (ESP8266_Init() != ESP_OK) {
        /* WiFi初始化失败, 系统仍可在离线模式下运行 */
        g_sys_status.wifi_connected = 0;
    }

    /* 连接MQTT服务器 */
    if (ESP8266_IsConnected()) {
        if (ESP8266_ConnectMQTT() == ESP_OK) {
            MQTT_Connect();
            MQTT_Subscribe(TOPIC_CMD, 1);
            MQTT_Subscribe(TOPIC_CFG, 1);
            g_sys_status.mqtt_connected = 1;
        }
    }

    while (1) {
        uint32_t now = xTaskGetTickCount() / configTICK_RATE_HZ;

        if (g_sys_status.mqtt_connected) {
            /* 数据上报 */
            if ((now - g_sys_status.last_report_tick) >= MQTT_REPORT_INTERVAL) {
                Comm_PackSensorJson(&g_sensor_data, json_buf, sizeof(json_buf));
                MQTT_Publish(TOPIC_DATA, json_buf, 1);
                g_sys_status.last_report_tick = now;
            }

            /* MQTT心跳 */
            if ((now - last_ping) >= (uint32_t)(MQTT_KEEPALIVE - 10)) {
                MQTT_Ping();
                last_ping = now;
            }

            /* 处理下行指令 */
            MQTT_ProcessIncoming();
        }

        /* 检查并处理命令队列 */
        if (xQueueReceive(xCmdQueue, cmd_buf, 0) == pdTRUE) {
            Comm_ParseCommand(cmd_buf);
        }

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000));
    }
}
