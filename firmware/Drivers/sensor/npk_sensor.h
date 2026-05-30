/**
 * @file    npk_sensor.h
 * @brief   土壤 NPK (氮磷钾) 传感器驱动
 *          接口: RS485 (USART2 软件模拟) + Modbus RTU
 *          传感器型号: JXCT 土壤氮磷钾三合一传感器 (或兼容型号)
 *          默认地址: 0x01, 波特率: 9600, 8N1
 *
 * 接线:
 *   PB0  → RS485 TX (软件模拟)
 *   PB1  → RS485 RX (软件模拟)
 *   PA8  → RS485 DE/RE (方向控制: 高=发送, 低=接收)
 */

#ifndef __NPK_SENSOR_H
#define __NPK_SENSOR_H

#include "main.h"

/* RS485 引脚 */
#define NPK_TX_PORT         GPIOB
#define NPK_TX_PIN          GPIO_PIN_0
#define NPK_RX_PORT         GPIOB
#define NPK_RX_PIN          GPIO_PIN_1
#define NPK_DE_PORT         GPIOA
#define NPK_DE_PIN          GPIO_PIN_8

#define RS485_TX()          HAL_GPIO_WritePin(NPK_DE_PORT, NPK_DE_PIN, GPIO_PIN_SET)
#define RS485_RX()          HAL_GPIO_WritePin(NPK_DE_PORT, NPK_DE_PIN, GPIO_PIN_RESET)

/* Modbus 参数 */
#define NPK_MODBUS_ADDR     0x01
#define NPK_BAUDRATE        9600
#define NPK_REG_START       0x0000  /* 起始寄存器 (N值) */
#define NPK_REG_COUNT       0x0003  /* 读取3个寄存器 (N, P, K) */

typedef struct {
    uint16_t nitrogen;      /* 氮含量 (mg/kg) */
    uint16_t phosphorus;    /* 磷含量 (mg/kg) */
    uint16_t potassium;     /* 钾含量 (mg/kg) */
    uint8_t  valid;         /* 数据有效标志 */
} npk_data_t;

/**
 * @brief  初始化NPK传感器: GPIO配置 + RS485方向控制
 */
void NPK_Init(void);

/**
 * @brief  读取土壤NPK值 (Modbus RTU查询)
 * @param  data: 结果指针
 * @return uint8_t: 0=成功, 1=超时, 2=CRC校验失败
 */
uint8_t NPK_Read(npk_data_t *data);

#endif /* __NPK_SENSOR_H */
