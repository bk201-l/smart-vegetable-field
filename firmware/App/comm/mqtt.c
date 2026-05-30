/**
 * @file    mqtt.c
 * @brief   简易MQTT客户端实现
 *
 * MQTT报文格式(简化):
 *   CONNECT: [0x10][剩余长度][协议名"MQTT"][协议级别4][连接标志][KeepAlive][ClientID]
 *   PUBLISH: [0x30|QoS][剩余长度][Topic长度][Topic][PacketID(QoS>0)][Payload]
 *   SUBSCRIBE: [0x82][剩余长度][PacketID][Topic长度][Topic][QoS]
 *   剩余长度: 每字节低7位, 最高位=1表示后续还有字节
 */

#include "mqtt.h"

static uint16_t mqtt_packet_id = 1;

/* ---- 剩余长度编解码 ---- */

/**
 * @brief  编码剩余长度为MQTT可变长度格式
 * @return 编码后的字节数
 */
static uint8_t EncodeRemaining(uint32_t length, uint8_t *buf)
{
    uint8_t i = 0;
    do {
        uint8_t byte = length % 128;
        length /= 128;
        if (length > 0) byte |= 0x80;
        buf[i++] = byte;
    } while (length > 0 && i < 4);
    return i;
}

/* ---- MQTT报文构建 ---- */

static uint16_t MQTT_BuildConnect(uint8_t *buf)
{
    uint16_t pos = 0;
    uint8_t client_id[] = MQTT_CLIENT_ID;
    uint8_t var_header[10];

    /* 可变头: 协议名 "MQTT" */
    var_header[0] = 0x00; var_header[1] = 0x04;
    var_header[2] = 'M';  var_header[3] = 'Q';
    var_header[4] = 'T';  var_header[5] = 'T';
    /* 协议级别: v3.1.1 = 4 */
    var_header[6] = 0x04;
    /* 连接标志: CleanSession=1, 无遗嘱, 无用户名密码 */
    var_header[7] = 0x02;
    /* KeepAlive (秒) */
    var_header[8] = (MQTT_KEEPALIVE >> 8) & 0xFF;
    var_header[9] = MQTT_KEEPALIVE & 0xFF;

    /* 有效载荷: Client ID */
    uint16_t payload_len = 2 + sizeof(client_id) - 1;  /* -1去掉\0 */
    uint32_t remaining = sizeof(var_header) + payload_len;

    /* 固定头: 类型 + 剩余长度 */
    buf[pos++] = MQTT_CONNECT;
    pos += EncodeRemaining(remaining, buf + pos);

    /* 可变头 */
    memcpy(buf + pos, var_header, sizeof(var_header));
    pos += sizeof(var_header);

    /* 有效载荷: ClientID */
    buf[pos++] = 0x00;
    buf[pos++] = sizeof(client_id) - 1;
    memcpy(buf + pos, client_id, sizeof(client_id) - 1);
    pos += sizeof(client_id) - 1;

    return pos;
}

static uint16_t MQTT_BuildPublish(const char *topic, const char *payload,
                                   uint8_t qos, uint8_t *buf)
{
    uint16_t pos = 0;
    uint16_t topic_len = strlen(topic);
    uint16_t payload_len = strlen(payload);

    uint32_t remaining = 2 + topic_len + payload_len;
    if (qos > 0) remaining += 2;  /* Packet Identifier */

    /* 固定头 */
    buf[pos++] = MQTT_PUBLISH | (qos << 1);
    pos += EncodeRemaining(remaining, buf + pos);

    /* Topic */
    buf[pos++] = (topic_len >> 8) & 0xFF;
    buf[pos++] = topic_len & 0xFF;
    memcpy(buf + pos, topic, topic_len);
    pos += topic_len;

    /* PacketID (QoS > 0) */
    if (qos > 0) {
        buf[pos++] = (mqtt_packet_id >> 8) & 0xFF;
        buf[pos++] = mqtt_packet_id & 0xFF;
        mqtt_packet_id++;
    }

    /* Payload */
    memcpy(buf + pos, payload, payload_len);
    pos += payload_len;

    return pos;
}

static uint16_t MQTT_BuildSubscribe(const char *topic, uint8_t qos, uint8_t *buf)
{
    uint16_t pos = 0;
    uint16_t topic_len = strlen(topic);
    uint32_t remaining = 2 + 2 + topic_len + 1;  /* PacketID + TopicLen + Topic + QoS */

    buf[pos++] = MQTT_SUBSCRIBE;
    pos += EncodeRemaining(remaining, buf + pos);

    /* Packet Identifier */
    uint16_t pid = mqtt_packet_id++;
    buf[pos++] = (pid >> 8) & 0xFF;
    buf[pos++] = pid & 0xFF;

    /* Topic */
    buf[pos++] = (topic_len >> 8) & 0xFF;
    buf[pos++] = topic_len & 0xFF;
    memcpy(buf + pos, topic, topic_len);
    pos += topic_len;

    /* Requested QoS */
    buf[pos++] = qos;

    return pos;
}

static uint16_t MQTT_BuildPingReq(uint8_t *buf)
{
    buf[0] = MQTT_PINGREQ;
    buf[1] = 0x00;
    return 2;
}

/* ---- MQTT 操作 ---- */

uint8_t MQTT_Connect(void)
{
    uint8_t buf[128];
    uint16_t len = MQTT_BuildConnect(buf);

    if (ESP8266_TCPSend((char *)buf, len) != ESP_OK) {
        return ESP_ERROR;
    }

    /* 等待CONNACK (4字节: 0x20 0x02 0x00 0x00) */
    HAL_Delay(500);

    return ESP_OK;
}

uint8_t MQTT_Publish(const char *topic, const char *payload, uint8_t qos)
{
    uint8_t buf[MQTT_BUF_SIZE];
    uint16_t len = MQTT_BuildPublish(topic, payload, qos, buf);

    return ESP8266_TCPSend((char *)buf, len);
}

uint8_t MQTT_Subscribe(const char *topic, uint8_t qos)
{
    uint8_t buf[128];
    uint16_t len = MQTT_BuildSubscribe(topic, qos, buf);

    return ESP8266_TCPSend((char *)buf, len);
}

uint8_t MQTT_Ping(void)
{
    uint8_t buf[4];
    uint16_t len = MQTT_BuildPingReq(buf);
    return ESP8266_TCPSend((char *)buf, len);
}

void MQTT_Disconnect(void)
{
    uint8_t buf[] = { MQTT_DISCONNECT, 0x00 };
    ESP8266_TCPSend((char *)buf, 2);
}

/**
 * @brief  解析Publish报文中的Topic和Payload
 * @note   简易解析, 仅支持QoS=0的Publish报文
 */
void MQTT_ProcessIncoming(void)
{
    char rx_buf[512];
    int rx_len = ESP8266_ReadData(rx_buf, sizeof(rx_buf));

    if (rx_len <= 0) return;

    /* 解析 +IPD,N: 头部 (ESP8266透传模式) */
    char *data_start = strstr(rx_buf, "+IPD,");
    if (data_start == NULL) return;

    /* 跳过 "+IPD," 和 长度字段 */
    data_start = strstr(data_start, ":");
    if (data_start == NULL) return;
    data_start++; /* 跳到数据起始 */

    /* 检查是否为Publish报文 */
    if ((data_start[0] & 0xF0) == MQTT_PUBLISH) {
        /* 简易解析: 跳过固定头+Topic, 提取Payload(JSON) */
        uint8_t remaining = data_start[1];
        uint16_t topic_len = (data_start[2] << 8) | data_start[3];
        char *payload = data_start + 4 + topic_len;

        /* 提取JSON指令, 发送到命令队列 */
        uint16_t payload_len = remaining - 2 - topic_len;
        if (payload_len > 0 && payload_len < 256) {
            char json_buf[256] = {0};
            memcpy(json_buf, payload, payload_len);
            xQueueSend(xCmdQueue, json_buf, 0);
        }
    }
}
