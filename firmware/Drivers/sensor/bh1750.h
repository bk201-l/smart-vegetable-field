/**
 * @file    bh1750.h
 * @brief   BH1750FVI 数字光照传感器驱动
 *          接口: I2C1 (PB6=SCL, PB7=SDA)
 *          量程: 1-65535 lux
 *          分辨率: 1 lux (H模式)
 */

#ifndef __BH1750_H
#define __BH1750_H

#include "main.h"
#include "i2c.h"

/* BH1750 指令 */
#define BH1750_POWER_ON     0x01    /* 上电 */
#define BH1750_POWER_OFF    0x00    /* 断电 */
#define BH1750_RESET        0x07    /* 复位 */
#define BH1750_CONT_H       0x10    /* 连续高分辨率模式: 1lux, 120ms */
#define BH1750_CONT_H2      0x11    /* 连续高分辨率模式2: 0.5lux, 120ms */
#define BH1750_CONT_L       0x13    /* 连续低分辨率模式: 4lux, 16ms */
#define BH1750_ONCE_H       0x20    /* 单次高分辨率模式 */
#define BH1750_ONCE_H2      0x21    /* 单次高分辨率模式2 */
#define BH1750_ONCE_L       0x23    /* 单次低分辨率模式 */

/**
 * @brief  初始化BH1750: 上电 → 设置连续高分辨率模式
 */
void BH1750_Init(void);

/**
 * @brief  读取光照强度
 * @return uint16_t: 光照值 (lux), 0xFFFF表示读取失败
 */
uint16_t BH1750_ReadLux(void);

#endif /* __BH1750_H */
