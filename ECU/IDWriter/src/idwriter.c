#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lwip/sockets.h> 
#include <lwip/netdb.h> 
#include <stdio.h>
#include "debug.h"
#include "idwrite.h"
#include "rtc.h"
#include "inverter_id.h"
#include "threadlist.h"
#include "client.h"
#include "file.h"
#include <dfs_posix.h> 
#include "channel.h"
#include "led.h"
#include "version.h"

#define SERVERPORT 4540
#define BACKLOG 2

int create_socket_idwrite(void)
{
	int sockfd;
	if(-1==(sockfd=socket(AF_INET,SOCK_STREAM,0))){
		printmsg("idwrite","socket error");
		return -1;
	}
	printmsg("idwrite","Create socket successfully!");
	return sockfd;
}

int bind_socket(int sockfd)
{
	struct sockaddr_in server_sockaddr;
	server_sockaddr.sin_family=AF_INET;
	server_sockaddr.sin_port=htons(SERVERPORT);
	server_sockaddr.sin_addr.s_addr=INADDR_ANY;
	memset(&(server_sockaddr.sin_zero),0x00,8);

	if(-1==bind(sockfd,(struct sockaddr *)&server_sockaddr,sizeof(struct sockaddr))){
		printmsg("idwrite","bind error");
		return -1;
	}
	printmsg("idwrite","Bind socket successfully!");
	return 0;
}

int listen_socket(int sockfd)
{
	if(-1==listen(sockfd,BACKLOG)){
		printmsg("idwrite","listen error");
		return -1;
	}
	printmsg("idwrite","Listen socket successfully!");
	return 0;
}

int accept_socket(int sockfd)
{
	int sin_size;
	int clientfd;
	struct sockaddr_in client_sockaddr;

	sin_size=sizeof(struct sockaddr_in);
	if(-1==(clientfd=accept(sockfd,(struct sockaddr *)&client_sockaddr,(socklen_t *)&sin_size))){
		printmsg("idwrite","accept error");
		return -1;
	}

	return clientfd;
}

int recv_cmd(int fd_sock, char *readbuff)
{
	fd_set rd;
	struct timeval timeout;
	int recvbytes, readbytes = 0, res;
	char temp[16];

	while(1)
	{
		FD_ZERO(&rd);
		FD_SET(fd_sock, &rd);
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;

		res = select(fd_sock+1, &rd, NULL, NULL, &timeout);
		if(res <= 0){
			//printmsg("Receive command timeout");
			return -1;
		}
		else{
			memset(readbuff, '\0', sizeof(readbuff));
			memset(temp, '\0', sizeof(temp));
			recvbytes = recv(fd_sock, readbuff, 200, 0);
			if(0 == recvbytes)
				return -1;
		
			readbytes += recvbytes;
			return readbytes;

		}
	}
}


int settime(char *datetime)
{
	return set_time(datetime);
}

int insertinverter(char *buff)
{

	FILE *fp;
	char id[16];
	int i;

	clear_id();
	for(i=0; i<(strlen(buff)+1)/13; i++)
	{

		strncpy(id, &buff[i*13], 12);
		id[12] = '\0';
		addInverter(id);
	}

	fp = fopen("/yuneng/autoflag.con", "w");
	fputs("0", fp);
	fclose(fp);
	fp = fopen("/yuneng/limiteid.con", "w");
	fputs("1", fp);
	fclose(fp);
	restartThread(TYPE_MAIN);

	return i;
}

int getrecord(char *record)
{
	char time[15];
	int flag;
	return search_readflag(record,time, &flag,'1');
}

int getevent(char *eve)
{
	FILE *fp;
	char data[200];
	char splitdata[3][32];
	int num = 0;
	
	memset(eve,'\0',sizeof(eve));
	fp = fopen("/home/record/event", "r");
	if(fp)
	{
		memset(data,0x00,200);
		
		while(NULL != fgets(data,200,fp))
		{
			memset(splitdata,0x00,3*32);
			splitString(data,splitdata);
			strcat(eve, splitdata[0]);	//12
			strcat(eve, splitdata[1]);	//14
			strcat(eve, splitdata[2]);	//24
			num++;
		}
		fclose(fp);
	}

	return num;
}

int clearrecord()
{
	FILE *fp;
	DIR *dirp;
	struct dirent *d;
	char path[100];
	char dir[30] = "/home/record/data";

	fp = fopen("/etc/yuneng/connect_time.conf", "w");
	fclose(fp);
	fp = fopen("/etc/yuneng/autoflag.conf", "w");
	fputs("0", fp);
	fclose(fp);

	clear_id();

	fp = fopen("/home/record/event", "w");
	fclose(fp);

	echo("/home/data/ltpower","0.000000");	

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
			sprintf(path,"%s/%s",dir,d->d_name);
			unlink(path);
		}
		/* �ر�Ŀ¼ */
		closedir(dirp);
	}

	restartThread(TYPE_MAIN);
	return 0;
}

void idwrite_thread_entry(void* parameter)
{
	char recvbuff[200] = {'\0'};
	int sockfd,clientfd;
	FILE *fp;
	char ecuid[13] = {'\0'};
	char mac[32] = {'\0'};
	char version[50] = {'\0'};
	char area[8] = {'\0'};
	char *record;
	char *eve;
	int row;
	char sendbuff[3];
	char gettime[14]={'\0'};

	
	rt_thread_delay(START_TIME_IDWRITE * RT_TICK_PER_SECOND);
	sockfd=create_socket_idwrite();
	bind_socket(sockfd);
	listen_socket(sockfd);

	while(1){
		clientfd = accept_socket(sockfd);
		memset(recvbuff, '\0', sizeof(recvbuff));
		
		recv_cmd(clientfd, recvbuff);
		printf("recvbuff: %s \n",recvbuff);
		//��д�Ͷ�ȡECU��ID
		if(!strncmp(recvbuff, "set_ecu_id", 10)){
			strncpy(ecuid, &recvbuff[11], 12);
			printf("ECU id:%s,  length:%d\n",ecuid,strlen(ecuid));
			fp=fopen("/yuneng/ecuid.con","w");
			fputs(ecuid,fp);
			fclose(fp);
			memset(ecuid,'\0',sizeof(ecuid));
			
			fp=fopen("/yuneng/ecuid.con","r");
			fgets(ecuid,13,fp);
			fclose(fp);
			printf("Send %d\n",send(clientfd,ecuid,strlen(ecuid),0));
		}
		if(!strncmp(recvbuff, "get_ecu_id", 10)){
			memset(ecuid,'\0',sizeof(ecuid));
			fp=fopen("/yuneng/ecuid.con","r");
			fgets(ecuid,13,fp);
			fclose(fp);
			printf("Send %d\n",send(clientfd,ecuid,strlen(ecuid),0));
		}

		//��д�Ͷ�ȡECU���������MAC
		if(!strncmp(recvbuff, "set_eth0_mac", 12)){
			mac[0] = recvbuff[13];
			mac[1] = recvbuff[14];
			mac[2] = ':';
			mac[3] = recvbuff[15];
			mac[4] = recvbuff[16];
			mac[5] = ':';
			mac[6] = recvbuff[17];
			mac[7] = recvbuff[18];
			mac[8] = ':';
			mac[9] = recvbuff[19];
			mac[10] = recvbuff[20];
			mac[11] = ':';
			mac[12] = recvbuff[21];
			mac[13] = recvbuff[22];
			mac[14] = ':';
			mac[15] = recvbuff[23];
			mac[16] = recvbuff[24];
			printf("ECU eth0 MAC address:%s,	length:%d\n",mac,strlen(mac));
			fp=fopen("/yuneng/ecumac.con","w");
			fputs(mac,fp);
			fclose(fp);
			memset(mac,'\0',sizeof(mac));
			
			fp=fopen("/yuneng/ecumac.con","r");
			fgets(mac,18,fp);
			fclose(fp);
			printf("Send %d\n",send(clientfd,mac,strlen(mac),0));
		}
		if(!strncmp(recvbuff, "get_eth0_mac", 12)){
			memset(mac,'\0',sizeof(mac));
			fp=fopen("/yuneng/ecumac.con","r");
			fgets(mac,18,fp);
			fclose(fp);
			printf("Send %d\n",send(clientfd,mac,strlen(mac),0));
		}

		//����ECU�ı���ʱ��
		if(!strncmp(recvbuff, "set_time", 8)){
			settime(&recvbuff[9]);
			send(clientfd, &recvbuff[9], 14, 0);
		}

		//�����������ID
		if(!strncmp(recvbuff, "set_inverter_id", 15)){
			/*
			if(1 == saveECUChannel(18))
				send(clientfd, "0x12", 4, 0);
			else
				send(clientfd, "change channel failed", 21, 0);
			*/
			row = insertinverter(&recvbuff[16]);
			snprintf(sendbuff, sizeof(sendbuff), "%02d", row);
			send(clientfd, sendbuff, 3, 0);
		}

		//��ȡPLC�Ĳ��Խ��
		if(!strncmp(recvbuff, "query_result", 12)){
			record = malloc(MAXINVERTERCOUNT*RECORDLENGTH+RECORDTAIL);
			if(getrecord(record) > 0)
				send(clientfd, record, strlen(record), 0);
			else
				send(clientfd, "Failed", 6, 0);
			free(record);
		}
		
		//��ȡ���������״̬
		if(!strncmp(recvbuff, "query_protection", 16)){
			eve = malloc(MAXINVERTERCOUNT*50+1);
			if(getevent(eve) > 0)
				send(clientfd, eve, strlen(eve), 0);
			else
				send(clientfd, "Failed", 6, 0);
			free(eve);
		}

		//��ղ��Լ�¼
		if(!strncmp(recvbuff, "clear", 5)){

			if(1 == saveECUChannel(16))
				send(clientfd, "0x10", 4, 0);
			else
				send(clientfd, "change channel failed", 21, 0);
			clearrecord();
			//system("rm /home/tmpdb");
			//system("rm /home/historical_data.db");
			//system("rm /home/record.db");
			send(clientfd, "clearok", 7, 0);
		}


		//��д�Ͷ�ȡECU�ĵ���
		if(!strncmp(recvbuff, "set_area", 8)){
			strncpy(area, &recvbuff[9], sizeof(area));
			fp=fopen("/yuneng/area.con", "w");
			fputs(area,fp);
			fclose(fp);

			memset(area,'\0',sizeof(area));
			
			fp=fopen("/yuneng/area.con","r");
			if(fp){
				fgets(area, sizeof(area), fp);
				fclose(fp);
			}
			printf("Send %d\n",send(clientfd,area,strlen(area),0));
		}

		if(!strncmp(recvbuff, "get_area", 8)){
			memset(area,'\0',sizeof(area));
			fp=fopen("/yuneng/area.con","r");
			if(fp){
				fgets(area, sizeof(area), fp);
				fclose(fp);
			}
			printf("Send %d\n",send(clientfd,area,strlen(area),0));
		}

		//��ȡECU�����汾��
		if(!strncmp(recvbuff, "get_version", 11)){
			memset(version, 0, sizeof(version));
			sprintf(version,"%s_%s_%s", ECU_M3_VERSION,MAJORVERSION,MINORVERSION);
			memset(area, 0, sizeof(area));
			fp = fopen("/yuneng/area.con", "r");
			if(fp){
				fgets(area, sizeof(area), fp);
				fclose(fp);
			}
			strcat(version, area);
			printf("Send %d\n",send(clientfd, version, strlen(version), 0));
		}

		//��ȡ����ʱ��
		if(!strncmp(recvbuff, "get_time", 8)){
			apstime(gettime);
			send(clientfd, gettime, 14, 0);
		}



		//����LED��
		if(!strncmp(recvbuff, "test_led", 8)){
			rt_hw_led_init();	//LED��ʼ��
			rt_hw_us_delay(500000);
			rt_hw_led_on();		//LED��
			rt_hw_us_delay(500000);
			rt_hw_led_off();		//LEDϨ��
			rt_hw_us_delay(500000);
			rt_hw_led_on();		//LED��
			rt_hw_us_delay(500000);
			rt_hw_led_off();		//LEDϨ��
			rt_hw_us_delay(500000);
			rt_hw_led_on();		//LED��
			rt_hw_us_delay(500000);
			rt_hw_led_off();		//LEDϨ��
		}

		closesocket(clientfd);
	}
}