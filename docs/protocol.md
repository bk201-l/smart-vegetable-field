# MQTT通信协议定义

## 基础信息

- **协议**: MQTT v3.1.1
- **传输**: TCP, ESP8266 WiFi
- **编码**: JSON
- **QoS**: 1 (数据上报), 1 (指令下发)

## Topic 定义

### 上行 (设备 → 云端)

| Topic | QoS | 频率 | 说明 |
|-------|-----|------|------|
| `vegfield/{device_id}/data` | 1 | 每5分钟 | 环境数据上报 |
| `vegfield/{device_id}/event` | 1 | 触发时 | 告警/事件上报 |
| `vegfield/{device_id}/status` | 1 | 上电时+每10分钟 | 设备状态心跳 |

### 下行 (云端 → 设备)

| Topic | QoS | 说明 |
|-------|-----|------|
| `vegfield/{device_id}/cmd` | 1 | 远程控制指令 |
| `vegfield/{device_id}/cfg` | 1 | 阈值/参数配置 |
| `vegfield/{device_id}/ota` | 1 | OTA升级(预留) |

## 数据格式

### 环境数据上报

```json
{
    "dev": "vegfield_001",
    "ts": 1717488000,
    "data": {
        "soil": [45.2, 52.1],
        "temp": 28.5,
        "humi": 65.3,
        "lux": 15420,
        "rain": 0
    },
    "ctrl": {
        "pump": 0,
        "led": 1,
        "fan": 0
    }
}
```

| 字段 | 类型 | 单位 | 说明 |
|-----|------|------|------|
| soil | float[2] | % | 两路土壤湿度 |
| temp | float | °C | 空气温度 |
| humi | float | %RH | 空气湿度 |
| lux | uint16 | lux | 光照强度 |
| rain | uint8 | 0/1 | 0=无雨, 1=有雨 |
| pump/led/fan | uint8 | 0/1 | 执行器状态 |

### 远程控制指令

```json
{
    "cmd": "pump_on",
    "duration": 300,
    "ts": 1717488000
}
```

| cmd 值 | 参数 | 说明 |
|--------|------|------|
| pump_on | duration(秒) | 开启水泵,最长1800s |
| pump_off | - | 关闭水泵 |
| led_on | brightness(0-100) | 开启补光灯 |
| led_off | - | 关闭补光灯 |
| fan_on | duration(秒) | 开启风扇 |
| fan_off | - | 关闭风扇 |
| auto_mode | - | 切换为自动模式 |
| manual_mode | - | 切换为手动模式 |
| reboot | - | 设备重启 |

### 参数配置 (下行)

```json
{
    "cfg": {
        "soil_min": 30,
        "soil_max": 60,
        "temp_max": 35,
        "lux_min": 1000,
        "pump_max_time": 1800,
        "report_interval": 300
    }
}
```

### 告警事件

```json
{
    "dev": "vegfield_001",
    "type": "alarm",
    "code": "pump_overtime",
    "msg": "Pump running over 30min, forced stop",
    "ts": 1717488000
}
```

| code | 触发条件 |
|------|---------|
| pump_overtime | 水泵超时 |
| temp_high | 温度过高 |
| soil_dry | 土壤过干且灌溉未生效 |
| sensor_fault | 传感器故障 |
