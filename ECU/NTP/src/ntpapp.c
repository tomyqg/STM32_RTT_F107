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


int ntpapp_thread_entry(void* parameter)
{
    int sockfd;
    int ret,times, i=0;
    struct timeval newtime;
    struct sockaddr_in serversocket;
    NTPPACKET receivepacket;

    sockfd = create_socket();
    if(-1==sockfd)
    {
      #ifdef DEBUG
        rt_kprintf("Create socket error!\n");
      #endif 
      return 0;
    }
    else{
      #ifdef DEBUG
				rt_kprintf("socket=%d\n",sockfd);
      #endif
    }
    ret = connecttoserver(sockfd, &serversocket);
    if(-1==ret)
    {
      #ifdef DEBUG
        rt_kprintf("Connect server error!\n");
      #endif
      return 0;
    }
    else{
      #ifdef DEBUG
      rt_kprintf("socket=%d\n",ret);
      #endif
    }

    for(i=0; i<5; i++){
		send_packet(sockfd);
		for(times=0;times<5;times++){

#ifdef DEBUG
			rt_kprintf("ret=%d\n",ret);
#endif
			if(-1!=receive_packet(sockfd, &receivepacket, &serversocket)){
				gettimepacket(&receivepacket, &newtime);
#ifdef DEBUG
				rt_kprintf("server time= %s\n",ctime(&(newtime.tv_sec)));
#endif
			}
			update_time(&newtime);
				break;
			}
		}


    
    close(sockfd);
    return 0;
}
