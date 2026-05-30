/**
 * @file    npk_sensor.c
 * @brief   NPK传感器驱动实现 - 软件UART + Modbus RTU
 *
 * 软件UART: 9600bps, 每个bit约104us
 * 在72MHz下每bit约7500个周期, 精度足够
 */

#include "npk_sensor.h"

/* ---- 微秒延时 (72MHz) ---- */
static void DWT_DelayUs(uint16_t us)
{
    uint32_t cnt = us * 8;
    while (cnt--) { __NOP(); }
}

/* ---- Modbus CRC16 ---- */
static uint16_t Modbus_CRC16(uint8_t *buf, uint8_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= buf[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}

/* ---- 软件UART (9600bps, 半双工) ---- */

#define BIT_DELAY_US    104   /* 9600bps = 104us/bit */

static void SoftUART_SendByte(uint8_t data)
{
    /* 起始位 (拉低) */
    RS485_TX();
    DWT_DelayUs(10);
    HAL_GPIO_WritePin(NPK_TX_PORT, NPK_TX_PIN, GPIO_PIN_RESET);
    DWT_DelayUs(BIT_DELAY_US);

    /* 数据位 (LSB first) */
    for (uint8_t i = 0; i < 8; i++) {
        if (data & 0x01)
            HAL_GPIO_WritePin(NPK_TX_PORT, NPK_TX_PIN, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(NPK_TX_PORT, NPK_TX_PIN, GPIO_PIN_RESET);
        DWT_DelayUs(BIT_DELAY_US);
        data >>= 1;
    }

    /* 停止位 (拉高) */
    HAL_GPIO_WritePin(NPK_TX_PORT, NPK_TX_PIN, GPIO_PIN_SET);
    DWT_DelayUs(BIT_DELAY_US);
}

static uint8_t SoftUART_RecvByte(void)
{
    uint8_t data = 0;

    RS485_RX();
    DWT_DelayUs(5);

    /* 等待起始位 (低电平) */
    uint16_t timeout = 0;
    while (HAL_GPIO_ReadPin(NPK_RX_PORT, NPK_RX_PIN)) {
        DWT_DelayUs(1);
        if (++timeout > 2000) return 0; /* 超时 */
    }

    /* 延迟到起始位中点 */
    DWT_DelayUs(BIT_DELAY_US / 2);

    /* 跳过起始位, 到第一个数据位中点 */
    DWT_DelayUs(BIT_DELAY_US);

    /* 读取8个数据位 (LSB first) */
    for (uint8_t i = 0; i < 8; i++) {
        data >>= 1;
        if (HAL_GPIO_ReadPin(NPK_RX_PORT, NPK_RX_PIN))
            data |= 0x80;
        DWT_DelayUs(BIT_DELAY_US);
    }

    return data;
}

/* ---- NPK 传感器操作 ---- */

void NPK_Init(void)
{
    /* TX/RX 引脚 */
    GPIO_InitTypeDef gpio = {0};
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;

    /* TX 推挽输出 */
    gpio.Pin  = NPK_TX_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(NPK_TX_PORT, &gpio);

    /* RX 输入 */
    gpio.Pin  = NPK_RX_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(NPK_RX_PORT, &gpio);

    /* DE/RE 方向控制 */
    gpio.Pin  = NPK_DE_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(NPK_DE_PORT, &gpio);

    RS485_RX();  /* 默认接收模式 */
    HAL_GPIO_WritePin(NPK_TX_PORT, NPK_TX_PIN, GPIO_PIN_SET);
}

uint8_t NPK_Read(npk_data_t *data)
{
    uint8_t tx_buf[8], rx_buf[11];
    uint16_t crc_calc, crc_recv;

    /* 构建Modbus查询帧: 01 03 00 00 00 03 CRC */
    tx_buf[0] = NPK_MODBUS_ADDR;
    tx_buf[1] = 0x03;               /* 功能码: 读保持寄存器 */
    tx_buf[2] = (NPK_REG_START >> 8) & 0xFF;
    tx_buf[3] = NPK_REG_START & 0xFF;
    tx_buf[4] = (NPK_REG_COUNT >> 8) & 0xFF;
    tx_buf[5] = NPK_REG_COUNT & 0xFF;
    uint16_t crc = Modbus_CRC16(tx_buf, 6);
    tx_buf[6] = crc & 0xFF;
    tx_buf[7] = (crc >> 8) & 0xFF;

    /* 发送查询 (关中断, 确保时序) */
    __disable_irq();
    for (uint8_t i = 0; i < 8; i++) {
        SoftUART_SendByte(tx_buf[i]);
    }
    RS485_RX();  /* 切换到接收 */
    __enable_irq();

    /* Modbus广播延迟: 3.5字符时间 ≈ 4ms (9600bps) */
    HAL_Delay(5);

    /* 接收响应: 最多11字节 */
    for (uint8_t i = 0; i < 11; i++) {
        rx_buf[i] = SoftUART_RecvByte();
    }

    /* 校验: 地址 + 功能码 */
    if (rx_buf[0] != NPK_MODBUS_ADDR || rx_buf[1] != 0x03) {
        data->valid = 0;
        return 1;
    }

    /* 校验: CRC16 */
    uint8_t data_len = rx_buf[2];  /* 数据字节数 (应为6) */
    crc_calc = Modbus_CRC16(rx_buf, 3 + data_len);
    crc_recv = rx_buf[3 + data_len] | (rx_buf[4 + data_len] << 8);

    if (crc_calc != crc_recv) {
        data->valid = 0;
        return 2;
    }

    /* 解析 N, P, K (每个占2字节, 单位mg/kg) */
    data->nitrogen   = (rx_buf[3] << 8) | rx_buf[4];
    data->phosphorus = (rx_buf[5] << 8) | rx_buf[6];
    data->potassium  = (rx_buf[7] << 8) | rx_buf[8];
    data->valid = 1;

    return 0;
}
