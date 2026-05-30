/**
 * @file    dht22.h
 * @brief   DHT22 (AM2302) 温湿度传感器驱动
 *          接口: 单总线 (PA6, TIM3_CH1 输入捕获)
 *          精度: 温度±0.5°C, 湿度±2%RH
 *          采样间隔: 最小2秒
 */

#ifndef __DHT22_H
#define __DHT22_H

#include "main.h"
#include "tim.h"

/* DHT22 时序参数 (us) */
#define DHT22_START_LOW     1200    /* 主机拉低起始信号 */
#define DHT22_START_HIGH    40      /* 主机释放后上拉等待 */
#define DHT22_RESP_LOW      80      /* 从机响应低电平 ~80us */
#define DHT22_RESP_HIGH     80      /* 从机响应高电平 ~80us */
#define DHT22_BIT_0          28      /* 数据位0: ~26-28us低电平 */
#define DHT22_BIT_1          70      /* 数据位1: ~70us低电平 */
#define DHT22_TIMEOUT       5000    /* 超时(us) */

typedef struct {
    float temperature;      /* 温度 °C */
    float humidity;         /* 湿度 %RH */
    uint8_t valid;          /* 数据有效标志: 校验通过为1 */
} dht22_data_t;

/**
 * @brief  读取DHT22温湿度数据
 * @param  data: 数据结果指针
 * @return uint8_t: 0=成功, 1=超时, 2=校验失败
 */
uint8_t DHT22_Read(dht22_data_t *data);

/**
 * @brief  初始化DHT22所用定时器输入捕获
 */
void DHT22_Init(void);

#endif /* __DHT22_H */
