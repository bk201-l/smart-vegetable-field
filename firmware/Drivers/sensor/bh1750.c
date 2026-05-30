/**
 * @file    bh1750.c
 * @brief   BH1750FVI 光照传感器驱动实现
 */

#include "bh1750.h"

void BH1750_Init(void)
{
    /* 发送上电指令 */
    uint8_t cmd = BH1750_POWER_ON;
    HAL_I2C_Master_Transmit(&BH1750_I2C, BH1750_ADDR << 1, &cmd, 1, 100);

    HAL_Delay(10);

    /* 设置连续高分辨率模式 */
    cmd = BH1750_CONT_H;
    HAL_I2C_Master_Transmit(&BH1750_I2C, BH1750_ADDR << 1, &cmd, 1, 100);

    HAL_Delay(180); /* 高分辨率模式首次测量需等待180ms */
}

uint16_t BH1750_ReadLux(void)
{
    uint8_t buf[2] = {0};

    if (HAL_I2C_Master_Receive(&BH1750_I2C, BH1750_ADDR << 1, buf, 2, 200) != HAL_OK) {
        return 0xFFFF;
    }

    uint16_t raw = ((uint16_t)buf[0] << 8) | buf[1];
    /* 高分辨率模式: 测量值 / 1.2 = lux */
    return (uint16_t)((float)raw / 1.2f);
}
