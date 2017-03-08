/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2013, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <board.h>
#include <rtthread.h>

#ifdef RT_USING_DFS
#include <dfs_fs.h>
#include <dfs_init.h>
#include <dfs_elm.h>
#endif

#ifdef RT_USING_LWIP
#include <eth@stm32.h>
#include <netif/ethernetif.h>
extern int lwip_system_init(void);
#endif

#ifdef RT_USING_FINSH
#include <shell.h>
#include <finsh.h>
#endif
#include "led.h"
#include "main-thread.h"

#ifdef W25QXX		
#include "w25q64flash.h"
#endif

#ifdef EEPROM		
#include "Flash_24L512.h"
#endif

#include "usr_wifi232.h"

ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t led_stack[200];
static struct rt_thread led_thread;
ALIGN(RT_ALIGN_SIZE)
rt_uint8_t main_stack[ 1024 ];
struct rt_thread main_thread;
static void led_thread_entry(void* parameter)
{
    unsigned int count=0;

    rt_hw_led_init();

    while (1)
    {
        /* led1 on */
        count++;
        rt_hw_led_on();
				//rt_kprintf("rt_hw_led_on:%d\n",count);
        rt_thread_delay( RT_TICK_PER_SECOND/2 ); /* sleep 0.5 second and switch to other thread */

        rt_hw_led_off();
				//rt_kprintf("rt_hw_led_off:%d\n",count);
        rt_thread_delay( RT_TICK_PER_SECOND/2 );
    }
}

void rt_init_thread_entry(void* parameter)
{
    {
        extern void rt_platform_init(void);
        rt_platform_init();
    }
#ifdef W25QXX		
	  SPI_init();
#endif
		
#ifdef EEPROM		
	  EEPROM_Init();
#endif
		
	/* Filesystem Initialization */
#if defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT)
	/* initialize the device file system */
	dfs_init();

	/* initialize the elm chan FatFS file system*/
	elm_init();
    
    /* mount sd card fat partition 1 as root directory */
    if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
    {
        rt_kprintf("File System initialized!\n");
    }
    else
    {
        rt_kprintf("File System initialzation failed!\n");
    }
#endif /* RT_USING_DFS && RT_USING_DFS_ELMFAT */


#ifdef RT_USING_LWIP
	/* initialize lwip stack */
	/* register ethernetif device */
	eth_system_device_init();

	/* initialize lwip system */
	lwip_system_init();
	rt_kprintf("TCP/IP initialized!\n");
#endif

#ifdef RT_USING_FINSH
	/* initialize finsh */
	finsh_system_init();
	finsh_set_device(RT_CONSOLE_DEVICE_NAME);
#endif
}

int rt_application_init(void)
{
    
		rt_err_t result;
	
		rt_thread_t tid;
    tid = rt_thread_create("init",
        rt_init_thread_entry, RT_NULL,
        2048, RT_THREAD_PRIORITY_MAX/3, 20);
    if (tid != RT_NULL) rt_thread_startup(tid);
	
    /* init led thread */
    result = rt_thread_init(&led_thread,
                            "led",
                            led_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&led_stack[0],
                            sizeof(led_stack),
                            20,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&led_thread);
    }
		
		/* init main thread */
		/*
    result = rt_thread_init(&main_thread,
                            "main",
                            main_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&main_stack[0],
                            sizeof(main_stack),
                            21,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&main_thread);
    }
		*/
		
		
		WiFi_Open();
    return 0;
}

/*@}*/
