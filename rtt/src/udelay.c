/*--------------File Info---------------------------------------------------------------------------------
** File Name:           udelay.c
** Last modified Date:  2017-02-13
** Last Version:        v1.00
** Description:         添加到RTT内核，主要实现us级别延时
**--------------------------------------------------------------------------------------------------------
** Created By:          shengfeng dong
** Created date:        2017-02-13
** Version:             v1.00
** Descriptions:        
**--------------------------------------------------------------------------------------------------------
** Modified by:         
** Modified date:       
** Version:             
** Description:         
*********************************************************************************************************/
#include <stm32f10x.h>
#include <core_cm3.h>
#include <rthw.h>

void rt_hw_us_delay(rt_uint32_t us)
{
    rt_uint32_t delta;
    rt_uint32_t current_delay;

        /*获得延时进过的tick数*/
    us = us * (SysTick->LOAD/(1000000/RT_TICK_PER_SECOND));

        /* 获得当前时间 */
    delta = SysTick->VAL;

        /* 轮训获得当前时间,直到达到指定时间后退出轮训 */
    do
    {
        if ( delta > SysTick->VAL )
            current_delay = delta - SysTick->VAL;
        else
        /*延时跨越一次OS tick的边界时的处理 */
            current_delay = SysTick->LOAD + delta - SysTick->VAL;
    } while( current_delay < us );
}
