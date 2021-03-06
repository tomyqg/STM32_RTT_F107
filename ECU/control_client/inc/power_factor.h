#ifndef __POWER_FACTOR_H__
#define __POWER_FACTOR_H__
/*****************************************************************************/
/* File      : power_factor.h                                                */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-11-30 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/

int response_inverter_power_factor(const char *recvbuffer, char *sendbuffer);
int set_all_inverter_power_factor(const char *recvbuffer, char *sendbuffer);

#endif	/*__POWER_FACTOR_H__*/
