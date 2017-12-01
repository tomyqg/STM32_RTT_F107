/*****************************************************************************/
/*  File      : setpower.c                                                   */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-03-05 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/


/*
power表格字段：
id,limitedpower,limitedresult,stationarypower,stationaryresult,flag
*/
/*
power表格字段：
maxpower,fixedpower,sysmaxpower
*/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "variation.h"
#include "zigbee.h"
#include <string.h>
#include "debug.h"
#include "myfile.h"
#include "file.h"
#include "setpower.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern rt_mutex_t record_data_lock ;
extern ecu_info ecu;
/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int getpower(inverter_info *inverter, int *limitedpower, int *stationarypower)	//读取逆变器的最大功率和固定功率
{
	char linedata[100] = "\0";
	char splitdata[6][32];
	int ret;
	
	//读取所在ID行
	ret = read_line("/home/data/power",linedata,inverter->id,12);
	if(1 == ret)
	{
		//将所在行分裂
		splitString(linedata,splitdata);
		*limitedpower = atoi(splitdata[1]);
		*stationarypower = atoi(splitdata[3]);

		printdecmsg(ECU_DBG_MAIN,"Maximun power", *limitedpower);
		printdecmsg(ECU_DBG_MAIN,"Fixed power", *stationarypower);
	}
	return 0;
}


int get_maximum_power(inverter_info *inverter)
{
	char linedata[100] = "\0";
	char splitdata[6][32];
	int ret;
	int power;

	//读取所在ID行
	ret = read_line("/home/data/power",linedata,inverter->id,12);
	if(1 == ret)
	{
		//将所在行分裂
		splitString(linedata,splitdata);
		if(0 !=  atoi(splitdata[1]))
		{
			power = atoi(splitdata[1]);
			return power;
		}else
		{
			return -1;
		}
	}
	else
		return -1;
}

int get_fixed_power(inverter_info *inverter)
{
	char linedata[100] = "\0";
	char splitdata[6][32];
	int power;
	//读取所在ID行
	if(1 == read_line("/home/data/power",linedata,inverter->id,12))
	{
		//将所在行分裂
		splitString(linedata,splitdata);
		power = atoi(splitdata[3]);

		printdecmsg(ECU_DBG_MAIN,"Fixed power", power);
	}



	return power;
}

int getsyspower()		//读取系统最大总功率
{
	char linedata[100] = "\0";
	char splitdata[3][32];
	int syspower;
	
	//读取一行数据
	file_get_one(linedata, 100, "/home/data/powerall");
	
	//将所在行分裂
	splitString(linedata,splitdata);

	syspower = atoi(splitdata[2]);

	printdecmsg(ECU_DBG_MAIN,"Maximun system power", syspower);

	return syspower;
}

int updatemaxpower(inverter_info *inverter, int limitedresult)//更新逆变器的最大功率结果
{	
	char linedata[100] = "\0";
	char splitdata[6][32];
	int i;
	
	//读取所在ID行
	if(1 == read_line("/home/data/power",linedata,inverter->id,12))
	{
		//将所在行分裂
		splitString(linedata,splitdata);
		memset(linedata,0x00,100);
		sprintf(linedata,"%s,%d,%d,%d,%d,0\n",inverter->id,limitedresult,atoi(splitdata[2]),atoi(splitdata[3]),atoi(splitdata[4]));
		//删除id所在行
		delete_line("/home/data/power","/home/data/_power",inverter->id,12);

		//更新所在行
		for(i=0; i<3; i++)
		{
			if(1 == insert_line("/home/data/power",linedata))
			{
				print2msg(ECU_DBG_MAIN,inverter->id, "Update maximun power successfully");
				break;
			}
			else
				print2msg(ECU_DBG_MAIN,inverter->id, "Failed to update maximun power");
		}
	}
	


	return 0;
}

int updatemaxflag(inverter_info *inverter)			//更新逆变器为最大功率模式
{
	char linedata[100] = "\0";
	char splitdata[6][32];
	int i;

	
	//读取所在ID行
	if(1 == read_line("/home/data/power",linedata,inverter->id,12))
	{
		//将所在行分裂
		splitString(linedata,splitdata);
		memset(linedata,0x00,100);
		sprintf(linedata,"%s,%d,%d,%d,%d,0\n",inverter->id,atoi(splitdata[1]),atoi(splitdata[2]),atoi(splitdata[3]),atoi(splitdata[4]));
		//删除id所在行
		delete_line("/home/data/power","/home/data/_power",inverter->id,12);
		
		for(i=0; i<3; i++)
		{
			if(1 == insert_line("/home/data/power",linedata))
			{
				print2msg(ECU_DBG_MAIN,inverter->id, "Update maximun power flag successfully");
					break;
			}
			else
				print2msg(ECU_DBG_MAIN,inverter->id, "Failed to update maximun flag power");
		}
		print2msg(ECU_DBG_MAIN,inverter->id, "has been changed to Maximun power Mode");
	}
	
	return 0;
}

int updatefixedpower(inverter_info *inverter, int stationaryresult)
{
	char linedata[100] = "\0";
	char splitdata[6][32];
	int i;
	
	//读取所在ID行
	if(1 == read_line("/home/data/power",linedata,inverter->id,12))
	{
		//将所在行分裂
		splitString(linedata,splitdata);
		memset(linedata,0x00,100);
		sprintf(linedata,"%s,%d,%d,%d,%d,0\n",inverter->id,atoi(splitdata[1]),atoi(splitdata[2]),atoi(splitdata[3]),stationaryresult);
		//删除id所在行
		delete_line("/home/data/power","/home/data/_power",inverter->id,12);

		for(i=0; i<3; i++)
		{
			if(1 == insert_line("/home/data/power",linedata))
			{
				print2msg(ECU_DBG_MAIN,inverter->id, "Update fixed power flag successfully");
					break;
			}
			else
				print2msg(ECU_DBG_MAIN,inverter->id, "Failed to update fixed flag power");
		}

		printdecmsg(ECU_DBG_MAIN,"Fixed power from inverter", stationaryresult);
	}
	
	return 0;
}

int updatefixedflag(inverter_info *inverter)
{
	char linedata[100] = "\0";
	char splitdata[6][32];
	int i;
	
	//读取所在ID行
	if (1 == read_line("/home/data/power",linedata,inverter->id,12))
	{	
		//将所在行分裂
		splitString(linedata,splitdata);
		memset(linedata,0x00,100);
		sprintf(linedata,"%s,%d,%d,%d,%d,1\n",inverter->id,atoi(splitdata[1]),atoi(splitdata[2]),atoi(splitdata[3]),atoi(splitdata[4]));
		//删除id所在行
		delete_line("/home/data/power","/home/data/_power",inverter->id,12);


		for(i=0; i<3; i++)
		{
			if(1 == insert_line("/home/data/power",linedata))
			{
				print2msg(ECU_DBG_MAIN,inverter->id, "Update fixed power flag successfully");
					break;
			}
			else
				print2msg(ECU_DBG_MAIN,inverter->id, "Failed to update fixed flag power");
		}

		print2msg(ECU_DBG_MAIN,inverter->id, "has been changed to fixed power Mode!");
	}
	
	return 0;
}


int calcount(inverter_info *firstinverter)
{
	int i, panelcount=0;
	inverter_info *curinverter = firstinverter;

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++){
		if(3 == curinverter->model)
			panelcount += 2;
		else
			panelcount++;
	}

	printdecmsg(ECU_DBG_MAIN,"Panel count", panelcount);

	return panelcount;
}

int setfixedpowerone(inverter_info *inverter, char power)//固定功率限定单播 zb_constpower_single
{
	int i = 0, ret;
	char sendbuff[256] = { '\0' };
	char data[256] = { '\0' };

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xC4 ;
	sendbuff[i++] = power;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_send_cmd(inverter, sendbuff, i);
	printmsg(ECU_DBG_MAIN,"Set fixed power single");
	ret = zb_get_reply(data, inverter);
	if ((13 == ret) && (0xFB == data[0]) && (0xFB == data[1])&& (0xFE == data[11]) && (0xFE == data[12]))
	{
		if (0xDE == data[3])
		{
			printmsg(ECU_DBG_MAIN,"Set success");
			return 1;
		}
		else if (0xDF == data[3])
		{
			printmsg(ECU_DBG_MAIN,"Set failed");
			return 2;
		}
		else
		{
			printmsg(ECU_DBG_MAIN,"Set failed");
			return -1;
		}
	}
	else
	{
		printmsg(ECU_DBG_MAIN,"Set failed");
		return -1;
	}
}

int setfixedpowerall(char power)	//固定功率限定广播，zb_constpower_broadcast
{
	int i = 0;
	char sendbuff[256] = { '\0' };

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xA4;
	sendbuff[i++] = power;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;

	zb_broadcast_cmd(sendbuff, i);
	printmsg(ECU_DBG_MAIN,"Set fixed power broadcast");
	return 1;

}

int setlimitedpowerone(inverter_info *inverter, char power)	//发送命令，设置单个逆变器限定功率
{
	int i = 0, ret;
	char sendbuff[256] = { '\0' };
	char data[256] = { '\0' };
	int check=0,j;

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xC3;
	sendbuff[i++] = power;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	for(j=2;j<9;j++)
		check=check+sendbuff[j];
	sendbuff[9]=check/256;
	sendbuff[10]=check%256;

	zb_send_cmd(inverter, sendbuff, i);
	printmsg(ECU_DBG_MAIN,"Set max power single");
	ret = zb_get_reply(data, inverter);
	if ((13 == ret) && (0xFB == data[0]) && (0xFB == data[1])&& (0xFE == data[11]) && (0xFE == data[12]))
	{
		if (0xDE == data[3])
		{
			printmsg(ECU_DBG_MAIN,"Set success");
			return 1;
		}
		else if (0xDF == data[3])
		{
			printmsg(ECU_DBG_MAIN,"Set failed");
			return 2;
		}
		else
		{
			printmsg(ECU_DBG_MAIN,"Set failed");
			return -1;
		}
	}
	else
	{
		printmsg(ECU_DBG_MAIN,"Set failed");
		return -1;
	}
}

int setlimitedpowerall(char power)	//最大功率限定广播，zb_powerlimited_broadcast
{
	int i = 0;
	char sendbuff[256] = { '\0' };
	int check=0,j;

	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0xFB;
	sendbuff[i++] = 0x06;
	sendbuff[i++] = 0xA3;
	sendbuff[i++] = power;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0x00;
	sendbuff[i++] = 0xFE;
	sendbuff[i++] = 0xFE;
	for(j=2;j<9;j++)
		check=check+sendbuff[j];
	sendbuff[9]=check/256;
	sendbuff[10]=check%256;
	
	zb_broadcast_cmd(sendbuff, i);
	printmsg(ECU_DBG_MAIN,"Set max power broadcast");
	return 0;
}


int save_max_power_result_all(void)			//设置所有逆变器功率的结果
{
	FILE *fp;
	char *set_max_power_result = NULL;	//[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL] = {'\0'};
	char inverter_result[64];
	char data[200];
	char splitdata[6][32];
	int num = 0;
	set_max_power_result = malloc(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL);
	memset(set_max_power_result,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL);

	strcpy(set_max_power_result, "APS13AAAAAA117AAA1");

	strcat(set_max_power_result, ecu.id);					//ECU的ID
	strcat(set_max_power_result, "0000");					//逆变器个数
	strcat(set_max_power_result, "00000000000000");		//时间戳，设置逆变器后返回的结果中时间戳为0
	strcat(set_max_power_result, "END");					//固定格式

	fp = fopen("/home/data/power", "r");
	if(fp)
	{
		memset(data,0x00,200);
		
		while(NULL != fgets(data,200,fp))
		{
			memset(splitdata,0x00,6*32);
			splitString(data,splitdata);
			
			memset(inverter_result, '\0', sizeof(inverter_result));
			sprintf(inverter_result, "%s%03d020250END", splitdata[0], atoi(splitdata[2]));
			strcat(set_max_power_result, inverter_result);
			memset(data,0x00,200);
			num++;
		}
		fclose(fp);
	}
	

	set_max_power_result[30] = num/1000 + 0x30;
	set_max_power_result[31] = (num/100)%10 + 0x30;
	set_max_power_result[32] = (num/10)%10 + 0x30;
	set_max_power_result[33] = num%10 + 0x30;

	if(strlen(set_max_power_result) > 10000)
		set_max_power_result[5] = (strlen(set_max_power_result)-1)/10000 + 0x30;
	if(strlen(set_max_power_result) > 1000)
		set_max_power_result[6] = ((strlen(set_max_power_result)-1)/1000)%10 + 0x30;
	if(strlen(set_max_power_result) > 100)
		set_max_power_result[7] = ((strlen(set_max_power_result)-1)/100)%10 + 0x30;
	if(strlen(set_max_power_result) > 10)
		set_max_power_result[8] = ((strlen(set_max_power_result)-1)/10)%10 + 0x30;
	if(strlen(set_max_power_result) > 0)
		set_max_power_result[9] = (strlen(set_max_power_result)-1)%10 + 0x30;

	save_process_result(117, set_max_power_result);
	free(set_max_power_result);
	set_max_power_result = NULL;
	return 0;
}



int save_max_power_result_one(inverter_info *inverter, int power)		//设置一个逆变器最大功率的结果
{
	char *inverter_result = NULL;	//[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL] = {'\0'};
	inverter_result = malloc(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL);
	memset(inverter_result,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL);
	
	memset(inverter_result, '\0', sizeof(inverter_result));
	sprintf(inverter_result, "%s%03d020300END", inverter->id, power);
	save_inverter_parameters_result(inverter, 117, inverter_result);
	free(inverter_result);
	inverter_result = NULL;
	return 0;
}

int process_max_power(inverter_info *firstinverter)
{
	int limitedpower,k, m, res;

	char limitedvalue;
	int limitedresult;
	char readpresetdata[128] = {'\0'};
	char data[200];
	char splitdata[6][32];
	int num = 0;
	inverter_info *curinverter = firstinverter;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	memset(data,0x00,200);
	while(-1 != read_line_end("/home/data/power",data,"1",1))
	{
		memset(splitdata,0x00,6*32);
		splitString(data,splitdata);
		if((1 == atoi(splitdata[5]))&& (0 != atoi(splitdata[1])))
		{
			curinverter = firstinverter;
			for(k=0; (k<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); k++, curinverter++)
			{
				if(!strncmp(splitdata[0], curinverter->id, 12))
				{
					updatemaxflag(curinverter);
					if(curinverter->model==0)		//读不到机型码不去设置
							continue;
					limitedpower = atoi(splitdata[1]);
					limitedvalue = (limitedpower * 7395) >> 14;
					if(curinverter->model==7)
							limitedvalue = limitedvalue/4;
					for(m=0; m<3; m++)
					{
						setlimitedpowerone(curinverter, limitedvalue);
						memset(readpresetdata, '\0', sizeof(readpresetdata));

						res = zb_query_protect_parameter(curinverter, readpresetdata);
						printdecmsg(ECU_DBG_MAIN,"res", res);
						if(1 == res)
						{
							limitedresult = (readpresetdata[5] << 14) / 7395;
							if(curinverter->model==7)
									limitedresult=limitedresult*4;
							updatemaxpower(curinverter, limitedresult);
							save_max_power_result_one(curinverter, limitedresult);
							break;
						}
					}
					break;
				}
			}
		}

		memset(data,0x00,200);
		num++;
	}
	rt_mutex_release(record_data_lock);
	return 1;
}

int read_max_power(inverter_info *firstinverter)
{
	FILE *fp;
	int i, j, res;
	char id[13];
	int limitedresult;
	char readpresetdata[128] = {'\0'};
	inverter_info *curinverter = firstinverter;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	memset(id, '\0', sizeof(id));
	fp = fopen("/tmp/maxpower.con", "r");
	if(fp)
	{
		fgets(id, 50, fp);
		fclose(fp);
	}
	if(!strcmp(id, "ALL"))
	{
		curinverter = firstinverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++)
		{
			for(j=0; j<3; j++)
			{
				memset(readpresetdata, '\0', sizeof(readpresetdata));
				res = zb_query_protect_parameter(curinverter, readpresetdata);
				if(1 == res)
				{
					limitedresult = (readpresetdata[5] << 14) / 7395;					//读取成功
					if(curinverter->model==7)
						limitedresult = limitedresult*4;
					updatemaxpower(curinverter, limitedresult);
					//updatemaxflag(curinverter);
					save_max_power_result_one(curinverter, limitedresult);
					break;
				}
			}
		}

		fp = fopen("/tmp/maxpower.con", "w");
		fclose(fp);
	//	save_max_power_result_all();
	}
	rt_mutex_release(record_data_lock);
	return 0;
}

int processpower(inverter_info *firstinverter)
{
	process_max_power(firstinverter);
//	process_fixed_power(firstinverter);
	read_max_power(firstinverter);
	return 1;
}
