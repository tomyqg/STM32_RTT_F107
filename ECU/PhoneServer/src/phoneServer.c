/*****************************************************************************/
/* File      : phoneServer.c                                                 */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-05-19 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "phoneServer.h"
#include "rtthread.h"
#include "usr_wifi232.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "debug.h"
#include "threadlist.h"
#include "wifi_comm.h"
#include "variation.h"

extern rt_mutex_t usr_wifi_lock;
extern ecu_info ecu;
extern inverter_info inverter[MAXINVERTERCOUNT];
	
//WIFI事件处理
void process_WIFI(unsigned char * ID,char *WIFI_RecvData)
{
	int ResolveFlag = 0,Data_Len = 0,Command_Id = 0;
	stBaseInfo baseInfo;
	ResolveFlag =  Resolve_RecvData((char *)WIFI_RecvData,&Data_Len,&Command_Id);
	if(ResolveFlag == 0)
	{
		switch(Command_Id)
		{
			case COMMAND_BASEINFO:						//获取基本信息请求
				printf("WIFI_Recv_Event%d %s\n",COMMAND_BASEINFO,WIFI_RecvData);
//					memcpy(baseInfo.ECUID,ecu.id,13);											//ECU ID
//	baseInfo.LifttimeEnergy = (int)(ecu.life_energy*10);				//ECU 历史发电量
//	unsigned short LastSystemPower;			//ECU 当前系统功率
//	unsigned short GenerationCurrentDay;//ECU 当天发电量
//	char LastToEMA[8];									//ECU 最后一次连接EMA的时间
//	unsigned short InvertersNum;				//ECU 逆变器总数
//	unsigned short LastInvertersNum;		//ECU 当前连接的逆变器总数
//	unsigned char Length;								//ECU 版本号长度
//	char Version[20];										//ECU 版本
//	unsigned char TimeZoneLength;				//ECU 时区长度
//	char TimeZone[20];									//ECU 时区
//	char MacAddress[7];									//ECU 有线Mac地址
//	char WifiMac[7];										//ECU 无线Mac地址
				APP_Response_BaseInfo(ID,baseInfo);
				break;	
			
			case COMMAND_POWERGENERATION:			//逆变器发电数据请求	
				printf("WIFI_Recv_Event%d %s\n",COMMAND_POWERGENERATION,WIFI_RecvData);
				//首先对比ECU ID是否匹配
				if(!memcmp(&WIFI_RecvData[11],ecu.id,12))
				{
					//匹配成功进行相应操作
					printf("COMMAND_POWERGENERATION  Mapping\n");
					APP_Response_PowerGeneration(0x00,ID,inverter,ecu.count);
				}else
				{	
					//不匹配，发送 匹配失败报文
					printf("COMMAND_POWERGENERATION   Not Mapping");
					APP_Response_PowerGeneration(0x01,ID,inverter,ecu.count);
				}
				break;
					
			case COMMAND_POWERCURVE:					//功率曲线请求
				printf("WIFI_Recv_Event%d %s\n",COMMAND_POWERCURVE,WIFI_RecvData);
				//首先对比ECU ID是否匹配
				if(!memcmp(&WIFI_RecvData[11],ecu.id,12))
				{
					//匹配成功进行相应操作
					printf("COMMAND_POWERCURVE  Mapping\n");
				}else
				{	
					//不匹配，发送 匹配失败报文
					
					printf("COMMAND_POWERCURVE   Not Mapping");
				}
				break;
						
			case COMMAND_GENERATIONCURVE:			//发电量曲线请求	
				printf("WIFI_Recv_Event%d %s\n",COMMAND_GENERATIONCURVE,WIFI_RecvData);
				//首先对比ECU ID是否匹配
				if(!memcmp(&WIFI_RecvData[11],ecu.id,12))
				{
					//匹配成功进行相应操作
					printf("COMMAND_GENERATIONCURVE  Mapping\n");
				}else
				{	
					//不匹配，发送 匹配失败报文
					
					printf("COMMAND_GENERATIONCURVE   Not Mapping");
				}
			
				break;
						
			case COMMAND_REGISTERID:					//逆变器ID注册请求	
				printf("WIFI_Recv_Event%d %s\n",COMMAND_REGISTERID,WIFI_RecvData);				
				//首先对比ECU ID是否匹配
				if(!memcmp(&WIFI_RecvData[11],ecu.id,12))
				{
					//匹配成功进行相应操作
					printf("COMMAND_REGISTERID  Mapping\n");
				}else
				{	
					//不匹配，发送 匹配失败报文
					
					printf("COMMAND_REGISTERID   Not Mapping");
				}
				break;
				
			case COMMAND_SETTIME:							//时间设置请求
				printf("WIFI_Recv_Event%d %s\n",COMMAND_SETTIME,WIFI_RecvData);
				//首先对比ECU ID是否匹配
				if(!memcmp(&WIFI_RecvData[11],ecu.id,12))
				{
					//匹配成功进行相应操作
					printf("COMMAND_SETTIME  Mapping\n");
				}else
				{	
					//不匹配，发送 匹配失败报文
					
					printf("COMMAND_SETTIME   Not Mapping");
				}
				break;
			
			case COMMAND_SETWIREDNETWORK:			//有线网络设置请求
				printf("WIFI_Recv_Event%d %s\n",COMMAND_SETWIREDNETWORK,WIFI_RecvData);	
				//首先对比ECU ID是否匹配
				if(!memcmp(&WIFI_RecvData[11],ecu.id,12))
				{
					//匹配成功进行相应操作
					printf("COMMAND_SETWIREDNETWORK  Mapping\n");
				}else
				{	
					//不匹配，发送 匹配失败报文
					
					printf("COMMAND_SETWIREDNETWORK   Not Mapping");
				}
				break;
			
			case COMMAND_SETWIFI:							//无线网络设置请求
				printf("WIFI_Recv_Event%d %s\n",COMMAND_SETWIFI,WIFI_RecvData);
				//首先对比ECU ID是否匹配
				if(!memcmp(&WIFI_RecvData[11],ecu.id,12))
				{
					//匹配成功进行相应操作
					printf("COMMAND_SETWIFI  Mapping\n");
				}else
				{	
					//不匹配，发送 匹配失败报文
					
					printf("COMMAND_SETWIFI   Not Mapping");
				}
				break;
			
			case COMMAND_SEARCHWIFISTATUS:		//无线网络连接状态请求
				printf("WIFI_Recv_Event%d %s\n",COMMAND_SEARCHWIFISTATUS,WIFI_RecvData);
				//首先对比ECU ID是否匹配
				if(!memcmp(&WIFI_RecvData[11],ecu.id,12))
				{
					//匹配成功进行相应操作
					printf("COMMAND_SEARCHWIFISTATUS  Mapping\n");
				}else
				{	
					//不匹配，发送 匹配失败报文
					
					printf("COMMAND_SEARCHWIFISTATUS   Not Mapping");
				}
				break;
			
			case COMMAND_SETWIFIPASSWD:				//AP密码设置请求
				printf("WIFI_Recv_Event%d %s\n",COMMAND_SETWIFIPASSWD,WIFI_RecvData);
				//首先对比ECU ID是否匹配
				if(!memcmp(&WIFI_RecvData[11],ecu.id,12))
				{
					//匹配成功进行相应操作
					printf("COMMAND_SETWIFIPASSWD  Mapping\n");
				}else
				{	
					//不匹配，发送 匹配失败报文
					
					printf("COMMAND_SETWIFIPASSWD   Not Mapping");
				}
				break;
		}
	}
}


/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Phone Server Application program entry                                  */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   parameter  unused                                                       */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/
void phone_server_thread_entry(void* parameter)
{
	char *data = NULL;
	int length = 0;
	unsigned char ID[9] = {'\0'};
	data = malloc(2048);	
	while(1)
	{
		memset(data,0x00,2048);
#ifdef WIFI_USE 	
		rt_mutex_take(usr_wifi_lock, RT_WAITING_FOREVER);
		//Recv socket A data by serial,If the data is received, the phone is sent.
		length = RecvSocketData(SOCKET_A,data,1);
		rt_mutex_release(usr_wifi_lock);
		if(length > 0)
		{			
			memcpy(ID,&data[1],8);
			ID[8] = '\0';
			print2msg(ECU_DBG_WIFI,"phone_server",&data[9]);
			
			//检查是哪个功能函数		
			process_WIFI(ID,&data[9]);
			
		}
#endif		
		rt_thread_delay(RT_TICK_PER_SECOND);
	}
	//free(data);
	
}
