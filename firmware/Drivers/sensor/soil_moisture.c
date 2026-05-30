/**
 * @file    soil_moisture.c
 * @brief   电容式土壤湿度传感器驱动实现
 *          注意: 电容式传感器比电阻式更耐腐蚀, 但仍需定期标定
 */

#include "soil_moisture.h"

static uint32_t soil_adc_channels[SOIL_ADC_NUM] = {
    SOIL1_ADC_CH,
    SOIL2_ADC_CH
};

uint16_t Soil_ReadRaw(uint8_t channel)
{
    if (channel >= SOIL_ADC_NUM) return 0;

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint16_t val = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);

    /* 切换通道 */
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = soil_adc_channels[channel];
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    return val;
}

float Soil_ADC2Percent(uint16_t adc_value)
{
    /* 传感器故障检测: 断线或短路时ADC接近0或4095 */
    if (adc_value < 100 || adc_value > 4000) {
        return -1.0f;
    }

    if (adc_value >= SOIL_ADC_AIR) {
        return 0.0f;
    }
    if (adc_value <= SOIL_ADC_WATER) {
        return 100.0f;
    }

    /* 线性映射: ADC越高 = 越干燥 = 百分比越低 */
    float percent = (float)(SOIL_ADC_AIR - adc_value) /
                    (float)(SOIL_ADC_AIR - SOIL_ADC_WATER) * 100.0f;

    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;

    return percent;
}

float Soil_ReadPercent(uint8_t channel)
{
    uint16_t raw = Soil_ReadRaw(channel);
    return Soil_ADC2Percent(raw);
}

void Soil_ReadAll(sensor_data_t *data)
{
    for (uint8_t i = 0; i < SOIL_ADC_NUM; i++) {
        data->soil_moisture[i] = Soil_ReadPercent(i);
    }
}
