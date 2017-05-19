#ifndef __EMA_CONTROL_H__
#define __EMA_CONTROL_H__
/*****************************************************************************/
/*  File      : ema_control.h                                                */
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
extern int write_gfdi_status(inverter_info *firstinverter);
extern int write_turn_on_off_status(inverter_info *firstinverter);
extern int read_gfdi_turn_on_off_status(inverter_info *firstinverter);
extern int save_gfdi_changed_result(inverter_info *firstinverter);   //ZK,EMA下发功能函数
extern int save_turn_on_off_changed_result(inverter_info *firstinverter);
extern int save_protect_result(inverter_info *firstinverter);
#endif /*__EMA_CONTROL_H__*/
