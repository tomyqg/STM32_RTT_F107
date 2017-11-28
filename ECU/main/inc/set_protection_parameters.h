#ifndef __SET_PROTECTION_PARAMETERS_H__
#define __SET_PROTECTION_PARAMETERS_H__
/*****************************************************************************/
/*  File      : set_protection_parameters.h                                  */
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
int get_recovery_time_from_inverter(inverter_info *inverter);
int get_parameters_from_inverter(inverter_info *inverter);
int set_protection_parameters(inverter_info *firstinverter);
int set_undervoltage_slow_yc1000_5(char *value);
int set_under_voltage_stage_3_yc1000(char *value);
int set_overvoltage_slow_yc1000(char *value);  //内围电压上限值
int set_underfrequency_slow_yc1000(char *value);
int set_overvoltage_slow_yc1000_5(char *value);
int set_underfrequency_slow_yc1000_5(char *value);
int set_overfrequency_slow_yc1000_5(char *value);
int set_grid_recovery_time_yc1000_5(char *value);
int set_active_antiisland_time_yc1000(char *value);				//直流启动时间

#endif /*__SET_PROTECTION_PARAMETERS_H__*/
