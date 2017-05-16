/******************************************************************************
name:ntpapp.c
date:2011.01.24
version:1.0
******************************************************************************/
#include <rtthread.h>
#include <lwip/netdb.h> /* 为了解析主机名，需要包含netdb.h头文件 */
#include <lwip/sockets.h> /* 使用BSD socket，需要包含sockets.h头文件 */
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ntpapp.h"
#include "debug.h"

int get_time_from_NTP()
{
	int sockfd;
  int ret,times, i=0;
  struct timeval newtime,timeout;
  struct sockaddr_in serversocket;
  NTPPACKET receivepacket;
  fd_set readfd;

  sockfd = create_socket_ntp();
  if(-1==sockfd)
  {
    printmsg(ECU_DBG_NTP,"Create socket error!");

    return -1;
  }
  else{
		printdecmsg(ECU_DBG_NTP,"socket",sockfd);

  }
  ret = connecttoserver(sockfd, &serversocket);
  if(-1==ret)
  {
    printmsg(ECU_DBG_NTP,"Connect server error!");
	closesocket(sockfd);
    return -1;
  }
  else{
    printdecmsg(ECU_DBG_NTP,"socket",ret);
  }

	for(i=0; i<2; i++){
		send_packet(sockfd);
		rt_hw_us_delay(1);
		for(times=0;times<2;times++){
			timeout.tv_sec = 6;
			timeout.tv_usec = 0;
			rt_hw_us_delay(1);
			ret = select(sockfd+1, &readfd, NULL, NULL, &timeout);
			
			printdecmsg(ECU_DBG_NTP,"ret",ret);

			if(ret>0){
				if(-1!=receive_packet(sockfd, &receivepacket, &serversocket)){
					gettimepacket(&receivepacket, &newtime);

					printdecmsg(ECU_DBG_NTP,"time_t ",newtime.tv_sec);

				}
				update_time(&newtime);
				closesocket(sockfd);
				return 0;
			}
		}
		if(ret>0)
			break;
    }
    
    closesocket(sockfd);
    return -1;
}

#ifdef RT_USING_FINSH
#include <finsh.h>
void ntpget()
{
	get_time_from_NTP();
}
FINSH_FUNCTION_EXPORT(ntpget, eg:ntpget());
#endif

