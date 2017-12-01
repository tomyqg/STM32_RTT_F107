/*****************************************************************************/
/*  File      : ema_control.c                                                */
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
#include <string.h>
#include "variation.h"
#include "datetime.h"
#include "ema_control.h"
#include "file.h"

//extern int caltype;		//计算方式，NA版和非NA版的区别
extern ecu_info ecu;

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int write_gfdi_status(inverter_info *firstinverter) 
{
	FILE *fp;
	int i;
	inverter_info *inverter = firstinverter;
	char *write_buff = NULL;
	write_buff = malloc(MAXINVERTERCOUNT*INVARTER_STATUS_PER_LEN+INVARTER_STATUS_PER_OTHER);	//ÿ̨16���ֽ�  20���ֽ���ΪԤ��
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++)
	{
		//if('0' != inverter->flag)
		//{
			strcat(write_buff, inverter->id);
			if(inverter->inverterstatus.last_gfdi_flag == 0)
			{
				write_buff[strlen(write_buff)] = '0';
			}else{
				write_buff[strlen(write_buff)] = '1';
			}
			
			strcat(write_buff, "END\n");
		//}
	}

	fp = fopen("/YUNENG/LASTGFDI.TXT", "w");
	if(fp)
	{
		fputs(write_buff, fp);
		fclose(fp);
	}
	free(write_buff);
	write_buff = NULL;
	return 0;
}

int write_turn_on_off_status(inverter_info *firstinverter)
{
	FILE *fp;
	int i;
	inverter_info *inverter = firstinverter;
	char *write_buff = NULL;
	write_buff = malloc(MAXINVERTERCOUNT*INVARTER_STATUS_PER_LEN+INVARTER_STATUS_PER_OTHER);	//ÿ̨16���ֽ�  20���ֽ���ΪԤ��
	
	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++)
	{
		//if('0' != inverter->flag)
		//{
			strcat(write_buff, inverter->id);
			if(inverter->inverterstatus.last_turn_on_off_flag == 0)
			{
				write_buff[strlen(write_buff)] = '0';
			}else{
				write_buff[strlen(write_buff)] = '1';
			}
			
			strcat(write_buff, "END\n");
		//}
	}

	fp = fopen("/YUNENG/LASTTURN.TXT", "w");
	if(fp)
	{
		fputs(write_buff, fp);
		fclose(fp);
	}
	free(write_buff);
	write_buff = NULL;
	return 0;
}

int read_gfdi_turn_on_off_status(inverter_info *firstinverter)
{
	FILE *fp;
	int i;
	char buff[256];
	inverter_info *inverter = firstinverter;

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++)
	{
		inverter->inverterstatus.last_gfdi_flag = 0;
		inverter->inverterstatus.last_turn_on_off_flag = 0;
	}

	fp = fopen("/YUNENG/LASTGFDI.TXT", "r");
	if(fp)
	{
		while(1)
		{
			memset(buff, '\0', sizeof(buff));
			fgets(buff, sizeof(buff), fp);
			if(0 == strlen(buff))
				break;
			else
			{
				inverter = firstinverter;
				for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++)
				{
					if(!strncmp(inverter->id, buff, 12))
					{
						if('1' == buff[12])
							inverter->inverterstatus.last_gfdi_flag = 1;
						else
							inverter->inverterstatus.last_gfdi_flag = 0;
						break;
					}
				}
			}
		}
		fclose(fp);
	}
	else
	{
		inverter = firstinverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++)
		{
			inverter->inverterstatus.last_gfdi_flag = 0;
		}
	}

	inverter = firstinverter;
	fp = fopen("/YUNENG/LASTTURN.TXT", "r");
	if(fp)
	{
		while(1)
		{
			memset(buff, '\0', sizeof(buff));
			fgets(buff, sizeof(buff), fp);
			if(0 == strlen(buff))
				break;
			else
			{
				inverter = firstinverter;
				for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++)
				{
					if(!strncmp(inverter->id, buff, 12))
					{
						if('1' == buff[12])
							inverter->inverterstatus.last_turn_on_off_flag = 1;
						else
							inverter->inverterstatus.last_turn_on_off_flag = 0;
						break;
					}
				}
			}
		}
		fclose(fp);
	}
	else
	{
		inverter = firstinverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++)
		{
			inverter->inverterstatus.last_turn_on_off_flag = 0;
		}
	}

	return 0;
}

int save_gfdi_changed_result(inverter_info *firstinverter)
{
	inverter_info *inverter = firstinverter;
	int i, count=0;
	char *gfdi_changed_result = NULL;

	gfdi_changed_result = malloc(PROCESS_RESULT_HEAD + PROCESS_RESULT_RECORD_LEN * MAXINVERTERCOUNT);
	
	strcpy(gfdi_changed_result, "APS13AAAAAA115AAA1");

	strcat(gfdi_changed_result, ecu.id);					//ECU的ID
	strcat(gfdi_changed_result, "0000");					//逆变器个数
	strcat(gfdi_changed_result, "00000000000000");		//时间戳，设置逆变器后返回的结果中时间戳为0
	strcat(gfdi_changed_result, "END");					//固定格式

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++)
	{
		if(1 == inverter->inverterstatus.gfdi_changed_flag)
		{
			strcat(gfdi_changed_result, inverter->id);
			if(inverter->inverterstatus.last_gfdi_flag == 0)
			{
				gfdi_changed_result[strlen(gfdi_changed_result)] = '0';
			}else{
				gfdi_changed_result[strlen(gfdi_changed_result)] = '1';
			}
			strcat(gfdi_changed_result, "END");
			count++;
		}
	}


	gfdi_changed_result[30] = count/1000 + 0x30;
	gfdi_changed_result[31] = (count/100)%10 + 0x30;
	gfdi_changed_result[32] = (count/10)%10 + 0x30;
	gfdi_changed_result[33] = count%10 + 0x30;

	if(strlen(gfdi_changed_result) > 10000)
		gfdi_changed_result[5] = strlen(gfdi_changed_result)/10000 + 0x30;
	if(strlen(gfdi_changed_result) > 1000)
		gfdi_changed_result[6] = (strlen(gfdi_changed_result)/1000)%10 + 0x30;
	if(strlen(gfdi_changed_result) > 100)
		gfdi_changed_result[7] = (strlen(gfdi_changed_result)/100)%10 + 0x30;
	if(strlen(gfdi_changed_result) > 10)
		gfdi_changed_result[8] = (strlen(gfdi_changed_result)/10)%10 + 0x30;
	if(strlen(gfdi_changed_result) > 0)
		gfdi_changed_result[9] = strlen(gfdi_changed_result)%10 + 0x30;

	//strcat(gfdi_changed_result, "\n");

	if(count >0)
		save_process_result(115, gfdi_changed_result);
	free(gfdi_changed_result);
	gfdi_changed_result = NULL;
	return 0;
}

int save_turn_on_off_changed_result(inverter_info *firstinverter)
{
	inverter_info *inverter = firstinverter;
	int i, count=0;
	char *turn_on_off_changed_result = NULL;
	turn_on_off_changed_result = malloc(PROCESS_RESULT_HEAD + PROCESS_RESULT_RECORD_LEN * MAXINVERTERCOUNT);
	memset(turn_on_off_changed_result,'\0',PROCESS_RESULT_HEAD + PROCESS_RESULT_RECORD_LEN * MAXINVERTERCOUNT);

	strcpy(turn_on_off_changed_result, "APS13AAAAAA116AAA1");

	strcat(turn_on_off_changed_result, ecu.id);					//ECU的ID
	strcat(turn_on_off_changed_result, "0000");					//逆变器个数
	strcat(turn_on_off_changed_result, "00000000000000");		//时间戳，设置逆变器后返回的结果中时间戳为0
	strcat(turn_on_off_changed_result, "END");					//固定格式

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++)
	{
		if(1 == inverter->inverterstatus.turn_on_off_changed_flag)
		{
			strcat(turn_on_off_changed_result, inverter->id);
			if(inverter->inverterstatus.last_turn_on_off_flag == 0)
			{
				turn_on_off_changed_result[strlen(turn_on_off_changed_result)] = '0';
			}else{
				turn_on_off_changed_result[strlen(turn_on_off_changed_result)] = '1';
			}
			
			strcat(turn_on_off_changed_result, "END");
			count++;
		}
	}

	turn_on_off_changed_result[30] = count/1000 + 0x30;
	turn_on_off_changed_result[31] = (count/100)%10 + 0x30;
	turn_on_off_changed_result[32] = (count/10)%10 + 0x30;
	turn_on_off_changed_result[33] = count%10 + 0x30;

	if(strlen(turn_on_off_changed_result) > 10000)
		turn_on_off_changed_result[5] = strlen(turn_on_off_changed_result)/10000 + 0x30;
	if(strlen(turn_on_off_changed_result) > 1000)
		turn_on_off_changed_result[6] = (strlen(turn_on_off_changed_result)/1000)%10 + 0x30;
	if(strlen(turn_on_off_changed_result) > 100)
		turn_on_off_changed_result[7] = (strlen(turn_on_off_changed_result)/100)%10 + 0x30;
	if(strlen(turn_on_off_changed_result) > 10)
		turn_on_off_changed_result[8] = (strlen(turn_on_off_changed_result)/10)%10 + 0x30;
	if(strlen(turn_on_off_changed_result) > 0)
		turn_on_off_changed_result[9] = strlen(turn_on_off_changed_result)%10 + 0x30;

	//strcat(turn_on_off_changed_result, "\n");

	if(count >0)
		save_process_result(116, turn_on_off_changed_result);
	free(turn_on_off_changed_result);
	turn_on_off_changed_result = NULL;

	return 0;
}

