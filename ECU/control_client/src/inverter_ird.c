#include <stdio.h>
#include <string.h>
#include "remote_control_protocol.h"
#include "debug.h"
#include "myfile.h"
#include "rtthread.h"
/*********************************************************************
ird表格字段：
id, result, set_value, set_flag
**********************************************************************/
extern rt_mutex_t record_data_lock;

/* 设置指定台数逆变器的IRD选项 */
int set_ird_num(const char *msg, int num)
{

	int i, ird, err_count = 0;
	char inverter_id[13] = {'\0'};
	char str[100];
	
	{
		for(i=0; i<num; i++)
		{
			//获取一台逆变器的ID号
			strncpy(inverter_id, &msg[i*16], 12);
			//获取IRD设置值
			ird = msg_get_int(&msg[i*16 + 12], 1);

			
			//如果存在该逆变器数据则删除该记录
			delete_line("/home/data/ird","/home/data/ird_t",inverter_id,12);
			sprintf(str,"%s,,%d,1\n",inverter_id,ird);
			//插入数据
			if(-1 == insert_line("/home/data/ird",str))
			{
				err_count++;
			}
		}

	}
	return err_count;
}

/* 【A126】EMA读取逆变器的IRD选项 */
int read_inverter_ird(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	char timestamp[15] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	//获取时间戳
	strncpy(timestamp, &recvbuffer[34], 14);

	//读取逆变器的IRD设置选项
	if(file_set_one("ALL", "/tmp/get_ird.con") < 0)
		ack_flag = FILE_ERROR;

	//拼接应答消息
	msg_ACK(sendbuffer, "A126", timestamp, ack_flag);
	rt_mutex_release(record_data_lock);
	return 0;
}

/* 【A127】EMA设置逆变器的IRD选项 */
int set_inverter_ird(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	int type, num, ird;
	char timestamp[15] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	
	//获取设置类型标志位: 0设置所有逆变器，1设置指定逆变器
	type = msg_get_int(&recvbuffer[30], 1);
	//获取逆变器数量
	num = msg_get_int(&recvbuffer[31], 4);
	//获取时间戳
	strncpy(timestamp, &recvbuffer[35], 14);

	switch(type)
	{
		case 0:
			//设置所有逆变器，存入配置文件
			ird = msg_get_int(&recvbuffer[52], 1);
			if(ird == 1)
				file_set_one("ALL,1", "/tmp/set_ird.con");
			else if(ird == 2)
				file_set_one("ALL,2", "/tmp/set_ird.con");
			else if(ird == 3)
				file_set_one("ALL,3", "/tmp/set_ird.con");
			else
				ack_flag = FORMAT_ERROR;
			break;
		case 1:
			//检查格式（逆变器数量）
			if(!msg_num_check(&recvbuffer[52], num, 13, 1)){
				ack_flag = FORMAT_ERROR;
			}
			else{
				//设置指定逆变器，存入数据库
				if(set_ird_num(&recvbuffer[52], num) > 0)
					ack_flag = DB_ERROR;
			}
			break;
		default:
			ack_flag = FORMAT_ERROR;
			break;
	}
	//拼接应答消息
	msg_ACK(sendbuffer, "A127", timestamp, ack_flag);
	rt_mutex_release(record_data_lock);
	return 0;
}
