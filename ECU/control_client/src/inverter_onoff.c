#include <stdio.h>
#include <string.h>
#include <remote_control_protocol.h>
#include "debug.h"
#include "myfile.h"
#include "rtthread.h"
#include "dfs_posix.h"
extern rt_mutex_t record_data_lock;


/*********************************************************************
turnonof表格字段：
id, set_flag
**********************************************************************/

/* 设置所有逆变器的开关机 */
int turn_onoff_all(int type)
{
	char inverter_ids[MAXINVERTERCOUNT][13] = {"\0"};
	int i, count = 0,num;
	char str[50];
	int fd = 0;
	fd = open("/home/data/turnonof", O_WRONLY | O_TRUNC | O_CREAT,0);
	if(fd >= 0)
	{
		//查询所有逆变器ID号
		num = get_num_from_id(inverter_ids);
		printdecmsg(ECU_DBG_CONTROL_CLIENT,"turn_onoff_all",num);
		for(i=1; i<=num; i++)
		{
			sprintf(str,"%s,%d\n",inverter_ids[i-1],type);
			//插入数据
			if(write(fd,str,strlen(str)) >= 0)
			{
				count++;
			}
			
		}
		close(fd);
	}
	
	return count;
}

/* 设置指定台数逆变器的开关机 */
int turn_onoff_num( const char *msg, int num)
{
	int i, onoff, err_count = 0;
	char inverter_id[13] = {'\0'};
	char str[50];
	int fd = 0;
	fd = open("/home/data/turnonof", O_WRONLY | O_TRUNC | O_CREAT,0);
	if(fd >= 0)
	{
		for(i=0; i<num; i++)
		{
			//获取一台逆变器的ID号
			strncpy(inverter_id, &msg[i*16], 12);
			//获取开机或关机(注意:协议中0是开,1是关;但在ECU数据库中1是开,2是关...所以这里加上1)
			onoff = msg_get_int(&msg[i*16 + 12], 1) + 1;
			
			sprintf(str,"%s,%d\n",inverter_id,onoff);
			//插入一条开关机指令
			if(write(fd,str,strlen(str)) <= 0)
			{
				err_count++;
			}
		}
		close(fd);
	}

	return err_count;
}

/* 【A111】EMA设置逆变器开关机 */
int set_inverter_onoff(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	int type, num;
	char timestamp[15] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);

	//获取设置类型标志位: 0全开; 1全关; 2指定逆变器开关
	type = msg_get_int(&recvbuffer[30], 1);
	//获取逆变器数量
	num = msg_get_int(&recvbuffer[31], 4);
	//获取时间戳
	strncpy(timestamp, &recvbuffer[35], 14);

	switch(type)
	{	
		case 0:
			turn_onoff_all(1);
			break;
		case 1:
			turn_onoff_all(2);
			break;
		case 2:
			//检查格式
			if(!msg_num_check(&recvbuffer[52], num, 13, 1)){
				ack_flag = FORMAT_ERROR;
			}
			else{
				if(turn_onoff_num(&recvbuffer[52], num) > 0)
					ack_flag = DB_ERROR;
			}
			break;
		default:
			ack_flag = FORMAT_ERROR;
			break;
	}
	rt_mutex_release(record_data_lock);
	//拼接应答消息
	msg_ACK(sendbuffer, "A111", timestamp, ack_flag);
	return 0;
}
