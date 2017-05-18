#include "phoneServer.h"
#include "rtthread.h"
void phone_server_thread_entry(void* parameter)
{
	while(1)
	{
		rt_thread_delay(RT_TICK_PER_SECOND);
	}
}
