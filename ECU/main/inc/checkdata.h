#ifndef __CHECHDATA_H__
#define __CHECHDATA_H__
#include "variation.h"
#include "debug.h"
int get_ecu_type(void);
int check_yc200_yc250(struct inverter_info_t *inverter);		//在解析函数的最后调用
int check_yc500(struct inverter_info_t *inverter);		//在解析函数的最后调用
int check_yc1000(struct inverter_info_t *inverter);		//在解析函数的最后调用
int check_optimizer_YC500(struct inverter_info_t *inverter);


#endif /*__CHECHDATA_H__*/
