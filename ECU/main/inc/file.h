#ifndef __FILE_H__
#define __FILE_H__
#include "variation.h"


void get_ecuid(char *ecuid);
int get_ecu_type(void);
unsigned short get_panid(void);
char get_channel(void);
float get_lifetime_power(void);	
void update_life_energy(float lifetime_power);



int splitString(char *data,char splitdata[20][13]);

int get_id_from_file(inverter_info *firstinverter);


#endif /*__FILE_H__*/
