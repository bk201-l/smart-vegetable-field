/**
 * @file    config.h
 * @brief   系统配置文件 - 所有可调参数集中管理
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* ============ 设备信息 ============ */
#define DEVICE_ID           "vegfield_001"
#define FIRMWARE_VERSION    "v1.0.0"

/* ============ 传感器采样周期 (ms) ============ */
#define SOIL_SAMPLE_MS      2000    /* 土壤湿度采样间隔 */
#define DHT22_SAMPLE_MS     3000    /* DHT22采样间隔(最少2s) */
#define BH1750_SAMPLE_MS    1000    /* 光照采样间隔 */
#define RAIN_SAMPLE_MS      1000    /* 雨量采样间隔 */

/* ============ 控制阈值 ============ */
#define SOIL_MOISTURE_MIN   30.0f   /* 土壤湿度下限(%): 低于此值开启灌溉 */
#define SOIL_MOISTURE_MAX   60.0f   /* 土壤湿度上限(%): 高于此值停止灌溉 */
#define TEMP_MAX            35.0f   /* 温度上限(°C): 高于此值开启通风 */
#define TEMP_VENT_STOP      28.0f   /* 温度下限(°C): 低于此值关闭通风 */
#define LUX_MIN             1000.0f /* 光照下限(lux): 低于此值开启补光 */
#define LUX_LED_OFF         5000.0f /* 光照上限(lux): 高于此值关闭补光 */

/* ============ 安全保护 ============ */
#define PUMP_MAX_ON_TIME    1800    /* 水泵最大连续运行时间(秒) */
#define PUMP_MIN_INTERVAL   600     /* 两次灌溉最小间隔(秒) */
#define ALARM_BUZZER_MS     3000    /* 蜂鸣器报警持续时间(ms) */

/* ============ 通信参数 ============ */
#define MQTT_REPORT_INTERVAL 300    /* 数据上报间隔(秒) */
#define MQTT_KEEPALIVE      60      /* MQTT心跳间隔(秒) */
#define MQTT_BUF_SIZE       512     /* MQTT消息缓冲区大小 */

/* ============ ESP8266 配置 ============ */
#define WIFI_SSID           "your_wifi_ssid"
#define WIFI_PASS           "your_wifi_password"
#define MQTT_BROKER_IP      "192.168.1.100"
#define MQTT_BROKER_PORT    1883

/* ============ 硬件引脚定义 ============ */

/* ADC */
#define SOIL1_ADC_CH        ADC_CHANNEL_0   /* PA0 */
#define SOIL2_ADC_CH        ADC_CHANNEL_1   /* PA1 */
#define SOIL_ADC_NUM        2

/* GPIO 输出 */
#define BUZZER_PORT         GPIOA
#define BUZZER_PIN          GPIO_PIN_2
#define PUMP_PORT           GPIOA
#define PUMP_PIN            GPIO_PIN_3
#define LED_PORT            GPIOA
#define LED_PIN             GPIO_PIN_4
#define FAN_PORT            GPIOA
#define FAN_PIN             GPIO_PIN_5

/* DHT22 */
#define DHT22_PORT          GPIOA
#define DHT22_PIN           GPIO_PIN_6

/* I2C1 */
#define I2C_SCL_PORT        GPIOB
#define I2C_SCL_PIN         GPIO_PIN_6
#define I2C_SDA_PORT        GPIOB
#define I2C_SDA_PIN         GPIO_PIN_7

/* USART - 调试串口 */
#define DEBUG_USART         USART1
#define DEBUG_TX_PORT       GPIOA
#define DEBUG_TX_PIN        GPIO_PIN_9
#define DEBUG_RX_PORT       GPIOA
#define DEBUG_RX_PIN        GPIO_PIN_10

/* USART3 - ESP8266 */
#define ESP_USART           USART3
#define ESP_TX_PORT         GPIOB
#define ESP_TX_PIN          GPIO_PIN_10
#define ESP_RX_PORT         GPIOB
#define ESP_RX_PIN          GPIO_PIN_11

/* 按键 */
#define KEY1_PORT           GPIOB
#define KEY1_PIN            GPIO_PIN_12   /* 模式切换 */
#define KEY2_PORT           GPIOB
#define KEY2_PIN            GPIO_PIN_13   /* 手动浇水 */

/* ============ OLED 显示 ============ */
#define OLED_ADDR           0x78          /* SSD1306 I2C地址 */
#define OLED_I2C            hi2c1

/* ============ BH1750 ============ */
#define BH1750_ADDR         0x46          /* ADDR引脚接地: 0x46, 接VCC: 0x5C */
#define BH1750_I2C          hi2c1

/* ============ DS18B20 (土壤温度) ============ */
#define DS18B20_SAMPLE_MS   3000    /* DS18B20采样间隔(转换需~750ms) */

/* ============ NPK 传感器 (RS485/Modbus) ============ */
#define NPK_SAMPLE_MS       10000   /* NPK采样间隔(10秒) */
#define NITROGEN_MIN        50      /* 氮含量下限(mg/kg): 低于此值建议施肥 */
#define PHOSPHORUS_MIN      30      /* 磷含量下限(mg/kg) */
#define POTASSIUM_MIN       80      /* 钾含量下限(mg/kg) */

/* ============ 报警推送 ============ */
#define PUSH_ENABLED        1       /* 是否启用远程推送 */
#define PUSHPLUS_TOKEN      "your_pushplus_token"  /* PushPlus 令牌 */

#endif /* __CONFIG_H */
