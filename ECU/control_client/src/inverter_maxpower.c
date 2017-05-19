/*****************************************************************************/
/* File      : inverter_maxpower.c                                           */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-04-03 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "remote_control_protocol.h"
#include "debug.h"
#include "myfile.h"
#include "rtthread.h"

/*********************************************************************
power表格字段：
id,limitedpower,limitedresult,stationarypower,stationaryresult,flag
**********************************************************************/
/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
#define MAXPOWER_RANGE "020300"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern rt_mutex_t record_data_lock ;

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/

/* 设置指定台数逆变器最大功率 */
int set_maxpower_num(const char *msg, int num)
{
	int i, maxpower, err_count = 0;
	char inverter_id[13] = {'\0'};
	char str[100];
	for(i=0; i<num; i++)
	{
		//获取一台逆变器的ID号
		strncpy(inverter_id, &msg[i*18], 12);
		//获取最大功率
		maxpower = msg_get_int(&msg[i*18 + 12], 3);
		if(maxpower < 0)
			continue;
	
		//如果存在该逆变器数据则删除该记录
		delete_line("/home/data/power","/home/data/power_t",inverter_id,12);
		sprintf(str,"%s,%3d,,,,1\n",inverter_id,maxpower);
		//插入数据
		if(-1 == insert_line("/home/data/power",str))
		{
			err_count++;
		}
	}

	return err_count;
}

/* 设置所有逆变器最大功率 */
int set_maxpower_all(int maxpower)
{
	char inverter_ids[MAXINVERTERCOUNT][13] = {"\0"};
	int i,num;
	char msg[2048] = {'\0'};

	//查询所有逆变器ID号
	num = get_num_from_id(inverter_ids);


	//将所有逆变器拼接成设置单台的形式
	for(i=0;i<num;i++){
		msgcat_s(msg, 12, inverter_ids[i]);
		msgcat_d(msg, 3, maxpower);
		msgcat_s(msg, 3, "END");
	}
	return set_maxpower_num( msg, strlen(msg)/18);
}

/* 【A110】EMA设置逆变器最大功率 */
int set_inverter_maxpower(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	int type, maxpower, num;
	char timestamp[15] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	
	//获取设置类型标志位: 0设置全部, 1设置指定逆变器
	type = msg_get_int(&recvbuffer[30], 1);
	//获取逆变器数量
	num = msg_get_int(&recvbuffer[31], 4);
	//获取时间戳
	strncpy(timestamp, &recvbuffer[35], 14);

	{
		switch(type)
		{
			case 0:
				maxpower = msg_get_int(&recvbuffer[52], 3);
				if(maxpower >= 0){
					if(set_maxpower_all(maxpower) > 0)
						ack_flag = DB_ERROR;
				}
				break;
			case 1:
				//检查格式
				if(!msg_num_check(&recvbuffer[52], num, 15, 1)){
					ack_flag = FORMAT_ERROR;
				}
				else{
					if(set_maxpower_num(&recvbuffer[52], num) > 0)
						ack_flag = DB_ERROR;
				}
				break;
			default:
				ack_flag = FORMAT_ERROR;
				break;
		}

	}
	//拼接应答消息
	msg_ACK(sendbuffer, "A110", timestamp, ack_flag);
	rt_mutex_release(record_data_lock);
	return 0;
}

/* 【A117】读取逆变器最大功率及范围 */
int response_inverter_maxpower(const char *recvbuffer, char *sendbuffer)
{

	int ack_flag = SUCCESS;

	char timestamp[15] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);

	//设置读取所有逆变器最大功率的指令
	if(file_set_one("ALL", "/tmp/maxpower.con") < 0){
		ack_flag = FILE_ERROR;
	}

	strncpy(timestamp, &recvbuffer[34], 14);

	//拼接应答消息
	msg_ACK(sendbuffer, "A117", timestamp, ack_flag);
	rt_mutex_release(record_data_lock);
	return 0;
}
