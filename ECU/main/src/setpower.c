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

#include <stdio.h>
#include <stdlib.h>
#include "variation.h"
#include "zigbee.h"
#include <string.h>
#include "debug.h"
#include "myfile.h"


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
	char sql[100]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow, ncolumn, syspower;

	sprintf(sql,"SELECT sysmaxpower FROM powerall WHERE item=0;");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	syspower = atoi(azResult[1]);
	sqlite3_free_table( azResult );

	printdecmsg("Maximun system power", syspower);

	return syspower;
}

int updatemaxpower(inverter_info *inverter, int limitedresult)//更新逆变器的最大功率结果
{
	char sql[100] = { '\0' };
	char *zErrMsg = 0;
	int i;

	sprintf(sql,"UPDATE power SET limitedresult=%d,flag=0 WHERE id=%s ", limitedresult, inverter->id);
	for(i=0; i<3; i++)
	{
		if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
		{
			print2msg(inverter->id, "Update maximun power successfully");
			break;
		}
		else
			print2msg(inverter->id, "Failed to update maximun power");
	}

	return 0;
}

int updatemaxflag(inverter_info *inverter)		//更新逆变器为最大功率模式
{
	char sql[100] = { '\0' };
	char *zErrMsg = 0;
	int i;

	sprintf(sql,"UPDATE power SET flag=0 WHERE id=%s ", inverter->id);

	for(i=0; i<3; i++)
	{
		if(SQLITE_OK == sqlite3_exec( db , sql , 0 , 0 , &zErrMsg ))
		{
			print2msg(inverter->id, "Update maximun power flag successfully");
				break;
		}
		else
			print2msg(inverter->id, "Failed to update maximun flag power");
	}


	print2msg(inverter->id, "has been changed to Maximun power Mode");
}

int updatefixedpower(inverter_info *inverter, int stationaryresult)
{
	char sql[100]={'\0'};
	char *zErrMsg=0;

	sprintf(sql,"UPDATE power SET stationaryresult=%d WHERE id=%s ", stationaryresult, inverter->id);
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	printdecmsg("Fixed power from inverter", stationaryresult);
}

int updatefixedflag(inverter_info *inverter)
{
	char sql[100]={'\0'};
	char *zErrMsg=0;

	sprintf(sql,"UPDATE power SET flag=1 WHERE id=%s ", inverter->id);
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	print2msg(inverter->id, "has been changed to fixed power Mode!\n");
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

	printdecmsg("Panel count", panelcount);

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
	printmsg("Set fixed power single");
	ret = zb_get_reply(data, inverter);
	if ((13 == ret) && (0xFB == data[0]) && (0xFB == data[1])&& (0xFE == data[11]) && (0xFE == data[12]))
	{
		if (0xDE == data[3])
		{
			printmsg("Set success");
			return 1;
		}
		else if (0xDF == data[3])
		{
			printmsg("Set failed");
			return 2;
		}
		else
		{
			printmsg("Set failed");
			return -1;
		}
	}
	else
	{
		printmsg("Set failed");
		return -1;
	}
}

int setfixedpowerall(char power)	//固定功率限定广播，zb_constpower_broadcast
{
	int i = 0, ret;
	char sendbuff[256] = { '\0' };
	char data[256] = { '\0' };

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
	printmsg("Set fixed power broadcast");
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
	printmsg("Set max power single");
	ret = zb_get_reply(data, inverter);
	if ((13 == ret) && (0xFB == data[0]) && (0xFB == data[1])&& (0xFE == data[11]) && (0xFE == data[12]))
	{
		if (0xDE == data[3])
		{
			printmsg("Set success");
			return 1;
		}
		else if (0xDF == data[3])
		{
			printmsg("Set failed");
			return 2;
		}
		else
		{
			printmsg("Set failed");
			return -1;
		}
	}
	else
	{
		printmsg("Set failed");
		return -1;
	}
}

int setlimitedpowerall(char power)	//最大功率限定广播，zb_powerlimited_broadcast
{
	int i = 0, ret;
	char sendbuff[256] = { '\0' };
	char data[256] = { '\0' };

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
	printmsg("Set max power broadcast");
	return 0;
}


int save_max_power_result_all(void)			//设置所有逆变器功率的结果
{
	char ecu_id[16];
	FILE *fp;
	sqlite3 *db;
	char *zErrMsg=0;
	int nrow=0,ncolumn=0;
	char **azResult;
	int i;
	char sql[1024];
	char set_max_power_result[65535] = {'\0'};
	char inverter_result[64];

	strcpy(set_max_power_result, "APS13AAAAAA117AAA1");
	fp = fopen("/etc/yuneng/ecuid.conf", "r");		//读取ECU的ID
	if(fp)
	{
		fgets(ecu_id, 13, fp);
		fclose(fp);
	}

	strcat(set_max_power_result, ecu_id);					//ECU的ID
	strcat(set_max_power_result, "0000");					//逆变器个数
	strcat(set_max_power_result, "00000000000000");		//时间戳，设置逆变器后返回的结果中时间戳为0
	strcat(set_max_power_result, "END");					//固定格式


	sqlite3_open("/home/database.db", &db);
	strcpy(sql, "SELECT id,limitedresult FROM power");
	for(i=0;i<3;i++)
	{
		if(SQLITE_OK == sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg ))
		{
			for(i=1; i<=nrow; i++)
			{
				memset(inverter_result, '\0', sizeof(inverter_result));
				sprintf(inverter_result, "%s%03d020250END", azResult[i*ncolumn], atoi(azResult[i*ncolumn+1]));
				strcat(set_max_power_result, inverter_result);
			}
			strcat(set_max_power_result, "\n");
			sqlite3_free_table(azResult);
			break;
		}
		else
			print2msg("SELECT FROM power", zErrMsg);
		sqlite3_free_table(azResult);
		sleep(1);
	}
	sqlite3_close(db);
	set_max_power_result[30] = nrow/1000 + 0x30;
	set_max_power_result[31] = (nrow/100)%10 + 0x30;
	set_max_power_result[32] = (nrow/10)%10 + 0x30;
	set_max_power_result[33] = nrow%10 + 0x30;

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
	char set_max_power_result[65535] = {'\0'};
	char inverter_result[64] = {'\0'};

	strcpy(set_max_power_result, "APS13AAAAAA117AAA1");
	fp = fopen("/etc/yuneng/ecuid.conf", "r");		//读取ECU的ID
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
	char inverter_result[65535] = {'\0'};

	memset(inverter_result, '\0', sizeof(inverter_result));
	sprintf(inverter_result, "%s%03d020300END", inverter->id, power);
	save_inverter_parameters_result(inverter, 117, inverter_result);

	return 0;
}

int process_max_power(inverter_info *firstinverter)
{
	int limitedpower, stationarypower, i, j, k, m, res;
	char command[16]={'\0'};
	char limitedvalue;
	int limitedresult;
	char readpresetdata[128] = {'\0'};
	inverter_info *curinverter = firstinverter;
	char sql[100]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int nrow=0, ncolumn=0;

	/*
	fp = fopen("/tmp/setmaxpower.conf", "r");
	if (fp)
	{
		curinverter = firstinverter;
		fgets(command, sizeof(command), fp);
		fclose(fp);
		if (!strlen(command))
			return 0;
		else if (!strncmp(command, "ALL", 3))
		{
			printmsg("Set all inverter's maximum power!");
			getpower(firstinverter, &limitedpower, &stationarypower);
			limitedvalue = (limitedpower * 7395) >> 14;
			setmaxpowerall(limitedvalue);

			for (i = 0;(i < MAXINVERTERCOUNT) && (12 == strlen(curinverter->id));i++, curinverter++)
			{
				memset(readpresetdata, '\0', 20);
				//if(1 == curinverter->flagyc500){
				res = zb_query_protect_parameter(curinverter, readpresetdata);
				if ((1 == res) && (limitedvalue == readpresetdata[5]))
				{
					limitedresult = (readpresetdata[5] << 14) / 7395;	//读取成功
					updatemaxpower(curinverter, limitedresult);
					update_maxpower_mode(curinverter);
				}
				else
				{
					for (j = 0; j < 3; j++)
					{
						setmaxpowerone(curinverter, limitedvalue);
						memset(readpresetdata, '\0', 20);
						res = zb_query_protect_parameter(curinverter,readpresetdata);
						if ((1 == res) && (limitedvalue == readpresetdata[5]))
						{
							limitedresult = (readpresetdata[5] << 14) / 7395;
							updatemaxpower(curinverter, limitedresult);
							update_maxpower_mode(curinverter);
							break;
						}
					}
				}
			}

			fp = fopen("/tmp/setmaxpower.conf", "w");
			fclose(fp);
		//	save_max_power_result_all();		//ZK,暂时屏蔽
		}
		else
		{
			printmsg("Set one inverter's maximum power!");

			for (i = 0;(i < MAXINVERTERCOUNT) && (12 == strlen(curinverter->id));i++, curinverter++)
			{
				if (!strncmp(command, curinverter->id, 12))
				{
					getpower(curinverter, &limitedpower, &stationarypower);
					limitedvalue = (limitedpower * 7395) >> 14;
					printdecmsg("limitedvalue", limitedvalue);
					for (j = 0; j < 4; j++)
					{
						setmaxpowerone(curinverter, limitedvalue);
						memset(readpresetdata, '\0', 20);
						res = zb_query_protect_parameter(curinverter,readpresetdata);
						printdecmsg("res", res);
						if(1 == res)
						{
							limitedresult = (readpresetdata[5] << 14) / 7395;
							updatemaxpower(curinverter, limitedresult);
							update_maxpower_mode(curinverter);
							save_max_power_result_one(curinverter->id, limitedresult);
							break;
						}
					}
					break;
				}
			}

			fp = fopen("/tmp/setmaxpower.conf", "w");
			fclose(fp);
		}
	}
	*/
	sprintf(sql,"SELECT id,limitedpower FROM power WHERE flag=1 AND limitedpower IS NOT NULL;");
	for(i=0; i<3; i++)
	{
		res = sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
		if(SQLITE_OK == res)
		{
			for(j=1; j<=nrow; j++)
			{
				curinverter = firstinverter;
				for(k=0; (k<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); k++, curinverter++)
				{
					if(!strncmp(azResult[j*ncolumn], curinverter->id, 12))
					{
						updatemaxflag(curinverter);
						limitedpower = atoi(azResult[j*ncolumn+1]);
						limitedvalue = (limitedpower * 7395) >> 14;

						for(m=0; m<3; m++)
						{
							setlimitedpowerone(curinverter, limitedvalue);
							memset(readpresetdata, '\0', sizeof(readpresetdata));

							res = zb_query_protect_parameter(curinverter, readpresetdata);
							printdecmsg("res", res);
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
			sqlite3_free_table( azResult );
			break;
		}
		else
			sqlite3_free_table( azResult );
	}
}
/*
int process_fixed_power(inverter_info *firstinverter)
{
	FILE *fp;
	int limitedpower, stationarypower, i, j, res;
	char command[16]={'\0'};
	char stationaryvalue;
	int stationaryresult;
	int syspower;
	int panelcount = 0;
	int yc200 = 0, yc500 = 0;
	char readpresetdata[20] = { '\0' };
	inverter_info *curinverter = firstinverter;

	fp = fopen("/tmp/setfixpower.conf", "r");
	if (fp)
	{
		curinverter = firstinverter;
		fgets(command, sizeof(command), fp);
		fclose(fp);

		if (!strlen(command))
			return 0;
		else if (!strncmp(command, "ALL", 3))
		{
			printmsg("Set all inverter's fixed power!");
			getpower(firstinverter, &limitedpower, &stationarypower);
			stationaryvalue = (stationarypower * 7395) >> 14;
			setfixedpowerall(stationaryvalue);

			for (i = 0;(i < MAXINVERTERCOUNT) && (12 == strlen(curinverter->id));i++, curinverter++)
			{
				memset(readpresetdata, '\0', 20);
				//if(1 == curinverter->flagyc500){
				res = zb_query_protect_parameter(curinverter, readpresetdata);
				if ((1 == res) && (stationaryvalue == readpresetdata[6]))
				{
					stationaryresult = (readpresetdata[6] << 14) / 7395;	//读取成功
					updatefixedpower(curinverter, stationaryresult);
					update_fixedpower_mode(curinverter);
				}
				else
				{
					for (j = 0; j < 3; j++)
					{
						setfixedpowerone(curinverter, stationaryvalue);
						memset(readpresetdata, '\0', 20);
						res = zb_query_protect_parameter(curinverter,readpresetdata);
						if ((1 == res) && (stationaryvalue == readpresetdata[6]))
						{
							stationaryresult = (readpresetdata[6] << 14) / 7395;
							updatefixedpower(curinverter, stationaryresult);
							update_fixedpower_mode(curinverter);
							break;
						}
					}
				}
			}

			fp = fopen("/tmp/setfixpower.conf", "w");
			fclose(fp);
		}
		else
		{
			printmsg("Set one inverter's fixed power!");

			for (i = 0;(i < MAXINVERTERCOUNT) && (12 == strlen(curinverter->id));i++, curinverter++)
			{
				if (!strncmp(command, curinverter->id, 12))
				{
					getpower(curinverter, &limitedpower, &stationarypower);
					stationaryvalue = (stationarypower * 7395) >> 14;
					printdecmsg("stationaryvalue", stationaryvalue);
					for (j = 0; j < 4; j++)
					{
						setfixedpowerone(curinverter, stationaryvalue);
						memset(readpresetdata, '\0', 20);
						res = zb_query_protect_parameter(curinverter,readpresetdata);
						printdecmsg("res", res);
						if(1 == res)
						{
							stationaryresult = (readpresetdata[6] << 14) / 7395;
							updatefixedpower(curinverter, stationaryresult);
							update_fixedpower_mode(curinverter);
							break;
						}
					}
					break;
				}
			}

			fp = fopen("/tmp/setfixpower.conf", "w");
			fclose(fp);
		}
	}
}
*/
int read_max_power(inverter_info *firstinverter)
{
	FILE *fp;
	int i, j, res;
	char id[13];
	int limitedresult;
	char readpresetdata[128] = {'\0'};
	inverter_info *curinverter = firstinverter;

	memset(id, '\0', sizeof(id));
	fp = fopen("/tmp/getmaxpower.conf", "r");
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

		fp = fopen("/tmp/getmaxpower.conf", "w");
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
}
