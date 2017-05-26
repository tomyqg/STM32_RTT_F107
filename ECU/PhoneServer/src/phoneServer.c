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
		//Recv socket A data by serial,If the data is received, the phone is sent.
		length = RecvSocketData(SOCKET_A,data,2);
		if(length > 0)
		{
			memcpy(ID,&data[2],8);
			ID[8] = '\0';
			print2msg(ECU_DBG_WIFI,"phone_server",&data[9]);
			
			//function part
		
		}
		rt_thread_delay(RT_TICK_PER_SECOND);
	}
	
}