#ifndef __FILE_H__
#define __FILE_H__
#include "variation.h"


void get_ecuid(char *ecuid);
int get_ecu_type(void);
unsigned short get_panid(void);
char get_channel(void);
float get_lifetime_power(void);	
void update_life_energy(float lifetime_power);

void updateID(void);


int splitString(char *data,char splitdata[20][13]);

int get_id_from_file(inverter_info *firstinverter);

int save_process_result(int item, char *result);	//设置保护参数,功率等完成后把结果存放在/home/data/proc_res目录下，control_client把结果传送给EMA

void save_record(char sendbuff[], char *date_time);			//ECU发送记录给EMA的同时,本地也保存一份

int save_status(char *result, char *date_time);	//设置保护参数，功率等完成后，把结果保存到文件中，control_client把结果发送给EMA


#endif /*__FILE_H__*/
