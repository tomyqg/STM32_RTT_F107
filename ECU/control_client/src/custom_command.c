#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include "remote_control_protocol.h"
#include "mydebug.h"
#include "mycommand.h"

/* 【A108】EMA向ECU发送自定义命令 */
int custom_command(const char *recvbuffer, char *sendbuffer)
{
	int res, ack_flag = SUCCESS;
	char command[256] = {'\0'};
	char timestamp[15] = {'\0'};

	//时间戳
	strncpy(timestamp, &recvbuffer[30], 14);

	//自定义命令
	if(msg_get_one_section(command, &recvbuffer[47]) <= 0){
		ack_flag = FORMAT_ERROR;
	}
	else{
		//结束程序命令
		if(!strncmp(command, "quit", 4)){
			debug_msg("Ready to quit");
			msg_ACK(sendbuffer, "A108", timestamp, ack_flag);
			return -1;
		}
		//执行自定义命令
		ack_flag = mysystem(command);
	}
	msg_ACK(sendbuffer, "A108", timestamp, ack_flag);
	return 0;
}
