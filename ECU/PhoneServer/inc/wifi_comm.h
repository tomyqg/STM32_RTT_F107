#ifndef __WIFI_COMM_H__
#define __WIFI_COMM_H__

#include "variation.h"

typedef enum
{ 
    COMMAND_BASEINFO         	= 1,		//获取基本信息请求
    COMMAND_POWERGENERATION  	= 2,		//逆变器发电数据请求
    COMMAND_POWERCURVE      	= 3,		//功率曲线请求
    COMMAND_GENERATIONCURVE  	= 4,		//发电量曲线请求
		COMMAND_REGISTERID 				= 5,		//逆变器ID注册请求
		COMMAND_SETTIME			 			= 6,		//时间设置请求
		COMMAND_SETWIREDNETWORK		= 7,		//有线网络设置请求
		COMMAND_SETWIFI 					= 8,		//无线网络设置请求
		COMMAND_SEARCHWIFISTATUS	= 9,		//无线网络连接状态请求
		COMMAND_SETWIFIPASSWD			= 10,		//AP密码设置请求
	
} eCommandID;// receive state machin


typedef struct
{
	char ECUID[13];											//ECU ID
	unsigned int LifttimeEnergy;				//ECU 历史发电量
	unsigned short LastSystemPower;			//ECU 当前系统功率
	unsigned short GenerationCurrentDay;//ECU 当天发电量
	char LastToEMA[8];									//ECU 最后一次连接EMA的时间
	unsigned short InvertersNum;				//ECU 逆变器总数
	unsigned short LastInvertersNum;		//ECU 当前连接的逆变器总数
	unsigned char Length;								//ECU 版本号长度
	char Version[20];										//ECU 版本
	unsigned char TimeZoneLength;				//ECU 时区长度
	char TimeZone[20];									//ECU 时区
	char MacAddress[7];									//ECU 有线Mac地址
	//char WifiMac[7];										//ECU 无线Mac地址
} stBaseInfo;


unsigned short packetlen(unsigned char *packet);

int Resolve_RecvData(char *RecvData,int* Data_Len,int* Command_Id);
int phone_add_inverter(int num,char *uidstring);
//01	获取基本信息请求
void APP_Response_BaseInfo(unsigned char *ID,stBaseInfo baseInfo);
//02	逆变器发电数据请求
void APP_Response_PowerGeneration(char mapping,unsigned char *ID,inverter_info *inverter,int VaildNum);
//03	功率曲线请求
void APP_Response_PowerCurve(char mapping,unsigned char *ID,char * date);
//04	发电量曲线请求
void APP_Response_GenerationCurve(char mapping,unsigned char *ID,char request_type);
//05	逆变器ID注册请求
void APP_Response_RegisterID(char mapping,unsigned char *ID);
//06	时间设置请求
void APP_Response_SetTime(char mapping,unsigned char *ID);
//07	有线网络设置请求
void APP_Response_SetWiredNetwork(char mapping,unsigned char *ID);
//08	无线网络设置请求
void APP_Response_SetWifi(char mapping,unsigned char *ID);
//09	无线网络连接状态请求
void APP_Response_SearchWifiStatus(char mapping,unsigned char *ID);
//10	AP密码设置请求
void APP_Response_SetWifiPasswd(char mapping,unsigned char *ID);

#endif /*__WIFI_COMM_H__*/
