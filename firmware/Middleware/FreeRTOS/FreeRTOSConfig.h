/**
 * @file    FreeRTOSConfig.h
 * @brief   FreeRTOS v10.x 配置文件 (STM32F103C8T6, HAL库)
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f1xx_hal.h"

/* ---- 基础配置 ---- */
#define configCPU_CLOCK_HZ              (SystemCoreClock)   /* 72MHz */
#define configTICK_RATE_HZ              1000                /* 1ms tick */
#define configMAX_PRIORITIES            6                   /* 0-5, 5=最高 */
#define configMINIMAL_STACK_SIZE        64                  /* 最小栈(字), 256B */
#define configTOTAL_HEAP_SIZE           12 * 1024           /* 12KB堆, C8T6共20KB RAM */
#define configMAX_TASK_NAME_LEN         12

/* ---- 调度模式 ---- */
#define configUSE_PREEMPTION            1
#define configUSE_TIME_SLICING          1
#define configIDLE_SHOULD_YIELD         1
#define configUSE_TASK_NOTIFICATIONS    1
#define configUSE_MUTEXES               1
#define configUSE_COUNTING_SEMAPHORES   1
#define configUSE_QUEUE_SETS            0
#define configUSE_RECURSIVE_MUTEXES     0

/* ---- 静态分配 (避免malloc) ---- */
#define configSUPPORT_STATIC_ALLOCATION 1
#define configSUPPORT_DYNAMIC_ALLOCATION 1

/* ---- 钩子函数 ---- */
#define configUSE_IDLE_HOOK             1
#define configUSE_TICK_HOOK             0
#define configCHECK_FOR_STACK_OVERFLOW  2   /* 方法2: 检查栈顶标记 */
#define configUSE_MALLOC_FAILED_HOOK    0

/* ---- Tick配置 ---- */
#define configUSE_16_BIT_TICKS          0   /* 0=32位, 49天后溢出 */

/* ---- 可选功能 ---- */
#define INCLUDE_vTaskPrioritySet        1
#define INCLUDE_uxTaskPriorityGet       1
#define INCLUDE_vTaskDelete             0
#define INCLUDE_vTaskSuspend            0
#define INCLUDE_vTaskDelayUntil         1
#define INCLUDE_vTaskDelay              1
#define INCLUDE_xTaskGetCurrentTaskHandle 0
#define INCLUDE_xTaskGetSchedulerState  0
#define INCLUDE_eTaskGetState           0

/* ---- 软件定时器 ---- */
#define configUSE_TIMERS                0

/* ---- 中断优先级 ---- */
/* STM32使用NVIC优先级4位, 仅高4位有效 */
#define configLIBRARY_KERNEL_INTERRUPT_PRIORITY         15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5
#define configKERNEL_INTERRUPT_PRIORITY                 (15 << 4)
#define configMAX_SYSCALL_INTERRUPT_PRIORITY            (5 << 4)

/* ---- 中断服务函数映射 ---- */
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

/* ---- 断言 ---- */
#define configASSERT(x)     if ((x) == 0) { taskDISABLE_INTERRUPTS(); while(1); }

#endif /* FREERTOS_CONFIG_H */
