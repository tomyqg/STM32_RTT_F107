#ifdef RT_USING_SERIAL
#include "serial.h"
#endif /* RT_USING_SERIAL */
#include <rtdef.h>
#include <rtthread.h>
#include <rthw.h>
#include <stm32f10x.h>
#include "usr_wifi232.h"
#include "debug.h"

extern struct rt_device serial1;		//串口1为WIFI收发串口
#define RD_DELAY 	(RT_TICK_PER_SECOND/2) //读取数据延时
#define WR_DELAY	(RT_TICK_PER_SECOND) //写数据延时
#define WIFI_SERIAL (serial1)

extern int WiFiReadFlag;
static int WiFireadtimeoutflag = 0;
//定时器超时函数
static void readtimeout_Wifi(void* parameter)
{
	WiFireadtimeoutflag = 1;
}

int selectWiFi(int timeout)			//Wifi串口数据检测 返回0 表示串口没有数据  返回1表示串口有数据
{	
	rt_timer_t readtimer;
	readtimer = rt_timer_create("read", /* 定时器名字为 read */
					readtimeout_Wifi, /* 超时时回调的处理函数 */
					RT_NULL, /* 超时函数的入口参数 */
					timeout*RT_TICK_PER_SECOND, /* 定时时间长度,以OS Tick为单位*/
					 RT_TIMER_FLAG_ONE_SHOT); /* 单周期定时器 */
	if (readtimer != RT_NULL) rt_timer_start(readtimer);
	WiFireadtimeoutflag = 0;
	while(1)
	{
		if(WiFireadtimeoutflag)
		{
			rt_timer_delete(readtimer);
			return 0;
		}else 
		{
			if(WiFiReadFlag == 1)	//串口数据监测,如果有数据则返回1
			{
				rt_timer_delete(readtimer);
				rt_thread_delay(RT_TICK_PER_SECOND/5);
				return 1;
			}
		}
	}
}

//WIFI串口打开函数
int WiFi_Open(void)
{
	int result = 0;
	GPIO_InitTypeDef GPIO_InitStructure;
	rt_device_t new;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_6);		//设置引脚为高电平输出，使能WIFI模块
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	new = rt_device_find("uart1");		//寻找WIFI串口并配置模式
	if (new != RT_NULL)
	{
		result = rt_device_open(new, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
		
		if(result && WIFI_STATUS)
		{
			rt_kprintf("open WIFI failed : %d %d\r\n",result,WIFI_STATUS);
		}else
		{
			rt_kprintf("open WIFI success\r\n");
		}
	}
	return result;
}

int WiFi_SendData(tcp_address_t address ,char *data ,int length)   //在串口指令模式下发送数据    发送TCP报文
{
	int domain_length = 0,msg_length = 0,index = 0;
	unsigned char check_sum = 0;
	char tcp_msg[1024];
	if ((data == RT_NULL) || (length <= 0))
	{
		rt_kprintf("WIFI_SendData failed ...\n");
		return -1;
	}
	
	tcp_msg[0] = 0x55;
	tcp_msg[1] = 0xAA;
	
	if(address.address_type == TYPE_IP)
	{
		tcp_msg[4] = 0x03;
		
		tcp_msg[9] = address.address.ip[3]; 
		tcp_msg[10] = address.address.ip[2];
		tcp_msg[11] = address.address.ip[1];
		tcp_msg[12] = address.address.ip[0];
		msg_length =  5 + 4 + length;
		tcp_msg[2] = msg_length/256;
		tcp_msg[3] = msg_length%256;
		rt_memcpy(&tcp_msg[13],data,length);
	}else if(address.address_type == TYPE_DOMAIN)
	{
		tcp_msg[4] = 0x03;
		domain_length = rt_strlen(address.address.domain);
		rt_memcpy(&tcp_msg[9],address.address.ip,domain_length);
		tcp_msg[9+domain_length] = '\0';
		domain_length++;
		msg_length =  5 + domain_length + length;
		tcp_msg[2] = msg_length/256;
		tcp_msg[3] = msg_length%256;
		rt_memcpy(&tcp_msg[9+domain_length],data,length);
	}else
	{
		rt_kprintf("WIFI_SendData failed ...\n");
		return -2;
	}
	
	tcp_msg[5] = 0x00;
	tcp_msg[6] = 0x00;
	tcp_msg[7] = address.port%256;
	tcp_msg[8] = address.port/256;
	
	for(index = 0 ; index < msg_length ; index++)
	{
		check_sum += tcp_msg[index+4]; 
	}
	tcp_msg[4+msg_length] = check_sum;
	WIFI_SERIAL.write(&WIFI_SERIAL, 0,tcp_msg, (msg_length+5));
	
	printhexmsg("WiFi_SendData", (char *)tcp_msg, (msg_length+5));
	return 	(msg_length+5);
}

int WiFi_RecvData(int timeout,char *data)
{
	int length = 0;
	if(data == RT_NULL)
	{
			return -1;
	}
	if(selectWiFi(timeout) <= 0)
	{
		printmsg("WIFI Get reply time out");
		return -1;
	}
	else
	{
		length = WIFI_SERIAL.read(&WIFI_SERIAL,0, data, 255);
		return length;
	}
}


#ifdef RT_USING_FINSH
#include <finsh.h>
void testWIFISend(int ip1,int ip2,int ip3,int ip4,int port)	//无线发送测试
{
	tcp_address_t address ;
	char data[11] = "YuNeng APS";
	int length = 10;
	address.address_type = TYPE_IP;
	address.address.ip[0] = ip1;
	address.address.ip[1] = ip2;
	address.address.ip[2] = ip3;
	address.address.ip[3] = ip4;
	address.port = port;
	WiFi_SendData(address ,data ,length);
}
FINSH_FUNCTION_EXPORT(testWIFISend , WIFI Send Test[ip1 ip2 ip3 ip4 port].)

void testWIFIRecv()	//无线接收测试
{
	char data[256] = {0};
	int length = 0;
	length = WiFi_RecvData(5,data);
	rt_kprintf("WiFi_RecvData:%d   %s\n",length,data);
}
FINSH_FUNCTION_EXPORT(testWIFIRecv , WIFI Recv Test.)
#endif
