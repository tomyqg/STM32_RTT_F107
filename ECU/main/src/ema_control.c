#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "variation.h"
#include "datetime.h"
#include "ema_control.h"


//extern int caltype;		//计算方式，NA版和非NA版的区别

int write_gfdi_status(inverter_info *firstinverter)
{
	FILE *fp;
	int i;
	inverter_info *inverter = firstinverter;
	char write_buff[300] = {'\0'};

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++)
	{
		//if('0' != inverter->flag)
		//{
			strcat(write_buff, inverter->id);
			write_buff[strlen(write_buff)] = inverter->last_gfdi_flag;
			strcat(write_buff, "END\n");
		//}
	}

	fp = fopen("/YUNENG/LASTGFDI.TXT", "w");
	if(fp)
	{
		fputs(write_buff, fp);
		fclose(fp);
	}

	return 0;
}

int write_turn_on_off_status(inverter_info *firstinverter)
{
	FILE *fp;
	int i;
	inverter_info *inverter = firstinverter;
	char write_buff[65535] = {'\0'};

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++)
	{
		//if('0' != inverter->flag)
		//{
			strcat(write_buff, inverter->id);
			write_buff[strlen(write_buff)] = inverter->last_turn_on_off_flag;
			strcat(write_buff, "END\n");
		//}
	}

	fp = fopen("/YUNENG/LASTTURN.TXT", "w");
	if(fp)
	{
		fputs(write_buff, fp);
		fclose(fp);
	}

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
		inverter->last_gfdi_flag = '0';
		inverter->last_turn_on_off_flag = '0';
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
							inverter->last_gfdi_flag = '1';
						else
							inverter->last_gfdi_flag = '0';
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
			inverter->last_gfdi_flag = '0';
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
							inverter->last_turn_on_off_flag = '1';
						else
							inverter->last_turn_on_off_flag = '0';
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
			inverter->last_turn_on_off_flag = '0';
		}
	}

	return 0;
}

int save_gfdi_changed_result(inverter_info *firstinverter)
{
	char ecu_id[16];
	FILE *fp;
	inverter_info *inverter = firstinverter;
	int i, count=0;
	char gfdi_changed_result[65535]={'\0'};

	strcpy(gfdi_changed_result, "APS13AAAAAA115AAA1");
	fp = fopen("/etc/yuneng/ecuid.conf", "r");		//读取ECU的ID
	if(fp)
	{
		fgets(ecu_id, 13, fp);
		fclose(fp);
	}

	strcat(gfdi_changed_result, ecu_id);					//ECU的ID
	strcat(gfdi_changed_result, "0000");					//逆变器个数
	strcat(gfdi_changed_result, "00000000000000");		//时间戳，设置逆变器后返回的结果中时间戳为0
	strcat(gfdi_changed_result, "END");					//固定格式

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++)
	{
		if(1 == inverter->gfdi_changed_flag)
		{
			/*memset(inverter_result, '\0', sizeof(inverter_result));
			if('1' == inverter->last_gfdi_flag)
				sprintf(inverter_result, "%sa1", inverter->inverterid);
			else
				sprintf(inverter_result, "%sa0", inverter->inverterid);
			strcat(gfdi_changed_result, inverter_result);
			if(1 == inverter->flagyc500)
			{
				memset(inverter_result, '\0', sizeof(inverter_result));
				if('1' == inverter->last_gfdi_flag)
					sprintf(inverter_result, "b1");
				else
					sprintf(inverter_result, "b0");
				strcat(gfdi_changed_result, inverter_result);
			}*/
			strcat(gfdi_changed_result, inverter->id);
			gfdi_changed_result[strlen(gfdi_changed_result)] = inverter->last_gfdi_flag;
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

	strcat(gfdi_changed_result, "\n");

	//if(count >0)
		//save_process_result(115, gfdi_changed_result);

	return 0;
}

int save_turn_on_off_changed_result(inverter_info *firstinverter)
{
	char ecu_id[16];
	FILE *fp;
	inverter_info *inverter = firstinverter;
	int i, count=0;
	char turn_on_off_changed_result[65535]={'\0'};
	//char inverter_result[64];

	strcpy(turn_on_off_changed_result, "APS13AAAAAA116AAA1");
	fp = fopen("/etc/yuneng/ecuid.conf", "r");		//读取ECU的ID
	if(fp)
	{
		fgets(ecu_id, 13, fp);
		fclose(fp);
	}

	strcat(turn_on_off_changed_result, ecu_id);					//ECU的ID
	strcat(turn_on_off_changed_result, "0000");					//逆变器个数
	strcat(turn_on_off_changed_result, "00000000000000");		//时间戳，设置逆变器后返回的结果中时间戳为0
	strcat(turn_on_off_changed_result, "END");					//固定格式

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++)
	{
		if(1 == inverter->turn_on_off_changed_flag)
		{
			/*memset(inverter_result, '\0', sizeof(inverter_result));
			if('1' == inverter->last_turn_on_off_flag)
				sprintf(inverter_result, "%s0", inverter->inverterid);
			else
				sprintf(inverter_result, "%s1", inverter->inverterid);
			strcat(turn_on_off_changed_result, inverter_result);*/

			strcat(turn_on_off_changed_result, inverter->id);
			turn_on_off_changed_result[strlen(turn_on_off_changed_result)] = inverter->last_turn_on_off_flag;
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

	strcat(turn_on_off_changed_result, "\n");

	//if(count >0)
		//save_process_result(116, turn_on_off_changed_result);

	return 0;
}


int save_protect_result(inverter_info *firstinverter)
{
	inverter_info *inverter = firstinverter;
	int i, count=0;
	char protect_result[65535] = {'\0'};
	char inverter_result[64];
	char ecu_id[16];
	FILE *fp;
	//int max_voltage, min_voltage, max_frequency, min_frequency, boot_time;

	strcpy(protect_result, "APS13AAAAAA114AAA1");

	fp = fopen("/etc/yuneng/ecuid.conf", "r");		//读取ECU的ID
	if(fp)
	{
		fgets(ecu_id, 13, fp);
		fclose(fp);
	}

	strcat(protect_result, ecu_id);					//ECU的ID
	strcat(protect_result, "0000");					//逆变器个数
	strcat(protect_result, "00000000000000");		//时间戳，设置逆变器后返回的结果中时间戳为0
	strcat(protect_result, "END");					//固定格式

	//get_protect_parameters(&max_voltage, &min_voltage, &max_frequency, &min_frequency, &boot_time);

	for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(inverter->id)); i++, inverter++)
	{
	//	if((-1 != inverter->protect_voltage_min) || (-1 != inverter->protect_voltage_max))
		{
			memset(inverter_result, '\0', sizeof(inverter_result));
			sprintf(inverter_result, "%s%03d%03d%03d%03d%05d", inverter->id, inverter->protect_voltage_min, inverter->protect_voltage_max, (int)(inverter->protect_frequency_min*10), (int)(inverter->protect_frequency_max*10), inverter->recovery_time);

		//	if(2 == caltype)
		//		strcat(inverter_result, "082118124155551600600649");
		//	else if(1 == caltype)
		//		strcat(inverter_result, "181239221298551600600649");
		//	else
				strcat(inverter_result, "149217221278451500500549");
			strcat(protect_result, inverter_result);
			strcat(protect_result, "END");
			count++;
		}
	}

	if(count>9999)
		count = 9999;

	protect_result[30] = count/1000 + 0x30;
	protect_result[31] = (count/100)%10 + 0x30;
	protect_result[32] = (count/10)%10 + 0x30;
	protect_result[33] = count%10 + 0x30;

	if(strlen(protect_result) > 10000)
		protect_result[5] = strlen(protect_result)/10000 + 0x30;
	if(strlen(protect_result) > 1000)
		protect_result[6] = (strlen(protect_result)/1000)%10 + 0x30;
	if(strlen(protect_result) > 100)
		protect_result[7] = (strlen(protect_result)/100)%10 + 0x30;
	if(strlen(protect_result) > 10)
		protect_result[8] = (strlen(protect_result)/10)%10 + 0x30;
	if(strlen(protect_result) > 0)
		protect_result[9] = strlen(protect_result)%10 + 0x30;

	strcat(protect_result, "\n");

	//save_process_result(114,protect_result);

	return 0;
}
