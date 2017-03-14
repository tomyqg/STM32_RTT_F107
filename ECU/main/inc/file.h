#ifndef __FILE_H__
#define __FILE_H__

void get_ecuid(char *ecuid);
int get_ecu_type(void);
unsigned short get_panid(void);
char get_channel(void);
float get_lifetime_power(void);	
void update_life_energy(float lifetime_power);


#endif /*__FILE_H__*/
