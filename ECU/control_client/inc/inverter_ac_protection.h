#ifndef __INVERTER_AC_PROTECTION_H__
#define __INVERTER_AC_PROTECTION_H__
/*****************************************************************************/
/* File      : inverter_ac_protection.h                                      */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-04-07 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
int set_inverter_ac_protection_5(const char *recvbuffer, char *sendbuffer);
int response_ecu_ac_protection_5(const char *recvbuffer, char *sendbuffer);
int read_inverter_ac_protection_5(const char *recvbuffer, char *sendbuffer);
int set_inverter_ac_protection_13(const char *recvbuffer, char *sendbuffer);
int response_ecu_ac_protection_13(const char *recvbuffer, char *sendbuffer);
int read_inverter_ac_protection_13(const char *recvbuffer, char *sendbuffer);
int set_inverter_ac_protection_17(const char *recvbuffer, char *sendbuffer);
int response_ecu_ac_protection_17(const char *recvbuffer, char *sendbuffer);
int read_inverter_ac_protection_17(const char *recvbuffer, char *sendbuffer);

#endif	/*__INVERTER_AC_PROTECTION_H__*/
