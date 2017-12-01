/*****************************************************************************/
/* File      : grid_quality.c                                                */
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
#include <string.h>
#include "remote_control_protocol.h"
#include "debug.h"
#include "myfile.h"
#include "rtthread.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern rt_mutex_t record_data_lock;

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
extern ecu_info ecu;
/* 【129】ECU上报系统的电网质量 */
int response_grid_quality(const char *recvbuffer, char *sendbuffer)
{
	char grid_quality[2] = {'\0'};
	char timestamp[15] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);

	//获取时间戳
	strncpy(timestamp, &recvbuffer[34], 14);
	//获取电网质量
	file_get_one(grid_quality, sizeof(grid_quality), "/yuneng/plcgridq.txt");

	//拼接信息
	msg_Header(sendbuffer, "A129");
	msgcat_s(sendbuffer, 12, ecu.id);
	msgcat_s(sendbuffer, 1, grid_quality);
	msgcat_s(sendbuffer, 14, timestamp);
	msgcat_s(sendbuffer, 3, "END");

	rt_mutex_release(record_data_lock);
	return 0;
}
