/**
 * @file    actuator.c
 * @brief   执行机构驱动实现
 */

#include "actuator.h"

void Actuator_InitAll(void)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;

    /* 全部初始化为低电平(关闭) */
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(PUMP_PORT,   PUMP_PIN,   GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_PORT,    LED_PIN,    GPIO_PIN_RESET);
    HAL_GPIO_WritePin(FAN_PORT,    FAN_PIN,    GPIO_PIN_RESET);
}

void Pump_On(void)
{
    HAL_GPIO_WritePin(PUMP_PORT, PUMP_PIN, GPIO_PIN_SET);
}

void Pump_Off(void)
{
    HAL_GPIO_WritePin(PUMP_PORT, PUMP_PIN, GPIO_PIN_RESET);
}

uint8_t Pump_IsOn(void)
{
    return (HAL_GPIO_ReadPin(PUMP_PORT, PUMP_PIN) == GPIO_PIN_SET) ? 1 : 0;
}

void Led_On(void)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
}

void Led_Off(void)
{
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
}

uint8_t Led_IsOn(void)
{
    return (HAL_GPIO_ReadPin(LED_PORT, LED_PIN) == GPIO_PIN_SET) ? 1 : 0;
}

void Fan_On(void)
{
    HAL_GPIO_WritePin(FAN_PORT, FAN_PIN, GPIO_PIN_SET);
}

void Fan_Off(void)
{
    HAL_GPIO_WritePin(FAN_PORT, FAN_PIN, GPIO_PIN_RESET);
}

uint8_t Fan_IsOn(void)
{
    return (HAL_GPIO_ReadPin(FAN_PORT, FAN_PIN) == GPIO_PIN_SET) ? 1 : 0;
}

void Buzzer_On(void)
{
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
}

void Buzzer_Off(void)
{
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
}

void Buzzer_Beep(uint16_t duration_ms)
{
    Buzzer_On();
    HAL_Delay(duration_ms);
    Buzzer_Off();
}

void Actuator_AllOff(void)
{
    Pump_Off();
    Led_Off();
    Fan_Off();
    Buzzer_Off();
}
