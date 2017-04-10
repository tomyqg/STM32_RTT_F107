#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "control_client.h"
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
#include "myfile.h"
#include "mysocket.h"
#include "remote_control_protocol.h"

#include "inverter_id.h"


ALIGN(RT_ALIGN_SIZE)
extern rt_uint8_t control_client_stack[ 14336 ];
extern struct rt_thread control_client_thread;

extern rt_mutex_t record_data_lock;
char ecuid[13] = {'\0'};

#define ARRAYNUM 6
#define MAXBUFFER 4096
#define FIRST_TIME_CMD_NUM 12

typedef struct socket_config
{
	int timeout;
	int report_interval;
	int port1;
	int port2;
	char domain[32];
	char ip[16];
}Socket_Cfg;

enum CommandID{
	A100, A101, A102, A103, A104, A105, A106, A107, A108, A109, //0-9
	A110, A111, A112, A113, A114, A115, A116, A117, A118, A119, //10-19
	A120, A121, A122, A123, A124, A125, A126, A127, A128, A129, //20-29
	A130, A131, A132, A133, A134, A135, A136, A137, A138, A139, //30-39
	A140, A141, A142, A143, A144, A145, A146, A147, A148, A149,
	A150,A151,
};
int (*pfun[100])(const char *recvbuffer, char *sendbuffer);
Socket_Cfg sockcfg = {'\0'};


void add_functions()
{
//  pfun[A102] = response_inverter_id; 			//上报逆变器ID
//  pfun[A103] = set_inverter_id; 				//设置逆变器ID
//  pfun[A104] = response_time_zone; 			//上报ECU本地时区
//	pfun[A105] = set_time_zone; 				//设置ECU本地时区
//	pfun[A106] = response_comm_config;			//上报ECU通讯配置参数
//	pfun[A107] = set_comm_config;				//设置ECU通讯配置参数
//	pfun[A108] = custom_command;				//向ECU发送自定义命令
//	pfun[A109] = set_inverter_ac_protection_5; 	//设置逆变器交流保护参数(5项)
//	pfun[A110] = set_inverter_maxpower;			//设置逆变器最大功率
//	pfun[A111] = set_inverter_onoff;			//设置逆变器开关机
//	pfun[A112] = clear_inverter_gfdi;			//设置逆变器GFDI
//	pfun[A113] = response_ecu_ac_protection_5;	//上报ECU级别交流保护参数(5项)
//	pfun[A114] = read_inverter_ac_protection_5; //读取逆变器的交流保护参数(5项)
//	pfun[A117] = response_inverter_maxpower;	//上报逆变器最大功率及范围
//	pfun[A119] = set_ecu_flag;					//设置ECU与EMA的通讯开关
//	pfun[A120] = response_ecu_ac_protection_13;	//上报ECU级别交流保护参数(13项)
//	pfun[A121] = read_inverter_ac_protection_13;//读取逆变器的交流保护参数(13项)
//	pfun[A122] = set_inverter_ac_protection_13;	//设置逆变器的交流保护参数(13项)
//	pfun[A124] = read_inverter_grid_environment;//读取逆变器电网环境
//	pfun[A125] = set_inverter_grid_environment;	//设置逆变器电网环境
//	pfun[A126] = read_inverter_ird;				//读取逆变器的IRD选项
//	pfun[A127] = set_inverter_ird;				//设置逆变器的IRD选项
//	pfun[A128] = read_inverter_signal_strength;	//读取逆变器的信号强度
//	pfun[A129] = response_grid_quality;			//上报系统的电网质量
//	pfun[A130] = response_ecu_ac_protection_17;	//上报ECU级别交流保护参数(17项)
//	pfun[A131] = read_inverter_ac_protection_17;//读取逆变器的交流保护参数(17项)
//	pfun[A132] = set_inverter_ac_protection_17;	//设置逆变器的交流保护参数(17项)
//	pfun[A134] = set_inverter_restore;			//设置逆变器的还原标志
//	pfun[A136] = set_inverter_update;			//设置逆变器的升级标志
//	pfun[A138] = set_autoflag_report;			//设置ECU自动上报功能
//	pfun[A148] = read_wrong_id;					//读取异常的3501uid
//	pfun[A149] = response_wrong_id;				//上报异常的3501uid
//	pfun[A150] = set_unnormal_id;				//设置3501正确的id
//	pfun[A151] = response_changed_id;			//上报修改成功的id
}

/* [A118] ECU初次连接EMA需要执行的打包命令 */
int first_time_info(const char *recvbuffer, char *sendbuffer)
{
	static int command_id = 0;
	int functions[FIRST_TIME_CMD_NUM] = {
			A102, A104, A106, A113, A114, A117,
			A120, A121, A124, A126, A130, A131,
	};

	//调用函数
	(*pfun[functions[command_id++]%100])(recvbuffer, sendbuffer);
//	debug_msg("cmd:A%d", functions[command_id - 1] + 100);
	rt_hw_us_delay(100000);

	if(command_id < FIRST_TIME_CMD_NUM)
		return 118;
	else
		return 0;
}
/* 将从文件中读取的键值对保存到socket配置结构体中 */
int get_socket_config(Socket_Cfg *cfg, MyArray *array)
{
	int i;

	for(i=0; i<ARRAYNUM; i++){
		if(!strlen(array[i].name))break;
		//超时时间
		if(!strcmp(array[i].name, "Timeout")){
			cfg->timeout = atoi(array[i].value);
		}
		//轮训时间
		else if(!strcmp(array[i].name, "Report_Interval")){
			cfg->report_interval = atoi(array[i].value);
		}
		//域名
		else if(!strcmp(array[i].name, "Domain")){
			strncpy(cfg->domain, array[i].value, 32);
		}
		//IP地址
		else if(!strcmp(array[i].name, "IP")){
			strncpy(cfg->ip, array[i].value, 16);
		}
		//端口1
		else if(!strcmp(array[i].name, "Port1")){
			cfg->port1 = atoi(array[i].value);
		}
		//端口2
		else if(!strcmp(array[i].name, "Port2")){
			cfg->port2 = atoi(array[i].value);
		}
	}
	return 0;
}

/* 随机取port1或port2 */
int randport(Socket_Cfg cfg)
{
	srand((unsigned)acquire_time());
	if(rand()%2)
		return cfg.port1;
	else
		return cfg.port2;
}


int detection_statusflag(char flag)		//检测/home/record/inversta目录下是否存在flag的记录    存在返回1，不存在返回0
{
	DIR *dirp;
	char dir[30] = "/home/record/inversta";
	struct dirent *d;
	char path[100];
	char buff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18]={'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		/* 打开dir目录*/
		dirp = opendir("/home/record/inversta");
		
		if(dirp == RT_NULL)
		{
			rt_kprintf("open directory error!\n");
		}
		else
		{
			/* 读取dir目录*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
				sprintf(path,"%s/%s",dir,d->d_name);
				//printf("%s\n",path);
				//打开文件一行行判断是否有flag=2的  如果存在直接关闭文件并返回1
				fp = fopen(path, "r");
				if(fp)
				{
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))
					{
						if(buff[strlen(buff)-2] == flag)			//检测最后一个字节的resendflag是否为flag   如果发现是flag  关闭文件并且return 1
						{
							fclose(fp);
							closedir(dirp);
							rt_mutex_release(record_data_lock);
							return 1;
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

//将status状态为2的标志改为1
int change_statusflag1()  //改变成功返回1
{
	DIR *dirp;
	char dir[30] = "/home/record/inversta";
	struct dirent *d;
	char path[100];
	char buff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18]={'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		/* 打开dir目录*/
		dirp = opendir("/home/record/inversta");
		
		if(dirp == RT_NULL)
		{
			rt_kprintf("open directory error!\n");
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

						if(buff[strlen(buff)-2] == '2')
						{
							fseek(fp,-2L,SEEK_CUR);
							fputc('1',fp);
							fclose(fp);
							closedir(dirp);
							rt_mutex_release(record_data_lock);
							return 1;
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

void delete_statusflag0()		//清空数据flag标志全部为0的目录
{
	DIR *dirp;
	char dir[30] = "/home/record/inversta";
	struct dirent *d;
	char path[100];
	char buff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18]={'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		/* 打开dir目录*/
		dirp = opendir("/home/record/inversta");
		
		if(dirp == RT_NULL)
		{
			rt_kprintf("open directory error!\n");
		}
		else
		{
			/* 读取dir目录*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
				sprintf(path,"%s/%s",dir,d->d_name);
				//打开文件一行行判断是否有flag!=0的  如果存在直接关闭文件并返回,如果不存在，删除文件
				fp = fopen(path, "r");
				if(fp)
				{
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))
					{
						if(buff[strlen(buff)-2] != '0')			//检测是否存在resendflag != 0的记录   若存在则直接退出函数
						{
							fclose(fp);
							closedir(dirp);
							rt_mutex_release(record_data_lock);
							return;
						}		
					}
					fclose(fp);
					//遍历完文件都没发现flag != 0的记录直接删除文件
					unlink(path);
				}			
			}
			/* 关闭目录 */
			closedir(dirp);
		}
	}
	rt_mutex_release(record_data_lock);
	return;
}

//根据时间点修改标志
int change_statusflag(char *time,char flag)  //改变成功返回1，未找到该时间点返回0
{
	DIR *dirp;
	char dir[30] = "/home/record/inversta";
	struct dirent *d;
	char path[100];
	char filetime[15] = {'\0'};
	char buff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18]={'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		/* 打开dir目录*/
		dirp = opendir("/home/record/inversta");
		
		if(dirp == RT_NULL)
		{
			rt_kprintf("open directory error!\n");
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
						memset(filetime,0,15);
						memcpy(filetime,&buff[strlen(buff)-17],14);				//获取每条记录的时间
						filetime[14] = '\0';
						if(!memcmp(time,filetime,14))						//每条记录的时间和传入的时间对比，若相同则变更flag				
						{
							fseek(fp,-2L,SEEK_CUR);
							fputc(flag,fp);
							//printf("%s\n",filetime);
							fclose(fp);
							closedir(dirp);
							rt_mutex_release(record_data_lock);
							return 1;
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

//查询一条flag为1的数据   查询到了返回1  如果没查询到返回0
/*
data:表示获取到的数据
time：表示获取到的时间
flag：表示是否还有下一条数据   存在下一条为1   不存在为0
*/
int search_statusflag(char *data,char * time, int *flag,char sendflag)	
{
	DIR *dirp;
	char dir[30] = "/home/record/inversta";
	struct dirent *d;
	char path[100];
	char buff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18]={'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		/* 打开dir目录*/
		dirp = opendir("/home/record/inversta");
		if(dirp == RT_NULL)
		{
			rt_kprintf("open directory error!\n");
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
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))  //读取一行数据
					{
						if(buff[strlen(buff)-2] == sendflag)			//检测最后一个字节的resendflag是否为1
						{
							memcpy(time,&buff[strlen(buff)-17],14);				//获取每条记录的时间
							memcpy(data,buff,(strlen(buff)-18));
							data[strlen(buff)-18] = '\n';
							//printf("time:%s   data:%s\n",time,data);
							rt_thread_delay(RT_TICK_PER_SECOND*1);
							while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))	//再往下读数据，寻找是否还有要发送的数据
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

							*flag = 0;
							fclose(fp);
							closedir(dirp);
							rt_mutex_release(record_data_lock);
							return 1;
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


void delete_pro_result_flag0()		//清空数据flag标志全部为0的目录
{
	DIR *dirp;
	char dir[30] = "/home/data/proc_res";
	struct dirent *d;
	char path[100];
	char buff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18]={'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		/* 打开dir目录*/
		dirp = opendir("/home/data/proc_res");
		
		if(dirp == RT_NULL)
		{
			rt_kprintf("open directory error!\n");
		}
		else
		{
			/* 读取dir目录*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
				sprintf(path,"%s/%s",dir,d->d_name);
				//打开文件一行行判断是否有flag!=0的  如果存在直接关闭文件并返回,如果不存在，删除文件
				fp = fopen(path, "r");
				if(fp)
				{
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))
					{
						if(buff[strlen(buff)-2] != '0')			//检测是否存在resendflag != 0的记录   若存在则直接退出函数
						{
							fclose(fp);
							closedir(dirp);
							rt_mutex_release(record_data_lock);
							return;
						}		
					}
					fclose(fp);
					//遍历完文件都没发现flag != 0的记录直接删除文件
					unlink(path);
				}			
			}
			/* 关闭目录 */
			closedir(dirp);
		}
	}
	rt_mutex_release(record_data_lock);
	return;
}

//根据时间点修改标志
int change_pro_result_flag(char *item,char flag)  //改变成功返回1，未找到该时间点返回0
{
	DIR *dirp;
	char dir[30] = "/home/data/proc_res";
	struct dirent *d;
	char path[100];
	char fileitem[4] = {'\0'};
	char buff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18]={'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		/* 打开dir目录*/
		dirp = opendir("/home/data/proc_res");
		
		if(dirp == RT_NULL)
		{
			rt_kprintf("open directory error!\n");
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
						memset(fileitem,0,4);
						memcpy(fileitem,&buff[strlen(buff)-6],3);				//获取每条记录的时间
						fileitem[3] = '\0';
						if(!memcmp(item,fileitem,4))						//每条记录的时间和传入的时间对比，若相同则变更flag				
						{
							fseek(fp,-2L,SEEK_CUR);
							fputc(flag,fp);
							//printf("%s\n",filetime);
							fclose(fp);
							closedir(dirp);
							rt_mutex_release(record_data_lock);
							return 1;
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
//查询一条flag为1的数据   查询到了返回1  如果没查询到返回0
/*
data:表示获取到的数据
item：表示获取的命令帧
flag：表示是否还有下一条数据   存在下一条为1   不存在为0
*/
int search_pro_result_flag(char *data,char * item, int *flag,char sendflag)	
{
	DIR *dirp;
	char dir[30] = "/home/data/proc_res";
	struct dirent *d;
	char path[100];
	char buff[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18]={'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		/* 打开dir目录*/
		dirp = opendir("/home/data/proc_res");
		if(dirp == RT_NULL)
		{
			rt_kprintf("open directory error!\n");
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
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))  //读取一行数据
					{
						if(buff[strlen(buff)-2] == sendflag)			//检测最后一个字节的resendflag是否为1
						{
							memcpy(item,&buff[strlen(buff)-6],3);				//获取每条记录的item
							memcpy(data,buff,(strlen(buff)-7));
							data[strlen(buff)-7] = '\n';
							//printf("time:%s   data:%s\n",time,data);
							rt_thread_delay(RT_TICK_PER_SECOND*1);
							while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))	//再往下读数据，寻找是否还有要发送的数据
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

							*flag = 0;
							fclose(fp);
							closedir(dirp);
							rt_mutex_release(record_data_lock);
							return 1;
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


//检查逆变器异常状态
int check_inverter_abnormal_status_sent(int hour)
{
	int sockfd;
	int i, flag, num = 0;
	char datetime[15] = {'\0'};
	char recv_buffer[4096] = {'\0'};
	char send_buffer[MAXBUFFER] = {'\0'};

	if(get_hour() != hour)
		return 0;

	//查询是否有flag=2的数据
	if(0 == detection_statusflag('2'))
		return 0;
	//有flag=2的数据,发送一条读取EMA已存时间戳命令
	printmsg("control_client",">>Start Check abnormal status sent");
	sockfd = client_socket_init(randport(sockcfg), sockcfg.ip, sockcfg.domain);
	if(sockfd < 0) return -1;
	strcpy(send_buffer, "APS13AAA51A123AAA0");
	strcat(send_buffer, ecuid);
	strcat(send_buffer, "000000000000000000END\n");
	send_socket(sockfd, send_buffer, strlen(send_buffer));
	//接收EMA应答
	if(recv_socket(sockfd, recv_buffer, sizeof(recv_buffer), sockcfg.timeout) <= 0){
		close(sockfd);
		return 0;
	}
	//校验命令
	if(msg_format_check(recv_buffer) < 0){
		close(sockfd);
		return 0;
	}
	//解析收到的时间戳,并删除EMA已存的数据(将其改为0)
	flag = msg_get_int(&recv_buffer[18], 1);
	num = 0;
	if(flag){
		num = msg_get_int(&recv_buffer[19], 2);
		for(i=0; i<num; i++){
			strncpy(datetime, &recv_buffer[21 + i*14], 14);
			change_statusflag(datetime,'0');
		}
	}

	//将flag=2的数据改为flag=1
	change_statusflag1();
	close(sockfd);
	
	//如果所有标志为0，则清空数据
	delete_statusflag0();
	return 0;
}

/* 从文件中查询是否存在逆变器异常状态 */
int exist_inverter_abnormal_status()
{
	int result = 0;
	//查询该逆变器ID在表中是否存在
	if(1 == detection_statusflag('1')){
				result = 1;
	}
	return result;
}

/* [A123]ECU上报逆变器的异常状态 */
int response_inverter_abnormal_status()
{
	int result = 0;
	int  j, sockfd, flag, num, cmd_id, next_cmd_id,havaflag;
	char datetime[15] = {'\0'};
	char recv_buffer[4096] = {'\0'};
	char command[4096] = {'\0'};
	char send_buffer[1024]={'\0'};
	char save_buffer[MAXBUFFER] = {'\0'};
	char data[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL] = {'\0'};//查询到的数据
	char time[15] = {'\0'};
	FILE *fp;	
	printmsg("control_client",">>Start Response Abnormal Status");

	//建立socket连接
	sockfd = client_socket_init(randport(sockcfg), sockcfg.ip, sockcfg.domain);
	if(sockfd < 0) return -1;
	//逐条发送逆变器异常状态
	while(search_statusflag(data,time,&havaflag,'1'))		//	获取一条resendflag为1的数据
	{	
		//发送一条逆变器异常状态信息
		if(send_socket(sockfd, data, strlen(data)) < 0){
			continue;
		}
		//接收EMA应答
		if(recv_socket(sockfd, recv_buffer, sizeof(recv_buffer), sockcfg.timeout) <= 0){
			close(sockfd);
			return 0;
		}
		//校验命令
		if(msg_format_check(recv_buffer) < 0){
			continue;
		}
		//将发送和接受都成功的那一条状态的标志置2
		change_statusflag(time,'2');
		//解析收到的时间戳,并删除EMA已存数据
		flag = msg_get_int(&recv_buffer[18], 1);
		num = 0;
		if(flag){
			num = msg_get_int(&recv_buffer[19], 2);
			for(j=0; j<num; j++){
				strncpy(datetime, &recv_buffer[21 + j*14], 14);
				change_statusflag(datetime,'0');
			}
		}
		//判断应答帧是否附带命令
		if(strlen(recv_buffer) > (24 + 14*num)){
			memset(command, 0, sizeof(command));
			strncpy(command, &recv_buffer[24 + 14*num], sizeof(command));
			print2msg("control_client","Command", command);
			//校验命令
			if(msg_format_check(command) < 0)
				continue;
			//解析命令号
			cmd_id = msg_cmd_id(command);

			if(cmd_id==118)
			{
				char da_time[20]={'\0'};
				strncpy(da_time, &recv_buffer[72],14);
				
				fp=fopen("/yuneng/A118.con","w");
				if(fp==NULL)
					return -1;
				else
					{
						fputs("1",fp);
						fclose(fp);
						
						memset(send_buffer,0x00,1024);
						msg_ACK(send_buffer, "A118", da_time, 0);
						send_socket(sockfd, send_buffer, strlen(send_buffer));
						printmsg("control_client",">>End");
						printdecmsg("control_client","socked",sockfd);
						result=1;break;
					}
			}
			//调用函数
			else if(pfun[cmd_id%100]){
				next_cmd_id = (*pfun[cmd_id%100])(command, save_buffer);
				save_process_result(cmd_id, save_buffer);
				if(next_cmd_id > 0){
					memset(command, 0, sizeof(command));
					snprintf(recv_buffer, 51+1, "APS13AAA51A101AAA0000000000000A%3d00000000000000END", next_cmd_id);
					(*pfun[next_cmd_id%100])(command, save_buffer);
					save_process_result(next_cmd_id, save_buffer);
				}
				else if(next_cmd_id < 0){
					result = -1;
				}
			}
		}
	}
	//清空inversta的flag标志位为0的标志
	delete_statusflag0();
	close(sockfd);
	return result;
}

/* 与EMA进行通讯 */
int communication_with_EMA(int next_cmd_id)
{
	int sockfd;
	int cmd_id;
	char timestamp[15] = "00000000000000";
	char recv_buffer[4096] = {'\0'};
	char send_buffer[MAXBUFFER] = {'\0'};
	int one_a118=0;
	
	while(1)
	{
		printmsg("control_client","Start Communication with EMA");
		sockfd = client_socket_init(randport(sockcfg), sockcfg.ip, sockcfg.domain);
		if(sockfd < 0) return -1;
		if(next_cmd_id <= 0)
		{
			//ECU向EMA发送请求命令指令
			msg_REQ(send_buffer);
			send_socket(sockfd, send_buffer, strlen(send_buffer));
			memset(send_buffer, '\0', sizeof(send_buffer));

			//接收EMA发来的命令
			if(recv_socket(sockfd, recv_buffer, sizeof(recv_buffer), sockcfg.timeout) < 0){
				close(sockfd);
				break;
			}
			
			//校验命令
			if(msg_format_check(recv_buffer) < 0){
				close(sockfd);
				continue;
			}
			
			//解析命令号
			cmd_id = msg_cmd_id(recv_buffer);
		}
		else{
			//生成下一条命令(用于设置命令结束后,上报设置后的ECU状态)
			cmd_id = next_cmd_id;
			next_cmd_id = 0;
			memset(recv_buffer, 0, sizeof(recv_buffer));
			snprintf(recv_buffer, 51+1, "APS13AAA51A101AAA0%.12sA%3d%.14sEND",
					ecuid, cmd_id, timestamp);
		}

		//ECU注册后初次和EMA通讯
		if(cmd_id == 118){
			if(one_a118==0){
				one_a118=1;
				//system("rm /etc/yuneng/fill_up_data.conf");
				//system("echo '1'>>/etc/yuneng/fill_up_data.conf");
				//system("killall main.exe");
			}
			strncpy(timestamp, &recv_buffer[34], 14);
			next_cmd_id = first_time_info(recv_buffer, send_buffer);
			if(next_cmd_id == 0){
				strncpy(timestamp, "00000000000000", 14);
			}
		}
		//根据命令号调用函数
		else if(pfun[cmd_id%100]){
			//若设置函数调用完毕后需要执行上报,则会返回上报函数的命令号,否则返回0
			next_cmd_id = (*pfun[cmd_id%100])(recv_buffer, send_buffer);
		}
		//EMA命令发送完毕
		else if(cmd_id == 100){
			close(sockfd);
			break;
		}
		else{
			//若命令号不存在,则发送设置失败应答(每条设置协议的时间戳位置不统一,返回时间戳是个问题...)
			memset(send_buffer, 0, sizeof(send_buffer));
			snprintf(send_buffer, 52+1, "APS13AAA52A100AAA0%sA%3d000000000000002END",
					ecuid, cmd_id);
		}

		//将消息发送给EMA(自动计算长度,补上回车)
		send_socket(sockfd, send_buffer, strlen(send_buffer));
		printmsg("control_client",">>End");
		close(sockfd);

		//如果功能函数返回值小于0,则返回-1,程序会自动退出
		if(next_cmd_id < 0){
			return -1;
		}
	}
	printmsg("control_client",">>End");
	return 0;
}

/* 上报process_result表中的信息 */
int response_process_result()
{
	//char sendbuffer[MAXBUFFER] = {'\0'};
	int sockfd, flag;
	//int num, i;
	//int item_num[32] = {0};
	char data[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL] = {'\0'};//查询到的数据
	char item[4] = {'\0'};
	

	{
		//查询所有[ECU级别]处理结果
		//逐条上报ECU级别处理结果
		while(search_pro_result_flag(data,item,&flag,'1'))
		{
			printmsg("control_client",">>Start Response ECU Process Result");
			sockfd = client_socket_init(randport(sockcfg), sockcfg.ip, sockcfg.domain);
			if(sockfd < 0) return -1;
			//发送一条记录
			if(send_socket(sockfd, data, strlen(data)) < 0){
				close(sockfd);
				continue;
			}
			//发送成功则将标志位置0
			change_pro_result_flag(item,'0');
			close(sockfd);
			printmsg("control_client",">>End");
		}				
		delete_pro_result_flag0();
		
		/*
		//查询[逆变器级别]处理结果的命令号,存入数组
		memset(sql, 0, sizeof(sql));
		snprintf(sql, sizeof(sql), "SELECT distinct(item) FROM inverter_process_result WHERE flag=1");
		if(get_data(db, sql, &azResult, &nrow, &ncolumn) != 0){
			close_db(db);
			return 0;
		}
		for(i=1; i<=nrow; i++)
		{
			item_num[i-1] = atoi(azResult[i]);
		}
		if(nrow)sqlite3_free_table(azResult);
		//最条拼接并上报逆变器级别处理结果
		num = 0;
		while(item_num[num] != 0)
		{
			debug_msg("item:%d", item_num[num]);
			item = item_num[num++];
			memset(sql, '\0', sizeof(sql));
			snprintf(sql, sizeof(sql), "SELECT result FROM inverter_process_result WHERE item=%d and flag=1", item);
			if(get_data(db, sql, &azResult, &nrow, &ncolumn) != 0){
				close_db(db);
				return 0;
			}
			//拼接数据
			memset(sendbuffer, 0, sizeof(sendbuffer));
			sprintf(sendbuffer, "APS1300000A%03dAAA0%.12s%04d00000000000000END", item, ecuid, nrow);
			for(i=1; i<=nrow; i++)
			{
				strcat(sendbuffer, azResult[i]);
			}
			if(nrow)sqlite3_free_table(azResult);
			//发送数据
			debug_msg(">>Start Response Inverter Process Result");
			sockfd = client_socket_init(randport(sockcfg), sockcfg.ip, sockcfg.domain);
			if(sockfd < 0) return -1;
			if(send_socket(sockfd, sendbuffer, strlen(sendbuffer)) < 0){
				close(sockfd);
				continue;
			}
			memset(sql, 0, sizeof(sql));
			snprintf(sql, sizeof(sql), "UPDATE inverter_process_result SET flag=0 WHERE item=%d", item);
			update_data(db, sql);
			close(sockfd);
		}
		close_db(db);
		*/
	}
	return 0;
}

void control_client_thread_entry(void* parameter)
{
	int result, ecu_time = 0, ecu_flag = 1;
	char buffer[16] = {'\0'};
	MyArray array[ARRAYNUM] = {'\0'};
	FILE *fp;
	delete_line("/test","/ttt","0",1);
	//添加功能函数
  add_functions();
	
	//获取ECU的通讯开关flag
	if(file_get_one(buffer, sizeof(buffer), "/yuneng/ecu_flag.con")){
		ecu_flag = atoi(buffer);
	}

	printdecmsg("control_client","ecu_flag", ecu_flag);
	
	file_get_one(ecuid, sizeof(ecuid), "/yuneng/ecuid.con");

	//从配置文件中获取socket通讯参数
	if(file_get_array(array, ARRAYNUM, "/yuneng/control.con") == 0){
		get_socket_config(&sockcfg, array);
	}
	
	/* ECU轮训主循环 */
	while(1)
	{
		//每天一点时向EMA确认逆变器异常状态是否被存储
		check_inverter_abnormal_status_sent(1);

		fp=fopen("/yuneng/A118.con","r");
		if(fp!=NULL)
		{
			char c='0';
			c=fgetc(fp);
			if(c=='1')
				result = communication_with_EMA(118);
			fclose(fp);
			unlink("/yuneng/A118.con");
		}
		
		if(exist_inverter_abnormal_status() && ecu_flag){
			ecu_time =  acquire_time();
			result = response_inverter_abnormal_status();
			response_process_result();
		}
		else if(compareTime(acquire_time() ,ecu_time,60*sockcfg.report_interval)){
			ecu_time = acquire_time();
			if(ecu_flag){ //如果ecu_flag = 0 则不上报处理结果
				response_process_result();
			}
			result = communication_with_EMA(0);
		}
		//程序自行跳过本次循环
		if(result < 0){
			result = 0;
			printmsg("control_client","Quit control_client");
			continue;
		}
		
		rt_thread_delay(RT_TICK_PER_SECOND*sockcfg.report_interval*60/3);
		
	}

}
