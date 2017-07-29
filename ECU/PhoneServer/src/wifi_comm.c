#include "wifi_comm.h"
#include "string.h"
#include "variation.h"
#include "usr_wifi232.h"
#include "stdio.h"
#include "version.h"
#include "file.h"
#include "rtc.h"

extern ecu_info ecu;
static char SendData[4096] = {'\0'};



int phone_add_inverter(int num,char *uidstring)
{
	int i = 0;
	char inverter_id[13] = { '\0' };
	char buff[25] = { '\0' };
	char allbuff[500] = { '\0' };
	for(i = 0; i < num; i++)
	{
		memset(inverter_id,'\0',13);
		memset(buff,'\0',25);
		inverter_id[0] =  (uidstring[0+i*6]/16) + '0';
		inverter_id[1] =  (uidstring[0+i*6]%16) + '0';
		inverter_id[2] =  (uidstring[1+i*6]/16) + '0';
		inverter_id[3] =  (uidstring[1+i*6]%16) + '0';
		inverter_id[4] =  (uidstring[2+i*6]/16) + '0';
		inverter_id[5] =  (uidstring[2+i*6]%16) + '0';
		inverter_id[6] =  (uidstring[3+i*6]/16) + '0';
		inverter_id[7] =  (uidstring[3+i*6]%16) + '0';
		inverter_id[8] =  (uidstring[4+i*6]/16) + '0';
		inverter_id[9] =  (uidstring[4+i*6]%16) + '0';
		inverter_id[10] = (uidstring[5+i*6]/16) + '0';
		inverter_id[11] = (uidstring[5+i*6]%16) + '0';
		sprintf(buff,"%s,,,,,,\n",inverter_id);
		
		memcpy(&allbuff[0+19*i],buff,19);
	}
	
	
	echo("/home/data/id",allbuff);
	echo("/yuneng/limiteid.con","1");
	return 0;
}




//解析报文长度
unsigned short packetlen(unsigned char *packet)
{
	unsigned short len = 0;
	len = ((packet[0]-'0')*1000 +(packet[1]-'0')*100 + (packet[2]-'0')*10 + (packet[3]-'0'));
	return len;
}

//解析收到的数据
int Resolve_RecvData(char *RecvData,int* Data_Len,int* Command_Id)
{
	//APS
	if(strncmp(RecvData, "APS", 3))
		return -1;
	//版本号 
	//长度
	*Data_Len = packetlen((unsigned char *)&RecvData[5]);
	//ID
	*Command_Id = (RecvData[9]-'0')*1000 + (RecvData[10]-'0')*100 + (RecvData[11]-'0')*10 + (RecvData[12]-'0');
	return 0;
}

//01 COMMAND_BASEINFO 					//获取基本信息请求
void APP_Response_BaseInfo(unsigned char *ID,stBaseInfo baseInfo)
{
	int packlength = 0;
	memset(SendData,'\0',4096);	
	//拼接需要发送的报文
	sprintf(SendData,"APS1100000001%s",baseInfo.ECUID);
	packlength = 25;

	SendData[packlength++] = '0';
	SendData[packlength++] = '1';
	
	SendData[packlength++] = (baseInfo.LastSystemPower/16777216)%256;
	SendData[packlength++] = (baseInfo.LastSystemPower/65536)%256;
	SendData[packlength++] = (baseInfo.LastSystemPower/256)%256;
	SendData[packlength++] =  baseInfo.LastSystemPower%256;

	SendData[packlength++] = (baseInfo.LastSystemPower/16777216)%256;
	SendData[packlength++] = (baseInfo.LastSystemPower/65536)%256;
	SendData[packlength++] = (baseInfo.LastSystemPower/256)%256;
	SendData[packlength++] = baseInfo.LastSystemPower%256;

	SendData[packlength++] = (baseInfo.GenerationCurrentDay/16777216)%256;
	SendData[packlength++] = (baseInfo.GenerationCurrentDay/65536)%256;
	SendData[packlength++] = (baseInfo.GenerationCurrentDay/256)%256;
	SendData[packlength++] = baseInfo.GenerationCurrentDay%256;
	
	
	memcpy(&SendData[packlength],baseInfo.LastToEMA,7);
	packlength += 7;
	
	SendData[packlength++] = baseInfo.InvertersNum/256;
	SendData[packlength++] = baseInfo.InvertersNum%256;
	
	SendData[packlength++] = baseInfo.LastInvertersNum/256;
	SendData[packlength++] = baseInfo.LastInvertersNum%256;
	
	SendData[packlength++] = '0';
	SendData[packlength++] = '0';
	SendData[packlength++] = '9';
	sprintf(&SendData[packlength],"%s_%s_%s",ECU_VERSION,MAJORVERSION,MINORVERSION);
	packlength += 9;
	
	SendData[packlength++] = '0';
	SendData[packlength++] = '0';
	SendData[packlength++] = '9';
	sprintf(&SendData[packlength],"Etc/GMT+8");
	packlength += 9;	
	
	memcpy(&SendData[packlength],baseInfo.MacAddress,6);
	packlength += 6;

	SendData[packlength++] = 'E';
	SendData[packlength++] = 'N';
	SendData[packlength++] = 'D';
	SendData[5] = (packlength/1000) + '0';
	SendData[6] = ((packlength/100)%10) + '0';
	SendData[7] = ((packlength/10)%10) + '0';
	SendData[8] = ((packlength)%10) + '0';
	SendData[packlength++] = '\n';
	SendToSocketA(SendData ,packlength,(char *)ID);
}

//02 COMMAND_POWERGENERATION		//逆变器发电数据请求  mapping :: 0x00 匹配  0x01 不匹配
void APP_Response_PowerGeneration(char mapping,unsigned char *ID,inverter_info *inverter,int VaildNum)
{
	int packlength = 0,index = 0;
	inverter_info *curinverter = inverter;
	memset(SendData,'\0',4096);	
	
	//匹配不成功
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS110015000201\n");
		packlength = 16;
		SendToSocketA(SendData ,packlength,(char *)ID);
		return ;
	}
	
	//匹配成功 
	if(ecu.had_data_broadcast_time[0] == '\0')
	{
		sprintf(SendData,"APS110015000202\n");
		packlength = 16;
		SendToSocketA(SendData ,packlength,(char *)ID);
		return ;
	}
	
	//拼接需要发送的报文
	sprintf(SendData,"APS110000000200");
	packlength = 15;
	
	SendData[packlength++] = VaildNum/256;
	SendData[packlength++] = VaildNum%256;
	memcpy(&SendData[packlength],ecu.had_data_broadcast_time,14);
	packlength += 14;
	
	for(index=0; (index<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); index++, curinverter++)
	{
		//UID
		SendData[packlength++] = ((curinverter->id[0]-'0') << 4) + (curinverter->id[1]-'0');
		SendData[packlength++] = ((curinverter->id[2]-'0') << 4) + (curinverter->id[3]-'0');
		SendData[packlength++] = ((curinverter->id[4]-'0') << 4) + (curinverter->id[5]-'0');
		SendData[packlength++] = ((curinverter->id[6]-'0') << 4) + (curinverter->id[7]-'0');
		SendData[packlength++] = ((curinverter->id[8]-'0') << 4) + (curinverter->id[9]-'0');
		SendData[packlength++] = ((curinverter->id[10]-'0') << 4)+ (curinverter->id[11]-'0');
		
		//电网频率
		SendData[packlength++] = (int)(curinverter->gf * 10) / 256;
		SendData[packlength++] = (int)(curinverter->gf * 10) % 256;

		//机内温度
		SendData[packlength++] = (curinverter->it + 100) /256;
		SendData[packlength++] = (curinverter->it + 100) %256;		
		
		//逆变器功率  A 
		SendData[packlength++] = curinverter->op / 256;
		SendData[packlength++] = curinverter->op % 256;;
			
		//电网电压    A
		SendData[packlength++] = curinverter->gv / 256;
		SendData[packlength++] = curinverter->gv % 256;
		
		//逆变器功率  B 
		SendData[packlength++] = curinverter->opb / 256;
		SendData[packlength++] = curinverter->opb % 256;
			
		//电网电压    B
		SendData[packlength++] = curinverter->gvb / 256;
		SendData[packlength++] = curinverter->gvb % 256;
		
	}
	
	SendData[packlength++] = 'E';
	SendData[packlength++] = 'N';
	SendData[packlength++] = 'D';
	
	SendData[5] = (packlength/1000) + '0';
	SendData[6] = ((packlength/100)%10) + '0';
	SendData[7] = ((packlength/10)%10) + '0';
	SendData[8] = ((packlength)%10) + '0';
	SendData[packlength++] = '\n';
	SendToSocketA(SendData ,packlength,(char *)ID);
}

//03 COMMAND_POWERCURVE					//功率曲线请求   mapping :: 0x00 匹配  0x01 不匹配   data 表示日期
void APP_Response_PowerCurve(char mapping,unsigned char *ID,char * date)
{
	int packlength = 0,length = 0;
	memset(SendData,'\0',4096);	
	
	//匹配不成功
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS110015000301\n");
		packlength = 16;
		SendToSocketA(SendData ,packlength,(char *)ID);
		return ;
	}

	//拼接需要发送的报文
	sprintf(SendData,"APS110015000300");
	packlength = 15;
	
	read_system_power(date,&SendData[13],&length);
	packlength += length;
	
	SendData[packlength++] = 'E';
	SendData[packlength++] = 'N';
	SendData[packlength++] = 'D';
	
	SendData[5] = (packlength/1000) + '0';
	SendData[6] = ((packlength/100)%10) + '0';
	SendData[7] = ((packlength/10)%10) + '0';
	SendData[8] = ((packlength)%10) + '0';
	SendData[packlength++] = '\n';
	
	SendToSocketA(SendData ,packlength,(char *)ID);
}

//04 COMMAND_GENERATIONCURVE		//发电量曲线请求    mapping :: 0x00 匹配  0x01 不匹配  
void APP_Response_GenerationCurve(char mapping,unsigned char *ID,char request_type)
{
	int packlength = 0,length = 0,len_body = 0;
	char date_time[15] = { '\0' };
	memset(SendData,'\0',4096);	
	apstime(date_time);
	//匹配不成功
	if(mapping == 0x01)
	{
		sprintf(SendData,"APS110015000401\n");
		packlength = 16;
		SendToSocketA(SendData ,packlength,(char *)ID);
		return ;
	}

	sprintf(SendData,"APS110015000400");
	packlength = 15;
	
	//拼接需要发送的报文
	if(request_type == 0x00)
	{//最近一周
		SendData[packlength++] = '0';
		SendData[packlength++] = '0';
		
		read_weekly_energy(date_time, &SendData[packlength],&len_body);
		packlength += len_body;
		
	}else if(request_type == 0x01)
	{//最近一个月
		SendData[packlength++] = '0';
		SendData[packlength++] = '1';
		read_monthly_energy(date_time, &SendData[packlength],&len_body);
		packlength += len_body;
		
	}else if(request_type == 0x02)
	{//最近一年
		SendData[packlength++] = '0';
		SendData[packlength++] = '2';
		read_yearly_energy(date_time, &SendData[packlength],&len_body);
		packlength += len_body;
		
	}

	SendData[packlength++] = 'E';
	SendData[packlength++] = 'N';
	SendData[packlength++] = 'D';
	
	SendData[5] = (packlength/1000) + '0';
	SendData[6] = ((packlength/100)%10) + '0';
	SendData[7] = ((packlength/10)%10) + '0';
	SendData[8] = ((packlength)%10) + '0';
	SendData[packlength++] = '\n';
	
	SendToSocketA(SendData ,packlength,(char *)ID);
}

//05 COMMAND_REGISTERID 				//逆变器ID注册请求 		mapping :: 0x00 匹配  0x01 不匹配  
void APP_Response_RegisterID(char mapping,unsigned char *ID)
{
	int packlength = 0;
	memset(SendData,'\0',4096);	
	
	//拼接需要发送的报文
	sprintf(SendData,"APS1100150005%02d\n",mapping);
	packlength = 16;
	
	SendToSocketA(SendData ,packlength,(char *)ID);
}

//06 COMMAND_SETTIME						//时间设置请求			mapping :: 0x00 匹配  0x01 不匹配  
void APP_Response_SetTime(char mapping,unsigned char *ID)
{
	int packlength = 0;
	memset(SendData,'\0',4096);	
	
	//拼接需要发送的报文
	sprintf(SendData,"APS1100150006%02d\n",mapping);
	packlength = 16;
	
	SendToSocketA(SendData ,packlength,(char *)ID);
}

//07 COMMAND_SETWIREDNETWORK		//有线网络设置请求			mapping :: 0x00 匹配  0x01 不匹配  
void APP_Response_SetWiredNetwork(char mapping,unsigned char *ID)
{
	int packlength = 0;
	memset(SendData,'\0',4096);	
	
	//拼接需要发送的报文
	sprintf(SendData,"APS1100150007%02d\n",mapping);
	packlength = 16;
	
	SendToSocketA(SendData ,packlength,(char *)ID);
}

//08 COMMAND_SETWIFI 						//无线网络设置请求			mapping :: 0x00 匹配  0x01 不匹配  
void APP_Response_SetWifi(char mapping,unsigned char *ID)
{
	int packlength = 0;
	memset(SendData,'\0',4096);	
	
	//拼接需要发送的报文
	sprintf(SendData,"APS1100150008%02d\n",mapping);
	packlength = 16;
	
	SendToSocketA(SendData ,packlength,(char *)ID);
}

//09 COMMAND_SEARCHWIFISTATUS		//无线网络连接状态请求			mapping :: 0x00 匹配  0x01 不匹配  
void APP_Response_SearchWifiStatus(char mapping,unsigned char *ID)
{
	int packlength = 0;
	memset(SendData,'\0',4096);	
	
	//拼接需要发送的报文
	sprintf(SendData,"APS1100150009%02d\n",mapping);
	packlength = 16;
	
	SendToSocketA(SendData ,packlength,(char *)ID);
}

//10 COMMAND_SETWIFIPASSWD			//AP密码设置请求			mapping :: 0x00 匹配  0x01 不匹配  
void APP_Response_SetWifiPasswd(char mapping,unsigned char *ID)
{
	int packlength = 0;
	memset(SendData,'\0',4096);	
	
	//拼接需要发送的报文
	sprintf(SendData,"APS1100150010%02d\n",mapping);
	packlength = 16;
	
	SendToSocketA(SendData ,packlength,(char *)ID);
}

