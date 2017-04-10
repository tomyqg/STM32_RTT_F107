#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <threadlist.h>
#include "led.h"
#include "main-thread.h"
#include "client.h"
#include "control_client.h"
#include "ntpapp.h"
#include <board.h>
#include <rtthread.h>
#include "arch/sys_arch.h"


#ifdef RT_USING_DFS
#include <dfs_fs.h>
#include <dfs_init.h>
#include <dfs_elm.h>
#endif

#ifdef RT_USING_LWIP
#include <stm32_eth.h>
#include <netif/ethernetif.h>
extern int lwip_system_init(void);
#endif

#ifdef RT_USING_FINSH
#include <shell.h>
#include <finsh.h>
#endif

#ifdef W25QXX		
#include "w25q64flash.h"
#endif

#ifdef EEPROM		
#include "Flash_24L512.h"
#endif
#include "ds1302z_rtc.h"

#ifdef THREAD_PRIORITY_LED
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t led_stack[200];
static struct rt_thread led_thread;
#endif

#ifdef THREAD_PRIORITY_MAIN
ALIGN(RT_ALIGN_SIZE)
rt_uint8_t main_stack[ 4096 ];
struct rt_thread main_thread;
#endif

#ifdef THREAD_PRIORITY_CLIENT
ALIGN(RT_ALIGN_SIZE)
rt_uint8_t client_stack[ 8192 ];
struct rt_thread client_thread;
#endif

#ifdef THREAD_PRIORITY_CONTROL_CLIENT
ALIGN(RT_ALIGN_SIZE)
rt_uint8_t control_client_stack[ 10240 ];
struct rt_thread control_client_thread;
#endif

#ifdef THREAD_PRIORITY_DHCPRESET
#ifdef RT_USING_LWIP
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t dhcp_stack[1024];
static struct rt_thread dhcp_thread;
#endif
#endif

#ifdef THREAD_PRIORITY_NTP
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t ntp_stack[1024];
static struct rt_thread ntp_thread;
#endif

rt_mutex_t record_data_lock = RT_NULL;

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
    
    /* mount flash fat partition 1 as root directory */
    if (dfs_mount("flash", "/", "elm", 0, 0) == 0)
    {
        rt_kprintf("File System initialized!\n");
    }
    else
    {
        rt_kprintf("File System initialzation failed!\n");
    }
#endif /* RT_USING_DFS && RT_USING_DFS_ELMFAT */

#ifdef RT_USING_LWIP
  /* initialize eth interface */
  rt_hw_stm32_eth_init();

	/* initialize lwip stack */
	/* register ethernetif device */
	eth_system_device_init();

	/* initialize lwip system */
	lwip_system_init();
	rt_kprintf("TCP/IP initialized!\n");
#endif

//	EXTIX_Init();		 	//外部中断初始化


#ifdef RT_USING_FINSH
	/* initialize finsh */
	finsh_system_init();
	finsh_set_device(RT_CONSOLE_DEVICE_NAME);
#endif
	rt_hw_rtc_init();		//实时时钟初始化
		
	//初始化一把用于/home/record/data 数据读写的锁   使用互斥量 在操作该目录时加锁，操作结束解锁
	record_data_lock = rt_mutex_create("record_data_lock", RT_IPC_FLAG_FIFO);
	if (record_data_lock != RT_NULL)
	{
		rt_kprintf("Initialize record_data_lock successful!\n");
	}
}


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

#ifdef THREAD_PRIORITY_DHCPRESET	
#ifdef RT_USING_LWIP
static void dhcp_reset_thread_entry(void* parameter)
{ 
	while(1)
	{
		rt_thread_delay( RT_TICK_PER_SECOND * 600);
		if(ETH_ReadPHYRegister(PHY_ADDRESS, PHY_BSR) & PHY_Linked_Status)
		{
			//写重新DHCP的代码
			//printf("network link\n");
			//dhcp_reset();
		}
	}
}
#endif
#endif
#ifdef THREAD_PRIORITY_NTP
static void ntp_thread_entry(void* parameter)
{
  while(1)
  {
    get_time_from_NTP();
    rt_thread_delay( RT_TICK_PER_SECOND*10);		//259200
  }  

}
#endif

void tasks_new(void)//创建任务线程
{
	rt_err_t result;
	rt_thread_t tid;
	
	/* init init thread */
  tid = rt_thread_create("init",rt_init_thread_entry, RT_NULL,768, THREAD_PRIORITY_INIT, 20);
	if (tid != RT_NULL) rt_thread_startup(tid);
#ifdef THREAD_PRIORITY_LED
  /* init led thread */
  result = rt_thread_init(&led_thread,"led",led_thread_entry,RT_NULL,(rt_uint8_t*)&led_stack[0],sizeof(led_stack),THREAD_PRIORITY_LED,5);
  if (result == RT_EOK)
  {
    rt_thread_startup(&led_thread);
  }
#endif

#ifdef THREAD_PRIORITY_DHCPRESET	
#ifdef RT_USING_LWIP
  result = rt_thread_init(&dhcp_thread,"dhcp_reset",dhcp_reset_thread_entry,RT_NULL,(rt_uint8_t*)&dhcp_stack[0],sizeof(dhcp_stack),THREAD_PRIORITY_DHCPRESET,5);
  if (result == RT_EOK)
  {
    rt_thread_startup(&dhcp_thread);
  }	
#endif
#endif
	
#ifdef THREAD_PRIORITY_NTP		
  /* init ntp thread */
	result = rt_thread_init(&ntp_thread,"ntp",ntp_thread_entry,RT_NULL,(rt_uint8_t*)&ntp_stack[0],sizeof(ntp_stack),THREAD_PRIORITY_NTP,5);
  if (result == RT_EOK)
  {
    rt_thread_startup(&ntp_thread);
  }
#endif
	
#ifdef THREAD_PRIORITY_MAIN
	/* init main thread */
	result = rt_thread_init(&main_thread,"main",main_thread_entry,RT_NULL,(rt_uint8_t*)&main_stack[0],sizeof(main_stack),THREAD_PRIORITY_MAIN,5);
  if (result == RT_EOK)
  {
    rt_thread_startup(&main_thread);
  }
#endif
	
#ifdef THREAD_PRIORITY_CLIENT
	/* init client thread */
	result = rt_thread_init(&client_thread,"client",client_thread_entry,RT_NULL,(rt_uint8_t*)&client_stack[0],sizeof(client_stack),THREAD_PRIORITY_CLIENT,5);
  if (result == RT_EOK)
  {
		rt_thread_startup(&client_thread);
  }	
#endif
	
#ifdef THREAD_PRIORITY_CONTROL_CLIENT
	result = rt_thread_init(&control_client_thread,"control_client",control_client_thread_entry,RT_NULL,(rt_uint8_t*)&control_client_stack[0],sizeof(control_client_stack),THREAD_PRIORITY_CONTROL_CLIENT,5);
  if (result == RT_EOK)
  {
		rt_thread_startup(&control_client_thread);
  }	
#endif
	
	
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(dhcp_reset, eg:dhcp_reset());
#endif
