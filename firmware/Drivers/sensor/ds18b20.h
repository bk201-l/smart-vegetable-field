/**
 * @file    ds18b20.h
 * @brief   DS18B20 土壤温度传感器驱动
 *          接口: 单总线 (PA7), 精度: ±0.5°C, 量程: -55~125°C
 */

#ifndef __DS18B20_H
#define __DS18B20_H

#include "main.h"

/* 单总线操作宏 */
#define DS18B20_PORT        GPIOA
#define DS18B20_PIN         GPIO_PIN_7

#define DS18B20_DQ_LOW()    HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_RESET)
#define DS18B20_DQ_HIGH()   HAL_GPIO_WritePin(DS18B20_PORT, DS18B20_PIN, GPIO_PIN_SET)
#define DS18B20_DQ_READ()   HAL_GPIO_ReadPin(DS18B20_PORT, DS18B20_PIN)

/* DS18B20 ROM 命令 */
#define DS18B20_CMD_SEARCH_ROM      0xF0
#define DS18B20_CMD_READ_ROM        0x33
#define DS18B20_CMD_MATCH_ROM       0x55
#define DS18B20_CMD_SKIP_ROM        0xCC
#define DS18B20_CMD_ALARM_SEARCH    0xEC

/* DS18B20 功能命令 */
#define DS18B20_CMD_CONVERT_T       0x44
#define DS18B20_CMD_READ_SCRATCHPAD 0xBE
#define DS18B20_CMD_WRITE_SCRATCHPAD 0x4E
#define DS18B20_CMD_COPY_SCRATCHPAD 0x48
#define DS18B20_CMD_RECALL_E2       0xB8

/**
 * @brief  初始化DS18B20: GPIO配置 + 检测设备存在
 * @return uint8_t: 0=检测到设备, 1=未检测到
 */
uint8_t DS18B20_Init(void);

/**
 * @brief  读取土壤温度
 * @param  temp: 温度值指针 (°C)
 * @return uint8_t: 0=成功, 1=失败
 */
uint8_t DS18B20_ReadTemp(float *temp);

#endif /* __DS18B20_H */
