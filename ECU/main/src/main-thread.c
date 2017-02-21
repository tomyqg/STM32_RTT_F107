#include "main-thread.h"
#include <board.h>
#include <rtthread.h>
#include "zigbee.h"
#include "resolve.h"

ALIGN(RT_ALIGN_SIZE)
extern rt_uint8_t main_stack[ 1024 ];
extern struct rt_thread main_thread;
void main_thread_entry(void* parameter)
{
		//char buff[15] = {1,2,3,4,5,6};
		printmsg("Start-------------------------------------------------");
		openzigbee();
		while(1)
		{
			//rt_kprintf("11111\n");
			//zb_shortaddr_cmd(111,buff, 6);
			rt_thread_delay(RT_TICK_PER_SECOND);
		}

  	
}
