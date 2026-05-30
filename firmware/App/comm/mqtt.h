/**
 * @file    mqtt.h
 * @brief   简易MQTT客户端 (MQTT v3.1.1)
 *          基于ESP8266已建立的TCP连接进行MQTT通信
 *          仅实现必要功能: CONNECT, PUBLISH, SUBSCRIBE, PINGREQ
 */

#ifndef __MQTT_H
#define __MQTT_H

#include "main.h"
#include "esp8266.h"
#include <string.h>
#include <stdio.h>

/* MQTT报文类型 */
#define MQTT_CONNECT        0x10
#define MQTT_CONNACK        0x20
#define MQTT_PUBLISH        0x30
#define MQTT_PUBACK         0x40
#define MQTT_SUBSCRIBE      0x82
#define MQTT_SUBACK         0x90
#define MQTT_PINGREQ        0xC0
#define MQTT_PINGRESP       0xD0
#define MQTT_DISCONNECT     0xE0

#define MQTT_CLIENT_ID      DEVICE_ID

/* Topic 定义 */
#define TOPIC_DATA          "vegfield/" DEVICE_ID "/data"
#define TOPIC_EVENT         "vegfield/" DEVICE_ID "/event"
#define TOPIC_STATUS        "vegfield/" DEVICE_ID "/status"
#define TOPIC_CMD           "vegfield/" DEVICE_ID "/cmd"
#define TOPIC_CFG           "vegfield/" DEVICE_ID "/cfg"

/**
 * @brief  MQTT连接 - 发送CONNECT报文并等待CONNACK
 */
uint8_t MQTT_Connect(void);

/**
 * @brief  MQTT发布消息
 * @param  topic:   主题
 * @param  payload: 消息体(JSON字符串)
 * @param  qos:     服务质量 (0/1)
 */
uint8_t MQTT_Publish(const char *topic, const char *payload, uint8_t qos);

/**
 * @brief  MQTT订阅主题
 * @param  topic: 主题
 * @param  qos:   服务质量
 */
uint8_t MQTT_Subscribe(const char *topic, uint8_t qos);

/**
 * @brief  MQTT心跳 - 发送PINGREQ
 */
uint8_t MQTT_Ping(void);

/**
 * @brief  MQTT断开连接
 */
void MQTT_Disconnect(void);

/**
 * @brief  检查并处理MQTT下行消息 (在CommTask中轮询调用)
 *         解析Publish报文中的JSON指令
 */
void MQTT_ProcessIncoming(void);

#endif /* __MQTT_H */
