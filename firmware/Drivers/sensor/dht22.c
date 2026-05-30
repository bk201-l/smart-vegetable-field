/**
 * @file    dht22.c
 * @brief   DHT22 驱动实现 - 使用微秒延时方式 (不依赖定时器捕获的简化版)
 *          如需高精度, 改用TIM输入捕获方式
 */

#include "dht22.h"

#define DHT22_GPIO      DHT22_PORT
#define DHT22_PIN_NUM   DHT22_PIN

/* 微秒级延时 (72MHz, 需根据实际主频调整) */
static void DHT22_DelayUs(uint16_t us)
{
    uint16_t cnt = us * 8;
    while (cnt--) {
        __NOP();
    }
}

static void DHT22_PinOut(void)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = DHT22_PIN_NUM;
    gpio.Mode  = GPIO_MODE_OUTPUT_OD;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT22_GPIO, &gpio);
}

static void DHT22_PinIn(void)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin  = DHT22_PIN_NUM;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT22_GPIO, &gpio);
}

static uint8_t DHT22_WaitLevel(uint8_t level, uint16_t timeout_us)
{
    uint16_t cnt = 0;
    while (HAL_GPIO_ReadPin(DHT22_GPIO, DHT22_PIN_NUM) == level) {
        DHT22_DelayUs(1);
        cnt++;
        if (cnt > timeout_us) return 1;
    }
    return 0;
}

void DHT22_Init(void)
{
    /* 上电后等待DHT22稳定, 约1秒 */
    HAL_Delay(1000);
}

uint8_t DHT22_Read(dht22_data_t *data)
{
    uint8_t buf[5] = {0};
    uint8_t i, j;

    __disable_irq();  /* 关中断保证时序 */

    /* Step 1: 主机发送起始信号 - 拉低>1ms */
    DHT22_PinOut();
    HAL_GPIO_WritePin(DHT22_GPIO, DHT22_PIN_NUM, GPIO_PIN_RESET);
    DHT22_DelayUs(DHT22_START_LOW);
    HAL_GPIO_WritePin(DHT22_GPIO, DHT22_PIN_NUM, GPIO_PIN_SET);
    DHT22_DelayUs(DHT22_START_HIGH);

    /* Step 2: 切换到输入, 等待从机响应 */
    DHT22_PinIn();

    /* 从机拉低 ~80us */
    if (DHT22_WaitLevel(0, DHT22_TIMEOUT)) {
        __enable_irq();
        return 1;  /* 超时: 无响应 */
    }
    /* 从机拉高 ~80us */
    if (DHT22_WaitLevel(1, DHT22_TIMEOUT)) {
        __enable_irq();
        return 1;
    }

    /* Step 3: 读取40位数据 (5字节) */
    for (j = 0; j < 5; j++) {
        for (i = 0; i < 8; i++) {
            /* 等待低电平开始 */
            if (DHT22_WaitLevel(0, DHT22_TIMEOUT)) {
                __enable_irq();
                return 1;
            }
            /* 等待高电平结束, 计时 */
            uint16_t high_cnt = 0;
            while (HAL_GPIO_ReadPin(DHT22_GPIO, DHT22_PIN_NUM)) {
                DHT22_DelayUs(1);
                high_cnt++;
                if (high_cnt > 150) { __enable_irq(); return 1; }
            }
            buf[j] <<= 1;
            if (high_cnt > 40) { /* >40us → 位1 */
                buf[j] |= 0x01;
            }
        }
    }

    __enable_irq();

    /* Step 4: 校验 */
    uint8_t checksum = buf[0] + buf[1] + buf[2] + buf[3];
    if (checksum != buf[4]) {
        data->valid = 0;
        return 2;
    }

    /* Step 5: 解析数据 */
    data->humidity    = (float)((buf[0] << 8) | buf[1]) / 10.0f;
    data->temperature = (float)(((buf[2] & 0x7F) << 8) | buf[3]) / 10.0f;
    if (buf[2] & 0x80) {
        data->temperature = -data->temperature;  /* 负温 */
    }
    data->valid = 1;
    return 0;
}
