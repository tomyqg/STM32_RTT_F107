/*
 * client.c
 * V1.2.2
 * modified on: 2013-08-13
 * ��EMA�첽ͨ��
 * update�������ݿ��������ʱ����ʱ��update
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
extern rt_uint8_t client_stack[ 8192 ];
extern struct rt_thread client_thread;

extern rt_mutex_t record_data_lock;


int writeconnecttime(void)			//�������һ�������Ϸ�������ʱ��
{
	char connecttime[20]={'\0'};
	FILE *fp;

	getcurrenttime(connecttime);
	fp=fopen("/yuneng/con_time.con","w");
	fprintf(fp,"%s",connecttime);
	fclose(fp);

	return 0;
}

void showconnected(void)		//������EMA
{
	FILE *fp;

	fp = fopen("/tmp/conemafl.txt", "w");
	if(fp)
	{
		fputs("connected", fp);
		fclose(fp);
	}

}

void showdisconnected(void)		//�޷�����EMA
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

int createsocket(void)					//�����׽���
{
	int fd_sock;

	fd_sock=socket(AF_INET,SOCK_STREAM,0);
	if(-1==fd_sock)
		printmsg("CLIENT","Failed to create socket");
	else
		printmsg("CLIENT","Create socket successfully");

	return fd_sock;
}

int connect_socket(int fd_sock)				//���ӵ�������
{
	char domain[100]={'\0'};		//EMA������
	char ip[20] = {'\0'};	//EMA��ȱʡIP
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
		printmsg("CLIENT","Resolve domain failure");
	}
	else
	{
		memset(ip, '\0', sizeof(ip));
		//printf("%s\n",*host->h_addr_list);
		//inet_ntop(AF_INET, *host->h_addr_list, ip, 32);
	}

	strcpy(ip, "139.168.200.158");
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

void close_socket(int fd_sock)					//�ر��׽���
{
	close(fd_sock);
	printmsg("CLIENT","Close socket");
}


int clear_send_flag(char *readbuff)
{
	int i, j, count;		//EMA���ض��ٸ�ʱ��(�м���ʱ��,��˵��EMA�����˶�������¼)
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
						print2msg("CLIENT","Clear send flag into database", "1");
						break;
					}
					else
						print2msg("CLIENT","Clear send flag into database", "0");
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
			print2msg("CLIENT","Update send flag into database", "1");
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

int detection_resendflag2()		//���ڷ���1�������ڷ���0
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
		/* ��dirĿ¼*/
		dirp = opendir("/home/record/data");
		
		if(dirp == RT_NULL)
		{
			rt_kprintf("open directory error!\n");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
				sprintf(path,"%s/%s",dir,d->d_name);
				//printf("%s\n",path);
				//���ļ�һ�����ж��Ƿ���flag=2��  �������ֱ�ӹر��ļ�������1
				fp = fopen(path, "r");
				if(fp)
				{
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))
					{
						if(buff[strlen(buff)-2] == '2')			//������һ���ֽڵ�resendflag�Ƿ�Ϊ2   ���������2  �ر��ļ�����return 1
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
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	rt_mutex_release(record_data_lock);
	return 0;
}

int change_resendflag(char *time,char flag)  //�ı�ɹ�����1��δ�ҵ���ʱ��㷵��0
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
		/* ��dirĿ¼*/
		dirp = opendir("/home/record/data");
		
		if(dirp == RT_NULL)
		{
			rt_kprintf("open directory error!\n");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
				sprintf(path,"%s/%s",dir,d->d_name);
				//���ļ�һ�����ж��Ƿ���flag=2��  �������ֱ�ӹر��ļ�������1
				fp = fopen(path, "r+");
				if(fp)
				{
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))
					{
						memset(filetime,0,15);
						memcpy(filetime,&buff[strlen(buff)-17],14);				//��ȡÿ����¼��ʱ��
						filetime[14] = '\0';
						if(!memcmp(time,filetime,14))						//ÿ����¼��ʱ��ʹ����ʱ��Աȣ�����ͬ����flag				
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
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	rt_mutex_release(record_data_lock);
	return 0;
	
}	

//��ѯһ��resendflagΪ1������   ��ѯ���˷���1  ���û��ѯ������0
/*
data:��ʾ��ȡ��������
time����ʾ��ȡ����ʱ��
flag����ʾ�Ƿ�����һ������   ������һ��Ϊ1   ������Ϊ0
*/
int search_readflag(char *data,char * time, int *flag,char sendflag)	
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
		/* ��dirĿ¼*/
		dirp = opendir("/home/record/data");
		if(dirp == RT_NULL)
		{
			rt_kprintf("open directory error!\n");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
				sprintf(path,"%s/%s",dir,d->d_name);
				fp = fopen(path, "r");
				if(fp)
				{
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))  //��ȡһ������
					{
						if(buff[strlen(buff)-2] == sendflag)			//������һ���ֽڵ�resendflag�Ƿ�Ϊ1
						{
							memcpy(time,&buff[strlen(buff)-17],14);				//��ȡÿ����¼��ʱ��
							memcpy(data,buff,(strlen(buff)-18));
							data[strlen(buff)-18] = '\n';
							//printf("time:%s   data:%s\n",time,data);
							rt_thread_delay(RT_TICK_PER_SECOND*1);
							while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))	//�����¶����ݣ�Ѱ���Ƿ���Ҫ���͵�����
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
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	rt_mutex_release(record_data_lock);
	return 0;
}


void delete_file_resendflag0()		//�������resend��־ȫ��Ϊ0��Ŀ¼
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
		/* ��dirĿ¼*/
		dirp = opendir("/home/record/data");
		
		if(dirp == RT_NULL)
		{
			rt_kprintf("open directory error!\n");
		}
		else
		{
			/* ��ȡdirĿ¼*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				memset(path,0,100);
				memset(buff,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18));
				sprintf(path,"%s/%s",dir,d->d_name);
				//printf("%s\n",path);
				//���ļ�һ�����ж��Ƿ���flag!=0��  �������ֱ�ӹر��ļ�������,��������ڣ�ɾ���ļ�
				fp = fopen(path, "r");
				if(fp)
				{
					while(NULL != fgets(buff,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL+18),fp))
					{
						if(buff[strlen(buff)-2] != '0')			//����Ƿ����resendflag != 0�ļ�¼   ��������ֱ���˳�����
						{
							fclose(fp);
							closedir(dirp);
							rt_mutex_release(record_data_lock);
							return;
						}		
					}
					fclose(fp);
					//�������ļ���û����flag != 0�ļ�¼ֱ��ɾ���ļ�
					unlink(path);
				}
				
			}
			/* �ر�Ŀ¼ */
			closedir(dirp);
		}
	}
	rt_mutex_release(record_data_lock);
	return;

}



int send_record(int fd_sock, char *sendbuff, char *send_date_time)			//�������ݵ�EMA  ע���ڴ洢��ʱ���βδ����'\n'  �ڷ���ʱ��ʱ��ǵ�����
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
		if('1' == readbuff[0])
			update_send_flag(send_date_time);
		clear_send_flag(readbuff);
		return 0;
	}
}


int preprocess()			//����ͷ��Ϣ��EMA,��ȡ�Ѿ�����EMA�ļ�¼ʱ��
{
	int sendbytes=0;
	char readbuff[1024] = {'\0'};
	char sendbuff[256] = {'\0'};
	FILE *fp;
	int fd_sock;

	if(0 == detection_resendflag2())		//	����Ƿ���resendflag='2'�ļ�¼
		return 0;

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
			if(recv_response(fd_sock, readbuff) > 3)
				clear_send_flag(readbuff);
			else
				break;
		}
	}

	close_socket(fd_sock);
	return 0;
}

int resend_record()
{
	int fd_sock;
	char data[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL] = {'\0'};//��ѯ��������
	char time[15] = {'\0'};
	int flag,res;
	
	//��/home/record/data/Ŀ¼�²�ѯresendflagΪ2�ļ�¼
	fd_sock = createsocket();
	printdecmsg("CLIENT","Socket", fd_sock);
	if(1 == connect_socket(fd_sock))
	{
		while(search_readflag(data,time,&flag,'2'))		//	��ȡһ��resendflagΪ1������
		{
				if(1 == flag)		// ��������Ҫ�ϴ�������
					data[78] = '1';
			printmsg("CLIENT",data);
			res = send_record(fd_sock, data, time);
			if(-1 == res)
				break;
		}
	}
	close_socket(fd_sock);
		
	return 0;
}


void client_thread_entry(void* parameter)
{
	char broadcast_hour_minute[3]={'\0'};
	char broadcast_time[16];
	int fd_sock;
	int thistime=0, lasttime=0,res,flag;
	char data[MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL] = {'\0'};//��ѯ��������
	char time[15] = {'\0'};
	rt_thread_delay(RT_TICK_PER_SECOND*20);
	printmsg("CLIENT","Started");
	
	while(1)
	{
		thistime = lasttime = acquire_time();
		
		if(1 == get_hour())
		{
			preprocess();		//Ԥ����,�ȷ���һ��ͷ��Ϣ��EMA,��EMA�ѱ��Ϊ2�ļ�¼��ʱ�䷵��ECU,Ȼ��ECU�ٰѱ��Ϊ2�ļ�¼�ٴη��͸�EMA,��ֹEMA�յ���¼�����յ���־��û�д������ݿ�������
			resend_record();
			delete_file_resendflag0();		//�������resend��־ȫ��Ϊ0��Ŀ¼
		}
		
		get_time(broadcast_time, broadcast_hour_minute);
		print2msg("client","time",broadcast_time);
		
		fd_sock = createsocket();
		printdecmsg("CLIENT","Socket", fd_sock);
		if(1 == connect_socket(fd_sock))
		{
			while(search_readflag(data,time,&flag,'1'))		//	��ȡһ��resendflagΪ1������
			{
				if(compareTime(thistime ,lasttime,300))
				{
					break;
				}
				if(1 == flag)		// ��������Ҫ�ϴ�������
						data[78] = '1';
				printmsg("CLIENT",data);
				res = send_record(fd_sock, data, time);
				if(-1 == res)
					break;
				thistime = acquire_time();
				memset(data,0,(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL));
				memset(time,0,15);
			}
		}
		close_socket(fd_sock);
		
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
		
		printf("\n");
	}
}

