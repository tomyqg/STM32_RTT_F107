#include "main-thread.h"
#include <board.h>
#include <rtthread.h>
#include "zigbee.h"

ALIGN(RT_ALIGN_SIZE)
extern rt_uint8_t main_stack[ 512 ];
extern struct rt_thread main_thread;
void main_thread_entry(void* parameter)
{
//		int count = 0;
//		char data[255];
		while(1)
		{
			//rt_kprintf("main_thread_entry:%d\r\n",count++);
			//openzigbee();

			//zb_shortaddr_reply(data,100,"123456789876");
		}

  	
}
