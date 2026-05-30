/**
 * @file    oled.h
 * @brief   0.96寸 OLED SSD1306 驱动 (I2C)
 *          分辨率: 128x64
 *          坐标系: 左上角为原点, X向右, Y向下
 *          页寻址模式: 每页8行, 共8页 (page 0-7)
 */

#ifndef __OLED_H
#define __OLED_H

#include "main.h"
#include "i2c.h"
#include <string.h>

/* SSD1306 基础指令 */
#define OLED_CMD_DISPLAY_OFF     0xAE
#define OLED_CMD_DISPLAY_ON      0xAF
#define OLED_CMD_SET_CONTRAST    0x81
#define OLED_CMD_SEG_REMAP       0xA1    /* 左右翻转 */
#define OLED_CMD_COM_SCAN_DEC    0xC8    /* COM扫描方向 */
#define OLED_CMD_MEM_ADDR_MODE   0x20    /* 内存寻址模式 */
#define OLED_CMD_SET_PAGE_ADDR   0x22    /* 页地址 */
#define OLED_CMD_SET_COL_ADDR    0x21    /* 列地址 */

/**
 * @brief  初始化OLED: 上电 → 设置显示参数 → 清屏
 */
void OLED_Init(void);

/**
 * @brief  清屏 (全部填充0)
 */
void OLED_Clear(void);

/**
 * @brief  显示字符串 (6x8 ASCII字体)
 * @param  x: 列坐标 (0-21, 每字符6像素)
 * @param  y: 页坐标 (0-7, 每页8像素)
 * @param  str: ASCII字符串
 */
void OLED_ShowString(uint8_t x, uint8_t y, const char *str);

/**
 * @brief  在指定位置显示数字
 * @param  x: 列坐标
 * @param  y: 页坐标
 * @param  num: 数字
 * @param  digits: 显示位数 (不足前补空格)
 */
void OLED_ShowNum(uint8_t x, uint8_t y, int32_t num, uint8_t digits);

/**
 * @brief  显示浮点数 (保留1位小数)
 * @param  x: 列坐标
 * @param  y: 页坐标
 * @param  num: 浮点数
 */
void OLED_ShowFloat(uint8_t x, uint8_t y, float num);

/**
 * @brief  刷新传感器数据页面
 */
void OLED_RefreshDataPage(sensor_data_t *data, actuator_status_t *act);

#endif /* __OLED_H */
