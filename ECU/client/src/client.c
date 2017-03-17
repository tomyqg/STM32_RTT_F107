/*
 * client.c
 * V1.2.2
 * modified on: 2013-08-13
 * 与EMA异步通信
 * update操作数据库出现上锁时，延时再update
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include <board.h>
#include "checkdata.h"
#include "datetime.h"
#include <dfs_posix.h> 
#include <rtthread.h>
#include "file.h"
#include "debug.h"
#include "datetime.h"
#include <lwip/netdb.h> 
#include <lwip/sockets.h> 

ALIGN(RT_ALIGN_SIZE)
extern rt_uint8_t client_stack[ 4096 ];
extern struct rt_thread client_thread;


int writeconnecttime(void)			//保存最后一次连接上服务器的时间
{
	char connecttime[20]={'\0'};
	FILE *fp;

	getcurrenttime(connecttime);
	fp=fopen("/yuneng/con_time.con","w");
	fprintf(fp,"%s",connecttime);
	fclose(fp);

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

int randvalue(void)
{
	int i;

	srand((unsigned)acquire_time());
	i = rand()%2;
	printdecmsg("CLIENT","Randvalue", i);

	return i;
}

int createsocket(void)					//创建套接字
{
	int fd_sock;

	fd_sock=socket(AF_INET,SOCK_STREAM,0);
	if(-1==fd_sock)
		printmsg("CLIENT","Failed to create socket");
	else
		printmsg("CLIENT","Create socket successfully");

	return fd_sock;
}

int connect_socket(int fd_sock)				//连接到服务器
{
	char domain[100]={'\0'};		//EMA的域名
	char ip[20] = {'\0'};	//EMA的缺省IP
	int port[2]={8995, 8996}, i;
	char buff[1024] = {'\0'};
	struct sockaddr_in serv_addr;
	struct hostent * host;
	FILE *fp;

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

	if(!strlen(domain))
		strcpy(domain, "ecu.apsema.com");
	if(!strlen(ip))
		strcpy(ip, "60.190.131.190");


	host = gethostbyname(domain);
	if(NULL == host)
	{
		printmsg("CLIENT","Resolve domain failure");
	}
	else
	{
		memset(ip, '\0', sizeof(ip));
		printf("%s\n",*host->h_addr_list);
		//inet_ntop(AF_INET, *host->h_addr_list, ip, 32);
		printmsg("CLIENT",ip);
	}


	print2msg("CLIENT","IP", ip);
	printdecmsg("CLIENT","Port1", port[0]);
	printdecmsg("CLIENT","Port2", port[1]);

	memset(&serv_addr,0,sizeof(struct sockaddr_in));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(port[randvalue()]);
	serv_addr.sin_addr.s_addr=inet_addr(ip);
	memset(&(serv_addr.sin_zero),0,8);

	if(-1==connect(fd_sock,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr))){
		showdisconnected();
		printmsg("client","Failed to connect to EMA");
		return -1;
	}
	else{
		showconnected();
		printmsg("client","Connect to EMA successfully");
		writeconnecttime();
		return 1;
	}
}

void close_socket(int fd_sock)					//关闭套接字
{
	close(fd_sock);
	printmsg("CLIENT","Close socket");
}


int recv_response(int fd_sock, char *readbuff)
{
	fd_set rd;
	struct timeval timeout;
	int recvbytes, res, count=0, readbytes = 0;
	char recvbuff[65535], temp[16];
	while(1)
	{
		FD_ZERO(&rd);
		FD_SET(fd_sock, &rd);
		timeout.tv_sec = 120;
		timeout.tv_usec = 0;

		res = select(fd_sock+1, &rd, NULL, NULL, &timeout);
		if(res <= 0){
			//printerrmsg("select");
			printmsg("CLIENT","Receive data reply from EMA timeout");
			return -1;
		}
		else{
			memset(recvbuff, '\0', sizeof(recvbuff));
			memset(temp, '\0', sizeof(temp));
			recvbytes = recv(fd_sock, recvbuff, sizeof(recvbuff), 0);
			if(0 == recvbytes)
				return -1;
			strcat(readbuff, recvbuff);
			readbytes += recvbytes;
			if(readbytes >= 3)
			{
				count = (readbuff[1]-0x30)*10 + (readbuff[2]-0x30);
				if(count==((strlen(readbuff)-3)/14))
					return readbytes;
			}
		}
	}
}

int send_record(int fd_sock, char *sendbuff, char *send_date_time)			//?????EMA
{
	int sendbytes=0, res=0;
	char readbuff[65535] = {'\0'};

	sendbytes = send(fd_sock, sendbuff, strlen(sendbuff), 0);

	if(-1 == sendbytes)
		return -1;

	if(-1 == recv_response(fd_sock, readbuff))
		return -1;
	else
	{
		//if('1' == readbuff[0])
			//update_send_flag(send_date_time);
		//clear_send_flag(readbuff);
		return 0;
	}

}

int preprocess()			//发送头信息到EMA,读取已经存在EMA的记录时间
{
	int sendbytes=0, res=0, recvbytes = 0;
	char readbuff[1024] = {'\0'};
	fd_set rd;
	struct timeval timeout;
	char sendbuff[256] = {'\0'};
	FILE *fp;
	int fd_sock;

	//strcpy(sql,"SELECT date_time FROM Data WHERE resendflag='2'");
	//sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	//print2msg("Select nrow of resendflag=2 from db", zErrMsg);
	//sqlite3_free_table( azResult );
	//if(nrow < 1)
		//return 0;

	strcpy(sendbuff, "APS13AAA22");
	fp = fopen("/yuneng/ecuid.con", "r");
	if(fp)
	{
		fgets(&sendbuff[10], 13, fp);
		fclose(fp);
	}
	strcat(sendbuff, "\n");
	print2msg("CLIENT","Sendbuff", sendbuff);

	fd_sock = createsocket();
	printdecmsg("CLIENT","Socket", fd_sock);
	if(1 == connect_socket(fd_sock))
	{
		while(1)
		{
			memset(readbuff, '\0', sizeof(readbuff));
			sendbytes = send(fd_sock, sendbuff, strlen(sendbuff), 0);
			if(-1 == sendbytes)
			{
				close_socket(fd_sock);
				return -1;
			}
			//if(recv_response(fd_sock, readbuff) > 3)
				//clear_send_flag(readbuff);
			//else
				//break;
		}
	}

	close_socket(fd_sock);
	return 0;
}
/*
int resend_record()
{
	char sql[1024]={'\0'};
	char *zErrMsg=0;
	char **azResult;
	int i, res, nrow, ncolumn;
	int fd_sock;

	memset(sql,'\0',sizeof(sql));
	strcpy(sql,"SELECT record,date_time FROM Data WHERE resendflag='2'");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	print2msg("Select result from db", zErrMsg);
	if(nrow >= 1)
	{
		printdecmsg("Record count(flag=2)", nrow);
		fd_sock = createsocket();
		printdecmsg("Socket", fd_sock);
		if(1 == connect_socket(fd_sock))
		{
			for(i=nrow; i>=1; i--){
				if(1 != i)
					azResult[i*ncolumn][78] = '1';
				printmsg(azResult[i*ncolumn]);
				res = send_record(fd_sock, azResult[i*ncolumn], azResult[i*ncolumn+1]);
				if(-1 == res)
					break;
			}
		}
		close_socket(fd_sock);
	}
	else
		printmsg("There are none record");
	sqlite3_free_table( azResult );

	return 0;
}
*/

void client_thread_entry(void* parameter)
{
	int fd_sock;
	int thistime=0, lasttime=0;
	
	printmsg("CLIENT","Started");
	
	while(1)
	{
		thistime = lasttime = acquire_time();
		
		//if(1 == get_hour())
		{
			preprocess();		//预处理,先发送一个头信息给EMA,让EMA把标记为2的记录的时间返回ECU,然后ECU再把标记为2的记录再次发送给EMA,防止EMA收到记录返回收到标志而没有存入数据库的请情况
			//resend_record();
		}
		
		
		printf("client_thread_entry>>>>>>>>>>>>>>..\n");
		rt_thread_delay(RT_TICK_PER_SECOND*20);
	}
}
