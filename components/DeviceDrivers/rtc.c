#include <string.h>
#include <rtthread.h>

int set_time(const char* time)
{
	rt_device_t device;
	if(strlen(time) != 14)
	{
		return -1;
	}
	
	device = rt_device_find("rtc");
  if (device == RT_NULL)
  {
    return -1;
  }
	rt_kprintf("time:%s\n",time);
	device->write(device, 0, time,14);
	return 0;
}

int list_time()
{
	rt_device_t device;
	char time[15] = {'\0'};
	device = rt_device_find("rtc");
  if (device == RT_NULL)
  {
    return -1;
  }
	device->read(device, 0, time,14);
	rt_kprintf("time:%s\n",time);
	return 0;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
#include <rtdevice.h>
FINSH_FUNCTION_EXPORT(set_time, set time. e.g: set_time("20170302112020"))
FINSH_FUNCTION_EXPORT(list_time, list time. e.g: list_time())
#endif
