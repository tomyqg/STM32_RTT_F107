#include "main-thread.h"
#include <board.h>
#include "zigbee.h"
#include "resolve.h"
#include "variation.h"
#include "checkdata.h"
#include "rtc.h"
#include "datetime.h"
#include <dfs_posix.h> 
#include <rtthread.h>
#include "file.h"

#define MAIN_VERSION "R-1.0.0"

ALIGN(RT_ALIGN_SIZE)
extern rt_uint8_t main_stack[ 4096 ];
extern struct rt_thread main_thread;

inverter_info inverter[MAXINVERTERCOUNT];
ecu_info ecu;

int init_ecu()
{
	get_ecuid(ecu.id);
	//获取panid
	ecu.panid = get_panid();
	//获取ECU信道
	ecu.channel = get_channel();
	
	rt_memset(ecu.ip, '\0', sizeof(ecu.ip));
	ecu.life_energy = get_lifetime_power();
	ecu.current_energy = 0;
	ecu.system_power = 0;
	ecu.count = 0;
	ecu.total = 0;
	ecu.flag_ten_clock_getshortaddr = 1;			//ZK
	ecu.polling_total_times=0;					//ECU一天之中轮训的次数清0, ZK
	ecu.type = 0;
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

	/*
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
	*/
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

int acquire_time()
{
	char datetime[15] = {'\0'};
	unsigned char hour, minute, second;
	apstime(datetime);
	hour = ((datetime[8] - 0x30) *10) + (datetime[9] - 0x30);
	minute = ((datetime[10] - 0x30) *10) + (datetime[11] - 0x30);
	second = ((datetime[12] - 0x30) *10) + (datetime[13] - 0x30);
	//rt_kprintf("%d %d %d \n",hour,minute,second);
	return (hour*60*60+minute*60+second);
}


void main_thread_entry(void* parameter)
{
	int thistime=0, durabletime=65535, reportinterval=300;					//thistime:本轮向逆变器发送广播要数据的时间;durabletime:ECU本轮向逆变器要数据的持续时间
	char broadcast_hour_minute[3]={'\0'};									//向逆变器发送广播命令时的时间
	int cur_time_hour;														//当前的时间小时


	rt_kprintf("\nmain.exe %s\n", MAIN_VERSION);
	printmsg("Start-------------------------------------------------");

	init_all(inverter);   //初始化所有逆变器
	
	while(1)
	{
		if((durabletime-thistime) >= reportinterval){
		//if((durabletime-thistime) >= 60){
			thistime = acquire_time();
			rt_memset(ecu.broadcast_time, '\0', sizeof(ecu.broadcast_time));				//清空本次广播时间

			cur_time_hour = get_time(ecu.broadcast_time, broadcast_hour_minute);					//重新获取本次广播事件

			printmsg("****************************************");
			print2msg("ecu.broadcast_time",ecu.broadcast_time);
			
			ecu.count = getalldata(inverter);			//获取所有逆变器数据,返回当前有数据的逆变器数量
			ecu.life_energy = ecu.life_energy + ecu.current_energy;				//计算系统历史发电量

			update_life_energy(ecu.life_energy);								//设置系统历史发电量

			//update_today_energy(ecu.current_energy);							//设置系统当天发电量

			/*
			if(ecu.count>0)
			{
				save_system_power(ecu.system_power,ecu.broadcast_time);			//ZK
				update_daily_energy(ecu.current_energy,ecu.broadcast_time);
				update_monthly_energy(ecu.current_energy,ecu.broadcast_time);
				update_yearly_energy(ecu.current_energy,ecu.broadcast_time);
				update_lifetime_energy(ecu.life_energy);
			}
			display_on_lcd_and_web(); //液晶屏显示信息
			*/
			/*
			if(ecu.count>0)
			{
				protocol_APS18(inverter, ecu.broadcast_time);
				protocol_status(inverter, ecu.broadcast_time);
				saveevent(inverter, ecu.broadcast_time);							//保存当前一轮逆变器时间
			}

			if(ecu.count>0)
			{
				displayonweb(inverter, ecu.broadcast_time);								//实时数据页面数据
			}
//			printinverterinfo(&inverter);										//打印逆变器解析信息,ZK
//			format(inverter, ecu.broadcast_time, ecu.system_power, ecu.current_energy, ecu.life_energy);

			reset_inverter(inverter);											//重置每个逆变器
			remote_update(inverter);

			if((cur_time_hour>9)&&(1 == ecu.flag_ten_clock_getshortaddr))
			{
				get_inverter_shortaddress(inverter);
				if(ecu.polling_total_times>3)
				{
					ecu.flag_ten_clock_getshortaddr = 0;							//每天10点执行完重新获取短地址后标志位置为0
				}
			}

			//对于轮训没有数据的逆变器进行重新获取短地址操作
			bind_nodata_inverter(inverter);
			*/
		}
		process_all(inverter);
		rt_thread_delay(RT_TICK_PER_SECOND);

		durabletime = acquire_time();				//如果轮训一边的时间不到5分钟,那么一直等到5分钟再轮训下一遍,超过5分钟则等待10分钟。。。5分钟起跳
		if((durabletime-thistime)<=300)
			reportinterval = 300;
		else if((durabletime-thistime)<=600)
			reportinterval = 600;
		else
			reportinterval = 900;
	}
	
}
