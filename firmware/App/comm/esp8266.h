/**
 * @file    esp8266.h
 * @brief   ESP8266 AT指令驱动
 *          通过 USART3 与 ESP8266 通信 (115200bps)
 *          使用 AT 固件, 最小化依赖
 */

#ifndef __ESP8266_H
#define __ESP8266_H

#include "main.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>

/* AT指令响应标志 */
#define ESP_OK      0
#define ESP_ERROR   1
#define ESP_TIMEOUT 2
#define ESP_BUSY    3

/**
 * @brief  初始化ESP8266
 *         - 测试AT通信
 *         - 设置Station模式
 *         - 连接WiFi
 */
uint8_t ESP8266_Init(void);

/**
 * @brief  发送AT指令并等待响应
 * @param  cmd: AT指令字符串
 * @param  expect: 期望的响应关键字 (如 "OK", "CONNECT")
 * @param  timeout_ms: 超时时间(ms)
 * @return uint8_t: ESP_OK/ESP_ERROR/ESP_TIMEOUT
 */
uint8_t ESP8266_SendCmd(const char *cmd, const char *expect, uint32_t timeout_ms);

/**
 * @brief  TCP连接并发送数据 (用于MQTT)
 * @param  data: 数据缓冲区
 * @param  len:  数据长度
 */
uint8_t ESP8266_TCPSend(const char *data, uint16_t len);

/**
 * @brief  检查ESP8266是否有数据可读, 读取到缓冲区
 * @param  buf:  接收缓冲区
 * @param  len:  期望读取长度
 * @return int: 实际读取字节数, -1=无数据
 */
int ESP8266_ReadData(char *buf, uint16_t len);

/**
 * @brief  检查ESP8266是否已连接WiFi
 */
uint8_t ESP8266_IsConnected(void);

/**
 * @brief  建立TCP连接到MQTT服务器
 */
uint8_t ESP8266_ConnectMQTT(void);

#endif /* __ESP8266_H */
