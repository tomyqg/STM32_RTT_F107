/*
 * 本文件的主要功能是设置单个逆变器的保护参数。同时兼容新的13项保护参数和老的5项参数。
 * Created by Zhyf
 * Created on 2014/08/22
 */
/*********************************************************************
setpropi表格参数：
id,parameter_name, parameter_value,set_flag         primary key(id, parameter_name)
**********************************************************************/

/*****************************************************************************/
/*  File      : set_protection_parameters_inverter.c                         */
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
#include <stdio.h>
#include <stdlib.h>
#include "variation.h"
#include <string.h>
#include "debug.h"
#include "zigbee.h"
#include "myfile.h"
#include "file.h"
#include "set_protection_parameters.h"
#include "set_protection_parameters_inverter.h"
#include "rtthread.h"
#include <rthw.h>

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern rt_mutex_t record_data_lock;
extern inverter_info inverter[MAXINVERTERCOUNT];

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int set_protection_yc600_one(int shortaddr,int order,int data,int num)
{

	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = order;			//CMD
	sendbuff[4] = 0x00;
	sendbuff[5] = 0x00;
	sendbuff[6] = 0x00;

	if(num==3)
	{
		sendbuff[4]=data/65536;
		sendbuff[5] = (data%65536/256);
		sendbuff[6] = (data%256);
	}
	else if(num==2)
	{
		sendbuff[4] = (data/256);
		sendbuff[5] = (data%256);
	}
	else if(num==1)
		sendbuff[4] = data;
	else ;
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg(ECU_DBG_MAIN,"Set yc600 single", (char *)sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 13);
	rt_hw_ms_delay(200);

	return 0;

}

/*读取设置的参数值和标志位*/
int get_value_flag_one(char *id, char *para_name, char *value)
{
	int index = 0,flag = 0;
	FILE *fp;
	char data[200];
	char splitdata[4][32];
	inverter_info *curinverter = inverter;
	fp = fopen("/home/data/setpropi", "r");
	if(fp)
	{
		memset(data,0x00,200);
		
		while(NULL != fgets(data,200,fp))
		{
			memset(splitdata,0x00,4*32);
			splitString(data,splitdata);
			if(1 == atoi(splitdata[3]))
			{
				flag = 1;
				strcpy(id, splitdata[0]);
				strcpy(para_name, splitdata[1]);
				strcpy(value, splitdata[2]);
				break;
			}
			memset(data,0x00,200);
		}
		fclose(fp);
	}
	if(1 != flag)
	{
		return -1;
	}
	curinverter = inverter;
	for(index=0; (index<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); index++, curinverter++)			//???????
	{
		if(!strcmp(curinverter->id,id))
		{
			return curinverter->shortaddr;
		}
	}
	return 0;
}

int get_under_voltage_slow_one_yc1000(char *id)
{

	int under_voltage_slow=0xFFFF;
	float temp;

	FILE *fp;
	char data[200];
	char splitdata[4][32];
	fp = fopen("/home/data/setpropi", "r");
	if(fp)
	{
		memset(data,0x00,200);
		
		while(NULL != fgets(data,200,fp))
		{
			memset(splitdata,0x00,4*32);
			splitString(data,splitdata);
			if((!strcpy(splitdata[0],id))&&(!strcpy(splitdata[1],"under_voltage_slow")))
			{
				temp = atof(splitdata[2]) * 11614.45;
				if((temp-(int)temp)>=0.5)
					under_voltage_slow = temp + 1;
				else
					under_voltage_slow = temp;

				break;
			}
			memset(data,0x00,200);
		}
		fclose(fp);
	}
	
	return under_voltage_slow;
}

int get_over_voltage_slow_one_yc1000(char *id)
{
	int over_voltage_slow=0xFFFF;
	float temp;

	FILE *fp;
	char data[200];
	char splitdata[4][32];
	fp = fopen("/home/data/setpropi", "r");
	if(fp)
	{
		memset(data,0x00,200);
		
		while(NULL != fgets(data,200,fp))
		{
			memset(splitdata,0x00,4*32);
			splitString(data,splitdata);
			if((!strcpy(splitdata[0],id))&&(!strcpy(splitdata[1],"over_voltage_slow")))
			{
				temp = atof(splitdata[2]) * 11614.45;
				if((temp-(int)temp)>=0.5)
					over_voltage_slow = temp + 1;
				else
					over_voltage_slow = temp;

				break;
			}
			memset(data,0x00,200);
		}
		fclose(fp);
	}
	
	return over_voltage_slow;
}

char get_under_frequency_slow_one_yc1000(char *id)
{
	char under_frequency_slow=0xFF;
	FILE *fp;
	char data[200];
	char splitdata[4][32];
	fp = fopen("/home/data/setpropi", "r");
	if(fp)
	{
		memset(data,0x00,200);
		
		while(NULL != fgets(data,200,fp))
		{
			memset(splitdata,0x00,4*32);
			splitString(data,splitdata);
			if((!strcpy(splitdata[0],id))&&(!strcpy(splitdata[1],"under_frequency_slow")))
			{
				under_frequency_slow = (256000/atoi(splitdata[2]));
				break;
			}
			memset(data,0x00,200);
		}
		fclose(fp);
	}
	
	return under_frequency_slow;
}

char get_over_frequency_slow_one_yc1000(char *id)
{

	char over_frequency_slow=0xFF;
	FILE *fp;
	char data[200];
	char splitdata[4][32];
	fp = fopen("/home/data/setpropi", "r");
	if(fp)
	{
		memset(data,0x00,200);
		
		while(NULL != fgets(data,200,fp))
		{
			memset(splitdata,0x00,4*32);
			splitString(data,splitdata);
			if((!strcpy(splitdata[0],id))&&(!strcpy(splitdata[1],"over_frequency_slow")))
			{
				over_frequency_slow = (256000/atoi(splitdata[2]));
				break;
			}
			memset(data,0x00,200);
		}
		fclose(fp);
	}

	return over_frequency_slow;
}



int get_grid_recovery_time_one(struct inverter_info_t *firstinverter, char *id)
{
	int recovery_time=300, recovery_time_result=-1, i, flag=0;
	struct inverter_info_t *inverter = firstinverter;
	FILE *fp;
	char data[200];
	char splitdata[22][32];
	fp = fopen("/home/data/setpropi", "r");
	if(fp)
	{
		memset(data,0x00,200);
		
		while(NULL != fgets(data,200,fp))
		{
			memset(splitdata,0x00,4*32);
			splitString(data,splitdata);
			if((!strcpy(splitdata[0],id))&&(!strcpy(splitdata[1],"grid_recovery_time")))
			{
				flag = 1;
				recovery_time = atof(splitdata[2]);
				break;
			}
			memset(data,0x00,200);
		}
		fclose(fp);
	}

	if(!flag){
	fp = fopen("/home/data/setpropa", "r");
	if(fp)
	{
		memset(data,0x00,200);
		
		while(NULL != fgets(data,200,fp))
		{
			memset(splitdata,0x00,3*32);
			splitString(data,splitdata);
			if(!strcpy(splitdata[0],"grid_recovery_time"))
			{
				flag = 1;
				recovery_time = atoi(splitdata[1]);
				break;
			}
			memset(data,0x00,200);
		}
		fclose(fp);
	}
	}

	if(!flag){
		fp = fopen("/home/data/proparas", "r");
		if(fp)
		{
			memset(data,0x00,200);
			
			while(NULL != fgets(data,200,fp))
			{
				memset(splitdata,0x00,22*32);
				splitString(data,splitdata);   
				if(0 != atoi(splitdata[14]))
				{
					flag = 1;
					recovery_time = atoi(splitdata[14]);
					break;
				}
				memset(data,0x00,200);
			}
			fclose(fp);
		}
	}

	if(!flag){
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++){
			recovery_time_result = get_recovery_time_from_inverter(inverter);
			if(-1 != recovery_time_result)
				return recovery_time_result;
		}
	}

	return recovery_time;
}


int set_regulated_dc_working_point_yc1000_one(int shortaddr, char *value)  			//直流稳压设置
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;


	data = (int)(atof(value) * 4096/82.5);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x60;			//CMD
	sendbuff[4] = data%65536/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg(ECU_DBG_MAIN,"Set dc voltage stabilization one", (char *)sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 13);
	rt_hw_ms_delay(200);

	return 0;
}

int set_undervoltage_slow_yc1000_one(int shortaddr, char *value)  //内围电压下限值
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;


	data = (int)(atof(value) * 11614.45);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x63;			//LENGTH
	sendbuff[4] = data/65536;			//DATA
	sendbuff[5] = data%65536/256;		//DATA
	sendbuff[6] = data%256;		//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg(ECU_DBG_MAIN,"Set undervoltage slow one", (char *)sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 13);
	rt_hw_ms_delay(200);

	return 0;
}

int set_overvoltage_slow_yc1000_one(int shortaddr, char *value)  //内围电压上限值
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;


	data = (int)(atof(value) * 11614.45);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x64;			//LENGTH
	sendbuff[4] = data/65536;			//DATA
	sendbuff[5] = data%65536/256;		//DATA
	sendbuff[6] = data%256;		//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg(ECU_DBG_MAIN,"Set overvoltage slow one", (char *)sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 13);
	rt_hw_ms_delay(200);

	return 0;
}

int set_overvoltage_fast_yc1000_one(int shortaddr, char *value)		//外围电压上限值
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = (int)(atof(value) * 16500);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x62;			//LENGTH
	sendbuff[4] = data/65536;			//DATA
	sendbuff[5] = data%65536/256;		//DATA
	sendbuff[6] = data%256;		//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg(ECU_DBG_MAIN,"Set overvoltage fast one", (char *)sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 13);
	rt_hw_ms_delay(200);

	return 0;
}

int set_undervoltage_fast_yc1000_one(int shortaddr, char *value)		//外围电压下限值
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = (int)(atof(value) * 16500);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x61;			//LENGTH
	sendbuff[4] = data/65536;			//DATA
	sendbuff[5] = data%65536/256;		//DATA
	sendbuff[6] = data%256;		//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg(ECU_DBG_MAIN,"Set undervoltage fast one", (char *)sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 13);
	rt_hw_ms_delay(200);

	return 0;
}


int set_underfrequency_fast_yc1000_one(int shortaddr, char *value)				//外围频率下限
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = (256000/atof(value));


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x67;			//LENGTH
	sendbuff[4] = data/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg(ECU_DBG_MAIN,"Set underfrequency fast one", (char *)sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 13);
	rt_hw_ms_delay(200);

	return 0;
}


int set_overfrequency_fast_yc1000_one(int shortaddr, char *value)				//外围频率上限
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = (256000/atof(value));


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x68;			//LENGTH
	sendbuff[4] = data/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg(ECU_DBG_MAIN,"Set overfrequency fast one", (char *)sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 13);
	rt_hw_ms_delay(200);

	return 0;
}


int set_underfrequency_slow_yc1000_one(int shortaddr, char *value)				//内围频率下限
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = (256000/atof(value));


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x69;			//LENGTH
	sendbuff[4] = data/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg(ECU_DBG_MAIN,"Set underfrequency slow one", (char *)sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 13);
	rt_hw_ms_delay(200);

	return 0;
}


int set_overfrequency_slow_yc1000_one(int shortaddr, char *value)				//内围频率上限
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = (256000/atof(value));


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//CMD
	sendbuff[3] = 0x6A;			//LENGTH
	sendbuff[4] = data/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg(ECU_DBG_MAIN,"Set overfrequency slow one", (char *)sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 13);
	rt_hw_ms_delay(200);

	return 0;
}



int set_voltage_triptime_fast_yc1000_one(int shortaddr, char *value)				//外围电压延迟保护时间
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = atof(value)*50;


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x65;			//CMD
	sendbuff[4] = data/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg(ECU_DBG_MAIN,"Set voltage triptime fast one", (char *)sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 13);
	rt_hw_ms_delay(200);

	return 0;
}


int set_voltage_triptime_slow_yc1000_one(int shortaddr, char *value)				//内围电压延迟保护时间
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = atof(value)*50;


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x66;			//CMD
	sendbuff[4] = data/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg(ECU_DBG_MAIN,"Set voltage triptime slow one", (char *)sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 13);
	rt_hw_ms_delay(200);

	return 0;
}


int set_frequency_triptime_fast_yc1000_one(int shortaddr, char *value)				//外围频率延迟保护时间
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = atof(value)*50;


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x6B;			//CMD
	sendbuff[4] = data/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg(ECU_DBG_MAIN,"Set frequency triptime fast one", (char *)sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 13);
	rt_hw_ms_delay(200);

	return 0;
}



int set_frequency_triptime_slow_yc1000_one(int shortaddr, char *value)				//内围频率延迟保护时间
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = atof(value)*50;


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x6C;			//CMD
	sendbuff[4] = data/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg(ECU_DBG_MAIN,"Set frequency triptime slow one", (char *)sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 13);
	rt_hw_ms_delay(200);

	return 0;
}


int set_grid_recovery_time_yc1000_one(int shortaddr, char *value)						//并网恢复时间设置
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = atof(value);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x6D;			//CMD
	sendbuff[4] = data/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid


	printhexmsg(ECU_DBG_MAIN,"Set grid recovery time one", (char *)sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 13);
	rt_hw_ms_delay(200);

	return 0;
}


int set_under_voltage_stage_2_yc1000_one(int shortaddr, char *value)  			//内内围电压设置
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;


	data = (int)(atof(value) * 11614.45);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x6E;			//CMD
	sendbuff[4] = data/65536;			//DATA
	sendbuff[5] = data%65536/256;		//DATA
	sendbuff[6] = data%256;		//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg(ECU_DBG_MAIN,"Set voltage slow2 one", (char *)sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 13);
	rt_hw_ms_delay(200);

	return 0;
}



int set_voltage_3_clearance_time_yc1000_one(int shortaddr, char *value)				//内内围电压延迟保护时间
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = atof(value)*50;


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x6F;			//CMD
	sendbuff[4] = data/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg(ECU_DBG_MAIN,"Set voltage triptime slow2 one", (char *)sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 13);
	rt_hw_ms_delay(200);

	return 0;
}

int set_start_time_yc1000_one(int shortaddr, char *value)				//直流启动时间
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = atof(value);


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x4D;			//CMD
	sendbuff[4] = data/256;		//DATA
	sendbuff[5] = data%256;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg(ECU_DBG_MAIN,"Set DC startime ", (char *)sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 13);
	rt_hw_ms_delay(200);

	return 0;
}

int set_active_antiisland_time_yc1000_one(int shortaddr, char *value)				//直流启动时间
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = (int)(atof(value)*2.5);


	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x3E;			//CMD
	sendbuff[4] = data;		//DATA
	sendbuff[5] = 0x00;		//DATA
	sendbuff[6] = 0x00;			//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg(ECU_DBG_MAIN,"Set active antiisland time ", (char *)sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 13);
	rt_hw_ms_delay(200);

	return 0;
}


/*设置完后，清除设置标志*/
int clear_flag_one(char *id, char *para_name)					//设置后清除数据库中参数的设置标志
{
	char str[50];
	sprintf(str,"%s,%s",id,para_name);

	//删除para_name所在行
	delete_line("/home/data/setpropi","/home/data/setpropi.t",str,strlen(str));

	return 0;
}

/*给逆变器设置保护参数，并且读取逆变器设置后的保护参数*/
int set_protection_paras_one(inverter_info *firstinverter)
{
	char inverter_id[16]={'\0'};
	char para_name[64];
	char value[16];
	int shortaddr,model;
	int data = 0;
	int bb_or_b1_single;
	int i = 0,j=0;
	inverter_info *curinverter = firstinverter;
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++,curinverter++)
	{
		curinverter->inverterstatus.response_protection_paras_one = 0;
	}
	while(1)
	{
		memset(inverter_id,'\0',16);
		memset(para_name,'\0',64);
		memset(value,'\0',16);
		shortaddr = 0;
		model = 0;
		bb_or_b1_single = 0;
		shortaddr = get_value_flag_one(inverter_id, para_name, value);
		if(shortaddr == -1)
		{
			break;
		}else if(shortaddr==0){
			clear_flag_one(inverter_id, para_name);
			continue;
		}
		
		curinverter = firstinverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++,curinverter++)
		{
			if(!memcmp(inverter_id,curinverter->id,12))
			{
				shortaddr = curinverter->shortaddr;
				model = curinverter->model;
				bb_or_b1_single =curinverter->inverterstatus.deputy_model;
				curinverter->inverterstatus.response_protection_paras_one = 1;
				break;
			}
		}
		
		if((shortaddr>0)&&((model==5)||(model==6)||(model==7)))
		{
			if(!strcmp("under_voltage_fast", para_name))
			{
				if(model==7){
					data = (int)(atof(value) * 1.3277);
					if(bb_or_b1_single==0)
						continue;
					else if(bb_or_b1_single==2)
						data = (int)(atof(value) * 1.33);
					set_protection_yc600_one(shortaddr,0x61,data,2);
				}
				else
					set_undervoltage_fast_yc1000_one(shortaddr, value);
				clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("over_voltage_fast", para_name))
			{
				if(model==7){
					data = (int)(atof(value) * 1.3277);
					if(bb_or_b1_single==0)
						continue;
					else if(bb_or_b1_single==2)
						data = (int)(atof(value) * 1.33);
					set_protection_yc600_one(shortaddr,0x62,data,2);
				}
				else
					set_overvoltage_fast_yc1000_one(shortaddr, value);
				clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("under_voltage_stage_2", para_name))
			{
				if(model==7){
					data = (int)(atof(value) * 1.3277);
					if(bb_or_b1_single==0)
						continue;
					else if(bb_or_b1_single==2)
						data = (int)(atof(value) * 1.33);
					set_protection_yc600_one(shortaddr,0x83,data,2);
				}
				else{
					set_undervoltage_slow_yc1000_one(shortaddr, value);
					set_undervoltage_slow_yc1000_one_5(shortaddr, value, inverter_id);
					set_under_voltage_stage_3_yc1000_one(shortaddr,value);
				}
				clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("over_voltage_slow", para_name))
			{
				if(model==7){
					data = (int)(atof(value) * 1.3277);
					if(bb_or_b1_single==0)
						continue;
					else if(bb_or_b1_single==2)
						data = (int)(atof(value) * 1.33);
					set_protection_yc600_one(shortaddr,0x64,data,2);
				}
				else{
					set_overvoltage_slow_yc1000_one(shortaddr, value);
					set_overvoltage_slow_yc1000_one_5(shortaddr, value, inverter_id);
				}
				clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("under_frequency_fast", para_name))
			{
				if(model==7)
				{
					data = (int)(50000000/atof(value));
					set_protection_yc600_one(shortaddr,0x68,data,3);
				}
				else
					set_underfrequency_fast_yc1000_one(shortaddr, value);
				clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("over_frequency_fast", para_name))
			{
				if(model==7)
				{
					data = (int)(50000000/atof(value));
					set_protection_yc600_one(shortaddr,0x67,data,3);
				}
				else
					set_overfrequency_fast_yc1000_one(shortaddr, value);
				clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("under_frequency_slow", para_name))
			{
				if(model==7){
					data = (int)(50000000/atof(value));
					set_protection_yc600_one(shortaddr,0x6A,data,3);
				}
				else{
					set_underfrequency_slow_yc1000_one(shortaddr, value);
					set_underfrequency_slow_yc1000_one_5(shortaddr, value, inverter_id);
				}
				clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("over_frequency_slow", para_name))
			{
				if(model==7){
					data = (int)(50000000/atof(value));
					set_protection_yc600_one(shortaddr,0x69,data,3);
				}
				else{
					set_overfrequency_slow_yc1000_one(shortaddr, value);
					set_overfrequency_slow_yc1000_one_5(shortaddr, value, inverter_id);
				}
				clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("voltage_triptime_fast", para_name))
			{
				if(model==7){
					data = (int)(atof(value)*100);
					set_protection_yc600_one(shortaddr,0x65,data,2);
				}
				else
					set_voltage_triptime_fast_yc1000_one(shortaddr, value);
					clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("voltage_triptime_slow", para_name))
			{
				if(model==7)
				{
					data = (int)(atof(value)*100);
					set_protection_yc600_one(shortaddr,0x66,data,2);
				}
				else
					set_voltage_triptime_slow_yc1000_one(shortaddr, value);
				clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("frequency_triptime_fast", para_name))
			{
				if(model==7){
					data = (int)(atof(value)*100);
					set_protection_yc600_one(shortaddr,0x6B,data,2);
				}
				else
					set_frequency_triptime_fast_yc1000_one(shortaddr, value);
				clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("frequency_triptime_slow", para_name))
			{
				if(model==7){
					data = (int)(atof(value)*100);
					set_protection_yc600_one(shortaddr,0x6C,data,2);
				}
				else
					set_frequency_triptime_slow_yc1000_one(shortaddr, value);
				clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("grid_recovery_time", para_name))
			{
				if(model==7){
					data = (int)(atof(value)*100);
					set_protection_yc600_one(shortaddr,0x6D,data,2);
				}
				else{
					set_grid_recovery_time_yc1000_one(shortaddr,value);
					set_grid_recovery_time_yc1000_one_5(shortaddr, value, inverter_id);
				}
				clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("regulated_dc_working_point", para_name))
			{
				if(model==7){
					data = (int)(atof(value) * 4096/82.5);
					set_protection_yc600_one(shortaddr,0x60,data,2);
				}
				else
					set_regulated_dc_working_point_yc1000_one(shortaddr,value);
				clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("under_voltage_slow", para_name))
			{
				if(model==7){
					data = (int)(atof(value) * 1.3277);
					if(bb_or_b1_single==0)
						continue;
					else if(bb_or_b1_single==2)
						data = (int)(atof(value) * 1.33);
					set_protection_yc600_one(shortaddr,0x6E,data,2);
				}
				else
					set_under_voltage_stage_2_yc1000_one(shortaddr,value);
				clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("voltage_3_clearance_time", para_name))
			{
				if(model==7){
					data = (int)(atof(value)*100);
					set_protection_yc600_one(shortaddr,0x6F,data,2);
				}
				else
					set_voltage_3_clearance_time_yc1000_one(shortaddr,value);
				clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("start_time", para_name))
			{
				if(model==7){
					data = (int)(atof(value)*100);
					set_protection_yc600_one(shortaddr,0x4D,data,2);
				}
				else
					set_start_time_yc1000_one(shortaddr, value);
				clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("active_antiisland_time", para_name))
			{
				if(model==7){
					data = (int)(atof(value)*2.5);
					set_protection_yc600_one(shortaddr,0x3E,data,1);
				}
				else
					set_active_antiisland_time_yc1000_one(shortaddr, value);
				clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
			}
			else if(!strcmp("power_factor", para_name))
			{
				if(model==7){
					data=(int)(atof(value));
					set_protection_yc600_one(shortaddr,0x4F,data,1);
				}
			}
			else if(!strcmp("relay_protect", para_name))
			{
				if(model==7){
					data=(int)(atof(value));
					if(data==0)
						set_protection_yc600_one(shortaddr,0x4A,0,0);
					else if(data==1)
						set_protection_yc600_one(shortaddr,0x4B,0,0);
					else ;
						//set_voltage_3_clearance_time_yc1000(value);
					clear_flag_one(inverter_id,para_name);					//设置后清除数据库中参数的设置标志
				}
			}
			else 
				clear_flag_one(inverter_id,para_name);
		
		}
		
	}

	curinverter = firstinverter;
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++,curinverter++)
	{
		if(curinverter->inverterstatus.response_protection_paras_one == 1)
		{
			for(j=0; j<3; j++){
				if(!get_parameters_from_inverter(curinverter))
					break;
				}
		}
	}
	return 0;
	
}

/*读取每一台逆变器的保护参数*/
int read_protection_parameters_one(inverter_info *firstinverter, char *id)
{
	int i, j;
	inverter_info *inverter = firstinverter;
	char set_result[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL];
	memset(set_result, '\0', sizeof(set_result));

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++){
		if(!strcmp(inverter->id, id)){
			for(j=0; j<3; j++){
				if(!get_parameters_from_inverter(inverter))
					break;
			}
		}
	}

	return 0;
}

/*设置逆变器的保护参数*/
int set_protection_parameters_inverter_one(struct inverter_info_t *firstinverter)
{
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	set_protection_paras_one(firstinverter);
	rt_mutex_release(record_data_lock);
	return 0;
}

//读取单点设置的交流保护参数（5项）
int read_protection_parameters_one_5(char *id, char *data)
{
	int grid_recovery_time = 0;
	int under_voltage_slow = 0;
	int over_voltage_slow = 0;
	int under_frequency_slow = 0;
	int over_frequency_slow = 0;

	//查询数据
	FILE *fp;
	char linedata[200];
	char splitdata[22][32];
	fp = fopen("/home/data/proparas", "r");
	if(fp)
	{
		memset(linedata,0x00,200);
		
		while(NULL != fgets(linedata,200,fp))
		{
			memset(splitdata,0x00,22*32);
			splitString(data,splitdata);
			if(!strcpy(splitdata[0],id))
			{
				under_voltage_slow = (int)(atof(splitdata[4]) * 11614.45);

				over_voltage_slow = (int)(atof(splitdata[5]) * 11614.45);

				under_frequency_slow = (256000/atof(splitdata[8]));

				over_frequency_slow = (256000/atof(splitdata[9]));

				grid_recovery_time = (int)atof(splitdata[14]);
				break;
			}

			memset(linedata,0x00,200);
		}
		fclose(fp);
	}
	
	//拼接数据
	data[0] = under_voltage_slow/65535;
	data[1] = under_voltage_slow%65535/256;
	data[2] = under_voltage_slow%256;

	data[3] = over_voltage_slow/65535;
	data[4] = over_voltage_slow%65535/256;
	data[5] = over_voltage_slow%256;

	data[6] = under_frequency_slow/256;
	data[7] = under_frequency_slow%256;

	data[8] = over_frequency_slow/256;
	data[9] = over_frequency_slow%256;

	data[10] = grid_recovery_time/256;
	data[11] = grid_recovery_time%256;

	data[12] = 0x00;
	data[13] = 0x00;
	data[14] = 0x00;

	return 0;
}

//内内围电压下限值(5项)
int set_undervoltage_slow_yc1000_one_5(int shortaddr, char *value, char *inverterid)
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int under_voltage_slow;
	char protection_data[16] = {'\0'};

	read_protection_parameters_one_5(inverterid, protection_data);
	under_voltage_slow = (int)(atof(value) * 11614.45);

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x10;
	sendbuff[3] = 0xCC;
	sendbuff[4] = under_voltage_slow/65536;
	sendbuff[5] = under_voltage_slow%65536/256;
	sendbuff[6] = under_voltage_slow%256;
	sendbuff[7] = protection_data[3];
	sendbuff[8] = protection_data[4];
	sendbuff[9] = protection_data[5];
	sendbuff[10] = protection_data[6];
	sendbuff[11] = protection_data[7];
	sendbuff[12] = protection_data[8];
	sendbuff[13] = protection_data[9];
	sendbuff[14] = protection_data[10];
	sendbuff[15] = protection_data[11];
	sendbuff[16] = protection_data[12];
	sendbuff[17] = protection_data[13];
	sendbuff[18] = protection_data[14];

	for (i=2; i<19; i++) {
		check = check + sendbuff[i];
	}

	sendbuff[19] = check >> 8;
	sendbuff[20] = check;
	sendbuff[21] = 0xFE;
	sendbuff[22] = 0xFE;

	printhexmsg(ECU_DBG_MAIN,"Set undervoltage slow one (5)", (char *)sendbuff, 23);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 23);
	rt_hw_ms_delay(200);

	return 0;
}

//内围电压上限值(5项)
int set_overvoltage_slow_yc1000_one_5(int shortaddr, char *value, char *inverterid)
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int over_voltage_slow;
	char protection_data[16] = {'\0'};

	read_protection_parameters_one_5(inverterid, protection_data);
	over_voltage_slow = (int)(atof(value) * 11614.45);

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x10;
	sendbuff[3] = 0xCC;
	sendbuff[4] = protection_data[0];
	sendbuff[5] = protection_data[1];
	sendbuff[6] = protection_data[2];
	sendbuff[7] = over_voltage_slow/65536;
	sendbuff[8] = over_voltage_slow%65536/256;
	sendbuff[9] = over_voltage_slow%256;
	sendbuff[10] = protection_data[6];
	sendbuff[11] = protection_data[7];
	sendbuff[12] = protection_data[8];
	sendbuff[13] = protection_data[9];
	sendbuff[14] = protection_data[10];
	sendbuff[15] = protection_data[11];
	sendbuff[16] = protection_data[12];
	sendbuff[17] = protection_data[13];
	sendbuff[18] = protection_data[14];

	for(i=2; i<19; i++){
		check = check + sendbuff[i];
	}

	sendbuff[19] = check >> 8;
	sendbuff[20] = check;
	sendbuff[21] = 0xFE;
	sendbuff[22] = 0xFE;

	printhexmsg(ECU_DBG_MAIN,"Set overvoltage slow one (5)", (char *)sendbuff, 23);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 23);
	rt_hw_ms_delay(200);

	return 0;
}

//内围频率下限(5项)
int set_underfrequency_slow_yc1000_one_5(int shortaddr, char *value, char *inverterid)
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int under_frequency_slow;
	char protection_data[16] = {'\0'};

	read_protection_parameters_one_5(inverterid, protection_data);
	under_frequency_slow = (256000/atof(value));

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x10;
	sendbuff[3] = 0xCC;
	sendbuff[4] = protection_data[0];
	sendbuff[5] = protection_data[1];
	sendbuff[6] = protection_data[2];
	sendbuff[7] = protection_data[3];
	sendbuff[8] = protection_data[4];
	sendbuff[9] = protection_data[5];
	sendbuff[10] = under_frequency_slow/256;
	sendbuff[11] = under_frequency_slow%256;
	sendbuff[12] = protection_data[8];
	sendbuff[13] = protection_data[9];
	sendbuff[14] = protection_data[10];
	sendbuff[15] = protection_data[11];
	sendbuff[16] = protection_data[12];
	sendbuff[17] = protection_data[13];
	sendbuff[18] = protection_data[14];


	for (i=2; i<19; i++) {
		check = check + sendbuff[i];
	}

	sendbuff[19] = check >> 8;
	sendbuff[20] = check;
	sendbuff[21] = 0xFE;
	sendbuff[22] = 0xFE;

	printhexmsg(ECU_DBG_MAIN,"Set underfrequency slow one (5)", (char *)sendbuff, 23);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 23);
	rt_hw_ms_delay(200);

	return 0;
}

//内围频率上限(5项)
int set_overfrequency_slow_yc1000_one_5(int shortaddr, char *value, char *inverterid)
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int over_frequency_slow;
	char protection_data[16] = {'\0'};

	read_protection_parameters_one_5(inverterid, protection_data);
	over_frequency_slow = (256000/atof(value));

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x10;
	sendbuff[3] = 0xCC;
	sendbuff[4] = protection_data[0];
	sendbuff[5] = protection_data[1];
	sendbuff[6] = protection_data[2];
	sendbuff[7] = protection_data[3];
	sendbuff[8] = protection_data[4];
	sendbuff[9] = protection_data[5];
	sendbuff[10] = protection_data[6];
	sendbuff[11] = protection_data[7];
	sendbuff[12] = over_frequency_slow/256;
	sendbuff[13] = over_frequency_slow%256;
	sendbuff[14] = protection_data[10];
	sendbuff[15] = protection_data[11];
	sendbuff[16] = protection_data[12];
	sendbuff[17] = protection_data[13];
	sendbuff[18] = protection_data[14];

	for (i=2; i<19; i++) {
		check = check + sendbuff[i];
	}

	sendbuff[19] = check >> 8;
	sendbuff[20] = check;
	sendbuff[21] = 0xFE;
	sendbuff[22] = 0xFE;

	printhexmsg(ECU_DBG_MAIN,"Set overfrequency slow one (5)", (char *)sendbuff, 23);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 23);
	rt_hw_ms_delay(200);

	return 0;
}

//并网恢复时间设置(5项)
int set_grid_recovery_time_yc1000_one_5(int shortaddr, char *value, char *inverterid)
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int grid_recovery_time;
	char protection_data[16] = {'\0'};

	read_protection_parameters_one_5(inverterid, protection_data);
	grid_recovery_time = atof(value);

	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x10;
	sendbuff[3] = 0xCC;
	sendbuff[4] = protection_data[0];
	sendbuff[5] = protection_data[1];
	sendbuff[6] = protection_data[2];
	sendbuff[7] = protection_data[3];
	sendbuff[8] = protection_data[4];
	sendbuff[9] = protection_data[5];
	sendbuff[10] = protection_data[6];
	sendbuff[11] = protection_data[7];
	sendbuff[12] = protection_data[8];
	sendbuff[13] = protection_data[9];
	sendbuff[14] = grid_recovery_time/256;
	sendbuff[15] = grid_recovery_time%256;
	sendbuff[16] = protection_data[12];
	sendbuff[17] = protection_data[13];
	sendbuff[18] = protection_data[14];

	for (i=2; i<19; i++) {
		check = check + sendbuff[i];
	}

	sendbuff[19] = check >> 8;
	sendbuff[20] = check;
	sendbuff[21] = 0xFE;
	sendbuff[22] = 0xFE;

	printhexmsg(ECU_DBG_MAIN,"Set grid recovery time one (5)", (char *)sendbuff, 23);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 23);
	rt_hw_ms_delay(200);

	return 0;
}

//欠压门限3阶
int set_under_voltage_stage_3_yc1000_one(int shortaddr, char *value)
{
	unsigned char sendbuff[256]={'\0'};
	unsigned short check=0x00;
	int i;
	int data;

	data = (int)(atof(value) * 11614.45);

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0x83;			//CMD
	sendbuff[4] = data/65536;			//DATA
	sendbuff[5] = data%65536/256;		//DATA
	sendbuff[6] = data%256;		//DATA
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;

	for(i=2; i<9; i++){
		check = check + sendbuff[i];
	}

	sendbuff[9] = check >> 8;	//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;		//ccuid
	sendbuff[12] = 0xFE;		//ccuid

	printhexmsg(ECU_DBG_MAIN,"Set under voltage (stage 3) one", (char *)sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 13);
	rt_hw_ms_delay(200);

	return 0;
}
