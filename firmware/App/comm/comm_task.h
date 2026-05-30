/**
 * @file    comm_task.h
 * @brief   通信任务 - ESP8266 WiFi + MQTT 数据收发
 */

#ifndef __COMM_TASK_H
#define __COMM_TASK_H

#include "main.h"
#include "cJSON.h"

/**
 * @brief  通信任务入口
 * @param  argument: 未使用
 * @note   优先级: 2 | 栈: 1024B | 周期: 5000ms
 */
void vCommTask(void *argument);

/**
 * @brief  将传感器数据打包为JSON字符串
 * @param  data: 传感器数据
 * @param  buf:  输出缓冲区
 * @param  len:  缓冲区大小
 * @return uint16_t: JSON字符串长度
 */
uint16_t Comm_PackSensorJson(sensor_data_t *data, char *buf, uint16_t len);

/**
 * @brief  解析远程控制指令
 * @param  json_str: JSON指令字符串
 * @return uint8_t: 0=成功, 1=解析失败
 */
uint8_t Comm_ParseCommand(const char *json_str);

#endif /* __COMM_TASK_H */
