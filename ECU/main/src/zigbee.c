#ifdef RT_USING_SERIAL
#include "serial.h"
#endif /* RT_USING_SERIAL */
#include <rtdef.h>
#include <rtthread.h>
#include "debug.h"
#include "zigbee.h"
#include <rthw.h>

extern struct rt_device serial4;		//串口4为Zigbee收发串口

/*extern*/ ecu_info ecu;

#define RD_DELAY 	(RT_TICK_PER_SECOND)
#define WR_DELAY	(RT_TICK_PER_SECOND)
#define ZIGBEE_SERIAL (serial4)

void clear_zbmodem(void)		//清空串口缓冲区的数据
{
	//清空缓冲器代码
	//。。。。
	
	rt_thread_delay(RD_DELAY);
}

int openzigbee(void)
{
	int result = 0;
	result = ZIGBEE_SERIAL.open(&ZIGBEE_SERIAL,RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX |
                          RT_DEVICE_FLAG_INT_TX |   RT_DEVICE_FLAG_DMA_RX);
	if(result)
	{
		rt_kprintf("open serial 2 failed : %d\r\n",result);
	}else
	{
		rt_kprintf("open serial 2 success\r\n");
	}
	return result;
}

//复位zigbee模块  通过PC7的电平置高置低然后达到复位的效果




int zb_shortaddr_cmd(int shortaddr, char *buff, int length)		//zigbee 短地址报头
{
	unsigned char sendbuff[512] = {'\0'};
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
		printhexmsg("zb_shortaddr_cmd", (char *)sendbuff, 15);
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
	
	rt_thread_delay( RD_DELAY );
	temp_size = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data_all, 255);
	if(temp_size > 0)
	{
		size = temp_size -12;

		for(i=0;i<size;i++)
		{
			data[i]=data_all[i+12];
		}
		printhexmsg("Reply", data_all, temp_size);
		rt_sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data_all[6],data_all[7],data_all[8],data_all[9],data_all[10],data_all[11]);
		if((size>0)&&(0xFC==data_all[0])&&(0xFC==data_all[1])&&(data_all[2]==shortaddr/256)&&(data_all[3]==shortaddr%256)&&(data_all[5]==0xA5)&&(0==rt_strcmp(id,inverterid)))
		{
				return size;
		}
		else
		{
			return -1;
		}
	}else
	{
		rt_kprintf("serial2.read failed \r\n");
		return -1;
	}

}


int zb_get_reply(char *data,inverter_info *inverter)			//读取逆变器的返回帧
{
	int i;
	char data_all[256];
	char inverterid[13] = {'\0'};
	int temp_size,size;
	
	rt_thread_delay( RD_DELAY );
	temp_size = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data_all, 255);
	if(temp_size > 0)
	{
		size = temp_size -12;

		for(i=0;i<size;i++)
		{
			data[i]=data_all[i+12];
		}
		printhexmsg("Reply", data_all, temp_size);
		rt_sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data_all[6],data_all[7],data_all[8],data_all[9],data_all[10],data_all[11]);
		if((size>0)&&(0xFC==data_all[0])&&(0xFC==data_all[1])&&(data_all[2]==inverter->shortaddr/256)&&(data_all[3]==inverter->shortaddr%256)&&(data_all[5]==0xA5)&&(0==rt_strcmp(inverter->inverterid,inverterid)))
		{
			inverter->signalstrength=data_all[4];
			return size;
		}
		else
		{
			inverter->signalstrength=0;
			return -1;
		}
	}else
	{
		rt_kprintf("serial1.read failed \r\n");
		return -1;
	}
	
}

int zb_get_reply_update_start(char *data,inverter_info *inverter)			//读取逆变器远程更新的Update_start返回帧，ZK，返回响应时间定为10秒
{
	int i;
	char data_all[256];
	char inverterid[13] = {'\0'};
	int temp_size,size;

	rt_thread_delay( RD_DELAY );
	temp_size = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data_all, 255);
	if(temp_size>0)
	{
		size = temp_size -12;

		for(i=0;i<size;i++)
		{
			data[i]=data_all[i+12];
		}
		printhexmsg("Reply", data_all, temp_size);
		rt_sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data_all[6],data_all[7],data_all[8],data_all[9],data_all[10],data_all[11]);
		if((size>0)&&(0xFC==data_all[0])&&(0xFC==data_all[1])&&(data_all[2]==inverter->shortaddr/256)&&(data_all[3]==inverter->shortaddr%256)&&(data_all[5]==0xA5)&&(0==rt_strcmp(inverter->inverterid,inverterid)))
		{
			return size;
		}
		else
		{
			return -1;
		}
	}else
	{
		rt_kprintf("serial1.read failed \r\n");
		return -1;
	}

}

int zb_get_reply_restore(char *data,inverter_info *inverter)			//读取逆变器远程更新失败，还原指令后的返回帧，ZK，因为还原时间比较长，所以单独写一个函数
{
	int i;
	char data_all[256];
	char inverterid[13] = {'\0'};
	int temp_size,size;

	rt_thread_delay( RD_DELAY );
	temp_size = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data_all, 255);
	if(temp_size > 0)
	{
		size = temp_size -12;

		for(i=0;i<size;i++)
		{
			data[i]=data_all[i+12];
		}
		printhexmsg("Reply", data_all, temp_size);
		rt_sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data_all[6],data_all[7],data_all[8],data_all[9],data_all[10],data_all[11]);

		if((size>0)&&(0xFC==data_all[0])&&(0xFC==data_all[1])&&(data_all[2]==inverter->shortaddr/256)&&(data_all[3]==inverter->shortaddr%256)&&(data_all[5]==0xA5)&&(0==rt_strcmp(inverter->inverterid,inverterid)))
		{
			return size;
		}
		else
		{
			return -1;
		}	
	}else
	{
		rt_kprintf("serial1.read failed \r\n");
		return -1;
	}
}

int zb_get_reply_from_module(char *data)			//读取zigbee模块的返回帧
{
	int size;

	rt_thread_delay( RD_DELAY );
	size = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data, 255);
	
	if(size > 0)
	{
		printhexmsg("Reply", data, size);
		return size;
	}else
	{
		rt_kprintf("serial1.read failed \r\n");
		return -1;
	}
}

int zb_get_id(char *data)			//获取逆变器ID
{
	int size;

	rt_thread_delay( RD_DELAY );
	size = ZIGBEE_SERIAL.read(&ZIGBEE_SERIAL,0, data, 255);
	
	if(size > 0)
	{
		printhexmsg("Reply", data, size);
		return size;
	}else
	{
		rt_kprintf("serial1.read failed \r\n");
		return -1;
	}

}

int zb_turnoff_limited_rptid(int short_addr,inverter_info *inverter)			//关闭限定单个逆变器上报ID功能
{
	unsigned char sendbuff[512] = {'\0'};
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
		printmsg("Turn off limited report id");
		ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0,sendbuff, 15);
		printhexmsg("sendbuff",(char *)sendbuff,15);
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
	unsigned char sendbuff[512] = {'\0'};
	int i=0;
	int check=0;
	printmsg("Turn off report id");

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
	unsigned char sendbuff[512] = {'\0'};
	int i=0, ret;
	char data[256];
	char inverterid[13] = {'\0'};
	int check=0;
	printmsg("Get inverter shortaddresssingle");

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

	sendbuff[15]=((inverter->inverterid[0]-0x30)*16+(inverter->inverterid[1]-0x30));
	sendbuff[16]=((inverter->inverterid[2]-0x30)*16+(inverter->inverterid[3]-0x30));
	sendbuff[17]=((inverter->inverterid[4]-0x30)*16+(inverter->inverterid[5]-0x30));
	sendbuff[18]=((inverter->inverterid[6]-0x30)*16+(inverter->inverterid[7]-0x30));
	sendbuff[19]=((inverter->inverterid[8]-0x30)*16+(inverter->inverterid[9]-0x30));
	sendbuff[20]=((inverter->inverterid[10]-0x30)*16+(inverter->inverterid[11]-0x30));

//	strcpy(&sendbuff[15],inverter->inverterid);


	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0, sendbuff, 21);
	printhexmsg("sendbuff",(char *)sendbuff,21);
	ret = zb_get_reply_from_module(data);

	rt_sprintf(inverterid,"%02x%02x%02x%02x%02x%02x",data[4],data[5],data[6],data[7],data[8],data[9]);

	if((11 == ret)&&(0xFF == data[2])&&(0==rt_strcmp(inverter->inverterid,inverterid)))
	{
		inverter->shortaddr = data[0]*256 + data[1];
//		update_inverter_addr(inverter->inverterid,inverter->shortaddr);
		return 1;
	}
	else
		return -1;

}


int zb_turnon_rtpid(inverter_info *firstinverter)			//开启逆变器自动上报ID
{
	char sendbuff[512] = {'\0'};
	char data[256];
	int i, count=0;
	int short_addr;
	char inverterid[256] = {'\0'};
	int check=0;
	printmsg("Turn on report id");

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
		rt_thread_delay(RT_TICK_PER_SECOND*5);//上报时间为10分钟
		rt_memset(data, '\0', sizeof(data));
		if((11 == zb_get_id(data)) && (0xFF == data[2]) && (0xFF == data[3]))
		{
			short_addr = data[0] * 256 + data[1];
			for(i=0; i<6; i++){
				inverterid[2*i] = (data[i+4]>>4) + 0x30;
				inverterid[2*i+1] = (data[i+4]&0x0f) + 0x30;
			}
			print2msg("inverterid",inverterid);
//			save_inverter_id(inverterid,short_addr);  //保存逆变器以及短地址
			rt_thread_delay(RT_TICK_PER_SECOND*5);
			zb_turnoff_rptid(short_addr);
			rt_memset(inverterid, '\0', sizeof(inverterid));
		}else
		{
			rt_thread_delay(RT_TICK_PER_SECOND*5);
		}
	}
	
//	count=get_id_from_db(firstinverter);
	return count;
}

int zb_change_inverter_panid_broadcast(void)	//广播改变逆变器的PANID，ZK
{
	char sendbuff[512] = {'\0'};
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
	printhexmsg("sendbuff",sendbuff,15);

	return 1;
}

int zb_change_inverter_panid_single(inverter_info *inverter)	//单点改变逆变器的PANID和信道，ZK
{
	char sendbuff[512] = {'\0'};
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
	sendbuff[15]=((inverter->inverterid[0]-0x30)*16+(inverter->inverterid[1]-0x30));
	sendbuff[16]=((inverter->inverterid[2]-0x30)*16+(inverter->inverterid[3]-0x30));
	sendbuff[17]=((inverter->inverterid[4]-0x30)*16+(inverter->inverterid[5]-0x30));
	sendbuff[18]=((inverter->inverterid[6]-0x30)*16+(inverter->inverterid[7]-0x30));
	sendbuff[19]=((inverter->inverterid[8]-0x30)*16+(inverter->inverterid[9]-0x30));
	sendbuff[20]=((inverter->inverterid[10]-0x30)*16+(inverter->inverterid[11]-0x30));
	
	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0, sendbuff, 21);
	printhexmsg("sendbuff",sendbuff,21);

	rt_thread_delay(RT_TICK_PER_SECOND);
	return 1;

}

int zb_restore_inverter_panid_channel_single_0x8888_0x10(inverter_info *inverter)	//单点还原逆变器的PANID到0X8888和信道0X10，ZK
{
	char sendbuff[512] = {'\0'};
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
	sendbuff[15]=((inverter->inverterid[0]-0x30)*16+(inverter->inverterid[1]-0x30));
	sendbuff[16]=((inverter->inverterid[2]-0x30)*16+(inverter->inverterid[3]-0x30));
	sendbuff[17]=((inverter->inverterid[4]-0x30)*16+(inverter->inverterid[5]-0x30));
	sendbuff[18]=((inverter->inverterid[6]-0x30)*16+(inverter->inverterid[7]-0x30));
	sendbuff[19]=((inverter->inverterid[8]-0x30)*16+(inverter->inverterid[9]-0x30));
	sendbuff[20]=((inverter->inverterid[10]-0x30)*16+(inverter->inverterid[11]-0x30));

	ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL, 0, sendbuff, 21);
	printhexmsg("sendbuff",sendbuff,21);

	rt_thread_delay(RT_TICK_PER_SECOND);
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
	printhexmsg("Set ECU PANID and Channel", (char *)sendbuff, 15);

	if ((3 == zb_get_reply_from_module(recvbuff))
			&& (0xAB == recvbuff[0])
			&& (0xCD == recvbuff[1])
			&& (0xEF == recvbuff[2])) {
		rt_thread_delay(RT_TICK_PER_SECOND*2); //延时2S，因为设置完ECU信道和PANID后会发6个FF
		return 1;
	}

	return -1;
}

int zb_restore_ecu_panid_0x8888(void)			//恢复ECU的PANID为0x8888,ZK
{
	unsigned char sendbuff[512] = {'\0'};
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
	printhexmsg("sendbuff",(char *)sendbuff,15);
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
	printhexmsg("Change ECU channel (PANID:0xFFFF)", (char *)sendbuff, 15);

	//接收反馈
	if ((3 == zb_get_reply_from_module(recvbuff))
			&& (0xAB == recvbuff[0])
			&& (0xCD == recvbuff[1])
			&& (0xEF == recvbuff[2])) {
		rt_thread_delay(RT_TICK_PER_SECOND*2);
		return 1;
	}

	return -1;
}

int zb_send_cmd(inverter_info *inverter, char *buff, int length)		//zigbee包头
{
	unsigned char sendbuff[512] = {'\0'};
	int i;
	int check=0;
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

	printdecmsg("shortaddr",inverter->shortaddr);
	for(i=0; i<length; i++)
	{
		sendbuff[15+i] = buff[i];
	}
	
	rt_thread_delay(RT_TICK_PER_SECOND/5);
	if(0!=inverter->shortaddr)
	{
		ZIGBEE_SERIAL.write(&ZIGBEE_SERIAL,0, sendbuff, length+15);
		//printhexmsg("Send", sendbuff, length+15);
		return 1;
	}
	else
		return -1;
}

int zb_broadcast_cmd(char *buff, int length)		//zigbee广播包头
{
	unsigned char sendbuff[512] = {'\0'};
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
	int  ret;
	char sendbuff[256];
	char data[256];
//	unsigned short check = 0x00;
	
	print2msg("Query inverter information",inverter->inverterid);
	clear_zbmodem();			//发送指令前先清空缓冲区
	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x02;
	sendbuff[3] = 0x01;
	sendbuff[4] = 0x06;
	sendbuff[5] = 0xDC;
	sendbuff[6] = 0x01;
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;
	sendbuff[9] = 0x00;
	sendbuff[10] = 0x00;
//	for(i=2; i<9; i++)
//		check = check + sendbuff[i];
//	sendbuff[9] = check >> 8;		//CHK
//	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0x25;		//CRC
	sendbuff[12] = 0x97;		//CRC
	sendbuff[13] = 0xFE;
	sendbuff[14] = 0xFE;
	
	zb_send_cmd(inverter, sendbuff, 15);
	ret = zb_get_reply(data,inverter);

	if((0x01 == data[2])&&(0x02 == data[3])&&(22 == ret)&&(0xFB == data[0])&&(0xFB == data[1])&&(0xDC == data[7])&&(0xFE == data[20])&&(0xFE == data[21]))
	{
//		inverter->model = data[9];
		inverter->version = data[8]*256+data[9];
		return 1;
	}
	else
	{
		return -1;
	}
}

int zb_query_data(inverter_info *inverter)		//请求逆变器实时数据
{
	int ret;
	char sendbuff[256];
	char data[256];
//	int signalstrength;
//	unsigned short check=0x00;
	

	print2msg("Query inverter data",inverter->inverterid);
	clear_zbmodem();			//发送指令前，先清空缓冲区
	sendbuff[0] = 0xFA;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x02;			//新加的源地址
	sendbuff[3] = 0x01;			//新加的目标地址
	sendbuff[4] = 0x06;
	sendbuff[5] = 0xBB;
	sendbuff[6] = 0x01;
	if(0==inverter->no_last_flag)
		sendbuff[7]=0x00;
	else
		sendbuff[7]=0x11;
	sendbuff[8] = 0x00;
	sendbuff[9] = 0x00;
	sendbuff[10] = 0x00;
//	for(i=2; i<9; i++)
//		check = check + sendbuff[i];
//	sendbuff[9] = check >> 8;		//CHK
//	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0x42;		//CRC
	sendbuff[12] = 0x30;		//CRC
	sendbuff[13] = 0xFE;		//TAIL
	sendbuff[14] = 0xFE;		//TAIL

	
	zb_send_cmd(inverter, sendbuff, 15);
	ret = zb_get_reply(data,inverter);

	//ret=34为逆变器,一般不会进入
	if((34 == ret)&&(0xFB == data[0])&&(0xFB == data[1])&&(0xFE == data[32])&&(0xFE == data[33]))
	{
		inverter->no_last_flag =1;
		inverter->no_getdata_num = 0;	//一旦接收到数据就清0,ZK
		inverter->dataflag = 1;	//接收到数据就置1

		if(1==inverter->model)
		{
//			resolvedata_500(&data[6], inverter);
		}
		else if(2==inverter->model)
		{
//			resolvedata_250(&data[6], inverter);
		}
		else
		{
//			resolvedata_250(&data[6], inverter);
		}

		return 1;
	}
	else if((0x01 == data[2])&&(0x02 == data[3])&&(74 == ret)&&(0xFB == data[0])&&(0xFB == data[1])&&(0xFE == data[72])&&(0xFE == data[73]))
	{
		inverter->no_getdata_num = 0;	//一旦接收到数据就清0,ZK
		inverter->dataflag = 1;	//接收到数据就置1
//		resolvedata_optimizer_YC500(&data[8], inverter);
//		save_optimizer_data(inverter,ecu.broadcast_time);
		inverter->no_last_flag=0;
		return 1;
	}
	else
	{
		inverter->no_last_flag=1;
		inverter->dataflag = 0;		//没有接收到数据就置为0
		return -1;
	}
}

int zb_test_communication(void)		//zigbee测试通信有没有断开
{
	unsigned char sendbuff[512] = {'\0'};
	int i=0, ret;
	char data[256];
	int check=0;

	printmsg("test zigbee communication");
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
	
	print2msg(inverter->inverterid,"Set protect parameters");
	printhexmsg("Set protect parameters", sendbuff, i);
	zb_send_cmd(inverter, sendbuff, i);
	ret = zb_get_reply(data,inverter);
	if((13 == ret) && (0xDE == data[3]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[11]) && (0xFE == data[12]))
		return 1;
	else
		return -1;
}

int zb_query_protect_parameter(inverter_info *inverter, char *protect_data_DA_reply)		//存储参数查询DD指令
{
	int i=0, ret;
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	/*
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
	*/
	
	clear_zbmodem();			//发送指令前,先清空缓冲区
	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x00;
	sendbuff[5] = 0XDD;			//CMD
	sendbuff[6] = 0x11;		//DATA
	sendbuff[7] = 0x04;
	sendbuff[8] = 0x01;
	for(i=2; i<9; i++)
		check = check + sendbuff[i];
	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//TAIL
	sendbuff[12] = 0xFE;		//TAIL

	zb_send_cmd(inverter, (char *)sendbuff, 13);
	print2msg(inverter->inverterid, "Query protect parameter");

	ret = zb_get_reply(protect_data_DA_reply,inverter);
	if((29 == ret) && (0xDA == protect_data_DA_reply[5]) && (0xFB == protect_data_DA_reply[0]) && (0xFB == protect_data_DA_reply[1]) && (0xFE == protect_data_DA_reply[27]) && (0xFE == protect_data_DA_reply[28]))
		return 1;
	else
		return -1;
}

int zb_afd_broadcast(void)		//AFD广播指令
{
	int i=0;
	char sendbuff[256];
//	char data[256];
	unsigned short check=0x00;

	
	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x07;
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x00;
	sendbuff[5] = 0xAF;
	sendbuff[6] = 0x00;
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x05;
	sendbuff[9] = 0x01;
	for(i=2; i<10; i++){
		check = check + sendbuff[i];
	}
	sendbuff[10] = check >> 8;		//CHK
	sendbuff[11] = check;		//CHK
	sendbuff[12] = 0xFE;
	sendbuff[13] = 0xFE;
	
	zb_broadcast_cmd(sendbuff, 14);

	return 1;
}

int zb_turnon_inverter_broadcast(void)		//开机指令广播,OK
{
//	int i=0, ret;
	char sendbuff[256];
//	char data[256];
//	unsigned short check=0x00;
	
	sendbuff[0] = 0xFA;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x02;
	sendbuff[3] = 0x01;
	sendbuff[4] = 0x06;
	sendbuff[5] = 0xC1;
	sendbuff[6] = 0x00;
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;
	sendbuff[9] = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;		//CHK
	sendbuff[12] = 0x00;		//CHK
	sendbuff[13] = 0xFE;
	sendbuff[14] = 0xFE;

	zb_broadcast_cmd(sendbuff, 15);
	return 1;
}

int zb_boot_single(inverter_info *inverter)		//开机指令单播,OK
{
	int  ret;
	char sendbuff[256];
	char data[256];
//	unsigned short check=0x00;

	sendbuff[0] = 0xFA;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x02;
	sendbuff[3] = 0x01;
	sendbuff[4] = 0x06;
	sendbuff[5] = 0xC3;
	sendbuff[6] = 0x00;
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;
	sendbuff[9] = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;		//CHK
	sendbuff[12] = 0x00;		//CHK
	sendbuff[13] = 0xFE;
	sendbuff[14] = 0xFE;


	zb_send_cmd(inverter, sendbuff, 15);
	ret = zb_get_reply(data,inverter);

	if((15 == ret) && (0xDE == data[5]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[13]) && (0xFE == data[14]))
		return 1;
	else
		return -1;
}

int zb_shutdown_broadcast(void)		//关机指令广播,OK
{
	char sendbuff[256];
//	char data[256];
//	unsigned short check=0x00;

	
	sendbuff[0] = 0xFA;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x02;
	sendbuff[3] = 0x01;
	sendbuff[4] = 0x06;
	sendbuff[5] = 0xC0;
	sendbuff[6] = 0x00;
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;
	sendbuff[9] = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;		//CHK
	sendbuff[12] = 0x00;		//CHK
	sendbuff[13] = 0xFE;
	sendbuff[14] = 0xFE;
	
	zb_broadcast_cmd(sendbuff, 15);
	return 1;
}

int zb_shutdown_single(inverter_info *inverter)		//关机指令单播,OK
{
	int  ret;
	char sendbuff[256];
	char data[256];
//	unsigned short check=0x00;
	
	sendbuff[0] = 0xFA;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x02;
	sendbuff[3] = 0x01;
	sendbuff[4] = 0x06;
	sendbuff[5] = 0xC2;
	sendbuff[6] = 0x00;
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;
	sendbuff[9] = 0x00;
	sendbuff[10] = 0x00;
	sendbuff[11] = 0x00;		//CHK
	sendbuff[12] = 0x00;		//CHK
	sendbuff[13] = 0xFE;
	sendbuff[14] = 0xFE;


	zb_send_cmd(inverter, sendbuff, 15);
	ret = zb_get_reply(data,inverter);

	if((15 == ret) && (0xDE == data[5]) && (0xFB == data[0]) && (0xFB == data[1]) && (0xFE == data[13]) && (0xFE == data[14]))
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
//	char data[256];

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

	zb_broadcast_cmd(sendbuff, 13);
	return 1;
}

int zb_clear_gfdi(inverter_info *inverter)		//清除GFDI,OK
{
	int i=0;
	char sendbuff[256];
//	char data[256];
	unsigned short check=0x00;

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x06;
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x00;
	sendbuff[5] = 0xA5;
	sendbuff[6] = 0x11;
	sendbuff[7] = 0x04;
	sendbuff[8] = 0x01;

	for(i=2; i<9; i++)
		check = check + sendbuff[i];

	sendbuff[9] = check >> 8;		//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;
	sendbuff[12] = 0xFE;
	
	zb_send_cmd(inverter, sendbuff, 13);

	return -1;
}

int zb_ipp_broadcast(void)		//IPP广播
{
	int i=0;
	char sendbuff[256];
//	char data[256];
	unsigned short check=0x00;

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x07;
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x00;
	sendbuff[5] = 0xC1;
	sendbuff[6] = 0x00;
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x05;
	sendbuff[9] = 0x01;
	for(i=2; i<10; i++)
		check = check + sendbuff[i];
	sendbuff[10] = check >> 8;		//CHK
	sendbuff[11] = check;		//CHK
	sendbuff[12] = 0xFE;
	sendbuff[13] = 0xFE;

	zb_broadcast_cmd(sendbuff, 14);
	return 1;
}

int zb_ipp_single(inverter_info *inverter)		//IPP单播,暂时不用,ZK
{
	int i=0;
	char sendbuff[256];
//	char data[256];
	unsigned short check=0x00;


	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x06;
	sendbuff[3] = 0x00;
	sendbuff[4] = 0x00;
	sendbuff[5] = 0xC2;
	sendbuff[6] = 0x00;
	sendbuff[7] = 0x04;
	sendbuff[8] = 0x01;
	for(i=2; i<9; i++)
		check = check + sendbuff[i];
	sendbuff[9] = check >> 8;		//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;
	sendbuff[12] = 0xFE;

	zb_send_cmd(inverter, sendbuff, 13);
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
