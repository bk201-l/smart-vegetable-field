/**
 * @file    main.h
 * @brief   主程序头文件 - 全局数据结构与队列定义
 */

#ifndef __MAIN_H
#define __MAIN_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "cmsis_os.h"
#include "config.h"

/* ============ 系统运行模式 ============ */
typedef enum {
    MODE_AUTO   = 0,    /* 自动模式: 根据传感器阈值自动控制 */
    MODE_MANUAL = 1     /* 手动模式: 仅响应远程指令 */
} sys_mode_t;

/* ============ 执行器状态 ============ */
typedef enum {
    ACT_OFF = 0,
    ACT_ON  = 1
} actuator_state_t;

/* ============ 传感器数据结构 ============ */
typedef struct {
    float   soil_moisture[SOIL_ADC_NUM]; /* 土壤湿度百分比 0-100% */
    float   temperature;                 /* 空气温度 °C */
    float   humidity;                    /* 空气湿度 %RH */
    uint16_t lux;                        /* 光照强度 lux */
    uint8_t  is_raining;                 /* 是否下雨 0/1 */
    uint32_t timestamp;                  /* 采集时间戳(tick) */
} sensor_data_t;

/* ============ 执行器状态结构 ============ */
typedef struct {
    actuator_state_t pump;
    actuator_state_t led;
    actuator_state_t fan;
    actuator_state_t buzzer;
    uint32_t pump_on_seconds;            /* 水泵本次已运行时间 */
} actuator_status_t;

/* ============ 系统状态结构 ============ */
typedef struct {
    sys_mode_t  mode;
    uint32_t    uptime_seconds;
    uint8_t     wifi_connected;
    uint8_t     mqtt_connected;
    uint32_t    last_report_tick;
    uint32_t    last_pump_stop_tick;     /* 上次水泵停止时间, 用于最小间隔保护 */
} system_status_t;

/* ============ 全局变量声明 ============ */
extern sensor_data_t    g_sensor_data;
extern actuator_status_t g_actuator;
extern system_status_t  g_sys_status;

/* ============ 消息队列句柄 ============ */
extern QueueHandle_t xSensorQueue;
extern QueueHandle_t xCmdQueue;
extern QueueHandle_t xDisplayQueue;

/* ============ 信号量句柄 ============ */
extern SemaphoreHandle_t xI2CMutex;
extern SemaphoreHandle_t xUart3Mutex;

#endif /* __MAIN_H */
