/**
 * @file    sensor_task.h
 * @brief   传感器采集任务 - 定时轮询所有传感器并写入消息队列
 */

#ifndef __SENSOR_TASK_H
#define __SENSOR_TASK_H

#include "main.h"
#include "soil_moisture.h"
#include "dht22.h"
#include "bh1750.h"

/**
 * @brief  传感器采集任务入口
 * @param  argument: 未使用
 * @note   优先级: 3 | 栈: 512B | 周期: 2000ms
 */
void vSensorTask(void *argument);

#endif /* __SENSOR_TASK_H */
