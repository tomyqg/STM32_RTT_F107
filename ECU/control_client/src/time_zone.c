/*****************************************************************************/
/* File      : time_zone.c                                                   */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-04-07 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "mycommand.h"
#include "remote_control_protocol.h"
#include "myfile.h"
#include "datetime.h"
#include "debug.h"
#include "threadlist.h"
#include "rthw.h"

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
/* 【A104】ECU上报本地时区 */
int response_time_zone(const char *recvbuffer, char *sendbuffer)
{
	char ecuid[13] = {'\0'};	  //ECU号码
	char local_time[15] = {'\0'}; //ECU本地时间
	char timestamp[15] = {'\0'};  //时间戳
	char timezone[64] = {'\0'};  //时区

	//获取参数
	file_get_one(ecuid, sizeof(ecuid), "/yuneng/ecuid.con");
	getcurrenttime(local_time);
	strncpy(timestamp, &recvbuffer[34], 14);
	file_get_one(timezone, sizeof(timezone), "/yuneng/timezone.con");

	//拼接信息
	msg_Header(sendbuffer, "A104");
	msgcat_s(sendbuffer, 12, ecuid);
	msgcat_s(sendbuffer, 14, local_time);
	msgcat_s(sendbuffer, 14, timestamp);
	strcat(sendbuffer, timezone);
	msgcat_s(sendbuffer, 3, "END");

	return 0;
}

/* 【A105】EMA设置ECU本地时区 */
int set_time_zone(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	char timestamp[15] = {'\0'};
	char timezone[64] = {'\0'};

	//获取时间戳
	strncpy(timestamp, &recvbuffer[30], 14);
	//获取时区
	strncpy(timezone, &recvbuffer[44], strlen(&recvbuffer[44])-3);
	print2msg(ECU_DBG_CONTROL_CLIENT,"zone", timezone);

	/*配置处理*/
	
	file_set_one(timezone, "/yuneng/timezone.con");//将时区保存到配置文件
	reboot_timer(10);
	rt_hw_us_delay(100000);
	
	//拼接应答消息
	msg_ACK(sendbuffer, "A105", timestamp, ack_flag);

	return 104;
}
