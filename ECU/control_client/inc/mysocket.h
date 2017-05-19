#ifndef __MYSOCKET_H__
#define __MYSOCKET_H__
/*****************************************************************************/
/* File      : mysocket.h                                                    */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-03-30 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
int client_socket_init(int port, const char *ip, const char *domain);
int send_socket(int sockfd, char *sendbuffer, int size);
int recv_socket(int sockfd, char *recvbuffer, int size, int timeout_s);

#endif	/*__MYSOCKET_H__*/
