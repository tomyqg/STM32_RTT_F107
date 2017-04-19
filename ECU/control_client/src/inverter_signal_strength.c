#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "remote_control_protocol.h"
#include "debug.h"
#include "myfile.h"
#include "rtthread.h"

/*********************************************************************
signstre表格字段：
id ,signal_strength,set_flag
**********************************************************************/
extern rt_mutex_t record_data_lock;

int read_signal_strength_num(const char *msg, int num)
{

	int i, err_count = 0;
	char inverter_id[13] = {'\0'};
	char str[100];
	
	for(i=0; i<num; i++)
	{
		//获取一台逆变器的ID号
		strncpy(inverter_id, &msg[i*12], 12);
		
		//如果存在该逆变器数据则删除该记录
		delete_line("/home/data/signstre","/home/data/signstre.t",inverter_id,12);
		sprintf(str,"%s,,1\n",inverter_id);
		//插入数据
		if(-1 == insert_line("/home/data/signstre",str))
		{
			err_count++;
		}
	}

	return err_count;
}

/*【A128】EMA读取逆变器的信号强度*/
int read_inverter_signal_strength(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	int type, num;
	char timestamp[15] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	//获取设置类型标志位: 0读取所有逆变器，1读取指定逆变器
	type = msg_get_int(&recvbuffer[30], 1);

	//获取时间戳
	strncpy(timestamp, &recvbuffer[31], 14);

	switch(type)
	{
		case 0:
			//读取所有逆变器，存入配置文件
			file_set_one("ALL", "/tmp/readalsg.con");
			break;
		case 1:
			//获取逆变器数量
			num = msg_get_int(&recvbuffer[48], 4);

			//检查格式（逆变器数量）
			if(!msg_num_check(&recvbuffer[52], num, 12, 0)){
				ack_flag = FORMAT_ERROR;
			}
			else{
				//读取指定逆变器，存入数据库
				if(read_signal_strength_num(&recvbuffer[52], num) > 0)
					ack_flag = DB_ERROR;
			}
			break;
		default:
			ack_flag = FORMAT_ERROR;
			break;
	}
	file_set_one("1", "/tmp/uploadsg.con");

	//拼接应答消息
	msg_ACK(sendbuffer, "A128", timestamp, ack_flag);
	rt_mutex_release(record_data_lock);
	return 0;
}
