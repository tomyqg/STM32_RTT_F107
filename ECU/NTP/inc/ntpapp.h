#ifndef __NTPAPP_H__
#define __NTPAPP_H__
/*****************************************************************************/
/* File      : ntpapp.h                                                      */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-03-20 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "lwip/sockets.h"

/*****************************************************************************/
/*  Definitions                                                              */
/*****************************************************************************/
typedef struct ntptime
{
    unsigned int sec;
    unsigned int usec;
}NTPTIME;

/********************************NTP协议包头（16Byte）************************/
typedef struct ntpheader
{
    union
    {
      struct
      {
        char precision;
        char poll;
        unsigned char stratum;
        unsigned char mode : 3;
        unsigned char VN   : 3;
        unsigned char LI   : 2;
      }NTPMode;
      unsigned int head;
    }NTPData;
}NTPHEADER;

/********************************NTP协议包************************************/
typedef struct ntppacket
{
    NTPHEADER header;
    unsigned int root_delay;
    unsigned int root_dispersion;
    unsigned int identifer;
    NTPTIME reftimestamp;
    NTPTIME oritimestamp;
    NTPTIME rectimestamp;
    NTPTIME tratimestamp;
}NTPPACKET;

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/

int create_socket_ntp( void );
int connecttoserver(int sockfd, struct sockaddr_in * serversocket_in);
void send_packet(int sockfd);
int receive_packet(int sockfd, NTPPACKET *recvpacked, struct sockaddr_in * serversocket_in);
void gettimepacket(NTPPACKET *receivepacket, struct timeval * new_time);
void update_time(struct timeval * new_time);
int get_time_from_NTP(void);
#endif /*__NTPAPP_H__*/

