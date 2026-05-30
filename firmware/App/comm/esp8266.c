/**
 * @file    esp8266.c
 * @brief   ESP8266 AT指令驱动实现
 */

#include "esp8266.h"

/* USART3 DMA接收缓冲区 */
static char esp_rx_buf[512];
static volatile uint16_t esp_rx_idx = 0;

/* 等待USART收到指定字符串, 超时返回ESP_TIMEOUT */
static uint8_t ESP8266_WaitStr(const char *str, uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < timeout_ms) {
        if (strstr(esp_rx_buf, str) != NULL) {
            return ESP_OK;
        }
        HAL_Delay(10);
    }
    return ESP_TIMEOUT;
}

static void ESP8266_ClearBuf(void)
{
    memset(esp_rx_buf, 0, sizeof(esp_rx_buf));
    esp_rx_idx = 0;
}

uint8_t ESP8266_SendCmd(const char *cmd, const char *expect, uint32_t timeout_ms)
{
    ESP8266_ClearBuf();

    /* 发送AT指令, 末尾加\r\n */
    char buf[256];
    snprintf(buf, sizeof(buf), "%s\r\n", cmd);
    HAL_UART_Transmit(&ESP_USART, (uint8_t *)buf, strlen(buf), 1000);

    /* 等待期望的响应 */
    if (expect != NULL) {
        return ESP8266_WaitStr(expect, timeout_ms);
    }
    return ESP_OK;
}

uint8_t ESP8266_Init(void)
{
    /* Step 1: 测试模块是否响应 */
    if (ESP8266_SendCmd("AT", "OK", 2000) != ESP_OK) {
        return ESP_ERROR;
    }

    /* Step 2: 关闭回显 */
    ESP8266_SendCmd("ATE0", "OK", 1000);

    /* Step 3: 设置为Station模式 */
    ESP8266_SendCmd("AT+CWMODE=1", "OK", 1000);

    /* Step 4: 连接WiFi */
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", WIFI_SSID, WIFI_PASS);
    if (ESP8266_SendCmd(cmd, "WIFI GOT IP", 15000) != ESP_OK) {
        return ESP_ERROR;
    }

    /* Step 5: 设置单连接模式 */
    ESP8266_SendCmd("AT+CIPMUX=0", "OK", 1000);

    g_sys_status.wifi_connected = 1;
    return ESP_OK;
}

uint8_t ESP8266_ConnectMQTT(void)
{
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",%d",
             MQTT_BROKER_IP, MQTT_BROKER_PORT);

    if (ESP8266_SendCmd(cmd, "CONNECT", 10000) != ESP_OK) {
        return ESP_ERROR;
    }
    return ESP_OK;
}

uint8_t ESP8266_TCPSend(const char *data, uint16_t len)
{
    /* 告知ESP8266进入透传发送模式 */
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%d", len);
    if (ESP8266_SendCmd(cmd, ">", 2000) != ESP_OK) {
        return ESP_ERROR;
    }

    /* 发送实际数据 */
    if (HAL_UART_Transmit(&ESP_USART, (uint8_t *)data, len, 5000) != HAL_OK) {
        return ESP_ERROR;
    }

    /* 等待发送完成 */
    return ESP8266_WaitStr("SEND OK", 5000);
}

int ESP8266_ReadData(char *buf, uint16_t len)
{
    if (esp_rx_idx == 0) return -1;

    uint16_t copy_len = (esp_rx_idx < len) ? esp_rx_idx : len;
    memcpy(buf, esp_rx_buf, copy_len);

    /* 移动剩余数据 */
    if (esp_rx_idx > copy_len) {
        memmove(esp_rx_buf, esp_rx_buf + copy_len, esp_rx_idx - copy_len);
        esp_rx_idx -= copy_len;
    } else {
        esp_rx_idx = 0;
    }

    return copy_len;
}

uint8_t ESP8266_IsConnected(void)
{
    return g_sys_status.wifi_connected;
}

/**
 * @brief  USART3中断回调: 将收到的数据存入接收缓冲区
 * @note   需在 stm32f1xx_it.c 中调用 HAL_UART_IRQHandler(&huart3)
 *         并在 HAL_UART_RxCpltCallback 中持续开启接收中断
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == ESP_USART) {
        static uint8_t ch;
        if (esp_rx_idx < sizeof(esp_rx_buf) - 1) {
            esp_rx_buf[esp_rx_idx++] = ch;
        }
        /* 继续接收下一个字节 */
        HAL_UART_Receive_IT(&ESP_USART, &ch, 1);
    }
}
