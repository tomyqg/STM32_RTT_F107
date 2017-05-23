/*****************************************************************************/
/* File      : client.c                                                      */
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
#include "threadlist.h"
#include "usr_wifi232.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern rt_mutex_t record_data_lock;


/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
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
	printdecmsg(ECU_DBG_CLIENT,"Randvalue", i);

	return i;
}

int createsocket(void)					//创建套接字
{
	int fd_sock;

	fd_sock=socket(AF_INET,SOCK_STREAM,0);
	if(-1==fd_sock)
		printmsg(ECU_DBG_CLIENT,"Failed to create socket");
	else
		printmsg(ECU_DBG_CLIENT,"Create socket successfully");

	return fd_sock;
}

int connect_socket(int fd_sock)				//连接到服务器
{
	char domain[100]={'\0'};		//EMA的域名
	char ip[20] = {'\0'};	//EMA的缺省IP
	int port[2]={8093, 8093};
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
		printmsg(ECU_DBG_CLIENT,"Resolve domain failure");
	}
	else
	{
		memset(ip, '\0', sizeof(ip));
		//inet_ntop(AF_INET, *host->h_addr_list, ip, 32);
	}

	strcpy(ip, "139.168.200.158");
	print2msg(ECU_DBG_CLIENT,"IP", ip);
	printdecmsg(ECU_DBG_CLIENT,"Port1", port[0]);
	printdecmsg(ECU_DBG_CLIENT,"Port2", port[1]);

	memset(&serv_addr,0,sizeof(struct sockaddr_in));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(port[randvalue()]);
	serv_addr.sin_addr.s_addr=inet_addr(ip);
	memset(&(serv_addr.sin_zero),0,8);

	if(-1==connect(fd_sock,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr))){
		showdisconnected();
		printmsg(ECU_DBG_CLIENT,"Failed to connect to EMA");
		return -1;
	}
	else{
		showconnected();
		printmsg(ECU_DBG_CLIENT,"Connect to EMA successfully");
		writeconnecttime();
		return 1;
	}
}

void close_socket(int fd_sock)					//关闭套接字
{
	closesocket(fd_sock);
	printmsg(ECU_DBG_CLIENT,"Close socket");
}


int clear_send_flag(char *readbuff)
{
	int i, j, count;		//EMA返回多少个时间(有几个时间,就说明EMA保存了多少条记录)
	char recv_date_time[16];
	
	if(strlen(readbuff) >= 3)
	{
		count = (readbuff[1] - 0x30) * 10 + (readbuff[2] - 0x30);
		if(count == (strlen(readbuff) - 3) / 14)
		{
			for(i=0; i<count; i++)
			{
				memset(recv_date_time, '\0', sizeof(recv_date_time));
				strncpy(recv_date_time, &readbuff[3+i*14], 14);
				
				for(j=0; j<3; j++)
				{
					if(1 == change_resendflag(recv_date_time,'0'))
					{
						print2msg(ECU_DBG_CLIENT,"Clear send flag into database", "1");
						break;
					}
					else
						print2msg(ECU_DBG_CLIENT,"Clear send flag into database", "0");
					rt_thread_delay(RT_TICK_PER_SECOND);
				}
			}
		}
	}

	return 0;
}

int update_send_flag(char *send_date_time)
{
	int i;
	for(i=0; i<3; i++)
	{
		if(1 == change_resendflag(send_date_time,'2'))
		{
			print2msg(ECU_DBG_CLIENT,"Update send flag into database", "1");
			break;
		}
		rt_thread_delay(RT_TICK_PER_SECOND * 5);
	}

	return 0;
}

int recv_response(int fd_sock, char *readbuff)
{
	fd_set rd;
	struct timeval timeout;
	int recvbytes, res, count=0, readbytes = 0;
	char recvbuff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18], temp[16];
	while(1)
	{
		FD_ZERO(&rd);
		FD_SET(fd_sock, &rd);
		timeout.tv_sec = 120;
		timeout.tv_usec = 0;

		res = select(fd_sock+1, &rd, NULL, NULL, &timeout);
		if(res <= 0){
			//printerrmsg("select");
			printmsg(ECU_DBG_CLIENT,"Receive data reply from EMA timeout");
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

int detection_resendflag2()		//存在返回1，不存在返回0
{
	DIR *dirp;
	char dir[30] = "/home/record/data";
	struct dirent *d;
	char path[100];
	char buff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18]={'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		/* 打开dir目录*/
		dirp = opendir("/home/record/data");
		
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_CLIENT,"detection_resendflag2 open directory error");
		}
		else
		{
			/* 读取dir目录*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
				sprintf(path,"%s/%s",dir,d->d_name);
				//print2msg(ECU_DBG_CLIENT,"detection_resendflag2",path);
				//打开文件一行行判断是否有flag=2的  如果存在直接关闭文件并返回1
				fp = fopen(path, "r");
				if(fp)
				{
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))
					{
						//检查是否符合格式
						if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
						{
							if(buff[strlen(buff)-2] == '2')			//检测最后一个字节的resendflag是否为2   如果发现是2  关闭文件并且return 1
							{
								fclose(fp);
								closedir(dirp);
								rt_mutex_release(record_data_lock);
								return 1;
							}		
						}
					}
					fclose(fp);
				}
				
			}
			/* 关闭目录 */
			closedir(dirp);
		}
	}
	rt_mutex_release(record_data_lock);
	return 0;
}

int change_resendflag(char *time,char flag)  //改变成功返回1，未找到该时间点返回0
{
	DIR *dirp;
	char dir[30] = "/home/record/data";
	struct dirent *d;
	char path[100];
	char filetime[15] = {'\0'};
	char buff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18]={'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		/* 打开dir目录*/
		dirp = opendir("/home/record/data");
		
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_CLIENT,"change_resendflag open directory error");
		}
		else
		{
			/* 读取dir目录*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
				sprintf(path,"%s/%s",dir,d->d_name);
				//打开文件一行行判断是否有flag=2的  如果存在直接关闭文件并返回1
				fp = fopen(path, "r+");
				if(fp)
				{
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))
					{
						if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
						{
							memset(filetime,0,15);
							memcpy(filetime,&buff[strlen(buff)-17],14);				//获取每条记录的时间
							filetime[14] = '\0';
							if(!memcmp(time,filetime,14))						//每条记录的时间和传入的时间对比，若相同则变更flag				
							{
								fseek(fp,-2L,SEEK_CUR);
								fputc(flag,fp);
								//print2msg(ECU_DBG_CLIENT,"change_resendflag",filetime);
								fclose(fp);
								closedir(dirp);
								rt_mutex_release(record_data_lock);
								return 1;
							}
						}
					}
					fclose(fp);
				}
				
			}
			/* 关闭目录 */
			closedir(dirp);
		}
	}
	rt_mutex_release(record_data_lock);
	return 0;
	
}	

//查询一条resendflag为1的数据   查询到了返回1  如果没查询到返回0
/*
data:表示获取到的数据
time：表示获取到的时间
flag：表示是否还有下一条数据   存在下一条为1   不存在为0
*/
int search_readflag(char *data,char * time, int *flag,char sendflag)	
{
	DIR *dirp;
	char dir[30] = "/home/record/data";
	struct dirent *d;
	char path[100];
	char buff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18]={'\0'};
	FILE *fp;
	int nextfileflag = 0;	//0表示当前文件找到了数据，1表示需要从后面的文件查找数据
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	*flag = 0;
	if(result == RT_EOK)
	{
		/* 打开dir目录*/
		dirp = opendir("/home/record/data");
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_CLIENT,"search_readflag open directory error");
		}
		else
		{
			/* 读取dir目录*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
				sprintf(path,"%s/%s",dir,d->d_name);
				fp = fopen(path, "r");
				if(fp)
				{
					if(0 == nextfileflag)
					{
						while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))  //读取一行数据
						{
							
							if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
							{
								if(buff[strlen(buff)-2] == sendflag)			//检测最后一个字节的resendflag是否为1
								{
									memcpy(time,&buff[strlen(buff)-17],14);				//获取每条记录的时间
									memcpy(data,buff,(strlen(buff)-18));
									data[strlen(buff)-18] = '\n';
									//print2msg(ECU_DBG_CLIENT,"search_readflag time",time);
									//print2msg(ECU_DBG_CLIENT,"search_readflag data",data);
									rt_thread_delay(RT_TICK_PER_SECOND*1);
									while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))	//再往下读数据，寻找是否还有要发送的数据
									{
										if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
										{
											if(buff[strlen(buff)-2] == sendflag)
											{
												*flag = 1;
												fclose(fp);
												closedir(dirp);
												rt_mutex_release(record_data_lock);
												return 1;
											}
										}			
									}

									nextfileflag = 1;
									break;
									/*
									*flag = 0;
									fclose(fp);
									closedir(dirp);
									rt_mutex_release(record_data_lock);
									return 1;
									*/
								}
							}
								
						}
					}else
					{
						while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))  //读取一行数据
						{
							if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
							{
								if(buff[strlen(buff)-2] == sendflag)
								{
									*flag = 1;
									fclose(fp);
									closedir(dirp);
									rt_mutex_release(record_data_lock);
									return 1;
								}
							}
						}
					}
					
					fclose(fp);
				}
			}
			/* 关闭目录 */
			closedir(dirp);
		}
	}
	rt_mutex_release(record_data_lock);

	return nextfileflag;
}


void delete_file_resendflag0()		//清空数据resend标志全部为0的目录
{
	DIR *dirp;
	char dir[30] = "/home/record/data";
	struct dirent *d;
	char path[100];
	char buff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18]={'\0'};
	FILE *fp;
	int flag = 0;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		/* 打开dir目录*/
		dirp = opendir("/home/record/data");
		
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_CLIENT,"delete_file_resendflag0 open directory error");
		}
		else
		{
			/* 读取dir目录*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
				sprintf(path,"%s/%s",dir,d->d_name);
				flag = 0;
				//print2msg(ECU_DBG_CLIENT,"delete_file_resendflag0 ",path);
				//打开文件一行行判断是否有flag!=0的  如果存在直接关闭文件并返回,如果不存在，删除文件
				fp = fopen(path, "r");
				if(fp)
				{
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))
					{
						if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
						{
							if(buff[strlen(buff)-2] != '0')			//检测是否存在resendflag != 0的记录   若存在则直接退出函数
							{
								flag = 1;
								break;
								//fclose(fp);
								//closedir(dirp);
								//rt_mutex_release(record_data_lock);
								//return;
							}
						}
						
					}
					fclose(fp);
					if(flag == 0)
					{
						print2msg(ECU_DBG_CLIENT,"unlink:",path);
						//遍历完文件都没发现flag != 0的记录直接删除文件
						unlink(path);
					}	
				}
				
			}
			/* 关闭目录 */
			closedir(dirp);
		}
	}
	rt_mutex_release(record_data_lock);
	return;

}



int send_record(int fd_sock, char *sendbuff, char *send_date_time)			//发送数据到EMA  注意在存储的时候结尾未添加'\n'  在发送时的时候记得添加
{
	int sendbytes=0;
	char readbuff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL] = {'\0'};
	sendbytes = send(fd_sock, sendbuff, strlen(sendbuff), 0);
	if(-1 == sendbytes)
		return -1;
	if(-1 == recv_response(fd_sock, readbuff))
		return -1;
	else
	{
		print2msg(ECU_DBG_CLIENT,"readbuff",readbuff);
		if('1' == readbuff[0])
			update_send_flag(send_date_time);
		clear_send_flag(readbuff);
		return 0;
	}
}

#ifdef WIFI_USE		
int wifi_socketb_format(char *data ,int length)
{
	char head[9] = {'\0'};
	char *p = NULL;
	int i = 0,retlength = 0;
	head[0] = 0x62;
	head[1] = 0x00;
	head[2] = 0x00;
	head[3] = 0x00;
	head[4] = 0x00;
	head[5] = 0x00;
	head[6] = 0x00;
	head[7] = 0x00;
	head[8] = 0x00;
	
	for(p = &data[9],i = 0;p <= (data+length-9);p++,i++)
	{
		if(!memcmp(p,head,9))
		{
			memcpy(p,p+9,(length-18-i));
			length -= 9;
			data[length - i+1] = '\0';
			retlength = length;
		}
	}

	return retlength;
}



int wifi_send_record(char *sendbuff, char *send_date_time)		//通过WIFI发送数据到EMA  注意在存储的时候结尾未添加'\n'  在发送时的时候记得添加
{
	int length = 0;
	char readbuff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL] = {'\0'};
	SendToSocketB(sendbuff, strlen(sendbuff));

	if(-1 == (length = RecvSocketData(SOCKET_B, readbuff,10)))
		return -1;
	else
	{
		//检查格式，如果是多个包在一起了，进行合并
		wifi_socketb_format(readbuff ,length);
		print2msg(ECU_DBG_CLIENT,"readbuff",&readbuff[9]);
		if('1' == readbuff[9])
			update_send_flag(send_date_time);
		clear_send_flag(&readbuff[9]);
		return 0;
	}

}
#endif

int preprocess()			//发送头信息到EMA,读取已经存在EMA的记录时间
{
	int sendbytes=0;
	char readbuff[1024] = {'\0'};
	char sendbuff[256] = {'\0'};
	FILE *fp;
	int fd_sock,length;

	if(0 == detection_resendflag2())		//	检测是否有resendflag='2'的记录
		return 0;

	strcpy(sendbuff, "APS13AAA22");
	fp = fopen("/yuneng/ecuid.con", "r");
	if(fp)
	{
		fgets(&sendbuff[10], 13, fp);
		fclose(fp);
	}
	strcat(sendbuff, "\n");
	print2msg(ECU_DBG_CLIENT,"Sendbuff", sendbuff);

	fd_sock = createsocket();
	printdecmsg(ECU_DBG_CLIENT,"Socket", fd_sock);
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
			if(recv_response(fd_sock, readbuff) > 3)
				clear_send_flag(readbuff);
			else
				break;
		}
#ifdef WIFI_USE		
	}else
	{	//连接服务器失败,使用WIFI传输数据
		if((1 == WIFI_QueryStatus(SOCKET_B)) || (0 == WIFI_Create(SOCKET_B)))
		{
			//创建成功
			while(1)
			{
				memset(readbuff, '\0', sizeof(readbuff));
				SendToSocketB(sendbuff, strlen(sendbuff));
				if((length = RecvSocketData(SOCKET_B,readbuff,5))> 9)
				{
					//检查格式，如果是多个包在一起了，进行合并
					wifi_socketb_format(readbuff ,length);
					clear_send_flag(&readbuff[9]);
				}	
				else
					break;
			}
		}
		WIFI_Close(SOCKET_B);
#endif
	}

	close_socket(fd_sock);
	return 0;
}

int resend_record()
{
	int fd_sock;
	char *data = NULL;//查询到的数据
	char time[15] = {'\0'};
	int flag,res;
	
	data = malloc(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL);
	memset(data,0x00,MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL);
	//在/home/record/data/目录下查询resendflag为2的记录
	fd_sock = createsocket();
	printdecmsg(ECU_DBG_CLIENT,"Socket", fd_sock);
	if(1 == connect_socket(fd_sock))
	{
		while(search_readflag(data,time,&flag,'2'))		//	获取一条resendflag为1的数据
		{
			if(1 == flag)		// 还存在需要上传的数据
					data[78] = '1';
			printmsg(ECU_DBG_CLIENT,data);
			res = send_record(fd_sock, data, time);
			if(-1 == res)
				break;
		}
#ifdef WIFI_USE 
	}else
	{
		//连接服务器失败,使用WIFI传输数据
		if((1 == WIFI_QueryStatus(SOCKET_B)) || (0 == WIFI_Create(SOCKET_B)))
		{
			//创建成功
			while(search_readflag(data,time,&flag,'2'))
			{
				if(1 == flag)		// 还存在需要上传的数据
					data[78] = '1';
				printmsg(ECU_DBG_CLIENT,data);
				res = wifi_send_record(data, time);
				if(-1 == res)
					break;
			}
		}
		WIFI_Close(SOCKET_B);
#endif
	}
	close_socket(fd_sock);
	free(data);
	return 0;
}


void client_thread_entry(void* parameter)
{
	char broadcast_hour_minute[3]={'\0'};
	char broadcast_time[16];
	int fd_sock;
	int thistime=0, lasttime=0,res,flag;
	char data[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL] = {'\0'};//查询到的数据
	char time[15] = {'\0'};
	rt_thread_delay(RT_TICK_PER_SECOND*START_TIME_CLIENT);
	printmsg(ECU_DBG_CLIENT,"Started");
	
	while(1)
	{
		thistime = lasttime = acquire_time();
		
		if((2 == get_hour())||(1 == get_hour()))
		//if(1)
		{
			preprocess();		//预处理,先发送一个头信息给EMA,让EMA把标记为2的记录的时间返回ECU,然后ECU再把标记为2的记录再次发送给EMA,防止EMA收到记录返回收到标志而没有存入数据库的请情况
			resend_record();
			delete_file_resendflag0();		//清空数据resend标志全部为0的目录
		}
		
		get_time(broadcast_time, broadcast_hour_minute);
		print2msg(ECU_DBG_CLIENT,"time",broadcast_time);
		
		fd_sock = createsocket();
		printdecmsg(ECU_DBG_CLIENT,"Socket", fd_sock);
		if(1 == connect_socket(fd_sock))
		{
			while(search_readflag(data,time,&flag,'1'))		//	获取一条resendflag为1的数据
			{
				if(compareTime(thistime ,lasttime,300))
				{
					break;
				}
				if(1 == flag)		// 还存在需要上传的数据
						data[78] = '1';
				printmsg(ECU_DBG_CLIENT,data);
				res = send_record(fd_sock, data, time);
				if(-1 == res)
					break;
				thistime = acquire_time();
				memset(data,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL));
				memset(time,0,15);
			}
#ifdef WIFI_USE			
		}else
		{
			//不需要先打开，在有数据需要发送的时候打开 就行。
			//if((1 == WIFI_QueryStatus(SOCKET_B)) || (0 == WIFI_Create(SOCKET_B)))
			{
				while(search_readflag(data,time,&flag,'1'))		//	获取一条resendflag为1的数据
				{
					if(compareTime(thistime ,lasttime,300))
					{
						break;
					}
					if(1 == flag)		// 还存在需要上传的数据
							data[78] = '1';
					printmsg(ECU_DBG_CLIENT,data);
					res = wifi_send_record(data, time);
					if(-1 == res)
						break;
					thistime = acquire_time();
					memset(data,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL));
					memset(time,0,15);
				}
			}
			WIFI_Close(SOCKET_B);
#endif	
		}
		close_socket(fd_sock);
		delete_file_resendflag0();		//清空数据resend标志全部为0的目录
		if((thistime < 300) && (lasttime > 300))
		{
			if((thistime+(24*60*60+1)-lasttime) < 300)
			{
				rt_thread_delay(300 - ((thistime+(24*60*60+1)) + lasttime) * RT_TICK_PER_SECOND);
			}
			
		}else
		{
			if((thistime-lasttime) < 300)
			{
				rt_thread_delay((300 + lasttime - thistime) * RT_TICK_PER_SECOND);
			}
		}
		
	}
}


