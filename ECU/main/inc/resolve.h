#ifndef __RESOLVE_H__
#define __RESOLVE_H__
#include "variation.h"
#include "debug.h"
#include "checkdata.h"
int resolvedata_250(char *inverter_data, struct inverter_info_t *inverter);
int resolvedata_500(char *data, struct inverter_info_t *inverter);
int resolvedata_1000(char *data, struct inverter_info_t *inverter);
int resolvedata_optimizer_YC500(char *inverter_data, struct inverter_info_t *inverter);

#endif /*__RESOLVE_H__*/
