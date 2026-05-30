/**
 * @file    actuator.h
 * @brief   执行机构统一接口 - 水泵/补光灯/风扇/蜂鸣器
 *          所有执行器通过GPIO高电平驱动(经继电器/MOS管)
 */

#ifndef __ACTUATOR_H
#define __ACTUATOR_H

#include "main.h"
#include "gpio.h"

/**
 * @brief  初始化所有执行器GPIO为推挽输出, 默认关闭(低电平)
 */
void Actuator_InitAll(void);

/* 水泵 */
void Pump_On(void);
void Pump_Off(void);
uint8_t Pump_IsOn(void);

/* 补光灯 */
void Led_On(void);
void Led_Off(void);
uint8_t Led_IsOn(void);

/* 风扇 */
void Fan_On(void);
void Fan_Off(void);
uint8_t Fan_IsOn(void);

/* 蜂鸣器 */
void Buzzer_On(void);
void Buzzer_Off(void);
void Buzzer_Beep(uint16_t duration_ms);

/* 全部关闭 (紧急停止) */
void Actuator_AllOff(void);

#endif /* __ACTUATOR_H */
