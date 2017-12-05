#include "clientSocket.h"
#include <lwip/netdb.h> 
#include <lwip/sockets.h> 
#include "lan8720rst.h"
#include <stdio.h>
#include "rtc.h"
#include "threadlist.h"
#include "usart5.h"
#include "debug.h"
#include "string.h"
#include "rthw.h"
#include "rtthread.h"
#include "rtc.h"
#include "datetime.h"

extern ecu_info ecu;
extern unsigned char LED_Status;


int writeconnecttime(void)			//保存最后一次连接上服务器的时间
{
	char connecttime[20]={'\0'};
	FILE *fp;

	getcurrenttime(connecttime);
	fp=fopen("/yuneng/con_time.con","w");
	fprintf(fp,"%s",connecttime);
	fclose(fp);

	memcpy(ecu.last_ema_time,connecttime,15);
	print2msg(ECU_DBG_CLIENT,"ecu.last_ema_time:",ecu.last_ema_time);
	return 0;
}

void showconnected(void)		//已连接EMA
{
	FILE *fp;

	fp = fopen("/tmp/conemafl.txt", "w");
	if(fp)
	{
		fputs("connected", fp);
		fclose(fp);
	}

}

void showdisconnected(void)		//无法连接EMA
{
	FILE *fp;

	fp = fopen("/tmp/conemafl.txt", "w");
	if(fp)
	{
		fputs("disconnected", fp);
		fclose(fp);
	}

}


int createsocket(void)					//创建SOCKET连接
{
	int fd_sock;

	fd_sock=socket(AF_INET,SOCK_STREAM,0);
	if(-1==fd_sock)
		printmsg(ECU_DBG_OTHER,"Failed to create socket");
	else
		printmsg(ECU_DBG_OTHER,"Create socket successfully");

	return fd_sock;
}


int connect_client_socket(int fd_sock)				//通过有线的方式连接服务器
{
	char domain[100]={'\0'};		//服务器域名
	char ip[20] = {'\0'};	//服务器IP地址
	int port[2]={CLIENT_SERVER_PORT1, CLIENT_SERVER_PORT2};	//服务器端口号
	struct sockaddr_in serv_addr;
	struct hostent * host;
	char buff[512] = {'\0'};
	FILE *fp;
	
	//不存在有线连接，直接关闭socket
	if(rt_hw_GetWiredNetConnect() == 0)
	{
		closesocket(fd_sock);
		return -1;
	}
	strcpy(domain, CLIENT_SERVER_DOMAIN);
	strcpy(ip, CLIENT_SERVER_IP);
	fp = fopen("/yuneng/datacent.con", "r");
	if(fp)
	{
		while(1)
		{
			memset(buff, '\0', sizeof(buff));
			fgets(buff, sizeof(buff), fp);
			if(!strlen(buff))
				break;
			if(!strncmp(buff, "Domain", 6))
			{
				strcpy(domain, &buff[7]);
				if('\n' == domain[strlen(domain)-1])
					domain[strlen(domain)-1] = '\0';
			}
			if(!strncmp(buff, "IP", 2))
			{
				strcpy(ip, &buff[3]);
				if('\n' == ip[strlen(ip)-1])
					ip[strlen(ip)-1] = '\0';
			}
			if(!strncmp(buff, "Port1", 5))
				port[0]=atoi(&buff[6]);
			if(!strncmp(buff, "Port2", 5))
				port[1]=atoi(&buff[6]);
		}
		fclose(fp);
	}

	host = gethostbyname(domain);
	if(NULL == host)
	{
		printmsg(ECU_DBG_CLIENT,"Resolve domain failure");
	}
	else
	{
		memset(ip, '\0', sizeof(ip));
		sprintf(ip,"%s",ip_ntoa((ip_addr_t*)*host->h_addr_list));
	}

	memset(&serv_addr,0,sizeof(struct sockaddr_in));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(port[0]);
	serv_addr.sin_addr.s_addr=inet_addr(ip);
	memset(&(serv_addr.sin_zero),0,8);

	if(-1==connect(fd_sock,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr))){
		showdisconnected();
		printmsg(ECU_DBG_CLIENT,"Failed to connect to EMA");
		closesocket(fd_sock);
		return -1;
	}
	else{
		showconnected();
		writeconnecttime();
		printmsg(ECU_DBG_CLIENT,"Connect to EMA Client successfully");
		return 1;
	}
}

void close_socket(int fd_sock)					//关闭socket连接
{
	closesocket(fd_sock);
	printmsg(ECU_DBG_OTHER,"Close socket");
}

int wifi_socketb_format(char *data ,int length)
{
	char head[9] = {'\0'};
	char *p = NULL;
	int i = 0,retlength = 0;

	head[0] = 'b';
#ifdef USR_MODULE	
	head[1] = 0x00;
#endif 
#ifdef RAK475_MODULE	
		head[1] = 0x30;
#endif 

	head[2] = 0x00;
	head[3] = 0x00;
	head[4] = 0x00;
	head[5] = 0x00;
	head[6] = 0x00;
	head[7] = 0x00;
	head[8] = 0x00;
	
	if(((length -3)%14) != 0)
	{
		for(p = data,i = 0;p <= (data+length-9);p++,i++)
		{
			if(!memcmp(p,head,9))
			{
				memcpy(p,p+9,(length-9-i));
				length -= 9;
				data[length] = '\0';
				retlength = length;
			}
		}
	}else
	{
		retlength = length;
	}
	
	return retlength;
}

//与Client服务器通讯 
//sendbuff[in]:发送数据的buff
//sendLength[in]:发送字节长度
//recvbuff[out]:接收数据buff
//recvLength[in/out]:in接收数据最大长度  out接收数据长度
//Timeout[in]	超时时间 ms
int serverCommunication_Client(char *sendbuff,int sendLength,char *recvbuff,int *recvLength,int Timeout)
{
	int socketfd = 0;
	int readbytes = 0,count =0,recvlen =0;
	char *readbuff = NULL;
	int length = 0,send_length = 0;
	length = sendLength;
	socketfd = createsocket();
	if(socketfd == -1) 	//创建socket失败
	{
		LED_Status = 0;
		return -1;
	}
		
	//创建socket成功
	if(1 == connect_client_socket(socketfd))
	{	//连接服务器成功
		int sendbytes = 0;
		int res = 0;
		fd_set rd;
		struct timeval timeout;

		while(length > 0)
		{
			if(length > SIZE_PER_SEND)
			{
				sendbytes = send(socketfd, &sendbuff[send_length], SIZE_PER_SEND, 0);
				send_length += SIZE_PER_SEND;
				length -= SIZE_PER_SEND;
			}else
			{	
				sendbytes = send(socketfd, &sendbuff[send_length], length, 0);

				length -= length;
			}

			if(-1 == sendbytes)
			{
				close_socket(socketfd);
				LED_Status = 0;
				return -1;
			}
			
			rt_hw_ms_delay(500);
		}

		

		readbuff = malloc((4+99*14));
		memset(readbuff,'\0',(4+99*14));
		
		while(1)
		{
			FD_ZERO(&rd);
			FD_SET(socketfd, &rd);
			timeout.tv_sec = Timeout/1000;
			timeout.tv_usec = Timeout%1000;
			
			res = select(socketfd+1, &rd, NULL, NULL, &timeout);
			
			if(res <= 0){
				printmsg(ECU_DBG_CLIENT,"Receive data reply from Server timeout");
				close_socket(socketfd);
				free(readbuff);
				readbuff = NULL;
				LED_Status = 0;
				return -1;
			}else
			{
				memset(readbuff, '\0', sizeof(readbuff));
				readbytes = recv(socketfd, readbuff, *recvLength, 0);
				if(readbytes <= 0)
				{
					free(readbuff);
					readbuff = NULL;
					*recvLength = 0;
					close_socket(socketfd);
					LED_Status = 0;
					return -1;
				}	
				strcat(recvbuff,readbuff);
				recvlen += readbytes;
				if(recvlen >= 3)
				{
					print2msg(ECU_DBG_CLIENT,"recvbuff:",recvbuff);
					printdecmsg(ECU_DBG_CLIENT,"recv length:",readbytes);
					*recvLength = recvlen;
					count = (recvbuff[1]-0x30)*10 + (recvbuff[2]-0x30);
					if(count==((strlen(recvbuff)-3)/14))
					{
						free(readbuff);
						readbuff = NULL;
						close_socket(socketfd);
						LED_Status = 1;
						return *recvLength;
					}else if(recvbuff[0] != '1')
					{
						free(readbuff);
						readbuff = NULL;
						close_socket(socketfd);
						*recvLength = 0;
						LED_Status = 0;
						return *recvLength;
					}
				}

			}
		}
		
		
	}else
	{
		//连接服务器失败
		//失败情况下通过无线发送
#ifdef WIFI_USE
		int ret = 0,i = 0;
		ret = SendToSocketB(sendbuff, sendLength);
		if(ret == -1)
		{
			LED_Status = 0;
			return -1;
		}
			
		for(i = 0;i<(Timeout/10);i++)
		{
			if(WIFI_Recv_SocketB_Event == 1)
			{
				LED_Status = 1;
				*recvLength = wifi_socketb_format((char *)WIFI_RecvSocketBData ,WIFI_Recv_SocketB_LEN);
				memcpy(recvbuff,WIFI_RecvSocketBData,*recvLength);
				recvbuff[*recvLength] = '\0';
					print2msg(ECU_DBG_CLIENT,"serverCommunication_Client",recvbuff);
				WIFI_Recv_SocketB_Event = 0;
				//WIFI_Close(SOCKET_B);
				return 0;
			}
			rt_hw_ms_delay(10);
		}
		//WIFI_Close(SOCKET_B);
		LED_Status = 0;
		return -1;
#endif

#ifndef WIFI_USE
		LED_Status = 0;
		return -1;
#endif
	}
	
}


