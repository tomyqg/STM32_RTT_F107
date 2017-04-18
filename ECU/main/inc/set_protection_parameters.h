#ifndef __SET_PROTECTION_PARAMETERS_H__
#define __SET_PROTECTION_PARAMETERS_H__
#include "variation.h"

int get_recovery_time_from_inverter(inverter_info *inverter);
int get_parameters_from_inverter(inverter_info *inverter);
int set_protection_parameters(inverter_info *firstinverter);
#endif /*__SET_PROTECTION_PARAMETERS_H__*/
