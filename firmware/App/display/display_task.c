/**
 * @file    display_task.c
 * @brief   OLED显示任务实现 - 每500ms刷新一次
 */

#include "display_task.h"

void vDisplayTask(void *argument)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    (void)argument;

    OLED_Init();

    while (1) {
        OLED_RefreshDataPage(&g_sensor_data, &g_actuator);
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
    }
}
