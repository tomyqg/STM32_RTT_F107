/*****************************************************************************/
/*  File      : turn_on_off.c                                                */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-03-05 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "variation.h"
#include <string.h>
#include "turn_on_off.h"
#include "zigbee.h"
#include "myfile.h"
#include "file.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern rt_mutex_t record_data_lock;

/*********************************************************************
turnonof±í¸ñ×Ö¶Î:
id, set_flag
**********************************************************************/

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/

int get_turn_on_off_flag(char *id, int *flag)
{
	int flag1=0;
	FILE *fp;
	char data[200];
	char splitdata[2][32];
	fp = fopen("/home/data/turnonof", "r");
	if(fp)
	{
		memset(data,0x00,200);
		
		while(NULL != fgets(data,200,fp))
		{
			memset(splitdata,0x00,2*32);
			splitString(data,splitdata);
			strcpy(id, splitdata[0]);
			*flag = atoi(splitdata[1]);
			flag1 = 1;
			break;
		}
		fclose(fp);
	}
	return flag1;
}

int clear_turn_on_off_flag(char *id)
{
	delete_line("/home/data/turnonof","/home/data/_turnof",id,12);
	return 0;
}

int turn_on_off(inverter_info *firstinverter)
{
	int i, j;
	char id[256] = {'\0'};
	inverter_info *curinverter = firstinverter;
	int flag;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	while(1){
		curinverter = firstinverter;
		memset(id, '\0', 256);
		if(!get_turn_on_off_flag(id, &flag))
			break;

		clear_turn_on_off_flag(id);
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++){
			if(!strncmp(id, curinverter->id, 12)){
				j = 0;
				if(1 == flag){
					while(j<3){
						if(1 == zb_boot_single(curinverter))
							break;
						j++;
					}
				}
				if(2 == flag){
					while(j<3){
						if(1 == zb_shutdown_single(curinverter))
							break;
						j++;
					}
				}

				break;
			}
			curinverter++;
		}
	}
	rt_mutex_release(record_data_lock);
	return 0;
}
