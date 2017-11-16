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
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "debug.h"
#include "threadlist.h"
#include "wifi_comm.h"
#include "variation.h"
#include "main-thread.h"
#include "file.h"
#include "rtc.h"
#include "version.h"
#include "arch/sys_arch.h"
#include "usart5.h"
#include "client.h"
#include "myfile.h"
#include <lwip/netdb.h> /* 为了解析主机名，需要包含netdb.h头文件 */
#include <lwip/sockets.h> /* 使用BSD socket，需要包含sockets.h头文件 */
#include <dfs_fs.h>
#include <dfs_file.h>
#include "file.h"
#include <dfs_posix.h> 
#include "lwip/netif.h"
#include "lwip/ip_addr.h"
#include "lwip/dns.h"
#include "channel.h"

#define MIN_FUNCTION_ID 0
#define MAX_FUNCTION_ID 14


extern rt_mutex_t wifi_uart_lock;
extern ecu_info ecu;
extern inverter_info inverter[MAXINVERTERCOUNT];
enum CommandID{
	P0000, P0001, P0002, P0003, P0004, P0005, P0006, P0007, P0008, P0009, //0-9
	P0010, P0011, P0012, P0013, P0014, P0015, P0016, P0017, P0018, P0019, //10-19
	P0020, P0021, P0022, P0023, P0024, P0025, P0026, P0027, P0028, P0029, //20-29
	P0030, P0031, P0032, P0033, P0034, P0035, P0036, P0037, P0038, P0039, //30-39
	P0040, P0041, P0042, P0043, P0044, P0045, P0046, P0047, P0048, P0049, //40-49
	P0050, P0051, P0052, P0053, P0054, P0055, P0056, P0057, P0058, P0059, //50-59
};


void (*pfun_Phone[100])(unsigned char * ID,int Data_Len,const char *recvbuffer);

void add_Phone_functions(void)
{
	pfun_Phone[P0001] = Phone_GetBaseInfo; 				//获取基本信息请求
	pfun_Phone[P0002] = Phone_GetGenerationData; 	//获取逆变器发电量数据
	pfun_Phone[P0003] = Phone_GetPowerCurve; 			//获取功率曲线
	pfun_Phone[P0004] = Phone_GetGenerationCurve; 			//获取发电量曲线
	pfun_Phone[P0005] = Phone_RegisterID; 			//逆变器ID注册
	pfun_Phone[P0006] = Phone_SetTime; 			//ECU时间设置
	pfun_Phone[P0007] = Phone_SetWiredNetwork; 			//有线网络设置
	pfun_Phone[P0008] = Phone_GetECUHardwareStatus; 	//查看当前ECU硬件状态
	//pfun_Phone[P0009] = Phone_SearchWIFIStatus; 			//无线网络连接状态
	pfun_Phone[P0010] = Phone_SetWIFIPasswd; 			//AP密码设置
	pfun_Phone[P0011] = Phone_GetIDInfo; 			//获取ID信息
	pfun_Phone[P0012] = Phone_GetTime; 			//获取时间
	pfun_Phone[P0013] = Phone_FlashSize; 			//获取FLASH空间
	pfun_Phone[P0014] = Phone_GetWiredNetwork; 			//获取网络配置
	pfun_Phone[P0015] = Phone_SetChannel;			//设置信道
	pfun_Phone[P0018] = Phone_GetShortAddrInfo;		//功率电流电压曲线
}


int getAddr(MyArray *array, int num,IPConfig_t *IPconfig)
{
	int i;
	ip_addr_t addr;
	for(i=0; i<num; i++){
		memset(&addr,0x00,sizeof(addr));
		if(!strlen(array[i].name))break;
		//IP地址
		if(!strcmp(array[i].name, "IPAddr")){
			ipaddr_aton(array[i].value,&addr);
			IPconfig->IPAddr.IP1 = (addr.addr&(0x000000ff))>>0;
			IPconfig->IPAddr.IP2 = (addr.addr&(0x0000ff00))>>8;
			IPconfig->IPAddr.IP3 = (addr.addr&(0x00ff0000))>>16;
			IPconfig->IPAddr.IP4 = (addr.addr&(0xff000000))>>24;
		}
		//掩码地址
		else if(!strcmp(array[i].name, "MSKAddr")){
			ipaddr_aton(array[i].value,&addr);
			IPconfig->MSKAddr.IP1 = (addr.addr&(0x000000ff))>>0;
			IPconfig->MSKAddr.IP2 = (addr.addr&(0x0000ff00))>>8;
			IPconfig->MSKAddr.IP3 = (addr.addr&(0x00ff0000))>>16;
			IPconfig->MSKAddr.IP4 = (addr.addr&(0xff000000))>>24;
		}
		//网关地址
		else if(!strcmp(array[i].name, "GWAddr")){
			ipaddr_aton(array[i].value,&addr);
			IPconfig->GWAddr.IP1 = (addr.addr&(0x000000ff))>>0;
			IPconfig->GWAddr.IP2 = (addr.addr&(0x0000ff00))>>8;
			IPconfig->GWAddr.IP3 = (addr.addr&(0x00ff0000))>>16;
			IPconfig->GWAddr.IP4 = (addr.addr&(0xff000000))>>24;
		}
		//DNS1地址
		else if(!strcmp(array[i].name, "DNS1Addr")){
			ipaddr_aton(array[i].value,&addr);
			IPconfig->DNS1Addr.IP1 = (addr.addr&(0x000000ff))>>0;
			IPconfig->DNS1Addr.IP2 = (addr.addr&(0x0000ff00))>>8;
			IPconfig->DNS1Addr.IP3 = (addr.addr&(0x00ff0000))>>16;
			IPconfig->DNS1Addr.IP4 = (addr.addr&(0xff000000))>>24;
		}
		//DNS2地址
		else if(!strcmp(array[i].name, "DNS2Addr")){
			ipaddr_aton(array[i].value,&addr);
			IPconfig->DNS2Addr.IP1 = (addr.addr&(0x000000ff))>>0;
			IPconfig->DNS2Addr.IP2 = (addr.addr&(0x0000ff00))>>8;
			IPconfig->DNS2Addr.IP3 = (addr.addr&(0x00ff0000))>>16;
			IPconfig->DNS2Addr.IP4 = (addr.addr&(0xff000000))>>24;
		}	
	}
	return 0;
}

	
int ResolveWifiPasswd(char *oldPasswd,int *oldLen,char *newPasswd,int *newLen,char *passwdstring)
{
	*oldLen = (passwdstring[0]-'0')*10+(passwdstring[1]-'0');
	memcpy(oldPasswd,&passwdstring[2],*oldLen);
	*newLen = (passwdstring[2+(*oldLen)]-'0')*10+(passwdstring[3+(*oldLen)]-'0');
	memcpy(newPasswd,&passwdstring[4+*oldLen],*newLen);
	
	return 0;
}	

int ResolveWifiSSID(char *SSID,int *SSIDLen,char *Auth,char *Encry,char *PassWD,int *PassWDLen,char *string)
{
	*SSIDLen = (string[0]-'0')*100+(string[1]-'0')*10+(string[2]-'0');
	memcpy(SSID,&string[3],*SSIDLen);
	*Auth = string[3+(*SSIDLen)];
	*Encry = string[4+(*SSIDLen)];
	*PassWDLen = (string[5+(*SSIDLen)]-'0')*100+(string[6+(*SSIDLen)]-'0')*10+(string[7+(*SSIDLen)]-'0');
	memcpy(PassWD,&string[8+*SSIDLen],*PassWDLen);
	return 0;
}	

//返回0 表示动态IP  返回1 表示固定IP   返回-1表示失败
int ResolveWired(const char *string,IP_t *IPAddr,IP_t *MSKAddr,IP_t *GWAddr,IP_t *DNS1Addr,IP_t *DNS2Addr)
{
	if(string[1] == '0')	//动态IP
	{
		return 0;
	}else if(string[1] == '1') //静态IP
	{
		IPAddr->IP1 = string[2];
		IPAddr->IP2 = string[3];
		IPAddr->IP3 = string[4];
		IPAddr->IP4 = string[5];
		MSKAddr->IP1 = string[6];
		MSKAddr->IP2 = string[7];
		MSKAddr->IP3 = string[8];
		MSKAddr->IP4 = string[9];
		GWAddr->IP1 = string[10];
		GWAddr->IP2 = string[11];
		GWAddr->IP3 = string[12];
		GWAddr->IP4 = string[13];	
		DNS1Addr->IP1 = string[14];	
		DNS1Addr->IP2 = string[15];	
		DNS1Addr->IP3 = string[16];	
		DNS1Addr->IP4 = string[17];	
		DNS2Addr->IP1 = string[18];	
		DNS2Addr->IP2 = string[19];	
		DNS2Addr->IP3 = string[20];	
		DNS2Addr->IP4 = string[21];	
		return 1;
	}else
	{
		return -1;
	}

}
	
//获取基本数据 
void Phone_GetBaseInfo(unsigned char * ID,int Data_Len,const char *recvbuffer) 				//获取基本信息请求
{
	stBaseInfo baseInfo;
	print2msg(ECU_DBG_WIFI,"WIFI_Recv_Event 01 ",(char *)recvbuffer);
			
	memcpy(baseInfo.ECUID,ecu.id,13);											//ECU ID		OK
	baseInfo.LifttimeEnergy = (int)(ecu.life_energy*10);				//ECU 历史发电量		OK
	baseInfo.LastSystemPower = ecu.system_power;			//ECU 当前系统功率		OK
	baseInfo.GenerationCurrentDay = (unsigned int)(ecu.today_energy * 100);//ECU 当天发电量
				
	memset(baseInfo.LastToEMA,'\0',8);	//ECU 最后一次连接EMA的时间
	baseInfo.LastToEMA[0] = (ecu.last_ema_time[0] - '0')*16+(ecu.last_ema_time[1] - '0');
	baseInfo.LastToEMA[1] = (ecu.last_ema_time[2] - '0')*16+(ecu.last_ema_time[3] - '0');
	baseInfo.LastToEMA[2] = (ecu.last_ema_time[4] - '0')*16+(ecu.last_ema_time[5] - '0');
	baseInfo.LastToEMA[3] = (ecu.last_ema_time[6] - '0')*16+(ecu.last_ema_time[7] - '0');
	baseInfo.LastToEMA[4] = (ecu.last_ema_time[8] - '0')*16+(ecu.last_ema_time[9] - '0');
	baseInfo.LastToEMA[5] = (ecu.last_ema_time[10] - '0')*16+(ecu.last_ema_time[11] - '0');
	baseInfo.LastToEMA[6] = (ecu.last_ema_time[12] - '0')*16+(ecu.last_ema_time[13] - '0');

	baseInfo.InvertersNum = ecu.total;				//ECU 逆变器总数
	baseInfo.LastInvertersNum = ecu.count;		//ECU 当前连接的逆变器总数
	baseInfo.Length = ECU_VERSION_LENGTH;								//ECU 版本号长度
	sprintf(baseInfo.Version,"%s_%s_%s",ECU_VERSION,MAJORVERSION,MINORVERSION);	//ECU 版本
	baseInfo.TimeZoneLength = 9;				//ECU 时区长度
	sprintf(baseInfo.TimeZone,"Etc/GMT-8");									//ECU 时区
	memset(baseInfo.MacAddress,'\0',7);
	memcpy(baseInfo.MacAddress,ecu.MacAddress,7);//ECU 有线Mac地址	
	sprintf(baseInfo.Channel,"%02x",ecu.channel);
	APP_Response_BaseInfo(ID,baseInfo);
}

//获取逆变器发电量数据
void Phone_GetGenerationData(unsigned char * ID,int Data_Len,const char *recvbuffer) 	//获取逆变器发电量数据
{
	print2msg(ECU_DBG_WIFI,"WIFI_Recv_Event 02 ",(char *)recvbuffer);
	
	if(!memcmp(&recvbuffer[13],ecu.id,12))
	{	//匹配成功进行相应操作
		APP_Response_PowerGeneration(0x00,ID,inverter,ecu.total);
	}else
	{
		APP_Response_PowerGeneration(0x01,ID,inverter,ecu.total);
	}
}

//获取功率曲线
void Phone_GetPowerCurve(unsigned char * ID,int Data_Len,const char *recvbuffer) 			//获取功率曲线
{
	char date[9] = {'\0'};
	print2msg(ECU_DBG_WIFI,"WIFI_Recv_Event 03 ",(char *)recvbuffer);
	memset(date,'\0',9);
	memcpy(date,&recvbuffer[28],8);
	
	if(!memcmp(&recvbuffer[13],ecu.id,12))
	{	//匹配成功进行相应操作
		APP_Response_PowerCurve(0x00,ID,date);
	}else
	{
		APP_Response_PowerCurve(0x01,ID,date);
	}

}

//获取发电量曲线
void Phone_GetGenerationCurve(unsigned char * ID,int Data_Len,const char *recvbuffer) 			//获取发电量曲线
{
	print2msg(ECU_DBG_WIFI,"WIFI_Recv_Event04 ",(char *)recvbuffer);
	if(!memcmp(&recvbuffer[13],ecu.id,12))
	{	//匹配成功进行相应操作
		APP_Response_GenerationCurve(0x00,ID,recvbuffer[29]);
	}else
	{
		APP_Response_GenerationCurve(0x01,ID,recvbuffer[29]);
	}
}

//逆变器ID注册
void Phone_RegisterID(unsigned char * ID,int Data_Len,const char *recvbuffer) 			//逆变器ID注册
{
	print2msg(ECU_DBG_WIFI,"WIFI_Recv_Event 05 ",(char *)recvbuffer);				
	if(!memcmp(&recvbuffer[13],ecu.id,12))
	{
		int i = 0;
		inverter_info *curinverter = inverter;
		int AddNum = 0;
		//匹配成功进行相应操作
		//计算台数
		AddNum = (Data_Len - 31)/6;
		APP_Response_RegisterID(0x00,ID);
		//添加ID到文件
		phone_add_inverter(AddNum,&recvbuffer[28]);
			
		for(i=0; i<MAXINVERTERCOUNT; i++, curinverter++)
		{
			rt_memset(curinverter->id, '\0', sizeof(curinverter->id));		//清空逆变器UI
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
			curinverter->no_getdata_num=0;	//ZK,清空连续获取不到的次数
			curinverter->disconnect_times=0;		//没有与逆变器通信上的次数清0, ZK
			curinverter->signalstrength=0;			//信号强度初始化为0

			curinverter->updating=0;
			curinverter->raduis=0;
		}
		
		get_id_from_file(inverter);
		
		//重启main线程
		restartThread(TYPE_MAIN);					
					
	}else
	{
		APP_Response_RegisterID(0x01,ID);
	}
}

//ECU时间设置
void Phone_SetTime(unsigned char * ID,int Data_Len,const char *recvbuffer) 			//ECU时间设置
{
	char setTime[15];
	char getTime[15];
	print2msg(ECU_DBG_WIFI,"WIFI_Recv_Event 06 ",(char *)recvbuffer);
	if(!memcmp(&recvbuffer[13],ecu.id,12))
	{	
		//匹配成功进行相应操作
		memset(setTime,'\0',15);
		memcpy(setTime,&recvbuffer[28],14);
		apstime(getTime);
		if(!memcmp("99999999",setTime,8))
		{	//如果年月日都是9 年月日取当前的
			memcpy(setTime,getTime,8);
		}

		if(!memcmp("999999",&setTime[8],6))
		{	//如果时间都是9 时间取当前的
			memcpy(&setTime[8],&getTime[8],6);
		}
			
		//设置时间
		set_time(setTime);
		//重启main线程
		restartThread(TYPE_MAIN);	
		APP_Response_SetTime(0x00,ID);
	}else
	{
		APP_Response_SetTime(0x01,ID);
	}

}

//有线网络设置
void Phone_SetWiredNetwork(unsigned char * ID,int Data_Len,const char *recvbuffer)			//有线网络设置
{
	print2msg(ECU_DBG_WIFI,"WIFI_Recv_Event 07 ",(char *)recvbuffer);	
	if(!memcmp(&recvbuffer[13],ecu.id,12))
	{
		int ModeFlag = 0;
		char buff[200] = {'\0'};
		IP_t IPAddr,MSKAddr,GWAddr,DNS1Addr,DNS2Addr;
		//匹配成功进行相应操作
		APP_Response_SetWiredNetwork(0x00,ID);
		//检查是DHCP  还是固定IP
		ModeFlag = ResolveWired(&recvbuffer[28],&IPAddr,&MSKAddr,&GWAddr,&DNS1Addr,&DNS2Addr);
		if(ModeFlag == 0x00)		//DHCP
		{
			printmsg(ECU_DBG_WIFI,"dynamic IP");
			unlink("/yuneng/staticIP.con");
			dhcp_reset();
		}else if (ModeFlag == 0x01)		//固定IP		
		{
			printmsg(ECU_DBG_WIFI,"static IP");
			//保存网络地址
			sprintf(buff,"IPAddr=%d.%d.%d.%d\nMSKAddr=%d.%d.%d.%d\nGWAddr=%d.%d.%d.%d\nDNS1Addr=%d.%d.%d.%d\nDNS2Addr=%d.%d.%d.%d\n",IPAddr.IP1,IPAddr.IP2,IPAddr.IP3,IPAddr.IP4,
			MSKAddr.IP1,MSKAddr.IP2,MSKAddr.IP3,MSKAddr.IP4,GWAddr.IP1,GWAddr.IP2,GWAddr.IP3,GWAddr.IP4,DNS1Addr.IP1,DNS1Addr.IP2,DNS1Addr.IP3,DNS1Addr.IP4,DNS2Addr.IP1,DNS2Addr.IP2,DNS2Addr.IP3,DNS2Addr.IP4);
			echo("/yuneng/staticIP.con",buff);
			//设置固定IP
			StaticIP(IPAddr,MSKAddr,GWAddr,DNS1Addr,DNS2Addr);
		}
	}	
	else
	{
		APP_Response_SetWiredNetwork(0x01,ID);
	}
}

//获取硬件信息
void Phone_GetECUHardwareStatus(unsigned char * ID,int Data_Len,const char *recvbuffer) 
{
	print2msg(ECU_DBG_WIFI,"WIFI_Recv_Event 8 ",(char *)recvbuffer);
	if(!memcmp(&WIFI_RecvSocketAData[13],ecu.id,12))
	{	//匹配成功进行相应的操作
		APP_Response_GetECUHardwareStatus(0x00,ID);
	}else
	{
		APP_Response_GetECUHardwareStatus(0x01,ID);
	}

}

//AP密码设置
void Phone_SetWIFIPasswd(unsigned char * ID,int Data_Len,const char *recvbuffer) 			//AP密码设置
{
	print2msg(ECU_DBG_WIFI,"WIFI_Recv_Event 10 ",(char *)recvbuffer);
	if(!memcmp(&recvbuffer[13],ecu.id,12))
	{
		char OldPassword[100] = {'\0'};
		char NewPassword[100] = {'\0'};
		char EEPROMPasswd[100] = {'\0'};
		int oldLen,newLen;
						
		//匹配成功进行相应操作
		printf("COMMAND_SETWIFIPASSWD  Mapping\n");
		//获取密码
		ResolveWifiPasswd(OldPassword,&oldLen,NewPassword,&newLen,(char *)&recvbuffer[28]);
		//读取旧密码，如果旧密码相同，设置新密码
		get_Passwd(EEPROMPasswd);
								
		if((!memcmp(EEPROMPasswd,OldPassword,oldLen))&&(oldLen == strlen(EEPROMPasswd)))
		{
			APP_Response_SetWifiPasswd(0x00,ID);
			WIFI_ChangePasswd(NewPassword);

			set_Passwd(NewPassword,newLen);
		}else
		{
			APP_Response_SetWifiPasswd(0x02,ID);
		}				
	}	else
	{
		APP_Response_SetWifiPasswd(0x01,ID);
	}
	
}

//获取ID信息
void Phone_GetIDInfo(unsigned char * ID,int Data_Len,const char *recvbuffer) 			//获取ID信息
{
	print2msg(ECU_DBG_WIFI,"WIFI_Recv_Event 11 ",(char *)recvbuffer);
	if(!memcmp(&recvbuffer[13],ecu.id,12))
	{
		APP_Response_GetIDInfo(0x00,ID,inverter);
	}else
	{
		APP_Response_GetIDInfo(0x01,ID,inverter);
	}
}

//获取时间
void Phone_GetTime(unsigned char * ID,int Data_Len,const char *recvbuffer) 			//获取时间
{
	char Time[15] = {'\0'};
	print2msg(ECU_DBG_WIFI,"WIFI_Recv_Event 12 ",(char *)recvbuffer);
	apstime(Time);
	Time[14] = '\0';
	if(!memcmp(&recvbuffer[13],ecu.id,12))
	{
		APP_Response_GetTime(0x00,ID,Time);
	}else
	{
		APP_Response_GetTime(0x01,ID,Time);
	}
}

//获取FLASH空间
void Phone_FlashSize(unsigned char * ID,int Data_Len,const char *recvbuffer) 			//获取时间
{
	int result;
	long long cap;
	struct statfs buffer;

	print2msg(ECU_DBG_WIFI,"WIFI_Recv_Event 13 ",(char *)recvbuffer);
	result = dfs_statfs("/", &buffer);
	if (result != 0)
	{
		APP_Response_FlashSize(0x00,ID,0);
		return;
	}
	cap = buffer.f_bsize * buffer.f_bfree / 1024;
	if(!memcmp(&recvbuffer[13],ecu.id,12))
	{
		APP_Response_FlashSize(0x00,ID,(unsigned int)cap);
	}else
	{
		APP_Response_FlashSize(0x01,ID,(unsigned int)cap);
	}
}

//获取网络配置
void Phone_GetWiredNetwork(unsigned char * ID,int Data_Len,const char *recvbuffer)			//获取有线网络设置
{
	print2msg(ECU_DBG_WIFI,"WIFI_Recv_Event 14 ",(char *)recvbuffer);	
	if(!memcmp(&recvbuffer[13],ecu.id,12))
	{
		unsigned int addr = 0;
		extern struct netif *netif_list;
		struct netif * netif;
		int ModeFlag = 0;
		ip_addr_t DNS;
		IP_t IPAddr,MSKAddr,GWAddr,DNS1Addr,DNS2Addr;
		//匹配成功进行相应操作
		printf("COMMAND_GETWIREDNETWORK  Mapping\n");
		netif = netif_list;
		addr = ip4_addr_get_u32(&netif->ip_addr);
		IPAddr.IP4 = (addr/16777216)%256;
		IPAddr.IP3 = (addr/65536)%256;
		IPAddr.IP2 = (addr/256)%256;
		IPAddr.IP1 = (addr/1)%256;
		
		addr = ip4_addr_get_u32(&netif->netmask);
		MSKAddr.IP4 = (addr/16777216)%256;
		MSKAddr.IP3 = (addr/65536)%256;
		MSKAddr.IP2 = (addr/256)%256;
		MSKAddr.IP1 = (addr/1)%256;
		
		addr = ip4_addr_get_u32(&netif->gw);
		GWAddr.IP4 = (addr/16777216)%256;
		GWAddr.IP3 = (addr/65536)%256;
		GWAddr.IP2 = (addr/256)%256;
		GWAddr.IP1 = (addr/1)%256;
		
		DNS = dns_getserver(0);
		addr = ip4_addr_get_u32(&DNS);
		DNS1Addr.IP4 = (addr/16777216)%256;
		DNS1Addr.IP3 = (addr/65536)%256;
		DNS1Addr.IP2 = (addr/256)%256;
		DNS1Addr.IP1 = (addr/1)%256;

		DNS = dns_getserver(1);
		addr = ip4_addr_get_u32(&DNS);
		DNS2Addr.IP4 = (addr/16777216)%256;
		DNS2Addr.IP3 = (addr/65536)%256;
		DNS2Addr.IP2 = (addr/256)%256;
		DNS2Addr.IP1 = (addr/1)%256;

		//检查是DHCP  还是固定IP
		ModeFlag = get_DHCP_Status();
			
		APP_Response_GetWiredNetwork(0x00,ID,ModeFlag,IPAddr,MSKAddr,GWAddr,DNS1Addr,DNS2Addr);

	}	else
	{
		int ModeFlag = 0;
		IP_t IPAddr,MSKAddr,GWAddr,DNS1Addr,DNS2Addr;
		APP_Response_GetWiredNetwork(0x01,ID,ModeFlag,IPAddr,MSKAddr,GWAddr,DNS1Addr,DNS2Addr);
	}
}


int switchChannel(unsigned char *buff)
{
	int ret=0x10;
	if((buff[0]>='0') && (buff[0]<='9'))
			buff[0] -= 0x30;
	if((buff[0]>='A') && (buff[0]<='F'))
			buff[0] -= 0x37;
	if((buff[0]>='a') && (buff[0]<='f'))
			buff[0] -= 0x57;
	if((buff[1]>='0') && (buff[1]<='9'))
			buff[1] -= 0x30;
	if((buff[1]>='A') && (buff[1]<='F'))
			buff[1] -= 0x37;
	if((buff[1]>='a') && (buff[1]<='f'))
			buff[1] -= 0x57;
	ret = (buff[0]*16+buff[1]);
	return ret;
}

//设置信道
void Phone_SetChannel(unsigned char * ID,int Data_Len,const char *recvbuffer)
{
	print2msg(ECU_DBG_WIFI,"WIFI_Recv_Event 15 ",(char *)recvbuffer);
	if(!memcmp(&recvbuffer[13],ecu.id,12))
	{	//匹配成功进行相应的操作
		unsigned char old_channel = 0x00;
		unsigned char new_channel = 0x00;
	
		APP_Response_SetChannel(ID,0x00,ecu.channel,0);
		old_channel = switchChannel(&WIFI_RecvSocketAData[28]);
		new_channel = switchChannel(&WIFI_RecvSocketAData[30]);	

		saveOldChannel(old_channel);
		saveNewChannel(new_channel);
		saveChannel_change_flag();
		//重启main线程
		restartThread(TYPE_MAIN);
	}else
	{
		APP_Response_SetChannel(ID,0x01,ecu.channel,0);
	}

}

void Phone_GetShortAddrInfo(unsigned char * ID,int Data_Len,const char *recvbuffer) 			//获取ID信息
{
	
	
	printf("WIFI_Recv_Event%d %s\n",18,recvbuffer);
	if(!memcmp(&WIFI_RecvSocketAData[13],ecu.id,12))
	{
		APP_Response_GetShortAddrInfo(0x00,ID,inverter);
	}else
	{
		APP_Response_GetShortAddrInfo(0x01,ID,inverter);
	}
	
}


//WIFI事件处理
void process_WIFI(unsigned char * ID,char *WIFI_RecvData)
{
//#ifdef WIFI_USE
	int ResolveFlag = 0,Data_Len = 0,Command_Id = 0;
	ResolveFlag =  Resolve_RecvData((char *)WIFI_RecvData,&Data_Len,&Command_Id);
	if(ResolveFlag == 0)
	{
		add_Phone_functions();
		if(pfun_Phone[Command_Id])
		{
			(*pfun_Phone[Command_Id])(ID,Data_Len,WIFI_RecvData);
		}
		
	}
//#endif
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
	MyArray array[5];
	int fileflag = 0; 
	IPConfig_t IPconfig;

	get_ecuid(ecu.id);
	get_mac((unsigned char*)ecu.MacAddress);			//ECU 有线Mac地址
	readconnecttime();
	
	add_Phone_functions();
		
	//3?ê??ˉIP
	fileflag = file_get_array(array, 5, "/yuneng/staticIP.con");
	if(fileflag == 0)
	{
		getAddr(array, 5,&IPconfig);
		StaticIP(IPconfig.IPAddr,IPconfig.MSKAddr,IPconfig.GWAddr,IPconfig.DNS1Addr,IPconfig.DNS2Addr);
	}

	
	while(1)
	{	
		//上锁
		rt_mutex_take(wifi_uart_lock, RT_WAITING_FOREVER);
		//获取WIFI事件
		WIFI_GetEvent();
		//解锁
		rt_mutex_release(wifi_uart_lock);
		
		if(WIFI_Recv_SocketA_Event == 1)
		{
			//print2msg(ECU_DBG_WIFI,"phone_server",(char *)WIFI_RecvSocketAData);
			WIFI_Recv_SocketA_Event = 0;
	
			process_WIFI(ID_A,(char *)WIFI_RecvSocketAData);
		}
		
		rt_thread_delay(RT_TICK_PER_SECOND/20);
	}
	
}
