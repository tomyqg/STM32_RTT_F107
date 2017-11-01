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
#include "variation.h"
#include "usart5.h"
#include "lan8720rst.h"
#include "clientSocket.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern rt_mutex_t record_data_lock;
extern ecu_info ecu;
extern rt_mutex_t usr_wifi_lock;


/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int readconnecttime(void)			//保存最后一次连接上服务器的时间
{
	char connecttime[20]={'\0'};
	FILE *fp;
	memset(ecu.last_ema_time,'0',15);
	ecu.last_ema_time[14] = '\0';
	fp=fopen("/yuneng/con_time.con","r");
	if(fp != NULL)
	{
		fgets(connecttime,20,fp);
		memcpy(ecu.last_ema_time,connecttime,15);
		fclose(fp);
	}
	print2msg(ECU_DBG_CLIENT,"ecu.last_ema_time:",ecu.last_ema_time);
	return 0;
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
					//rt_hw_s_delay(1);
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
		rt_hw_s_delay(1);
	}

	return 0;
}

int detection_resendflag2()		//存在返回1，不存在返回0
{
	DIR *dirp;
	char dir[30] = "/home/record/data";
	struct dirent *d;
	char path[100];
	char *buff = NULL;
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	buff = malloc(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
	memset(buff,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
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
						if(strlen(buff) > 18)
						{
							//检查是否符合格式
							if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
							{
								if(buff[strlen(buff)-2] == '2')			//检测最后一个字节的resendflag是否为2   如果发现是2  关闭文件并且return 1
								{
									fclose(fp);
									closedir(dirp);
									free(buff);
									buff = NULL;
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
	free(buff);
	buff = NULL;
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
	char *buff = NULL;
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	buff = malloc(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
	memset(buff,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
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
						if(strlen(buff) > 18)
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
									free(buff);
									buff = NULL;
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
	free(buff);
	buff = NULL;
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
	char *buff = NULL;
	FILE *fp;
	int nextfileflag = 0;	//0表示当前文件找到了数据，1表示需要从后面的文件查找数据
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	buff = malloc(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
	memset(buff,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
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
							if(strlen(buff) > 18)
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
										rt_hw_s_delay(1);
										while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))	//再往下读数据，寻找是否还有要发送的数据
										{
											if(strlen(buff) > 18)
											{
												if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
												{
													if(buff[strlen(buff)-2] == sendflag)
													{
														*flag = 1;
														fclose(fp);
														closedir(dirp);
														free(buff);
														buff = NULL;
														rt_mutex_release(record_data_lock);
														return 1;
													}
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
								
						}
					}else
					{
						while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))  //读取一行数据
						{
							if(strlen(buff) > 18)
							{
								if((buff[strlen(buff)-3] == ',') && (buff[strlen(buff)-18] == ',') )
								{
									if(buff[strlen(buff)-2] == sendflag)
									{
										*flag = 1;
										fclose(fp);
										closedir(dirp);
										free(buff);
										buff = NULL;
										rt_mutex_release(record_data_lock);
										return 1;
									}
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
	free(buff);
	buff = NULL;
	rt_mutex_release(record_data_lock);

	return nextfileflag;
}


void delete_file_resendflag0()		//清空数据resend标志全部为0的目录
{
	DIR *dirp;
	char dir[30] = "/home/record/data";
	struct dirent *d;
	char path[100];
	char *buff = NULL;
	FILE *fp;
	int flag = 0;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	buff = malloc(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
	memset(buff,'\0',MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18);
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
						if(strlen(buff) > 18)
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
	free(buff);
	buff = NULL;
	rt_mutex_release(record_data_lock);
	return;

}



int send_record(char *sendbuff, char *send_date_time)			//发送数据到EMA  注意在存储的时候结尾未添加'\n'  在发送时的时候记得添加
{
	int sendbytes=0,readbytes = 4+99*14;
	char *readbuff = NULL;
	readbuff = malloc((4+99*14));
	memset(readbuff,'\0',(4+99*14));
	
	sendbytes = serverCommunication_Client(sendbuff,strlen(sendbuff),readbuff,&readbytes,10000);
	if(-1 == sendbytes)
	{
		free(readbuff);
		readbuff = NULL;
		return -1;
	}
		
	if(readbytes < 3)
	{
		free(readbuff);
		readbuff = NULL;
		return -1;
	}
	else
	{
		//print2msg(ECU_DBG_CLIENT,"readbuff",readbuff);
		if('1' == readbuff[0])
			update_send_flag(send_date_time);
		clear_send_flag(readbuff);
		free(readbuff);
		readbuff = NULL;
		return 0;
	}
}


int preprocess()			//发送头信息到EMA,读取已经存在EMA的记录时间
{
	int sendbytes = 0,readbytes = 0;
	char *readbuff = NULL;
	char sendbuff[50] = {'\0'};

	if(0 == detection_resendflag2())		//	检测是否有resendflag='2'的记录
		return 0;
	readbuff = malloc((4+99*14));
	memset(readbuff,0x00,(4+99*14));
	readbytes = 4+99*14;
	strcpy(sendbuff, "APS13AAA22");
	memcpy(&sendbuff[10],ecu.id,12);
	strcat(sendbuff, "\n");
	print2msg(ECU_DBG_CLIENT,"Sendbuff", sendbuff);

	//发送到服务器
	sendbytes = serverCommunication_Client(sendbuff,strlen(sendbuff),readbuff,&readbytes,10000);
	if(-1 == sendbytes)
	{
		free(readbuff);
		readbuff = NULL;
		return -1;
	}
	if(readbytes >3)
	{
		clear_send_flag(readbuff);
		
	}else
	{
		free(readbuff);
		readbuff = NULL;
		return -1;
	}
	free(readbuff);
	readbuff = NULL;
	return 0;

}

int resend_record()
{
	char *data = NULL;//查询到的数据
	char time[15] = {'\0'};
	int flag,res;
	
	data = malloc(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
	memset(data,0x00,CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
	//在/home/record/data/目录下查询resendflag为2的记录
	while(search_readflag(data,time,&flag,'2'))		//	获取一条resendflag为1的数据
	{
		//if(1 == flag)		// 还存在需要上传的数据
		//		data[78] = '1';
		//printmsg(ECU_DBG_CLIENT,data);
		res = send_record(data, time);
		if(-1 == res)
			break;
	}

	free(data);
	data = NULL;
	return 0;

}


void client_thread_entry(void* parameter)
{
	char broadcast_hour_minute[3]={'\0'};
	char broadcast_time[16];
	int thistime=0, lasttime=0,res,flag;
	char *data = NULL;//查询到的数据
	char time[15] = {'\0'};
	rt_thread_delay(RT_TICK_PER_SECOND*START_TIME_CLIENT);
	printmsg(ECU_DBG_CLIENT,"Started");

	data = malloc(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
	memset(data,0x00,CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER);
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
		
		while(search_readflag(data,time,&flag,'1'))		//	获取一条resendflag为1的数据
		{
			if(compareTime(thistime ,lasttime,300))
			{
				break;
			}
			//if(1 == flag)		// 还存在需要上传的数据
				//data[78] = '1';
			//printmsg(ECU_DBG_CLIENT,data);
			res = send_record( data, time);
			if(-1 == res)
				break;
			thistime = acquire_time();
			memset(data,0,(CLIENT_RECORD_HEAD+CLIENT_RECORD_ECU_HEAD+CLIENT_RECORD_INVERTER_LENGTH*MAXINVERTERCOUNT+CLIENT_RECORD_OTHER));
			memset(time,0,15);
		}
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


