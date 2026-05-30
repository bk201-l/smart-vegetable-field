/**
 * @file    display_task.h
 * @brief   OLED显示刷新任务
 */

#ifndef __DISPLAY_TASK_H
#define __DISPLAY_TASK_H

#include "main.h"
#include "oled.h"

/**
 * @brief  显示任务入口
 * @param  argument: 未使用
 * @note   优先级: 1 (最低) | 栈: 256B | 周期: 500ms
 */
void vDisplayTask(void *argument);

#endif /* __DISPLAY_TASK_H */
