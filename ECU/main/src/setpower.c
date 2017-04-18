/*
 * setpower.c
 *	Version: 1.2
 *  Modified on: 2013-8-7
 *  	修改内容：数据库表更新,更新标志更新在数据库中
 *	Author: aps
 *      使用方法：在plc.c中的process_all()中添加processpower()，在plc.c开头添加processpower()的原型声明
 */

/*
power表格字段：
id,limitedpower,limitedresult,stationarypower,stationaryresult,flag
*/
/*
power表格字段：
maxpower,fixedpower,sysmaxpower
*/

#include <stdio.h>
#include <stdlib.h>
#include "variation.h"
#include "zigbee.h"
#include <string.h>
#include "debug.h"
#include "myfile.h"
#include "file.h"
#include "setpower.h"


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

		printdecmsg("main","Maximun power", *limitedpower);
		printdecmsg("main","Fixed power", *stationarypower);
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
	read_line("/home/data/power",linedata,inverter->id,12);

	//将所在行分裂
	splitString(linedata,splitdata);
	power = atoi(splitdata[3]);

	printdecmsg("main","Fixed power", power);

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

	printdecmsg("main","Maximun system power", syspower);

	return syspower;
}

int updatemaxpower(inverter_info *inverter, int limitedresult)//更新逆变器的最大功率结果
{	
	char linedata[100] = "\0";
	char splitdata[6][32];
	int i;
	
	//读取所在ID行
	read_line("/home/data/power",linedata,inverter->id,12);
	
	//将所在行分裂
	splitString(linedata,splitdata);
	memset(linedata,0x00,100);
	sprintf(linedata,"%s,%d,%d,%d,%d,0\n",inverter->id,limitedresult,atoi(splitdata[2]),atoi(splitdata[3]),atoi(splitdata[4]));
	//删除id所在行
	delete_line("/home/data/power","/home/data/power.t",inverter->id,12);

	//更新所在行
	for(i=0; i<3; i++)
	{
		if(1 == insert_line("/home/data/power",linedata))
		{
			print2msg("main",inverter->id, "Update maximun power successfully");
			break;
		}
		else
			print2msg("main",inverter->id, "Failed to update maximun power");
	}

	return 0;
}

int updatemaxflag(inverter_info *inverter)			//更新逆变器为最大功率模式
{
	char linedata[100] = "\0";
	char splitdata[6][32];
	int i;
	

	
	//读取所在ID行
	read_line("/home/data/power",linedata,inverter->id,12);
	
	//将所在行分裂
	splitString(linedata,splitdata);
	memset(linedata,0x00,100);
	sprintf(linedata,"%s,%d,%d,%d,%d,0\n",inverter->id,atoi(splitdata[1]),atoi(splitdata[2]),atoi(splitdata[3]),atoi(splitdata[4]));
	//删除id所在行
	delete_line("/home/data/power","/home/data/power.t",inverter->id,12);
	
	for(i=0; i<3; i++)
	{
		if(1 == insert_line("/home/data/power",linedata))
		{
			print2msg("main",inverter->id, "Update maximun power flag successfully");
				break;
		}
		else
			print2msg("main",inverter->id, "Failed to update maximun flag power");
	}
	print2msg("main",inverter->id, "has been changed to Maximun power Mode");
	return 0;
}

int updatefixedpower(inverter_info *inverter, int stationaryresult)
{
	char linedata[100] = "\0";
	char splitdata[6][32];
	int i;
	
	//读取所在ID行
	read_line("/home/data/power",linedata,inverter->id,12);
	
	//将所在行分裂
	splitString(linedata,splitdata);
	memset(linedata,0x00,100);
	sprintf(linedata,"%s,%d,%d,%d,%d,0\n",inverter->id,atoi(splitdata[1]),atoi(splitdata[2]),atoi(splitdata[3]),stationaryresult);
	//删除id所在行
	delete_line("/home/data/power","/home/data/power.t",inverter->id,12);

	for(i=0; i<3; i++)
	{
		if(1 == insert_line("/home/data/power",linedata))
		{
			print2msg("main",inverter->id, "Update fixed power flag successfully");
				break;
		}
		else
			print2msg("main",inverter->id, "Failed to update fixed flag power");
	}

	printdecmsg("main","Fixed power from inverter", stationaryresult);
	return 0;
}

int updatefixedflag(inverter_info *inverter)
{
	char linedata[100] = "\0";
	char splitdata[6][32];
	int i;
	
	//读取所在ID行
	read_line("/home/data/power",linedata,inverter->id,12);
	
	//将所在行分裂
	splitString(linedata,splitdata);
	memset(linedata,0x00,100);
	sprintf(linedata,"%s,%d,%d,%d,%d,1\n",inverter->id,atoi(splitdata[1]),atoi(splitdata[2]),atoi(splitdata[3]),atoi(splitdata[4]));
	//删除id所在行
	delete_line("/home/data/power","/home/data/power.t",inverter->id,12);


	for(i=0; i<3; i++)
	{
		if(1 == insert_line("/home/data/power",linedata))
		{
			print2msg("main",inverter->id, "Update fixed power flag successfully");
				break;
		}
		else
			print2msg("main",inverter->id, "Failed to update fixed flag power");
	}

	print2msg("main",inverter->id, "has been changed to fixed power Mode!\n");
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

	printdecmsg("main","Panel count", panelcount);

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
	printmsg("main","Set fixed power single");
	ret = zb_get_reply(data, inverter);
	if ((13 == ret) && (0xFB == data[0]) && (0xFB == data[1])&& (0xFE == data[11]) && (0xFE == data[12]))
	{
		if (0xDE == data[3])
		{
			printmsg("main","Set success");
			return 1;
		}
		else if (0xDF == data[3])
		{
			printmsg("main","Set failed");
			return 2;
		}
		else
		{
			printmsg("main","Set failed");
			return -1;
		}
	}
	else
	{
		printmsg("main","Set failed");
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
	printmsg("main","Set fixed power broadcast");
	return 1;

}

int setlimitedpowerone(inverter_info *inverter, char power)	//发送命令，设置单个逆变器限定功率
{
	int i = 0, ret;
	char sendbuff[256] = { '\0' };
	char data[256] = { '\0' };

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

	zb_send_cmd(inverter, sendbuff, i);
	printmsg("main","Set max power single");
	ret = zb_get_reply(data, inverter);
	if ((13 == ret) && (0xFB == data[0]) && (0xFB == data[1])&& (0xFE == data[11]) && (0xFE == data[12]))
	{
		if (0xDE == data[3])
		{
			printmsg("main","Set success");
			return 1;
		}
		else if (0xDF == data[3])
		{
			printmsg("main","Set failed");
			return 2;
		}
		else
		{
			printmsg("main","Set failed");
			return -1;
		}
	}
	else
	{
		printmsg("main","Set failed");
		return -1;
	}
}

int setlimitedpowerall(char power)	//最大功率限定广播，zb_powerlimited_broadcast
{
	int i = 0;
	char sendbuff[256] = { '\0' };

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

	zb_broadcast_cmd(sendbuff, i);
	printmsg("main","Set max power broadcast");
	return 0;
}


int save_max_power_result_all(void)			//设置所有逆变器功率的结果
{
	char ecu_id[16];
	FILE *fp;
	char set_max_power_result[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL] = {'\0'};
	char inverter_result[64];
	char data[200];
	char splitdata[6][32];
	int num = 0;

	strcpy(set_max_power_result, "APS13AAAAAA117AAA1");
	fp = fopen("/yuneng/ecuid.con", "r");		//读取ECU的ID
	if(fp)
	{
		fgets(ecu_id, 13, fp);
		fclose(fp);
	}

	strcat(set_max_power_result, ecu_id);					//ECU的ID
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

	return 0;
}

#if 0
int save_max_power_result_one(char *inverterid, int power)		//设置一个逆变器最大功率的结果
{
	char ecu_id[16];
	FILE *fp;
	char set_max_power_result[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL] = {'\0'};
	char inverter_result[64] = {'\0'};

	strcpy(set_max_power_result, "APS13AAAAAA117AAA1");
	fp = fopen("/etc/yuneng/ecuid.con", "r");		//读取ECU的ID
	if(fp)
	{
		fgets(ecu_id, 13, fp);
		fclose(fp);
	}

	strcat(set_max_power_result, ecu_id);					//ECU的ID
	strcat(set_max_power_result, "0000");					//逆变器个数
	strcat(set_max_power_result, "00000000000000");		//时间戳，设置逆变器后返回的结果中时间戳为0
	strcat(set_max_power_result, "END");					//固定格式

	sprintf(inverter_result, "%s%03d020250END\n", inverterid, power);
	strcat(set_max_power_result, inverter_result);

	set_max_power_result[33] = '1';

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

	return 0;
}
#endif

int save_max_power_result_one(inverter_info *inverter, int power)		//设置一个逆变器最大功率的结果
{
	char inverter_result[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL] = {'\0'};

	memset(inverter_result, '\0', sizeof(inverter_result));
	sprintf(inverter_result, "%s%03d020300END", inverter->id, power);
	save_inverter_parameters_result(inverter, 117, inverter_result);

	return 0;
}

int process_max_power(inverter_info *firstinverter)
{
	int limitedpower,k, m, res;
	FILE * fp;
	char limitedvalue;
	int limitedresult;
	char readpresetdata[128] = {'\0'};
	char data[200];
	char splitdata[6][32];
	int num = 0;
	inverter_info *curinverter = firstinverter;

	fp = fopen("/home/data/power", "r");
	if(fp)
	{
		memset(data,0x00,200);
		
		while(NULL != fgets(data,200,fp))
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
						limitedpower = atoi(splitdata[1]);
						limitedvalue = (limitedpower * 7395) >> 14;

						for(m=0; m<3; m++)
						{
							setlimitedpowerone(curinverter, limitedvalue);
							memset(readpresetdata, '\0', sizeof(readpresetdata));

							res = zb_query_protect_parameter(curinverter, readpresetdata);
							printdecmsg("main","res", res);
							if(1 == res)
							{
								limitedresult = (readpresetdata[5] << 14) / 7395;
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
		fclose(fp);
	}
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

	return 0;
}

int processpower(inverter_info *firstinverter)
{
	process_max_power(firstinverter);
//	process_fixed_power(firstinverter);
	read_max_power(firstinverter);
	return 1;
}
