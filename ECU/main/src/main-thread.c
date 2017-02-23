#include "main-thread.h"
#include <board.h>
#include <rtthread.h>
#include "zigbee.h"
#include "resolve.h"

#define MAIN_VERSION "R-1.0.0"

ALIGN(RT_ALIGN_SIZE)
extern rt_uint8_t main_stack[ 1024 ];
extern struct rt_thread main_thread;

inverter_info inverter[MAXINVERTERCOUNT];
int init_all(inverter_info *inverter)
{

	openzigbee();
	//init_ecu();
	//init_inverter(inverter);

	return 0;
}


void main_thread_entry(void* parameter)
{
	//int thistime=0, durabletime=65535, reportinterval=300;					//thistime:本轮向逆变器发送广播要数据的时间;durabletime:ECU本轮向逆变器要数据的持续时间
	//char broadcast_hour_minute[3]={'\0'};									//想逆变器发送广播命令时的时间
	//int cur_time_hour;														//当前的时间小时


	rt_kprintf("\nmain.exe %s\n", MAIN_VERSION);
	printmsg("Start-------------------------------------------------");
  
	init_all(inverter);   //初始化所有逆变器
	while(1)
	{
	}
}
