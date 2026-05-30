/**
 * @file    main.c
 * @brief   智能化菜地种植系统 - 主程序入口
 *
 * 系统启动流程:
 *   1. HAL初始化 (时钟/GPIO/外设)
 *   2. FreeRTOS内核启动
 *   3. 创建全局队列/信号量
 *   4. 创建各功能任务
 *   5. 调度器接管 → 任务循环运行
 */

#include "main.h"
#include "actuator.h"
#include "sensor_task.h"
#include "control_task.h"
#include "comm_task.h"
#include "display_task.h"

/* ---- 全局变量 ---- */
sensor_data_t     g_sensor_data = {0};
actuator_status_t g_actuator    = {0};
system_status_t   g_sys_status  = {
    .mode              = MODE_AUTO,
    .uptime_seconds    = 0,
    .wifi_connected    = 0,
    .mqtt_connected    = 0,
    .last_report_tick  = 0,
    .last_pump_stop_tick = 0,
};

/* ---- 消息队列句柄 ---- */
QueueHandle_t xSensorQueue  = NULL;  /* 传感器数据队列, 容量1, 覆盖旧数据 */
QueueHandle_t xCmdQueue     = NULL;  /* 远程命令队列, 容量5 */
QueueHandle_t xDisplayQueue = NULL;  /* 显示数据队列, 容量2 */

/* ---- 信号量句柄 ---- */
SemaphoreHandle_t xI2CMutex   = NULL;  /* I2C总线互斥 */
SemaphoreHandle_t xUart3Mutex = NULL;  /* USART3互斥 */

/* ---- 任务句柄 ---- */
static TaskHandle_t hSensorTask  = NULL;
static TaskHandle_t hControlTask = NULL;
static TaskHandle_t hCommTask    = NULL;
static TaskHandle_t hDisplayTask = NULL;
static TaskHandle_t hWatchdogTask = NULL;

/* ---- 任务栈分配 ---- */
static StackType_t  xSensorStack[128];   /* 128 * 4 = 512B */
static StackType_t  xControlStack[64];   /*  64 * 4 = 256B */
static StackType_t  xCommStack[256];     /* 256 * 4 = 1024B */
static StackType_t  xDisplayStack[64];   /*  64 * 4 = 256B */
static StackType_t  xWatchdogStack[32];  /*  32 * 4 = 128B */

static StaticTask_t xSensorTaskBuf;
static StaticTask_t xControlTaskBuf;
static StaticTask_t xCommTaskBuf;
static StaticTask_t xDisplayTaskBuf;
static StaticTask_t xWatchdogTaskBuf;

/* ---- 队列存储 ---- */
static uint8_t ucSensorQueueStorage[sizeof(sensor_data_t)];
static char    ucCmdQueueStorage[5 * 256];
static uint8_t ucDisplayQueueStorage[2 * sizeof(sensor_data_t)];
static StaticQueue_t xSensorQueueBuf;
static StaticQueue_t xCmdQueueBuf;
static StaticQueue_t xDisplayQueueBuf;

/* ---- 按键扫描 ---- */
static void Key_Scan(void)
{
    static uint32_t last_key1_tick = 0;
    uint32_t now = xTaskGetTickCount();

    /* KEY1: 模式切换 (防抖300ms) */
    if (HAL_GPIO_ReadPin(KEY1_PORT, KEY1_PIN) == GPIO_PIN_RESET) {
        if ((now - last_key1_tick) > pdMS_TO_TICKS(300)) {
            last_key1_tick = now;
            g_sys_status.mode = (g_sys_status.mode == MODE_AUTO) ? MODE_MANUAL : MODE_AUTO;
            if (g_sys_status.mode == MODE_MANUAL) {
                Actuator_AllOff();  /* 切手动时先全部关闭 */
            }
        }
    }

    /* KEY2: 手动模式下一键浇水 */
    if (HAL_GPIO_ReadPin(KEY2_PORT, KEY2_PIN) == GPIO_PIN_RESET) {
        if (g_sys_status.mode == MODE_MANUAL) {
            /* 长按1秒触发 (防误触) */
            HAL_Delay(1000);
            if (HAL_GPIO_ReadPin(KEY2_PORT, KEY2_PIN) == GPIO_PIN_RESET) {
                if (!Pump_IsOn()) {
                    Pump_On();
                } else {
                    Pump_Off();
                }
            }
        }
    }
}

/* ---- 看门狗任务 ---- */
static void vWatchdogTask(void *argument)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    (void)argument;

    while (1) {
        /* 更新系统运行时间 */
        g_sys_status.uptime_seconds += 10;

        /* 按键扫描 */
        Key_Scan();

        /* 喂硬件看门狗 (如果使能了IWDG) */
        // HAL_IWDG_Refresh(&hiwdg);

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10000));
    }
}

/* ---- 系统初始化 ---- */

static void System_Init(void)
{
    /* GPIO初始化 */
    Actuator_InitAll();

    /* 按键: 上拉输入 */
    GPIO_InitTypeDef gpio = {0};
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    gpio.Pin  = KEY1_PIN;
    HAL_GPIO_Init(KEY1_PORT, &gpio);
    gpio.Pin  = KEY2_PIN;
    HAL_GPIO_Init(KEY2_PORT, &gpio);

    /* 创建队列 */
    xSensorQueue = xQueueCreateStatic(
        1, sizeof(sensor_data_t),
        ucSensorQueueStorage, &xSensorQueueBuf);

    xCmdQueue = xQueueCreateStatic(
        5, 256,
        ucCmdQueueStorage, &xCmdQueueBuf);

    xDisplayQueue = xQueueCreateStatic(
        2, sizeof(sensor_data_t),
        ucDisplayQueueStorage, &xDisplayQueueBuf);

    /* 创建互斥信号量 */
    xI2CMutex   = xSemaphoreCreateMutex();
    xUart3Mutex = xSemaphoreCreateMutex();
}

static void Tasks_Create(void)
{
    /* 控制任务: 最高优先级, 保证执行器动作及时 */
    hControlTask = xTaskCreateStatic(
        vControlTask, "Control",
        sizeof(xControlStack) / sizeof(StackType_t),
        NULL, 4, xControlStack, &xControlTaskBuf);

    /* 传感器任务 */
    hSensorTask = xTaskCreateStatic(
        vSensorTask, "Sensor",
        sizeof(xSensorStack) / sizeof(StackType_t),
        NULL, 3, xSensorStack, &xSensorTaskBuf);

    /* 通信任务 */
    hCommTask = xTaskCreateStatic(
        vCommTask, "Comm",
        sizeof(xCommStack) / sizeof(StackType_t),
        NULL, 2, xCommStack, &xCommTaskBuf);

    /* 显示任务: 低优先级 */
    hDisplayTask = xTaskCreateStatic(
        vDisplayTask, "Display",
        sizeof(xDisplayStack) / sizeof(StackType_t),
        NULL, 1, xDisplayStack, &xDisplayTaskBuf);

    /* 看门狗/系统维护任务 */
    hWatchdogTask = xTaskCreateStatic(
        vWatchdogTask, "Watchdog",
        sizeof(xWatchdogStack) / sizeof(StackType_t),
        NULL, 5, xWatchdogStack, &xWatchdogTaskBuf);

    configASSERT(hControlTask != NULL);
    configASSERT(hSensorTask  != NULL);
    configASSERT(hCommTask    != NULL);
    configASSERT(hDisplayTask != NULL);
    configASSERT(hWatchdogTask != NULL);
}

/* ---- 主函数 ---- */

int main(void)
{
    /* HAL库初始化 */
    HAL_Init();
    SystemClock_Config();  /* CubeMX生成: 72MHz */

    /* 外设初始化 (CubeMX生成的MX_xxx_Init) */
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_I2C1_Init();
    MX_USART1_UART_Init();   /* 调试串口 */
    MX_USART3_UART_Init();   /* ESP8266 */
    MX_TIM3_Init();          /* DHT22时序 */

    /* 应用层初始化 */
    System_Init();

    /* 串口输出启动信息 */
    char boot_msg[] = "\r\n"
        "====================================\r\n"
        " Smart Vegetable Field System\r\n"
        " Version: " FIRMWARE_VERSION "\r\n"
        " Device:  " DEVICE_ID "\r\n"
        "====================================\r\n";
    HAL_UART_Transmit(&DEBUG_USART, (uint8_t *)boot_msg, sizeof(boot_msg), 1000);

    /* 创建任务 */
    Tasks_Create();

    /* 启动调度器 */
    vTaskStartScheduler();

    /* 永远不会运行到这里 */
    while (1) {
        /* FreeRTOS调度器接管后, 此行不会执行 */
    }
}

/**
 * @brief  FreeRTOS钩子: 任务栈溢出检测
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    /* 栈溢出: 强制停止, 蜂鸣器长鸣 */
    Actuator_AllOff();
    while (1) {
        Buzzer_On();
        HAL_Delay(500);
        Buzzer_Off();
        HAL_Delay(500);
    }
}

/**
 * @brief  FreeRTOS钩子: 空闲任务钩子
 */
void vApplicationIdleHook(void)
{
    /* 可在此进入低功耗模式 */
    // __WFI();
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    /* 断言失败: 输出文件名+行号到调试串口 */
    char buf[64];
    snprintf(buf, sizeof(buf), "ASSERT: %s:%lu\r\n", file, line);
    HAL_UART_Transmit(&DEBUG_USART, (uint8_t *)buf, strlen(buf), 1000);
    while (1);
}
#endif
