/*
 * File      : led.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */
#include <rtthread.h>
#include <stm32f10x.h>

#define LAN8720_RCC                  	RCC_APB2Periph_GPIOB
#define LAN8720_GPIO              		GPIOB
#define LAN8720_PIN                  	(GPIO_Pin_14)

void rt_hw_lan8720_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(LAN8720_RCC,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = LAN8720_PIN;
    GPIO_Init(LAN8720_GPIO, &GPIO_InitStructure);
		GPIO_SetBits(LAN8720_GPIO, LAN8720_PIN);
}

void rt_hw_lan8720_rst(void)
{
  GPIO_ResetBits(LAN8720_GPIO, LAN8720_PIN);
	rt_thread_delay(RT_TICK_PER_SECOND/5);
	GPIO_SetBits(LAN8720_GPIO, LAN8720_PIN);
}



#ifdef RT_USING_FINSH
#include <finsh.h>
void lanrst()
{
	rt_hw_lan8720_rst();
}
FINSH_FUNCTION_EXPORT(lanrst, lanrst.)
#endif

