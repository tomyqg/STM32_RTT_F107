/*****************************************************************************/
/* File      : phoneServer.c                                                 */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-05-19 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "phoneServer.h"
#include "rtthread.h"
#include "usr_wifi232.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "debug.h"
#include "threadlist.h"

extern rt_mutex_t usr_wifi_lock;


/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   Phone Server Application program entry                                  */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   parameter  unused                                                       */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/
void phone_server_thread_entry(void* parameter)
{
	char *data = NULL;
	int length = 0;
	char ID[9] = {'\0'};
	data = malloc(2048);	
	while(1)
	{
		memset(data,0x00,2048);
#ifdef WIFI_USE 	
		rt_mutex_take(usr_wifi_lock, RT_WAITING_FOREVER);
		//Recv socket A data by serial,If the data is received, the phone is sent.
		length = RecvSocketData(SOCKET_A,data,1);
		rt_mutex_release(usr_wifi_lock);
		if(length > 0)
		{
			//printf("ID 1 :%x %x %x %x %x %x %x %x\n",data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8]);
			
			memcpy(ID,&data[1],8);
			//printf("ID 2 :%x %x %x %x %x %x %x %x\n",ID[0],ID[1],ID[2],ID[3],ID[4],ID[5],ID[6],ID[7]);
			ID[8] = '\0';
			print2msg(ECU_DBG_WIFI,"phone_server",&data[9]);
			
			//检查是哪个功能函数
			//function part
			if(!memcmp(&data[9],"APS11001401END",14))
			{
				//printf("------1111111111111111111111111111111\n");
				SendToSocketA("APS11001402END" ,14,ID);
			}
			
			
		}
#endif		
		rt_thread_delay(RT_TICK_PER_SECOND);
	}
	//free(data);
	
}
