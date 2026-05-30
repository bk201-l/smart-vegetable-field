# 基于STM32的智能化菜地种植系统

## 系统概述

以 STM32F103C8T6 为主控芯片，配合多种传感器实时监测菜地环境参数，通过阈值判断自动控制灌溉、补光、通风等执行机构，支持 ESP8266 WiFi 联网，通过 MQTT 协议将数据上传至云平台，实现手机 APP 远程监控。

## 项目结构

```
smart-vegetable-field/
├── docs/                    # 文档
│   ├── architecture.md      # 系统架构设计
│   ├── hardware.md          # 硬件选型清单
│   └── protocol.md          # MQTT通信协议定义
├── firmware/                # STM32固件代码
│   ├── Core/                # 核心代码
│   │   ├── Inc/             # 头文件 (config.h, main.h)
│   │   └── Src/             # 源文件 (main.c)
│   ├── Drivers/             # 外设驱动层
│   │   ├── sensor/          # 传感器驱动
│   │   └── actuator/        # 执行器驱动
│   ├── App/                 # 应用逻辑层
│   │   ├── sensor/          # 传感采集任务
│   │   ├── control/         # 自动控制逻辑
│   │   ├── comm/            # 通信模块
│   │   └── display/         # OLED显示
│   └── Middleware/          # 中间件 (FreeRTOS等)
├── hardware/                # 硬件原理图(预留)
├── tools/                   # 上位机调试工具
└── cloud/                   # 云平台对接(预留)
```

## 快速开始

1. 使用 STM32CubeMX 生成基于 FreeRTOS 的工程模板
2. 将 `firmware/` 目录下的代码添加到对应位置
3. 编译下载到 STM32F103C8T6
4. 配置 ESP8266 WiFi 和 MQTT 服务器地址

## 开发环境

- **MCU**: STM32F103C8T6 (Blue Pill)
- **IDE**: Keil MDK-ARM V5 / STM32CubeIDE
- **库**: STM32 HAL Library + FreeRTOS
- **调试工具**: SecureCRT (串口), 逻辑分析仪
