/**
 * @file    ds18b20.c
 * @brief   DS18B20 单总线驱动实现 (关中断保证时序)
 */

#include "ds18b20.h"

/* 微秒延时 (72MHz主频, 约8周期/us) */
static void DWT_DelayUs(uint16_t us)
{
    uint32_t cnt = us * 8;
    while (cnt--) { __NOP(); }
}

static void DS18B20_SetOutput(void)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = DS18B20_PIN;
    gpio.Mode  = GPIO_MODE_OUTPUT_OD;
    gpio.Pull  = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DS18B20_PORT, &gpio);
}

static void DS18B20_SetInput(void)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin  = DS18B20_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DS18B20_PORT, &gpio);
}

/**
 * @brief  发送复位脉冲, 检测设备存在
 * @return 0=设备存在, 1=无设备
 */
static uint8_t DS18B20_Reset(void)
{
    uint8_t presence;

    DS18B20_SetOutput();
    DS18B20_DQ_LOW();
    DWT_DelayUs(500);   /* 拉低 >480us */
    DS18B20_DQ_HIGH();
    DWT_DelayUs(40);    /* 等待 */

    DS18B20_SetInput();
    DWT_DelayUs(10);
    presence = DS18B20_DQ_READ(); /* 0=存在, 1=不存在 */
    DWT_DelayUs(450);   /* 等待复位完成 */

    return presence;
}

static void DS18B20_WriteBit(uint8_t bit)
{
    DS18B20_SetOutput();
    DS18B20_DQ_LOW();
    DWT_DelayUs(2);

    if (bit) {
        DS18B20_DQ_HIGH();
        DWT_DelayUs(60);
    } else {
        DWT_DelayUs(60);
        DS18B20_DQ_HIGH();
        DWT_DelayUs(2);
    }
}

static uint8_t DS18B20_ReadBit(void)
{
    uint8_t bit;

    DS18B20_SetOutput();
    DS18B20_DQ_LOW();
    DWT_DelayUs(2);
    DS18B20_DQ_HIGH();

    DS18B20_SetInput();
    DWT_DelayUs(5);
    bit = DS18B20_DQ_READ() ? 1 : 0;
    DWT_DelayUs(55);

    return bit;
}

static void DS18B20_WriteByte(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++) {
        DS18B20_WriteBit(data & 0x01);
        data >>= 1;
    }
}

static uint8_t DS18B20_ReadByte(void)
{
    uint8_t data = 0;
    for (uint8_t i = 0; i < 8; i++) {
        data >>= 1;
        if (DS18B20_ReadBit()) data |= 0x80;
    }
    return data;
}

uint8_t DS18B20_Init(void)
{
    return DS18B20_Reset();
}

uint8_t DS18B20_ReadTemp(float *temp)
{
    uint8_t lsb, msb;
    int16_t raw;

    __disable_irq();

    /* Step 1: 复位 + 检测 */
    if (DS18B20_Reset()) {
        __enable_irq();
        return 1;
    }

    /* Step 2: 跳过ROM + 启动温度转换 */
    DS18B20_WriteByte(DS18B20_CMD_SKIP_ROM);
    DS18B20_WriteByte(DS18B20_CMD_CONVERT_T);

    __enable_irq();
    HAL_Delay(750);  /* 12位分辨率转换时间 ~750ms, 关中断期间不阻塞 */

    __disable_irq();

    /* Step 3: 复位 + 跳过ROM + 读暂存器 */
    if (DS18B20_Reset()) {
        __enable_irq();
        return 1;
    }

    DS18B20_WriteByte(DS18B20_CMD_SKIP_ROM);
    DS18B20_WriteByte(DS18B20_CMD_READ_SCRATCHPAD);

    lsb = DS18B20_ReadByte();
    msb = DS18B20_ReadByte();

    __enable_irq();

    raw = ((int16_t)msb << 8) | lsb;
    *temp = raw / 16.0f;

    return 0;
}
