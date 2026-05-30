/**
 * @file    control_task.h
 * @brief   自动控制任务 - 读取传感器数据, 阈值判断, 驱动执行器
 */

#ifndef __CONTROL_TASK_H
#define __CONTROL_TASK_H

#include "main.h"
#include "actuator.h"

/**
 * @brief  自动控制任务入口
 * @param  argument: 未使用
 * @note   优先级: 4 (最高) | 栈: 256B | 周期: 1000ms
 */
void vControlTask(void *argument);

#endif /* __CONTROL_TASK_H */
