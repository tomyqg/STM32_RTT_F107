/*****************************************************************************/
/*  File      : zigbee.c                                                     */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-03-05 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#ifdef RT_USING_SERIAL
#include "serial.h"
#endif /* RT_USING_SERIAL */
#include <rtdef.h>
#include <rtthread.h>
#include "debug.h"
#include "zigbee.h"
#include <rthw.h>
#include <stm32f10x.h>
#include "resolve.h"
#include <dfs_posix.h> 
#include <stdio.h>
#include "ema_control.h"
#include "file.h"
#include "rthw.h"

#include "setpower.h"
#include "set_ird.h"
#include "turn_on_off.h"
#include "clear_gfdi.h"
#include "set_protection_parameters.h"
#include "set_protection_parameters_inverter.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern struct rt_device serial4;		//串口4为Zigbee收发串口
extern ecu_info ecu;
extern int zigbeeReadFlag;
static int zigbeereadtimeoutflag = 0;

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/

#define ZIGBEE_SERIAL (serial4)

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
//定时器超时函数
static void readtimeout_Zigbee(void* parameter)
{
	zigbeereadtimeoutflag = 1;
}

int selectZigbee(int timeout)			//zigbee串口数据检测 返回0 表示串口没有数据  返回1表示串口有数据
{
	
	rt_timer_t readtimer;
	readtimer = rt_timer_create("read", /* 定时器名字为 read */
					readtimeout_Zigbee, /* 超时时回调的处理函数 */
					RT_NULL, /* 超时函数的入口参数 */
					timeout*RT_TICK_PER_SECOND, /* 定时时间长度,以OS Tick为单位*/
					 RT_TIMER_FLAG_ONE_SHOT); /* 单周期定时器 */
	if (readtimer != RT_NULL) rt_timer_start(readtimer);
	zigbeereadtimeoutflag = 0;

	while(1)
	{
		if(zigbeereadtimeoutflag)
		{
			rt_timer_delete(readtimer);
			return 0;
		}else 
		{
			rt_hw_ms_delay(10);
			if(zigbeeReadFlag == 1)	//串口数据监测,如果有数据则返回1
			{
				rt_timer_delete(readtimer);
				rt_hw_ms_delay(200);
				return 1;
			}
		}
	}
}

void clear_zbmodem(void)		//清空串口缓冲区的数据
{
	char data[256];
	//清空缓冲器代码	通过将接收缓冲区的所有数据都读取出来，从而清空数据
	ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data, 255);
	rt_hw_ms_delay(20);
	ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data, 255);
	rt_hw_ms_delay(20);
	ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data, 255);
	rt_hw_ms_delay(20);
}

int openzigbee(void)
{
	int result = 0;
	GPIO_InitTypeDef GPIO_InitStructure;
	rt_device_t new;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_7);		//设置引脚为高电平输出，使能Zigbbe模块
	
	new = rt_device_find("uart4");		//寻找zigbee串口并配置模式
	if (new != RT_NULL)
	{
		result = rt_device_open(new, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
		if(result)
		{
			printdecmsg(ECU_DBG_MAIN,"open Zigbee failed ",result);
		}else
		{
			printmsg(ECU_DBG_MAIN,"open Zigbee success");
		}
	}

	return result;
}

//复位zigbee模块  通过PC7的电平置高置低然后达到复位的效果
void zigbee_reset(void)
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_7);		//设置引脚为低电平输出
	rt_hw_ms_delay(1000);
	GPIO_SetBits(GPIOC, GPIO_Pin_7);		//设置引脚为高电平输出
	rt_hw_s_delay(10);
	printmsg(ECU_DBG_MAIN,"zigbee reset successful");
}

int zb_shortaddr_cmd(int shortaddr, char *buff, int length)		//zigbee 短地址报头
{
	unsigned char sendbuff[256] = {'\0'};
	int i;
	int check=0;
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x55;
	sendbuff[5]  = shortaddr>>8;
	sendbuff[6]  = shortaddr;
	sendbuff[7]  = 0x00;
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = length;

	for(i=0; i<length; i++)
	{
		sendbuff[15+i] = buff[i];
	}
	
	if(0!=shortaddr)
	{
		ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0,sendbuff, (length+15));
		printhexmsg(ECU_DBG_MAIN,"zb_shortaddr_cmd", (char *)sendbuff, 15);
		return 1;
	}
	else
		return -1;

}

int zb_shortaddr_reply(char *data,int shortaddr,char *id)			//读取逆变器的返回帧,短地址模式
{
	int i;
	char data_all[256];
	char inverterid[13] = {'\0'};
	int temp_size,size;
	
	if(selectZigbee(2) <= 0)
	{
		printmsg(ECU_DBG_MAIN,"Get reply time out");
	
		return -1;
	}
	else
	{
		temp_size = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data_all, 255);
	
		size = temp_size -12;

		for(i=0;i<size;i++)
		{
			data[i]=data_all[i+12];
		}
		printhexmsg(ECU_DBG_MAIN,"Reply", data_all, temp_size);
		rt_sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data_all[6],data_all[7],data_all[8],data_all[9],data_all[10],data_all[11]);
		if((size>0)&&(0xFC==data_all[0])&&(0xFC==data_all[1])&&(data_all[2]==shortaddr/256)&&(data_all[3]==shortaddr%256)&&(data_all[5]==0xA5)&&(0==rt_strcmp(id,inverterid)))
		{
			return size;
		}
		else
		{
			return -1;
		}
	}
}


int zb_get_reply(char *data,inverter_info *inverter)			//读取逆变器的返回帧
{
	int i;
	char data_all[256];
	char inverterid[13] = {'\0'};
	int temp_size,size;

	if(selectZigbee(2) <= 0)
	{
		printmsg(ECU_DBG_MAIN,"Get reply time out");
		inverter->signalstrength=0;
		return -1;
	}
	else
	{
		temp_size = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data_all, 255);
		size = temp_size -12;

		for(i=0;i<size;i++)
		{
			data[i]=data_all[i+12];
		}
		printhexmsg(ECU_DBG_MAIN,"Reply", data_all, temp_size);
		rt_sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data_all[6],data_all[7],data_all[8],data_all[9],data_all[10],data_all[11]);
		if((size>0)&&(0xFC==data_all[0])&&(0xFC==data_all[1])&&(data_all[2]==inverter->shortaddr/256)&&(data_all[3]==inverter->shortaddr%256)&&(0==rt_strcmp(inverter->id,inverterid)))
		{
			inverter->raduis=data_all[5];
			inverter->signalstrength=data_all[4];
			return size;
		}
		else
		{
			inverter->signalstrength=0;
			return -1;
		}
	}

}

int zb_get_reply_update_start(char *data,inverter_info *inverter)			//读取逆变器远程更新的Update_start返回帧，ZK，返回响应时间定为10秒
{
	int i;
	char data_all[256];
	char inverterid[13] = {'\0'};
	int temp_size,size;

	if(selectZigbee(10) <= 0)
	{
		printmsg(ECU_DBG_MAIN,"Get reply time out");
		return -1;
	}
	else
	{
		temp_size = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data_all, 255);
		size = temp_size -12;

		for(i=0;i<size;i++)
		{
			data[i]=data_all[i+12];
		}
		printhexmsg(ECU_DBG_MAIN,"Reply", data_all, temp_size);
		rt_sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data_all[6],data_all[7],data_all[8],data_all[9],data_all[10],data_all[11]);
		if((size>0)&&(0xFC==data_all[0])&&(0xFC==data_all[1])&&(data_all[2]==inverter->shortaddr/256)&&(data_all[3]==inverter->shortaddr%256)&&(data_all[5]==0xA5)&&(0==rt_strcmp(inverter->id,inverterid)))
		{
			return size;
		}
		else
		{
			return -1;
		}
	}

}

int zb_get_reply_restore(char *data,inverter_info *inverter)			//读取逆变器远程更新失败，还原指令后的返回帧，ZK，因为还原时间比较长，所以单独写一个函数
{
	int i;
	char data_all[256];
	char inverterid[13] = {'\0'};
	int temp_size,size;

	if(selectZigbee(200) <= 0)
	{
		printmsg(ECU_DBG_MAIN,"Get reply time out");
		return -1;
	}
	else
	{
		temp_size = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data_all, 255);
		size = temp_size -12;
		
		for(i=0;i<size;i++)
		{
			data[i]=data_all[i+12];
		}
		printhexmsg(ECU_DBG_MAIN,"Reply", data_all, temp_size);
		rt_sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data_all[6],data_all[7],data_all[8],data_all[9],data_all[10],data_all[11]);

		if((size>0)&&(0xFC==data_all[0])&&(0xFC==data_all[1])&&(data_all[2]==inverter->shortaddr/256)&&(data_all[3]==inverter->shortaddr%256)&&(data_all[5]==0xA5)&&(0==rt_strcmp(inverter->id,inverterid)))
		{
			return size;
		}
		else
		{
			return -1;
		}	
	}
}

int zb_get_reply_from_module(char *data)			//读取zigbee模块的返回帧
{
	int size = 0;

	if(selectZigbee(1) <= 0)
	{
		printmsg(ECU_DBG_MAIN,"Get reply time out");
		return -1;
	}else
	{
		size = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data, 255);
		if(size > 0)
		{
			printhexmsg(ECU_DBG_MAIN,"Reply", data, size);
			return size;
		}else
		{
			return -1;
		}
	}	
}

int zb_get_id(char *data)			//获取逆变器ID
{
	int size;

	if(selectZigbee(30) <= 0){
		printmsg(ECU_DBG_MAIN,"Get id time out");
		return -1;
	}
	else{
		size = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data, 255);
		printhexmsg(ECU_DBG_MAIN,"Reply", data, size);
		return size;
	}
}

int zb_turnoff_limited_rptid(int short_addr,inverter_info *inverter)			//关闭限定单个逆变器上报ID功能
{
	unsigned char sendbuff[256] = {'\0'};
	int i=0, ret;
	char data[256];
	int check=0;

	clear_zbmodem();			//发送指令前，先清空缓冲区
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x08;
	sendbuff[5]  = short_addr>>8;
	sendbuff[6]  = short_addr;
	sendbuff[7]  = 0x08;//panid
	sendbuff[8]  = 0x88;
	sendbuff[9]  = 0x19;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0xA0;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;

	if(0!=inverter->shortaddr)
	{
		printmsg(ECU_DBG_MAIN,"Turn off limited report id");
		ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0,sendbuff, 15);
		printhexmsg(ECU_DBG_MAIN,"sendbuff",(char *)sendbuff,15);
		ret = zb_get_reply_from_module(data);
		if((11 == ret)&&(0xA5 == data[2])&&(0xA5 == data[3]))
		{
			if(inverter->zigbee_version!=data[9])
			{
				inverter->zigbee_version = data[9];
//				update_zigbee_version(inverter);
			}
			return 1;
		}
		else
			return -1;
	}
	else
		return -1;

}

int zb_turnoff_rptid(int short_addr)			//关闭单个逆变器上报ID功能
{
	unsigned char sendbuff[256] = {'\0'};
	int i=0;
	int check=0;
	printmsg(ECU_DBG_MAIN,"Turn off report id");

	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x08;
	sendbuff[5]  = short_addr>>8;
	sendbuff[6]  = short_addr;
	sendbuff[7]  = 0x08;//panid
	sendbuff[8]  = 0x88;
	sendbuff[9]  = 0x19;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;

	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0, sendbuff, 15);
//	ret = zb_get_reply(data);
//	if((11 == ret)&&(0xA5 == data[2])&&(0xA5 == data[3]))
		return 1;
//	else
//		return -1;
}

int zb_get_inverter_shortaddress_single(inverter_info *inverter)			//获取单台指定逆变器短地址，ZK
{
	unsigned char sendbuff[256] = {'\0'};
	int i=0, ret;
	char data[256];
	char inverterid[13] = {'\0'};
	int check=0;
	printmsg(ECU_DBG_MAIN,"Get inverter shortaddresssingle");

	clear_zbmodem();			//发送指令前先清空缓冲区
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x0E;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0x00;//panid
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x06;

	sendbuff[15]=((inverter->id[0]-0x30)*16+(inverter->id[1]-0x30));
	sendbuff[16]=((inverter->id[2]-0x30)*16+(inverter->id[3]-0x30));
	sendbuff[17]=((inverter->id[4]-0x30)*16+(inverter->id[5]-0x30));
	sendbuff[18]=((inverter->id[6]-0x30)*16+(inverter->id[7]-0x30));
	sendbuff[19]=((inverter->id[8]-0x30)*16+(inverter->id[9]-0x30));
	sendbuff[20]=((inverter->id[10]-0x30)*16+(inverter->id[11]-0x30));

//	strcpy(&sendbuff[15],inverter->inverterid);


	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0, sendbuff, 21);
	printhexmsg(ECU_DBG_MAIN,"sendbuff",(char *)sendbuff,21);
	ret = zb_get_reply_from_module(data);

	rt_sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data[4],data[5],data[6],data[7],data[8],data[9]);

	if((11 == ret)&&(0xFF == data[2])&&(0==rt_strcmp(inverter->id,inverterid)))
	{
		inverter->shortaddr = data[0]*256 + data[1];
		updateID();
		return 1;
	}
	else
		return -1;

}

//组网(令逆变器上报ID,并对存在于ID表中的逆变器进行绑定)
int zb_turnon_limited_rtpid(inverter_info *firstinverter)
{
	
	return 0;
}

int zb_turnon_rtpid(inverter_info *firstinverter)			//开启逆变器自动上报ID
{
	char sendbuff[256] = {'\0'};
	char data[256];
	int i, count=0;
	int short_addr;
	char inverterid[256] = {'\0'};
	int check=0;
	printmsg(ECU_DBG_MAIN,"Turn on report id");

	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x02;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0x00;
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;

	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0, sendbuff, 15);

	for(i = 0;i<60;i++)
	{
		rt_hw_s_delay(5);//上报时间为10分钟
		rt_memset(data, '\0', sizeof(data));
		if((11 == zb_get_id(data)) && (0xFF == data[2]) && (0xFF == data[3]))
		{
			short_addr = data[0] * 256 + data[1];
			for(i=0; i<6; i++){
				inverterid[2*i] = (data[i+4]>>4) + 0x30;
				inverterid[2*i+1] = (data[i+4]&0x0f) + 0x30;
			}
			print2msg(ECU_DBG_MAIN,"inverterid",inverterid);
//			save_inverter_id(inverterid,short_addr);  //保存逆变器以及短地址
			rt_hw_s_delay(5);
			zb_turnoff_rptid(short_addr);
			rt_memset(inverterid, '\0', sizeof(inverterid));
		}else
		{
			rt_hw_s_delay(5);
		}
	}
	
//	count=get_id_from_db(firstinverter);
	return count;
}

int zb_change_inverter_panid_broadcast(void)	//广播改变逆变器的PANID，ZK
{
	char sendbuff[256] = {'\0'};
	int i;
	int check=0;
	clear_zbmodem();
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x03;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = ecu.panid>>8;
	sendbuff[8]  = ecu.panid;
	sendbuff[9]  = ecu.channel;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;

	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0, sendbuff, 15);
	printhexmsg(ECU_DBG_MAIN,"sendbuff",sendbuff,15);

	return 1;
}

int zb_change_inverter_panid_single(inverter_info *inverter)	//单点改变逆变器的PANID和信道，ZK
{
	char sendbuff[256] = {'\0'};
	int i;
	int check=0;
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x0F;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = ecu.panid>>8;
	sendbuff[8]  = ecu.panid;
	sendbuff[9]  = ecu.channel;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0xA0;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x06;
	sendbuff[15]=((inverter->id[0]-0x30)*16+(inverter->id[1]-0x30));
	sendbuff[16]=((inverter->id[2]-0x30)*16+(inverter->id[3]-0x30));
	sendbuff[17]=((inverter->id[4]-0x30)*16+(inverter->id[5]-0x30));
	sendbuff[18]=((inverter->id[6]-0x30)*16+(inverter->id[7]-0x30));
	sendbuff[19]=((inverter->id[8]-0x30)*16+(inverter->id[9]-0x30));
	sendbuff[20]=((inverter->id[10]-0x30)*16+(inverter->id[11]-0x30));
	
	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0, sendbuff, 21);
	printhexmsg(ECU_DBG_MAIN,"sendbuff",sendbuff,21);

	rt_hw_s_delay(1);
	return 1;

}

int zb_restore_inverter_panid_channel_single_0x8888_0x10(inverter_info *inverter)	//单点还原逆变器的PANID到0X8888和信道0X10，ZK
{
	char sendbuff[256] = {'\0'};
	int i;
	int check=0;
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x0F;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0x88;
	sendbuff[8]  = 0x88;
	sendbuff[9]  = 0x10;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0xA0;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x06;
	sendbuff[15]=((inverter->id[0]-0x30)*16+(inverter->id[1]-0x30));
	sendbuff[16]=((inverter->id[2]-0x30)*16+(inverter->id[3]-0x30));
	sendbuff[17]=((inverter->id[4]-0x30)*16+(inverter->id[5]-0x30));
	sendbuff[18]=((inverter->id[6]-0x30)*16+(inverter->id[7]-0x30));
	sendbuff[19]=((inverter->id[8]-0x30)*16+(inverter->id[9]-0x30));
	sendbuff[20]=((inverter->id[10]-0x30)*16+(inverter->id[11]-0x30));

	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0, sendbuff, 21);
	printhexmsg(ECU_DBG_MAIN,"sendbuff",sendbuff,21);

	rt_hw_s_delay(1);
	return 1;

}

//设置ECU的PANID和信道
int zb_change_ecu_panid(void)
{
	unsigned char sendbuff[16] = {'\0'};
	char recvbuff[256] = {'\0'};
	int i;
	int check=0;
	clear_zbmodem();
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x05;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = ecu.panid>>8;
	sendbuff[8]  = ecu.panid;
	sendbuff[9]  = ecu.channel;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;

	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0, sendbuff, 15);
	printhexmsg(ECU_DBG_MAIN,"Set ECU PANID and Channel", (char *)sendbuff, 15);

	if ((3 == zb_get_reply_from_module(recvbuff))
			&& (0xAB == recvbuff[0])
			&& (0xCD == recvbuff[1])
			&& (0xEF == recvbuff[2])) {
		rt_hw_s_delay(2); //延时2S，因为设置完ECU信道和PANID后会发6个FF
		return 1;
	}

	return -1;
}

int zb_restore_ecu_panid_0x8888(void)			//恢复ECU的PANID为0x8888,ZK
{
	unsigned char sendbuff[256] = {'\0'};
	int i=0, ret;
	char data[256];
	int check=0;
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x05;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0x88;
	sendbuff[8]  = 0x88;
	sendbuff[9]  = ecu.channel;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;

	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0, sendbuff, 15);
	printhexmsg(ECU_DBG_MAIN,"sendbuff",(char *)sendbuff,15);
	ret = zb_get_reply_from_module(data);
	if((3 == ret)&&(0xAB == data[0])&&(0xCD == data[1])&&(0xEF == data[2]))
		return 1;
	else
		return -1;
}

//设置ECU的PANID为0xFFFF,信道为指定信道(注:向逆变器发送设置命令时,需将ECU的PANID设为0xFFFF)
int zb_restore_ecu_panid_0xffff(int channel)
{
	unsigned char sendbuff[15] = {'\0'};
	char recvbuff[256];
	int i;
	int check=0;
	//向ECU发送命令
	clear_zbmodem();
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x05;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0xFF;
	sendbuff[8]  = 0xFF;
	sendbuff[9]  = channel;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;
	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL,0, sendbuff, 15);
	printhexmsg(ECU_DBG_MAIN,"Change ECU channel (PANID:0xFFFF)", (char *)sendbuff, 15);

	//接收反馈
	if ((3 == zb_get_reply_from_module(recvbuff))
			&& (0xAB == recvbuff[0])
			&& (0xCD == recvbuff[1])
			&& (0xEF == recvbuff[2])) {
		return 1;
	}

	return -1;
}

int zb_send_cmd(inverter_info *inverter, char *buff, int length)		//zigbee包头
{
	unsigned char sendbuff[256] = {'\0'};
	int i;
	int check=0;

	clear_zbmodem();			//发送数据前,清空缓冲区
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x55;
	sendbuff[5]  = inverter->shortaddr>>8;
	sendbuff[6]  = inverter->shortaddr;
	sendbuff[7]  = 0x00;
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = length;

	printdecmsg(ECU_DBG_MAIN,"shortaddr",inverter->shortaddr);
	for(i=0; i<length; i++)
	{
		sendbuff[15+i] = buff[i];
	}

	if(0!=inverter->shortaddr)
	{
		ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL,0, sendbuff, length+15);
		//printhexmsg("Send", (char *)sendbuff, length+15);
		return 1;
	}
	else
		return -1;
}

int zb_broadcast_cmd(char *buff, int length)		//zigbee广播包头
{
	unsigned char sendbuff[256] = {'\0'};
	int i;
	int check=0;
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x55;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0x00;
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = length;

	for(i=0; i<length; i++)
	{
		sendbuff[15+i] = buff[i];
	}

	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL,0, sendbuff, length+15);

	return 1;
}

int zb_query_inverter_info(inverter_info *inverter)		//请求逆变器的机型码
{
	int i = 0;
	char sendbuff[256];
	char recvbuff[256];
	
	clear_zbmodem();
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xDC;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xE2;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	zb_send_cmd(inverter, sendbuff, i);
	print2msg(ECU_DBG_MAIN,"Query Inverter's Model and Version", inverter->id);

	if ((16 == zb_get_reply(recvbuff, inverter))
			&& (0xFB == recvbuff[0])
			&& (0xFB == recvbuff[1])
			&& (0x09 == recvbuff[2])
			&& (0xDC == recvbuff[3])
			//&& (0xDC == recvbuff[5])
			&& (0xFE == recvbuff[14])
			&& (0xFE == recvbuff[15])) {
		inverter->model = recvbuff[4];
		inverter->version = (recvbuff[5]*256 + recvbuff[6])*1000+(recvbuff[8]*256+recvbuff[9]);
		/*
		if(turn_on_flag==1)
		{
			if(recvbuff[7]==1)
				inverter->inverterstatus.fill_up_data_flag=1;
			else if(recvbuff[7]==0)
				inverter->inverterstatus.fill_up_data_flag=2;
			else
				inverter->inverterstatus.fill_up_data_flag=3;
			save_inverter_replacement_model(inverter->id,(inverter->inverterstatus.fill_up_data_flag%3));
		}
		*/
		return 1;
	}

	return -1;
}

int zb_query_data(inverter_info *inverter)		//请求逆变器实时数据
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];
	int check=0;

	print2msg(ECU_DBG_MAIN,"Query inverter data",inverter->id);
	clear_zbmodem();			//发送指令前,先清空缓冲区
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xBB;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xC1;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(data,inverter);
	
	if((0 != ret)&&(ret%88 == 0)&&(0xFB == data[0])&&(0xFB == data[1])&&(0xFE == data[86])&&(0xFE == data[87]))
	{
		for(i=2;i<84;i++)
			check=check+data[i];
		
		if(((data[84]==0x00)&&(data[85]==0x00))||((check/256 == data[84])&&(check%256 == data[85])))
		{
			inverter->no_getdata_num = 0;	//一旦接收到数据就清0,ZK
			inverter->inverterstatus.dataflag = 1;	//接收到数据置为1
			if(7==inverter->model)
			{
				if(0xBB == data[3])
				{
					resolvedata_600(&data[4], inverter);
					inverter->inverterstatus.deputy_model = 1;
				}
				else if(0xB1 == data[3])
				{
					resolvedata_600_new(&data[4], inverter);
					inverter->inverterstatus.deputy_model = 2;
				}
				else ;
			}

			else if(5==inverter->model)
				resolvedata_1000(&data[4], inverter);
			else if(6==inverter->model)
				resolvedata_1000(&data[4], inverter);
			else
				{;}

			return 1;
		}else
		{
			inverter->inverterstatus.dataflag = 0;		//娌℃湁鎺ュ彈鍒版暟鎹氨缃负0
			return -1;
		}
	}
	else
	{
		inverter->inverterstatus.dataflag = 0;		//没有接收到数据就置为0
		return -1;
	}

}

int zb_test_communication(void)		//zigbee测试通信有没有断开
{
	unsigned char sendbuff[256] = {'\0'};
	int i=0, ret = 0;
	char data[256] =  {'\0'};
	int check=0;

	printmsg(ECU_DBG_MAIN,"test zigbee communication");
	clear_zbmodem();			//发送指令前,先清空缓冲区
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x0D;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = 0x00;
	sendbuff[8]  = 0x00;
	sendbuff[9]  = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x00;

	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL,0, sendbuff, 15);
	ret = zb_get_reply_from_module(data);
	if((3 == ret)&&(0xAB == data[0])&&(0xCD == data[1])&&(0xEF == data[2]))
		return 1;
	else
		return -1;

}

int zb_set_protect_parameter(inverter_info *inverter, char *protect_parameter)		//参数修改CC指令
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];
	
	clear_zbmodem();			//发送数据前,清空缓冲区
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x10;
	sendbuff[i++] = 0xCC;
	sendbuff[i++] = protect_parameter[0];
	sendbuff[i++] = protect_parameter[1];
	sendbuff[i++] = protect_parameter[2];
	sendbuff[i++] = protect_parameter[3];
	sendbuff[i++] = protect_parameter[4];
	sendbuff[i++] = protect_parameter[5];
	sendbuff[i++] = protect_parameter[6];
	sendbuff[i++] = protect_parameter[7];
	sendbuff[i++] = protect_parameter[8];
	sendbuff[i++] = protect_parameter[9];
	sendbuff[i++] = protect_parameter[10];
	sendbuff[i++] = protect_parameter[11];
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	print2msg(ECU_DBG_MAIN,inverter->id,"Set protect parameters");
	printhexmsg(ECU_DBG_MAIN,"Set protect parameters", sendbuff, i);
	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(data,inverter);
	if((13 == ret) && (0xDE == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[11]) && (0xFE == data[12]))
		return 1;
	else
		return -1;
}

int zb_query_protect_parameter(inverter_info *inverter, char *protect_data_DD_reply)		//存储参数查询DD指令
{
	int i=0,ret;
	char sendbuff[256];
	
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xDD;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	print2msg(ECU_DBG_MAIN,inverter->id, "zb_query_protect_parameter");
	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(protect_data_DD_reply,inverter);
	if((33 == ret) && (0xDD == protect_data_DD_reply[3]) && (0xFB == protect_data_DD_reply[0]) && (0xFB == protect_data_DD_reply[1]) && (0xFE == protect_data_DD_reply[31]) && (0xFE == protect_data_DD_reply[32]))
		return 1;
	if((58 == ret) &&
				(0xFB == protect_data_DD_reply[0]) &&
				(0xFB == protect_data_DD_reply[1]) &&
				(0xDA == protect_data_DD_reply[3]) &&
				(0xFE == protect_data_DD_reply[56]) &&
				(0xFE == protect_data_DD_reply[57]))
		return 1;
	else
		return -1;
}

int zb_afd_broadcast(void)		//AFD广播指令
{
	int i=0;
	char sendbuff[256];
	
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x07;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xAF;
	sendbuff[i++] = 0xFF;
	sendbuff[i++] = 0xFF;
	sendbuff[i++] = 0x05;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	zb_broadcast_cmd(sendbuff, i);

	return 1;
}

int zb_turnon_inverter_broadcast(void)		//开机指令广播,OK
{
	int i=0;
	char sendbuff[256];
	
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xA1;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	zb_broadcast_cmd(sendbuff, i);
	return 1;
}

int zb_boot_single(inverter_info *inverter)		//开机指令单播,OK
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];
	
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xC1;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	printmsg(ECU_DBG_MAIN,"zb_boot_single");
	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(data,inverter);
	if((13 == ret) && (0xDE == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[11]) && (0xFE == data[12]))
		return 1;
	else
		return -1;
}

int zb_shutdown_broadcast(void)		//关机指令广播,OK
{
	int i=0;
	char sendbuff[256];
	
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xA2;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	zb_broadcast_cmd(sendbuff, i);
	return 1;
}

int zb_shutdown_single(inverter_info *inverter)		//关机指令单播,OK
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];
	
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xC2;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	printmsg(ECU_DBG_MAIN,"zb_shutdown_single");
	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(data,inverter);
	if((13 == ret) && (0xDE == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[11]) && (0xFE == data[12]))
		return 1;
	else
		return -1;
}

int zb_boot_waitingtime_single(inverter_info *inverter)		//开机等待时间启动控制单播,OK
{
	int i=0;
	char sendbuff[256];
//	char data[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xCD;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_send_cmd(inverter, sendbuff, i);
	return 1;
}

int zb_clear_gfdi_broadcast(void)		//清除GFDI广播,OK
{
	int i=0;
	char sendbuff[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xCF;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_broadcast_cmd(sendbuff, i);
	return 1;
}

int zb_clear_gfdi(inverter_info *inverter)		//清除GFDI,OK
{
	int i=0, ret;
	char sendbuff[256];
	char data[256];
	
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xAF;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	
	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(data,inverter);
	if((13 == ret) && (0xDE == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[11]) && (0xFE == data[12]))
		return 1;
	else
		return -1;
}

int zb_ipp_broadcast(void)		//IPP广播
{
	int i=0;
	char sendbuff[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xA5;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_broadcast_cmd(sendbuff, i);
	return 1;
}

int zb_ipp_single(inverter_info *inverter)		//IPP单播
{
	int i=0;
	char sendbuff[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xC5;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_send_cmd(inverter, sendbuff, i);
	return 1;
}

int zb_frequency_protectime_broadcast(void)		//欠频保护时间广播
{
	int i=0;
	char sendbuff[256];
//	char data[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x07;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xC5;
	sendbuff[i++] = 0xFF;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x05;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_broadcast_cmd(sendbuff, i);
	return 1;
}

int zb_frequency_protectime_single(inverter_info *inverter)		//欠频保护时间单播
{
	int i=0;
	char sendbuff[256];
//	char data[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xC6;
	sendbuff[i++] = 0xFF;
	sendbuff[i++] = 0x04;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_send_cmd(inverter, sendbuff, i);
	return 1;
}

int zb_voltage_protectime_broadcast(void)		//欠压保护时间广播
{
	int i=0;
	char sendbuff[256];
//	char data[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x07;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xC9;
	sendbuff[i++] = 0xFF;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x05;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_broadcast_cmd(sendbuff, i);
	return 1;
}

int zb_voltage_protectime_single(inverter_info *inverter)		//欠压保护时间单播
{
	int i=0;
	char sendbuff[256];
//	char data[256];

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xCA;
	sendbuff[i++] = 0xFF;
	sendbuff[i++] = 0x04;
	sendbuff[i++] = 0x01;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_send_cmd(inverter, sendbuff, i);
	return 1;
}

int process_gfdi(inverter_info *firstinverter)
{
	int i,j;
	FILE *fp;
	char command[256] = {'\0'};
	inverter_info *curinverter = firstinverter;

	fp = fopen("/tmp/procgfdi.con", "r");
	while(1){
		curinverter = firstinverter;
		memset(command, '\0', 256);
		fgets(command, 256, fp);
		if(!strlen(command))
			break;
		if('\n' == command[strlen(command)-1])
			command[strlen(command)-1] = '\0';
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++){
			if(!strcmp(command, curinverter->id))
			{
				j=0;
				while(j<3)
				{
					if(1 == zb_clear_gfdi(curinverter))
					{
						print2msg(ECU_DBG_MAIN,"Clear GFDI", curinverter->id);
						break;
					}
					j++;
				}
				break;
			}
			curinverter++;
		}
	}

	fclose(fp);
	fp = fopen("/tmp/procgfdi.con", "w");
	fclose(fp);

	return 0;
}

int compare_protect_data(char *set_protect_data, char *actual_protect_data)				//比较逆变器返回的预设值和页面上输入的预设值
{
	int i;

	for(i=0; i<12; i++)
	{
		if(set_protect_data[i] != actual_protect_data[i])
			return -1;
	}

	return 0;
}

int resolve_presetdata(inverter_info *inverter, char * protect_data_result)	//?????????????
{
	float temp;

	/*if(1 == ecu.type)
		temp = (presetdata[0]*256 + presetdata[1])/2.90345;
	else
		temp = (presetdata[0]*256 + presetdata[1])/1.48975;
	if((temp-(int)temp)>0.5)
		inverter->protect_vl1 = (int)temp +1;
	else
		inverter->protect_vl1 = (int)temp;

	if(1 == ecu.type)
		temp = (presetdata[2]*256 + presetdata[3])/2.90345;
	else
		temp = (presetdata[2]*256 + presetdata[3])/1.48975;
	if((temp-(int)temp)>0.5)
		inverter->protect_vu1 = (int)temp +1;
	else
		inverter->protect_vu1 = (int)temp;*/

	/*
	if(1 == ecu.type)
		temp = (protect_data_result[4]*256 + protect_data_result[5])/2.90345;
	else
		temp = (protect_data_result[4]*256 + protect_data_result[5])/1.48975;
	if((temp-(int)temp)>0.5)
		inverter->protect_voltage_min = (int)temp +1;
	else
		inverter->protect_voltage_min = (int)temp;

	if(1 == ecu.type)
		temp = (protect_data_result[6]*256 + protect_data_result[7])/2.90345;
	else
		temp = (protect_data_result[6]*256 + protect_data_result[7])/1.48975;
	if((temp-(int)temp)>0.5)
		inverter->protect_voltage_max = (int)temp +1;
	else
		inverter->protect_voltage_max = (int)temp;

	if(1 == ecu.type)
		inverter->protect_frequency_min = 600-protect_data_result[8];
	else
		inverter->protect_frequency_min = 500-protect_data_result[8];
	if(1 == ecu.type)
		inverter->protect_frequency_max = 600+protect_data_result[9];
	else
		inverter->protect_frequency_max = 500+protect_data_result[9];
	inverter->recovery_time = protect_data_result[10]*256 + protect_data_result[11];
*/
	if((1==inverter->model)||(2==inverter->model)||(3==inverter->model)||(4==inverter->model))	//??????
		temp = (protect_data_result[3]*65536 + protect_data_result[4]*256 + protect_data_result[5])/26204.64;
	if((5==inverter->model)||(6==inverter->model))
		temp = (protect_data_result[3]*65536 + protect_data_result[4]*256 + protect_data_result[5])/11614.45;

	if((temp-(int)temp)>0.5)
		inverter->protect_voltage_min = (int)temp +1;
	else
		inverter->protect_voltage_min = (int)temp;

	if((1==inverter->model)||(2==inverter->model)||(3==inverter->model)||(4==inverter->model))	//??????
		temp = (protect_data_result[6]*65536 + protect_data_result[7]*256 + protect_data_result[8])/26204.64;
	if((5==inverter->model)||(6==inverter->model))
		temp = (protect_data_result[6]*65536 + protect_data_result[7]*256 + protect_data_result[8])/11614.45;
	if((temp-(int)temp)>0.5)
		inverter->protect_voltage_max = (int)temp +1;
	else
		inverter->protect_voltage_max = (int)temp;

	if((1==inverter->model)||(2==inverter->model)||(3==inverter->model)||(4==inverter->model))
		inverter->protect_frequency_min = 223750/(protect_data_result[9]*256 + protect_data_result[10]);
	if((5==inverter->model)||(6==inverter->model))
		inverter->protect_frequency_min = 256000/(protect_data_result[9]*256 + protect_data_result[10]);

	if((1==inverter->model)||(2==inverter->model)||(3==inverter->model)||(4==inverter->model))
		inverter->protect_frequency_max = 223750/(protect_data_result[11]*256 + protect_data_result[12]);
	if((5==inverter->model)||(6==inverter->model))
		inverter->protect_frequency_max = 256000/(protect_data_result[11]*256 + protect_data_result[12]);

	inverter->recovery_time = protect_data_result[13]*256 + protect_data_result[14];

	return 1;
}

int process_turn_on_off(inverter_info *firstinverter)
{
	int i, j;
	FILE *fp;
	char command[256] = {'\0'};
	inverter_info *curinverter = firstinverter;

	fp = fopen("/tmp/connect.con", "r");
	if(!fp)
		return -1;
	while(1){
		curinverter = firstinverter;
		memset(command, '\0', 256);
		fgets(command, 256, fp);
		if(!strlen(command))
			break;
		if('\n' == command[strlen(command)-1])
			command[strlen(command)-1] = '\0';
		if(!strncmp(command, "connect all", 11)){
			zb_turnon_inverter_broadcast();
			printmsg(ECU_DBG_MAIN,"turn on all");
			break;
		}
		if(!strncmp(command, "disconnect all", 14)){
			zb_shutdown_broadcast();
			printmsg(ECU_DBG_MAIN,"turn off all");
			break;
		}

		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++){
			if(!strncmp(command, curinverter->id, 12))
			{
				j = 0;
				if('c' == command[12])
				{
					while(j<3)
					{
						if(1 == zb_boot_single(curinverter))
						{
							print2msg(ECU_DBG_MAIN,"turn on", curinverter->id);
							break;
						}
						j++;
					}
				}
				if('d' == command[12])
				{
					while(j<3)
					{
						if(1 == zb_shutdown_single(curinverter))
						{
							print2msg(ECU_DBG_MAIN,"turn off", curinverter->id);
							break;
						}
						j++;
					}
				}
			}
			curinverter++;
		}
	}

	fclose(fp);
	fp = fopen("/tmp/connect.con", "w");
	fclose(fp);

	return 0;

}

int process_quick_boot(inverter_info *firstinverter)
{
	int i;
	FILE *fp;
	char flag_quickboot = '0';				//快速启动标志
	inverter_info *curinverter = firstinverter;

	fp = fopen("/tmp/qckboot.con", "r");

	if(fp)
	{
		flag_quickboot = fgetc(fp);
		fclose(fp);
		if('1' == flag_quickboot)
		{
			curinverter = firstinverter;
			for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++)
			{
				zb_boot_waitingtime_single(curinverter);
				print2msg(ECU_DBG_MAIN,"quick boot",curinverter->id);
				curinverter++;
			}
		}
		fp = fopen("/tmp/qckboot.con", "w");
		if(fp)
		{
			fputs("0",fp);
			fclose(fp);
		}
	}
	return 0;

}

int process_ipp(inverter_info *firstinverter)
{
	int i, j;
	FILE *fp;
	char command[256] = {'\0'};
	inverter_info *curinverter = firstinverter;

	fp = fopen("/tmp/ipp.con", "r");

	if(fp)
	{
		while(1)
		{
			curinverter = firstinverter;
			memset(command, '\0', 256);
			fgets(command, 256, fp);
			if(!strlen(command))
				break;
			if('\n' == command[strlen(command)-1])
				command[strlen(command)-1] = '\0';
			if(!strncmp(command, "set ipp all", 11)){
				zb_ipp_broadcast();
				printmsg(ECU_DBG_MAIN,"set ipp all");
				break;
			}

			for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++){
				if(!strcmp(command, curinverter->id))
				{
					j=0;
					while(j<3)
					{
						if(1 == zb_ipp_single(curinverter))
						{
							print2msg(ECU_DBG_MAIN,"set ipp single", curinverter->id);
							break;
						}
						j++;
					}
					break;
				}
				curinverter++;
			}
		}

		fclose(fp);
	}
	fp = fopen("/tmp/ipp.con", "w");
	fclose(fp);

	return 0;


}




int process_all(inverter_info *firstinverter)
{
	processpower(firstinverter);			//设置功率预设值,ZK,3.10有改动	     OK
//	process_gfdi(firstinverter);			//清GFDI标志
//	process_protect_data(firstinverter);	//设置预设值
	process_turn_on_off(firstinverter);		//开关机 	用于DRM功能
	process_quick_boot(firstinverter);		//快速启动
	process_ipp(firstinverter);				//IPP设定
	process_ird_all(firstinverter);		//0K
	process_ird(firstinverter);
	turn_on_off(firstinverter);								//开关机,ZK,3.10所加	   OK
	clear_gfdi(firstinverter);								//清GFDI标志,ZK,3.10所加 OK
	set_protection_parameters(firstinverter);				//设置预设值广播,ZK,3.10所加
	set_protection_parameters_inverter_one(firstinverter);  //设置预设值单点,ZK,3.10所加

	
	//save_A145_inverter_to_all();
	return 0;
}

int getalldata(inverter_info *firstinverter,int time_linux)		//获取每个逆变器的数据
{
	int i, j,dataflag_clear = 0;
	inverter_info *curinverter = firstinverter;
	int count=0, syspower=0;
	float curenergy=0;
	int out_flag = 0;	//跳出查询标志
#if 0
	char buff[50] = {'\0'};
	int fd;
#endif 

	for(i=0;i<3;i++)
	{
		if(-1==zb_test_communication())
		{
			printf("11111\n");
			zigbee_reset();
			
		}	
		else
			break;
	}
	//calibration_time_broadcast(firstinverter, time_linux); 	//YC1000补报：广播校准时间
	for(j=0; j<5; j++)
	{
		out_flag = 0;
		curinverter = firstinverter;
		
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++)			//每个逆变器最多要5次数据
		{
			if(dataflag_clear == 0)
			{
				curinverter->inverterstatus.dataflag = 0;		//没有接收到数据就置为0
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
			
			if((0 == curinverter->inverterstatus.dataflag) && (0 != curinverter->shortaddr))
			{
				process_all(firstinverter);
				if(1 != curinverter->inverterstatus.bindflag)
				{
					if(1 == zb_turnoff_limited_rptid(curinverter->shortaddr,curinverter))
					{
						curinverter->inverterstatus.bindflag = 1;			//绑定逆变器标志位1
						updateID();	
					}else
						out_flag = 1;
					
				}
				if((0 == curinverter->model) )//&& (1 == curinverter->inverterstatus.bindflag))
				{
					if(1 == zb_query_inverter_info(curinverter))
						updateID();
					else
						out_flag = 1;
					
				}
				
				if((0 != curinverter->model) )//&& (1 == curinverter->inverterstatus.bindflag))
				{
					//print2msg(ECU_DBG_MAIN,"querydata",curinverter->id);
					if(-1 == zb_query_data(curinverter))
						out_flag = 1;
					rt_hw_us_delay(200000);
				}
			}
			curinverter++;
		}
		dataflag_clear = 1;
		if(out_flag == 0) break;
	}
	ecu.polling_total_times++;				//ECU总轮训加1 ,ZK
#if 0	
	fd = open("/TMP/DISCON.TXT", O_WRONLY | O_CREAT | O_TRUNC, 0);
	if (fd >= 0) {
		curinverter = firstinverter;
		for(i=0; i<MAXINVERTERCOUNT; i++, curinverter++)						//统计当前一天逆变器与ECU没通讯上的总次数, ZK
		{
			if((0 == curinverter->inverterstatus.dataflag)&&(12 == strlen(curinverter->id)))
			{
				curinverter->disconnect_times++;
				if(curinverter->disconnect_times > 254)
				{
					curinverter->disconnect_times= 254;
				}
				sprintf(buff, "%s-%d-%d\n", curinverter->id,curinverter->disconnect_times,ecu.polling_total_times);
				write(fd, buff, strlen(buff));
			}
			else if((1 == curinverter->inverterstatus.dataflag)&&(12 == strlen(curinverter->id)))
			{
				sprintf(buff, "%s-%d-%d\n", curinverter->id,curinverter->disconnect_times,ecu.polling_total_times);
				write(fd, buff, strlen(buff));
			}
		}
		close(fd);
	}
#endif 
	curinverter = firstinverter;
	for(i=0; i<MAXINVERTERCOUNT; i++, curinverter++)		//统计连续没有获取到数据的逆变器 ZK,一旦接收到数据，此变量清零
	{
		if((0 == curinverter->inverterstatus.dataflag)&&(12 == strlen(curinverter->id)))
		{
			curinverter->no_getdata_num++;
			if(curinverter->no_getdata_num>254)
			{
				curinverter->no_getdata_num = 254;
			}
		}
	}

	curinverter = firstinverter;
	for(i=0; i<MAXINVERTERCOUNT; i++, curinverter++){							//统计当前多少个逆变器
		if((1 == curinverter->inverterstatus.dataflag)&&(12 == strlen(curinverter->id)))
			count++;
	}
	ecu.count = count;

	curinverter = firstinverter;
	for(syspower=0, i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++){		//计算当前一轮系统功率
		if(1 == curinverter->inverterstatus.dataflag){
			syspower += curinverter->op;
			syspower += curinverter->opb;
			syspower += curinverter->opc;
			syspower += curinverter->opd;
		}
	}
	ecu.system_power = syspower;

	curinverter = firstinverter;
	for(curenergy=0, i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++){		//计算当前一轮发电量
		if(1 == curinverter->inverterstatus.dataflag){
			if((curinverter->model==5) || (curinverter->model==6) || (curinverter->model==7))
				{
					curenergy += curinverter->curgeneration;
					curenergy += curinverter->curgenerationb;
					curenergy += curinverter->curgenerationc;
					curenergy += curinverter->curgenerationd;
				}else
				{
					curenergy += curinverter->output_energy;
					curenergy += curinverter->output_energyb;
					curenergy += curinverter->output_energyc;			
				}

		}
	}
	ecu.current_energy = curenergy;
	update_tmpdb(firstinverter);
#if 0
	fd = open("/TMP/IDNOBIND.TXT", O_WRONLY | O_CREAT | O_TRUNC, 0); 	//为了统计显示有短地址但是没有绑定的逆变器ID
	if (fd >= 0) 
	{						

		curinverter = firstinverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++)
		{
			if((1!=curinverter->inverterstatus.bindflag)&&(0!=curinverter->shortaddr))
			{
				memset(buff,0,50);
				sprintf(buff,"%s\n",curinverter->id);
				write(fd, buff, strlen(buff));
			}
		}
		close(fd);
	}

	fd = open("/TMP/SIGNALST.TXT", O_WRONLY | O_CREAT | O_TRUNC, 0);
	if (fd >= 0) 
	{			
		curinverter = firstinverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++)	 //统计每天逆变器的信号强度, ZK
		{
			memset(buff,0,50);
			sprintf(buff, "%s%X\n", curinverter->id,curinverter->signalstrength);
			write(fd, buff, strlen(buff));

		}
		close(fd);
	}

	fd = open("/TMP/RADUIS.TXT", O_WRONLY | O_CREAT | O_TRUNC, 0);
	if (fd >= 0) 
	{			
		curinverter = firstinverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++)	 //统计每台逆变器的信号强度, ZK
		{
			memset(buff,0,50);
			sprintf(buff, "%s%X\n", curinverter->id,curinverter->raduis);
			write(fd, buff, strlen(buff));
		}
		close(fd);
	}


	write_gfdi_status(firstinverter);
	write_turn_on_off_status(firstinverter);
#endif
	save_turn_on_off_changed_result(firstinverter);
	save_gfdi_changed_result(firstinverter);

	return ecu.count;
}

int get_inverter_shortaddress(inverter_info *firstinverter)		//获取没有数据的逆变器的短地址
{
	int i;
	inverter_info *curinverter = firstinverter;
	unsigned short current_panid;				//Zigbee即时的PANID
	int flag = 0;


	curinverter = firstinverter;
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==rt_strlen(curinverter->id)); i++)
	{
		if((0==curinverter->shortaddr)||(curinverter->no_getdata_num>5))
		{
			flag=1;
			break;
		}
		curinverter++;
	}

	if(1==flag)
	{
		if(1==zb_restore_ecu_panid_0xffff(0x10))		//把ECU的PANID设置为默认的0xffff
			current_panid = 0xffff;
		printdecmsg(ECU_DBG_MAIN,"PANID",current_panid);
		rt_hw_s_delay(5);


		curinverter = firstinverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==rt_strlen(curinverter->id)); i++)
		{
			if((0==curinverter->shortaddr)||(curinverter->no_getdata_num>5))
			{
				zb_change_inverter_panid_single(curinverter);		//把没有数据的逆变器PANID修改成ECU的PANID
				rt_hw_us_delay(500000);
			}
			curinverter++;
		}

		if(1==zb_change_ecu_panid())						//把ECU的panid改成此台ECU原本panid
			current_panid = ecu.panid;
		printdecmsg(ECU_DBG_MAIN,"PANID",current_panid);
		rt_hw_s_delay(5);

		curinverter = firstinverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==rt_strlen(curinverter->id)); i++)
		{
			printdecmsg(ECU_DBG_MAIN,"no_getdata_num",curinverter->no_getdata_num);
			if((0==curinverter->shortaddr)||(curinverter->no_getdata_num>5))
			{
				zb_get_inverter_shortaddress_single(curinverter);
			}
			curinverter++;
		}
	}
	return 1;
}

int bind_nodata_inverter(inverter_info *firstinverter)		//绑定没有数据的逆变器,并且获取短地址
{
	int i;
	inverter_info *curinverter = firstinverter;

	curinverter = firstinverter;
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==rt_strlen(curinverter->id)); i++)
	{
		if(curinverter->no_getdata_num>0)
		{
//			zb_turnoff_limited_rptid(curinverter->shortaddr,curinverter);
			zb_get_inverter_shortaddress_single(curinverter);
		}
		curinverter++;
	}
	return 0;
}

//单点改变逆变器的PANID为ECU的MAC地址后四位,信道为指定信道(注:需要将ECU的PANID改为0xFFFF(万能发送))
int zb_change_inverter_channel_one(char *inverter_id, int channel)
{
	char sendbuff[256] = {'\0'};
	int i;
	int check=0;
	rt_hw_ms_delay(500); 

	clear_zbmodem();
	sendbuff[0]  = 0xAA;
	sendbuff[1]  = 0xAA;
	sendbuff[2]  = 0xAA;
	sendbuff[3]  = 0xAA;
	sendbuff[4]  = 0x0F;
	sendbuff[5]  = 0x00;
	sendbuff[6]  = 0x00;
	sendbuff[7]  = ecu.panid>>8;
	sendbuff[8]  = ecu.panid;
	sendbuff[9]  = channel;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0xA0;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[14] = 0x06;
	sendbuff[15]=((inverter_id[0]-0x30)*16+(inverter_id[1]-0x30));
	sendbuff[16]=((inverter_id[2]-0x30)*16+(inverter_id[3]-0x30));
	sendbuff[17]=((inverter_id[4]-0x30)*16+(inverter_id[5]-0x30));
	sendbuff[18]=((inverter_id[6]-0x30)*16+(inverter_id[7]-0x30));
	sendbuff[19]=((inverter_id[8]-0x30)*16+(inverter_id[9]-0x30));
	sendbuff[20]=((inverter_id[10]-0x30)*16+(inverter_id[11]-0x30));
	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL,0, sendbuff, 21);
	printhexmsg(ECU_DBG_MAIN,"Change Inverter Channel (one)", sendbuff, 21);

	rt_hw_s_delay(1); //此处延时必须大于1S
	return 0;
}

//改变传入逆变器ID的信道
int zb_change_channel(int num, char **ids)
{
	int i, j;
	char ecu_channel[16];

	clear_zbmodem();
	ecu_channel[0] = 0x0B;
	ecu_channel[1] = 0x0C;
	ecu_channel[2] = 0x0D;
	ecu_channel[3] = 0x0E;
	ecu_channel[4] = 0x0F;
	ecu_channel[5] = 0x10;
	ecu_channel[6] = 0x11;
	ecu_channel[7] = 0x12;
	ecu_channel[8] = 0x13;
	ecu_channel[9] = 0x14;
	ecu_channel[10] = 0x15;
	ecu_channel[11] = 0x16;
	ecu_channel[12] = 0x17;
	ecu_channel[13] = 0x18;
	ecu_channel[14] = 0x19;
	ecu_channel[15] = 0x1A;

	//改变每台逆变器的信道
	for (i=0; i<16; i++) {
		zb_restore_ecu_panid_0xffff(ecu_channel[i]); //改变ECU信道
		for (j=1; j<=num; j++) {
			if (RT_NULL != ids[j] && rt_strlen(ids[j])) {
				print2msg(ECU_DBG_MAIN,"inverter", ids[j]);
				zb_change_inverter_channel_one(ids[j], ecu.channel);
			}
		}
	}

	//设置ECU信道为配置文件中的信道
	zb_change_ecu_panid();

	return 0;
}


//还原逆变器信道
int zb_reset_channel(int num, char **ids)
{
	int i, j;
	char ecu_channel[16];

	clear_zbmodem();
	ecu_channel[0] = 0x0B;
	ecu_channel[1] = 0x0C;
	ecu_channel[2] = 0x0D;
	ecu_channel[3] = 0x0E;
	ecu_channel[4] = 0x0F;
	ecu_channel[5] = 0x10;
	ecu_channel[6] = 0x11;
	ecu_channel[7] = 0x12;
	ecu_channel[8] = 0x13;
	ecu_channel[9] = 0x14;
	ecu_channel[10] = 0x15;
	ecu_channel[11] = 0x16;
	ecu_channel[12] = 0x17;
	ecu_channel[13] = 0x18;
	ecu_channel[14] = 0x19;
	ecu_channel[15] = 0x1A;

	//还原每台逆变器的信道
	for (i=0; i<16; i++) {
		zb_restore_ecu_panid_0xffff(ecu_channel[i]); //改变ECU信道
		for (j=1; j<=num; j++) {
			if (RT_NULL != ids[j] && rt_strlen(ids[j])) {
				print2msg(ECU_DBG_MAIN,"inverter", ids[j]);
				zb_change_inverter_channel_one(ids[j], 0x10);
			}
		}
	}

	//设置ECU的信道为默认信道
	ecu.channel = 0x10;
	zb_change_ecu_panid();

	return 0;
}

//关闭逆变器ID上报 + 绑定逆变器
int zb_off_report_id_and_bind(int short_addr)
{
	int times = 3;
	char sendbuff[16] = {'\0'};
	char recvbuff[256] = {'\0'};
	int i;
	int check=0;

	do {
		//发送关闭逆变器ID上报+绑定操作
		clear_zbmodem();
		sendbuff[0]  = 0xAA;
		sendbuff[1]  = 0xAA;
		sendbuff[2]  = 0xAA;
		sendbuff[3]  = 0xAA;
		sendbuff[4]  = 0x08;
		sendbuff[5]  = short_addr>>8;
		sendbuff[6]  = short_addr;
		sendbuff[7]  = 0x00; //PANID(逆变器不解析)
		sendbuff[8]  = 0x00; //PANID(逆变器不解析)
		sendbuff[9]  = 0x00; //信道(逆变器不解析)
		sendbuff[10] = 0x00; //功率(逆变器不解析)
		sendbuff[11] = 0xA0;
		for(i=4;i<12;i++)
			check=check+sendbuff[i];
		sendbuff[12] = check/256;
		sendbuff[13] = check%256;
		sendbuff[14] = 0x00;
		ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL,0, sendbuff, 15);
		printhexmsg(ECU_DBG_MAIN,"Bind ZigBee", sendbuff, 15);

		//接收逆变器应答(短地址,ZigBee版本号,信号强度)
		if ((11 == zb_get_reply_from_module(recvbuff))
				&& (0xA5 == recvbuff[2])
				&& (0xA5 == recvbuff[3])) {
			//update_turned_off_rpt_flag(short_addr, (int)recvbuff[9]);
			//update_bind_zigbee_flag(short_addr);
			printmsg(ECU_DBG_MAIN,"Bind Successfully");
			return 1;
		}
	}while(--times);

	return 0;
}

int zigbeeRecvMsg(char *data, int timeout_sec)
{
	int count;
	if (selectZigbee(timeout_sec) <= 0) {
		printmsg(ECU_DBG_MAIN,"Get reply time out");
		return -1;
	} else {
		
		count = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data, 255);
		printhexmsg(ECU_DBG_MAIN,"Reply", data, count);
		return count;
	}
}

#if 0
#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(zigbee_reset, reset zigbee module.)
void Zbtestcomm()		//测试通信
{
	zb_test_communication();
}
FINSH_FUNCTION_EXPORT(Zbtestcomm,zigbee test communication.)

void Zbecupanidffff(int channel)		//设置ECU panid为ffff 信道为channel
{
	zb_restore_ecu_panid_0xffff(channel);
}
FINSH_FUNCTION_EXPORT(Zbecupanidffff,zb_restore_ecu_panid_0xffff channel[int].)
	
void Zbecupanid()		//设置ECU panid 以及 信道   为ECU全局变量的值	
{
	zb_change_ecu_panid();
}
FINSH_FUNCTION_EXPORT(Zbecupanid,zb_change_ecu_panid.)

void Zbecupanid8888(int channel)	//设置ECU panid为0x8888  信道我ECU全局变量的值
{
	zb_restore_ecu_panid_0x8888();
}
FINSH_FUNCTION_EXPORT(Zbecupanid8888,zb_restore_ecu_panid_0x8888.)

void Zbinvchone(char *inverter_id, int channel)	//更改逆变器panid和信道 其中panid为全局变量ecu的信道
{
	zb_change_inverter_channel_one(inverter_id,channel);
}
FINSH_FUNCTION_EXPORT(Zbinvchone,zb_change_inverter_channel_one inverter_id[char *12] channel[int].)

void Zbgetshadd(char *inverter_id)	//获取逆变器短地址
{
	inverter_info inverter;
	rt_memset(&inverter,0x00,sizeof(inverter));
	rt_memcpy(inverter.id,inverter_id,12);
	zb_get_inverter_shortaddress_single(&inverter);
}
FINSH_FUNCTION_EXPORT(Zbgetshadd,zb_get_inverter_shortaddress_single inverter_id[char *12].)

void Zbgetdata(char *inverter_id)	//获取数据
{
	inverter_info inverter;
	rt_memset(&inverter,0x00,sizeof(inverter));
	rt_memcpy(inverter.id,inverter_id,12);
	inverter.model = 7;
	zb_get_inverter_shortaddress_single(&inverter);
	zb_query_data(&inverter);
}
FINSH_FUNCTION_EXPORT(Zbgetdata,zb_query_data inverter_id[char *12].)
#endif
#endif

