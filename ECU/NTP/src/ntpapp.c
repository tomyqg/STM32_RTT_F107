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
    #ifdef NTPDEBUG
      rt_kprintf("Create socket error!\n");
    #endif 
    return 0;
  }
  else{
    #ifdef NTPDEBUG
    rt_kprintf("socket=%d\n",sockfd);
    #endif
  }
  ret = connecttoserver(sockfd, &serversocket);
  if(-1==ret)
  {
    #ifdef NTPDEBUG
      rt_kprintf("Connect server error!\n");
    #endif
    return 0;
  }
  else{
    #ifdef NTPDEBUG
    rt_kprintf("socket=%d\n",ret);
    #endif
  }

	for(i=0; i<5; i++){
		send_packet(sockfd);
		for(times=0;times<5;times++){
			timeout.tv_sec = 6;
			timeout.tv_usec = 0;
			ret = select(sockfd+1, &readfd, NULL, NULL, &timeout);
			
#ifdef NTPDEBUG
		rt_kprintf("ret=%d\n",ret);
#endif

			if(ret>0){
				if(-1!=receive_packet(sockfd, &receivepacket, &serversocket)){
					gettimepacket(&receivepacket, &newtime);
#ifdef NTPDEBUG
				rt_kprintf("%d\n",newtime.tv_sec);
#endif
				}
				//update_time(&newtime);
				break;
			}
		}
		if(ret>0)
			break;
    }
    
    closesocket(sockfd);
    return 0;
}
