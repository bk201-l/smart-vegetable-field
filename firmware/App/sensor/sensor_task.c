/**
 * @file    sensor_task.c
 * @brief   传感器采集任务实现
 *          每2秒轮询所有传感器, 采集完成后发送到数据队列供控制任务使用
 */

#include "sensor_task.h"

void vSensorTask(void *argument)
{
    sensor_data_t sensor_data;
    dht22_data_t  dht_data;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    (void)argument;

    /* 初始化传感器 */
    DHT22_Init();
    BH1750_Init();

    while (1) {
        /* 读取土壤湿度 (ADC) */
        Soil_ReadAll(&sensor_data);

        /* 读取DHT22温湿度 (单总线, 可能失败需重试) */
        uint8_t dht_retry = 0;
        do {
            DHT22_Read(&dht_data);
            dht_retry++;
        } while (!dht_data.valid && dht_retry < 3);

        if (dht_data.valid) {
            sensor_data.temperature = dht_data.temperature;
            sensor_data.humidity    = dht_data.humidity;
        } else {
            sensor_data.temperature = -99.0f;  /* 错误标记 */
            sensor_data.humidity    = -99.0f;
        }

        /* 读取光照强度 */
        sensor_data.lux = BH1750_ReadLux();

        /* 读取雨滴传感器 (简单的数字量) */
        sensor_data.is_raining = 0; /* TODO: 接入雨滴传感器GPIO */

        /* 时间戳 */
        sensor_data.timestamp = xTaskGetTickCount();

        /* 发送到队列 (非阻塞, 覆盖旧数据) */
        xQueueOverwrite(xSensorQueue, &sensor_data);

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SOIL_SAMPLE_MS));
    }
}
