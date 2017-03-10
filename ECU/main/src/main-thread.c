#include "main-thread.h"
#include <board.h>
#include <rtthread.h>
#include "zigbee.h"
#include "resolve.h"
#include "variation.h"
#include "checkdata.h"
#include "rtc.h"

#define MAIN_VERSION "R-1.0.0"

ALIGN(RT_ALIGN_SIZE)
extern rt_uint8_t main_stack[ 1024 ];
extern struct rt_thread main_thread;

inverter_info inverter[MAXINVERTERCOUNT];
ecu_info ecu;


int init_ecu()
{
	char *ecuid = "888888888888";		//后续修改为从flash中获取
	rt_memcpy(ecu.id,ecuid,12);//获取ECU  ID
	ecu.panid = 0x88;
	//获取ECU信道
	ecu.channel = 0x10;
	rt_memset(ecu.ip, '\0', sizeof(ecu.ip));
	ecu.life_energy = 0;			//后续从flash中获取历史发电量
	ecu.current_energy = 0;
	ecu.system_power = 0;
	ecu.count = 0;
	ecu.total = 0;
	ecu.flag_ten_clock_getshortaddr = 1;			//ZK
	ecu.polling_total_times=0;					//ECU一天之中轮训的次数清0, ZK
	ecu.type = 1;
	ecu.zoneflag = 0;				//时区
	printecuinfo(&ecu);
	return 1;
}

int init_inverter(inverter_info *inverter)
{
	int i;
	char flag_limitedid = '0';				//限定ID标志

	inverter_info *curinverter = inverter;

	for(i=0; i<MAXINVERTERCOUNT; i++, curinverter++)
	{
		rt_memset(curinverter->id, '\0', sizeof(curinverter->id));		//清空逆变器UID
		rt_memset(curinverter->tnuid, '\0', sizeof(curinverter->tnuid));			//清空逆变器ID

		curinverter->model = 0;

		curinverter->dv=0;			//清空当前一轮直流电压
		curinverter->di=0;			//清空当前一轮直流电流
		curinverter->op=0;			//清空当前逆变器输出功率
		curinverter->gf=0;			//清空电网频率
		curinverter->it=0;			//清空逆变器温度
		curinverter->gv=0;			//清空电网电压
		curinverter->dvb=0;			//B路清空当前一轮直流电压
		curinverter->dib=0;			//B路清空当前一轮直流电流
		curinverter->opb=0;			//B路清空当前逆变器输出功率
		curinverter->gvb=0;
		curinverter->dvc=0;
		curinverter->dic=0;
		curinverter->opc=0;
		curinverter->gvc=0;
		curinverter->dvd=0;
		curinverter->did=0;
		curinverter->opd=0;
		curinverter->gvd=0;



		curinverter->curgeneration = 0;	//清空逆变器当前一轮发电量
		curinverter->curgenerationb = 0;	//B路清空当前一轮发电量

		curinverter->preaccgen = 0;
		curinverter->preaccgenb = 0;
		curinverter->curaccgen = 0;
		curinverter->curaccgenb = 0;
		curinverter->preacctime = 0;
		curinverter->curacctime = 0;

		rt_memset(curinverter->status_web, '\0', sizeof(curinverter->status_web));		//清空逆变器状态
		rt_memset(curinverter->status, '\0', sizeof(curinverter->status));		//清空逆变器状态
		rt_memset(curinverter->statusb, '\0', sizeof(curinverter->statusb));		//B路清空逆变器状态

		curinverter->dataflag = 0;		//上一轮有数据的标志置位
	//	curinverter->bindflag=0;		//绑定逆变器标志位置清0
		curinverter->no_getdata_num=0;	//ZK,清空连续获取不到的次数
		curinverter->disconnect_times=0;		//没有与逆变器通信上的次数清0, ZK
		curinverter->signalstrength=0;			//信号强度初始化为0

		curinverter->updating=0;
		curinverter->raduis=0;
	}

	get_ecu_type();		//获取ECU型号

	flag_limitedid = '1';		//后续从flash中获取限定ID标志


	if ('1' == flag_limitedid) {
		while(1) {
			//bind_inverters(); //绑定逆变器
			//ecu.total = get_id_from_db(inverter); //获取逆变器数量
			if (ecu.total > 0) {
				break; //直到逆变器数量大于0时退出循环
			} else {
				//display_input_id(); //提示用户输入逆变器ID
				rt_thread_delay(5*RT_TICK_PER_SECOND);
			}
		}
		flag_limitedid = '0';
	}
	else {
		while(1) {
			//ecu.total = get_id_from_db(inverter);
			if (ecu.total > 0) {
				break; //直到逆变器数量大于0时退出循环
			} else {
				//display_input_id(); //提示用户输入逆变器ID
				rt_thread_delay(5*RT_TICK_PER_SECOND);
			}
		}
	}

	return 1;
}


int init_all(inverter_info *inverter)
{

	openzigbee();
	init_ecu();
	init_inverter(inverter);

	return 0;
}

int reset_inverter(inverter_info *inverter)
{
	int i;
	inverter_info *curinverter = inverter;

	for(i=0; i<MAXINVERTERCOUNT; i++, curinverter++)
	{
		curinverter->dataflag = 0;

		curinverter->dv=0;
		curinverter->di=0;
		curinverter->op=0;
		curinverter->gf=0;
		curinverter->it=0;
		curinverter->gv=0;

		curinverter->dvb=0;
		curinverter->dib=0;
		curinverter->opb=0;

		curinverter->curgeneration = 0;
		curinverter->curgenerationb = 0;
		curinverter->status_send_flag=0;

		rt_memset(curinverter->status_web, '\0', sizeof(curinverter->status_web));		//????????
		rt_memset(curinverter->status, '\0', sizeof(curinverter->status));
		rt_memset(curinverter->statusb, '\0', sizeof(curinverter->statusb));
	}

	return 1;
}



void main_thread_entry(void* parameter)
{
	int thistime=0, durabletime=65535, reportinterval=300;					//thistime:本轮向逆变器发送广播要数据的时间;durabletime:ECU本轮向逆变器要数据的持续时间
	//char broadcast_hour_minute[3]={'\0'};									//向逆变器发送广播命令时的时间
	//int cur_time_hour;														//当前的时间小时


	rt_kprintf("\nmain.exe %s\n", MAIN_VERSION);
	printmsg("Start-------------------------------------------------");

	init_all(inverter);   //初始化所有逆变器
	
	while(1)
	{
		if((durabletime-thistime) >= reportinterval){
		//if((durabletime-thistime) >= 60){
			thistime = time(RT_NULL);
		
		}
		
	}
	
}
