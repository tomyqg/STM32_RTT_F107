/*
 * File      : watchdog.h
 * Change Logs:
 * Date           Author       		Notes
 * 2017-05-17     shengfeng dong  first version.
 */

#include <watchdog.h>
#include <stm32f10x.h>
#include "stm32f10x_iwdg.h"
void rt_hw_watchdog_init(void)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); //使能对
	IWDG_SetPrescaler(IWDG_Prescaler_256); //设置IWDG预分频值
	IWDG_SetReload(3125); //设置IWDG重装载值
	IWDG_ReloadCounter(); //按照IWDG重装载寄存器的值 重装载IWDG计数器
	IWDG_Enable(); //使能IWDG
}

void kickwatchdog(void)
{
	IWDG_ReloadCounter();//reload
}


