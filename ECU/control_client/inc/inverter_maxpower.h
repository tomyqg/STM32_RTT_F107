#ifndef __INVERTER_MAXPOWER_H__
#define __INVERTER_MAXPOWER_H__
/*****************************************************************************/
/* File      : inverter_maxpower.h                                           */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-04-03 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
int set_inverter_maxpower(const char *recvbuffer, char *sendbuffer);
int response_inverter_maxpower(const char *recvbuffer, char *sendbuffer);

#endif	/*__INVERTER_MAXPOWER_H__*/
