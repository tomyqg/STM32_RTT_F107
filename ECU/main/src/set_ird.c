/*****************************************************************************/
/*  File      : set_ird.c                                                    */
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
#include <string.h>
#include "zigbee.h"
#include "variation.h"
#include "debug.h"
#include "file.h"
#include "myfile.h"
#include "rtthread.h"
#include "rthw.h"

/*
ird表格字段
id, result, set_value, set_flag
*/
/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern rt_mutex_t record_data_lock;
extern inverter_info inverter[MAXINVERTERCOUNT];

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int send_ird_command_single(int shortaddr, char value)		//单台设置逆变器ird
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0xC6;			//CMD
	sendbuff[4] = value;		//DATA
	sendbuff[5] = 0x00;
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

	printhexmsg(ECU_DBG_MAIN,"Set IRD to single", (char *)sendbuff, 13);

	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 13);
	rt_hw_s_delay(2);

	return 0;
}

int send_ird_command_all(char value)		//广播设置逆变器ird
{
	unsigned char sendbuff[512]={'\0'};
	unsigned short check=0x00;
	int i;
	

	sendbuff[0] = 0xFB;			//HEAD
	sendbuff[1] = 0xFB;			//HEAD
	sendbuff[2] = 0x06;			//LENGTH
	sendbuff[3] = 0xA6;			//CMD
	sendbuff[4] = value;		//DATA
	sendbuff[5] = 0x00;
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

	printhexmsg(ECU_DBG_MAIN,"Set IRD to all", (char *)sendbuff, 13);
	zb_broadcast_cmd((char *)sendbuff, 13);
	rt_hw_s_delay(10);
	
	return 0;
}

int resolve_ird(char *id, char *readbuff)		//解析并保存IRD设置结果
{
	char inverter_result[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL]={'\0'};
	int i, mode;
	char data[200];
	char splitdata[4][32];
	mode = ((readbuff[3+1] >> 1) & 0x03);
	
	//读取所在ID行
	if(1 == read_line("/home/data/ird",data,id,12))
	{	
		//将所在行分裂
		splitString(data,splitdata);
		memset(data,0x00,200);
		sprintf(data,"%s,%d,,0\n",id,mode);

		//删除id所在行
		delete_line("/home/data/ird","/home/data/ird.t",id,12);
		//更新所在行
		for(i=0; i<3; i++)
		{
			if(1 == insert_line("/home/data/ird",data))
			{
				print2msg(ECU_DBG_MAIN,id, "Update resolve ird successfully");
				break;
			}
			else
				print2msg(ECU_DBG_MAIN,id, "Failed to resolve ird power");
		}
		sprintf(inverter_result, "%s%01dEND", id, mode);				//这里先注释掉
		save_inverter_parameters_result2(id, 126, inverter_result);		//把结果保存到数据库，通过远程控制程序上传给EMA
	}else
	{
		memset(data,0x00,200);
		sprintf(data,"%s,%d,,0\n",id,mode);

		//删除id所在行
		delete_line("/home/data/ird","/home/data/ird.t",id,12);
		//更新所在行
		for(i=0; i<3; i++)
		{
			if(1 == insert_line("/home/data/ird",data))
			{
				print2msg(ECU_DBG_MAIN,id, "Update resolve ird successfully");
				break;
			}
			else
				print2msg(ECU_DBG_MAIN,id, "Failed to resolve ird power");
		}
		sprintf(inverter_result, "%s%01dEND", id, mode);				//这里先注释掉
		save_inverter_parameters_result2(id, 126, inverter_result);		//把结果保存到数据库，通过远程控制程序上传给EMA
	}


	return 0;
}

int resolve_ird_DD(char *id, char *readbuff)		//解析并保存IRD设置结果
{
	char inverter_result[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL]={'\0'};
	char data[200];
	char splitdata[4][32];
	int i, mode;
	
	mode = (int)readbuff[3+19];
//读取所在ID行
	if(1 == read_line("/home/data/ird",data,id,12))
	{
		//将所在行分裂
		splitString(data,splitdata);
		memset(data,0x00,200);
		sprintf(data,"%s,%d,,0\n",id,mode);

		//删除id所在行
		delete_line("/home/data/ird","/home/data/ird.t",id,12);
		//更新所在行
		for(i=0; i<3; i++)
		{
			if(1 == insert_line("/home/data/ird",data))
			{
				print2msg(ECU_DBG_MAIN,id, "Update resolve ird DD successfully");
				break;
			}
			else
				print2msg(ECU_DBG_MAIN,id, "Failed to resolve ird DD power");
		}

		sprintf(inverter_result, "%s%01dEND", id, mode);				//这里先注释掉
		save_inverter_parameters_result2(id, 126, inverter_result);		//把结果保存到数据库，通过远程控制程序上传给EMA
	}
	else
	{
		memset(data,0x00,200);
		sprintf(data,"%s,%d,,0\n",id,mode);

		//删除id所在行
		delete_line("/home/data/ird","/home/data/ird.t",id,12);
		//更新所在行
		for(i=0; i<3; i++)
		{
			if(1 == insert_line("/home/data/ird",data))
			{
				print2msg(ECU_DBG_MAIN,id, "Update resolve ird DD successfully");
				break;
			}
			else
				print2msg(ECU_DBG_MAIN,id, "Failed to resolve ird DD power");
		}

		sprintf(inverter_result, "%s%01dEND", id, mode);				//这里先注释掉
		save_inverter_parameters_result2(id, 126, inverter_result);		//把结果保存到数据库，通过远程控制程序上传给EMA
	}

	return 0;
}

int get_ird_single(int shortaddr,char* id)		//从逆变器读取实际IRD
{
	unsigned char sendbuff[256]={'\0'};
	unsigned char readbuff[256]={'\0'};
	unsigned short check=0x00;
	int i, res;

	clear_zbmodem();			//发送指令前，先清空缓冲区
	sendbuff[0] = 0xFB;
	sendbuff[1] = 0xFB;
	sendbuff[2] = 0x06;
	sendbuff[3] = 0xDD;
	sendbuff[4] = 0x00;
	sendbuff[5] = 0x00;
	sendbuff[6] = 0x00;
	sendbuff[7] = 0x00;
	sendbuff[8] = 0x00;
	for(i=2; i<9; i++)
	{
		check = check + sendbuff[i];
	}
	sendbuff[9] = check >> 8;		//CHK
	sendbuff[10] = check;		//CHK
	sendbuff[11] = 0xFE;
	sendbuff[12] = 0xFE;

	print2msg(ECU_DBG_MAIN,id, "get_ird_single");
	zb_shortaddr_cmd(shortaddr, (char *)sendbuff, 13);

	res = zb_shortaddr_reply((char *)readbuff,shortaddr,id);


	if((58 == res) &&
			(0xFB == readbuff[0]) &&
			(0xFB == readbuff[1]) &&
			(0xDA == readbuff[3]) &&
			(0xFE == readbuff[56]) &&
			(0xFE == readbuff[57]))
	{
		resolve_ird(id, (char *)readbuff);		//解析和保存IRD
		return 0;
	}
	else if ((33 == res) &&
				(0xFB == readbuff[0]) &&
				(0xFB == readbuff[1]) &&
				(0xDD == readbuff[3]) &&
				(0xFE == readbuff[31]) &&
				(0xFE == readbuff[32])) {
		resolve_ird_DD(id, (char *)readbuff);		//解析和保存IRD
		return 0;
	}
	else
		return -1;

}

int get_ird_id_value(char *id, char *value)		//从数据库中获取一台要设置电网的逆变器的ID和IRD
{
	int index = 0,flag = 0;
	FILE *fp;
	char data[200];
	char splitdata[4][32];
	inverter_info *curinverter = inverter;
	fp = fopen("/home/data/ird", "r");
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
	for(index=0; (index<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); index++, curinverter++)			//有效逆变器轮训
	{
		if(!strcmp(curinverter->id,id))
		{
			return curinverter->shortaddr;
		}
	}
	return 0;
}

int clear_ird_flag_single(char *id)					//设置后清除数据库中参数的设置标志
{
	char data[200];
	char splitdata[4][32];
	int i;

	//读取所在ID行
	if(1 == read_line("/home/data/ird",data,id,12))
	{
		//将所在行分裂
		splitString(data,splitdata);
		memset(data,0x00,200);
		sprintf(data,"%s,%d,,0\n",id,atoi(splitdata[1]));

		//删除id所在行
		delete_line("/home/data/ird","/home/data/ird.t",id,12);
		//更新所在行
		for(i=0; i<3; i++)
		{
			if(1 == insert_line("/home/data/ird",data))
			{
				print2msg(ECU_DBG_MAIN,id, "Update clear_ird_flag_single successfully");
				break;
			}
			else
				print2msg(ECU_DBG_MAIN,id, "Failed to clear_ird_flag_single power");
		}
	}

	return 0;
}

int clear_ird_flag_all()					//设置后清除数据库中参数的设置标志
{
	FILE *fp;

	fp = fopen("/tmp/set_ird.con", "w");		//清空设置文件
	
	if(fp)
	{
		fclose(fp);
	}

	return 0;
}

int get_ird_all(struct inverter_info_t *firstinverter)		//读取所有逆变器的IRD
{
	int i, j;
	struct inverter_info_t *inverter = firstinverter;
	
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++){
		for(j=0; j<3; j++){
			if(!get_ird_single(inverter->shortaddr,inverter->id))		//读取一台逆变器的IRD
				break;
		}
	}
	
	return 0;
}

int set_ird_single()		//设置单台逆变器IRD
{
	char id[16]={'\0'};
	char value[16]={'\0'};
	int shortaddr;
	int index = 0;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	while(1){
		shortaddr= get_ird_id_value(id, value);
		if(-1 == shortaddr)		//从数据库中获取一台要设置电网的逆变器的ID和IRD，没有就退出
			break;
		else if(0 == shortaddr)
			clear_ird_flag_single(id);
		else{
			clear_ird_flag_single(id);
			send_ird_command_single(shortaddr, atoi(value));	//设置一台逆变器IRD
			for(index=0; index<3; index++){
				if(!get_ird_single(shortaddr,id))	//读取逆变器的设置结果
					break;			
			}
		}
	}
	rt_mutex_release(record_data_lock);
	return 0;
}

int set_ird_all(inverter_info *firstinverter)		//设置所有逆变器IRD
{
	FILE *fp;
	char buff[256]={'\0'};
	char id[256]={'\0'};
	char value[256]={'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	
	fp = fopen("/tmp/set_ird.con", "r");
	if(fp)
	{
		fgets(buff, 255, fp);
		fclose(fp);
		
		if(strlen(buff)>0){
			sscanf(buff, "%[^,],%s", &id, &value);
			
			clear_ird_flag_all();

			if(!strcmp(id, "ALL")){
				send_ird_command_all(atoi(value));		//广播设置
				get_ird_all(firstinverter);	//读取所有逆变器结果
			}
		}
	}
	rt_mutex_release(record_data_lock);
	return 0;
}

int get_ird_from_inverters(inverter_info *firstinverter)		//设置所有逆变器IRD
{
	FILE *fp;
	char buff[256]={'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	//set_grid_environment_all(firstinverter);	//在系统中有逆变器上传实时数据时才设置所有

	fp = fopen("/tmp/get_ird.con", "r");	//不设置只读
	if(fp){
		fgets(buff, sizeof(buff), fp);
		fclose(fp);

		if(!strcmp(buff, "ALL")){
			fp = fopen("/tmp/get_ird.con", "w");	//清空文件后读取
			if(fp){
				fclose(fp);
			}
			get_ird_all(firstinverter);
		}
	}
	rt_mutex_release(record_data_lock);
	return 0;
}

int process_ird(inverter_info *firstinverter)
{
	set_ird_single();
	return 0;
}

int process_ird_all(inverter_info *firstinverter)
{
	set_ird_all(firstinverter);
	get_ird_from_inverters(firstinverter);
	return 0;
}
