#ifndef __SET_PROTECTION_PARAMETERS_INVERTER_H__
#define __SET_PROTECTION_PARAMETERS_INVERTER_H__
/*****************************************************************************/
/*  File      : set_protection_parameters_inverter.h                         */
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
#include "variation.h"

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
int set_protection_parameters_inverter_one(struct inverter_info_t *firstinverter);
int set_undervoltage_slow_yc1000_one_5(int shortaddr, char *value, char *inverterid);
int set_under_voltage_stage_3_yc1000_one(int shortaddr, char *value);
int set_overvoltage_slow_yc1000_one_5(int shortaddr, char *value, char *inverterid);
int set_underfrequency_slow_yc1000_one_5(int shortaddr, char *value, char *inverterid);
int set_overfrequency_slow_yc1000_one_5(int shortaddr, char *value, char *inverterid);
int set_grid_recovery_time_yc1000_one_5(int shortaddr, char *value, char *inverterid);
int set_active_antiisland_time_yc1000_one(int shortaddr, char *value);				//直流启动时间

#endif /*__SET_PROTECTION_PARAMETERS_INVERTER_H__*/
