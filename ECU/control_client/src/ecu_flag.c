#include <string.h>
#include "remote_control_protocol.h"
#include "debug.h"
#include "myfile.h"

/* 【A119】EMA设置ECU的通信开关 */
int set_ecu_flag(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	int ecu_flag = 1;
	char timestamp[15] = {'\0'};

	//获取时间戳
	strncpy(timestamp, &recvbuffer[30], 14);
	//通信标志位
	ecu_flag = msg_get_int(&recvbuffer[44], 1);

	if(ecu_flag == 0)
		file_set_one("0", "/yuneng/ecu_flag.con");
	else if(ecu_flag == 1)
		file_set_one("1", "/yuneng/ecu_flag.con");
	else
		ack_flag = FORMAT_ERROR;

	//拼接应答消息
	msg_ACK(sendbuffer, "A119", timestamp, ack_flag);
	return -1;
}
