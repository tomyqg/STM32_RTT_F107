#include <stdio.h>
#include <string.h>
#include "remote_control_protocol.h"
#include "debug.h"
#include "myfile.h"
#include "rtthread.h"

/*********************************************************************
need_id表格字段：
wrongid,correct_id,set_flag
**********************************************************************/
extern rt_mutex_t record_data_lock;

/*【A148】读取异常3501uid*/
int read_wrong_id(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	char a;
	char timestamp[15] = {'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	strncpy(timestamp, &recvbuffer[34], 14);
	
	fp = fopen("/yuneng/getbadid.con", "w");
	if(fp){
		fputs("1",fp);
		fclose(fp);
	}
	fp = fopen("/yuneng/getbadid.con", "r");
	if(fp){
		a = fgetc(fp);
		fclose(fp);
	}
	if(a!='1')ack_flag=FILE_ERROR;
	msg_ACK(sendbuffer, "A148", timestamp,ack_flag);
	rt_mutex_release(record_data_lock);
	return 0;
}

int set_unnormal_id(const char *recvbuffer, char *sendbuffer)
{

	int num,i;
	char yuid[13];
	char nuid[13];
	int ack_flag = SUCCESS;
	char timestamp[15] = {'\0'};
	char str[100];
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	strncpy(timestamp, &recvbuffer[34], 14);	
	
	num = msg_get_int(&recvbuffer[30], 4);
	for(i=0;i<num;i++)
	{
		strncpy(yuid,&recvbuffer[51+28*i],12);
		strncpy(nuid,&recvbuffer[64+28*i],12);
		
		//如果存在该逆变器数据则删除该记录
		delete_line("/home/data/need_id","/home/data/need_id.t",nuid,12);
		sprintf(str,"%s,%s,1\n",nuid,yuid);
		//插入数据
		if(-1 == insert_line("/home/data/need_id",str))
		{
			ack_flag=DB_ERROR;
		}

	}
	
	msg_ACK(sendbuffer, "A150", timestamp, ack_flag);
	rt_mutex_release(record_data_lock);
	return 0;
}
