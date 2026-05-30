/**
 * @file    soil_moisture.h
 * @brief   电容式土壤湿度传感器驱动
 *          接口: ADC (PA0, PA1)
 *          输出: 0-100% 湿度百分比 (需根据实际标定)
 */

#ifndef __SOIL_MOISTURE_H
#define __SOIL_MOISTURE_H

#include "main.h"
#include "adc.h"

/* 标定参数 - 根据实际传感器在空气中和水中的ADC值调整 */
#define SOIL_ADC_AIR    2800    /* 空气中ADC值 (0%湿度) */
#define SOIL_ADC_WATER  1200    /* 水中ADC值 (100%湿度) */

/**
 * @brief  读取指定通道的土壤湿度ADC值(原始值)
 * @param  channel: 通道编号 (0 = SOIL1, 1 = SOIL2)
 * @return uint16_t: ADC原始值 (0-4095)
 */
uint16_t Soil_ReadRaw(uint8_t channel);

/**
 * @brief  将ADC值转换为湿度百分比
 * @param  adc_value: ADC原始值
 * @return float: 湿度百分比 (0.0 - 100.0), -1.0表示传感器故障
 */
float Soil_ADC2Percent(uint16_t adc_value);

/**
 * @brief  读取指定通道的土壤湿度百分比
 * @param  channel: 通道编号
 * @return float: 湿度百分比
 */
float Soil_ReadPercent(uint8_t channel);

/**
 * @brief  读取所有土壤湿度传感器, 填充到数据结构
 * @param  data: 传感器数据结构指针
 */
void Soil_ReadAll(sensor_data_t *data);

#endif /* __SOIL_MOISTURE_H */
