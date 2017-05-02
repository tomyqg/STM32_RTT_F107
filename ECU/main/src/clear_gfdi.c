#include <stdio.h>
#include <stdlib.h>
#include "variation.h"
#include <string.h>
#include "clear_gfdi.h"
#include "file.h"
#include "myfile.h"
#include "zigbee.h"
#include "debug.h"

/*
clrgfdi±í¸ñ×Ö¶Î:
id, set_flag
*/
extern rt_mutex_t record_data_lock;

int get_clear_gfdi_flag(char *id)
{
	int flag1=0;
	FILE *fp;
	char data[200];
	char splitdata[2][32];
	fp = fopen("/home/data/clrgfdi", "r");
	if(fp)
	{
		memset(data,0x00,200);
		
		while(NULL != fgets(data,200,fp))
		{
			memset(splitdata,0x00,2*32);
			splitString(data,splitdata);

			strcpy(id, splitdata[0]);
			flag1 = 1;
			break;
		}
		fclose(fp);
	}
	return flag1;
}

int clear_clear_gfdi_flag(char *id)
{
	delete_line("/home/data/clrgfdi","/home/data/clrgfdi.t",id,12);
	return 0;
}

int clear_gfdi(inverter_info *firstinverter)
{
	int i, j;
	char id[256] = {'\0'};
	inverter_info *curinverter = firstinverter;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);

	for(j=0; j<3; j++)
	{
		curinverter = firstinverter;
		memset(id, '\0', 256);
		if(!get_clear_gfdi_flag(id))
			break;

		clear_clear_gfdi_flag(id);
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++)
		{
			if(!strcmp(id, curinverter->id))
			{
			//	send_clean_gfdi(curinverter);
				zb_clear_gfdi(curinverter);
				print2msg(ECU_DBG_MAIN,curinverter->id, "Clear GFDI");

				break;
			}
			curinverter++;
		}
	}
	rt_mutex_release(record_data_lock);

	return 0;
}
