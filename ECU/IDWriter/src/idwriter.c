#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lwip/sockets.h> 
#include <lwip/netdb.h> 
#include <stdio.h>
#include "debug.h"



#define SERVERPORT 4540
#define BACKLOG 1

int create_socket(void)
{
	int sockfd;
	if(-1==(sockfd=socket(AF_INET,SOCK_STREAM,0))){
		printmsg("idwrite","socket error");
		return -1;
	}
	printdecmsg("idwrite","Create socket successfully!  socket:%d\n",sockfd);
	return sockfd;
}

int bind_socket(int sockfd)
{
	struct sockaddr_in server_sockaddr;
	server_sockaddr.sin_family=AF_INET;
	server_sockaddr.sin_port=htons(SERVERPORT);
	server_sockaddr.sin_addr.s_addr=INADDR_ANY;
	bzero(&(server_sockaddr.sin_zero),8);

	if(-1==bind(sockfd,(struct sockaddr *)&server_sockaddr,sizeof(struct sockaddr))){
		printmsg("idwrite","bind error");
		return 0;
	}
	printmsg("idwrite","Bind socket successfully!");
}

int listen_socket(int sockfd)
{
	if(-1==listen(sockfd,BACKLOG)){
		printmsg("idwrite","listen error");
		return -1;
	}
	printmsg("idwrite","Listen socket successfully!");
}

int accept_socket(int sockfd)
{
	int sin_size;
	int clientfd;
	struct sockaddr_in client_sockaddr;

	sin_size=sizeof(struct sockaddr_in);
	if(-1==(clientfd=accept(sockfd,(struct sockaddr *)&client_sockaddr,&sin_size))){
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
	char recvbuff[65535], temp[16];

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
			memset(recvbuff, '\0', sizeof(recvbuff));
			memset(temp, '\0', sizeof(temp));
			recvbytes = recv(fd_sock, recvbuff, 65535, 0);
			if(0 == recvbytes)
				return -1;
			strcat(readbuff, recvbuff);
			readbytes += recvbytes;
			return readbytes;
//			if(readbytes >= 10)
//			{
//				for(i=5, j=0; i<10; i++)
//				{
//					if(readbuff[i] <= '9' && readbuff[i] >= '0')
//						temp[j++] = readbuff[i];
//				}
//				if(atoi(temp) == strlen(readbuff))
//				{
//					print2msg("Recv", readbuff);
//					printdecmsg("Receive", readbytes);
//					return readbytes;
//				}
//			}
		}
	}
}

void version_change(char *version)
{
    char sql[1000]={'\0'};
    char *zErrMsg=0;
    sqlite3 *db;

    sqlite3_open("/home/database.db", &db);

	if((version[strlen(version)-2] == 'N') && (version[strlen(version)-1] == 'A'))
		sprintf(sql,"UPDATE preset SET lv1=119, uv1=295, lv2=211, uv2=264, lf=593, uf=605, rt=300 WHERE id=1;");
	else if(!strncmp(&version[strlen(version)-6], "NA-120", 6))
		sprintf(sql,"UPDATE preset SET lv1=79, uv1=181, lv2=95, uv2=155, lf=570, uf=620, rt=300 WHERE id=1;");
	else
		sprintf(sql,"UPDATE preset SET lv1=119, uv1=295, lv2=200, uv2=270, lf=475, uf=505, rt=80 WHERE id=1;");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	sqlite3_close(db);
}

int settime(char *datetime)
{
	time_t tm;
	struct tm set_tm;
	struct timeval tv;
	time_t timep;

	char year[5] = {'\0'};
	char month[5] = {'\0'};
	char day[5] = {'\0'};
	char hour[5] = {'\0'};
	char minute[5] = {'\0'};
	char second[5] = {'\0'};

	strncpy(year, datetime, 4);
	strncpy(month, &datetime[4], 2);
	strncpy(day, &datetime[6], 2);
	strncpy(hour, &datetime[8], 2);
	strncpy(minute, &datetime[10], 2);
	strncpy(second, &datetime[12], 2);

	int fp;
	fp = open("/dev/rtc2",O_WRONLY);
	write(fp, datetime, strlen(datetime));
	close(fp);

	set_tm.tm_year = atoi(year)-1900;
	set_tm.tm_mon = atoi(month)-1;
	set_tm.tm_mday = atoi(day);
	set_tm.tm_hour = atoi(hour);
	set_tm.tm_min = atoi(minute);
	set_tm.tm_sec = atoi(second);

    timep = mktime(&set_tm);
//	if(set_tm.tm_isdst>0)
//		timep -= set_tm.tm_isdst*3600;
    tv.tv_sec = timep;
    tv.tv_usec = 0;

	if(settimeofday(&tv,NULL)<0)
		return -1;
	else
		return 0;
}

int insertinverter(char *buff)
{
	char sql[100] = {'\0'};
	char *zErrMsg = 0;
	sqlite3 *db;
	int nrow = 0, ncolumn = 0;
	char **azResult;
	FILE *fp;
	char id[16];
	int i;

	sqlite3_open("/home/database.db", &db);
	strcpy(sql, "DELETE FROM id WHERE item>=0");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );
	for(i=0; i<(strlen(buff)+1)/13; i++)
	{
		memset(sql, '\0', sizeof(sql));
		memset(id, '\0', sizeof(id));
		strncpy(id, &buff[i*13], 12);
		//sprintf(sql,"INSERT INTO inverter_info (id) VALUES('%s');", id);
		sprintf(sql,"INSERT INTO id (item, ID, flag) VALUES(%d,'%s',%d);", i, id, 0);
		sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );
	}
	sqlite3_close(db);

	fp = fopen("/etc/yuneng/autoflag.conf", "w");
	fputs("0", fp);
	fclose(fp);
	system("killall main.exe");

	sqlite3_open("/home/database.db", &db);
	strcpy(sql, "SELECT ID from id");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	sqlite3_free_table(azResult);
	sqlite3_close(db);

	return nrow;
}

int getrecord(char *record)
{
	char sql[100] = {'\0'};
	char *zErrMsg = 0;
	sqlite3 *db;
	int nrow = 0, ncolumn = 0;
	char **azResult;
	int i;

	sqlite3_open("/home/record.db", &db);
	strcpy(sql, "SELECT record FROM Data ORDER BY date_time DESC LIMIT 1");
	sqlite3_get_table( db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
	if(nrow > 0)
		strcpy(record, azResult[nrow]);
	sqlite3_free_table( azResult );
	sqlite3_close(db);

	return nrow;
}

int serialanswer()
{
	int fd, res;
	char sendbuff[256]={'\0'};
	struct termios newtio;

	fd = open("/dev/ttyO1", O_RDWR | O_NOCTTY);		//打开串口
	if(fd<0){
		perror("MODEMDEVICE");
	}

	bzero(&newtio, sizeof(newtio));					//设置串口参数
	newtio.c_cflag = B9600 | CS8 | CLOCAL | CREAD | CRTSCTS;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 10;
	newtio.c_cc[VMIN] = 51;
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	write(fd, "serialtest", 10);
	close(fd);

	return 0;
}

int clearrecord()
{
	char sql[100] = {'\0'};
	char *zErrMsg = 0;
	sqlite3 *db;
	int nrow = 0, ncolumn = 0;
	char **azResult;
	FILE *fp;

	fp = fopen("/etc/yuneng/connect_time.conf", "w");
	fclose(fp);
	fp = fopen("/etc/yuneng/autoflag.conf", "w");
	fputs("0", fp);
	fclose(fp);

	sqlite3_open("/home/database.db", &db);
	strcpy(sql, "DELETE FROM ID WHERE item >= 0");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	memset(sql, '\0', sizeof(sql));
	strcpy(sql, "DELETE FROM Data WHERE item >= 0");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	memset(sql, '\0', sizeof(sql));
	strcpy(sql, "UPDATE ltpower SET power=0.0 WHERE item >= 0");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	memset(sql, '\0', sizeof(sql));
	strcpy(sql, "DELETE FROM tdpower");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

	sqlite3_close(db);

	sqlite3_open("/home/record.db", &db);
	memset(sql, '\0', sizeof(sql));
	strcpy(sql, "DELETE FROM Data WHERE item >= 0");
	sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );
	sqlite3_close(db);

	system("killall main.exe");
}

int main(void)
{
	char recvbuff[65535] = {'\0'};
	int sockfd,clientfd;
	int recvbytes;
	FILE *fp;
	char *cmd;
	char ecuid[13] = {'\0'};
	char mac[32] = {'\0'};
	char version[16] = {'\0'};
	char area[8] = {'\0'};
	char record[65535];
	int row;
	char sendbuff[3];

	sockfd=create_socket();
	bind_socket(sockfd);
	listen_socket(sockfd);

	while(1){
		clientfd = accept_socket(sockfd);

		memset(recvbuff, '\0', sizeof(recvbuff));

		recv_cmd(clientfd, recvbuff);

		//烧写和读取ECU的ID
		if(!strncmp(recvbuff, "set_ecu_id", 10)){
			strncpy(ecuid, &recvbuff[11], 12);
			printf("ECU id:%s,  length:%d\n",ecuid,strlen(ecuid));
			fp=fopen("/etc/yuneng/ecuid.conf","w");
			fputs(ecuid,fp);
			fclose(fp);
			memset(ecuid,'\0',sizeof(ecuid));
			sleep(1);
			fp=fopen("/etc/yuneng/ecuid.conf","r");
			fgets(ecuid,13,fp);
			fclose(fp);
			printf("Send %d\n",send(clientfd,ecuid,strlen(ecuid),0));
		}
		if(!strncmp(recvbuff, "get_ecu_id", 10)){
			memset(ecuid,'\0',sizeof(ecuid));
			fp=fopen("/etc/yuneng/ecuid.conf","r");
			fgets(ecuid,13,fp);
			fclose(fp);
			printf("Send %d\n",send(clientfd,ecuid,strlen(ecuid),0));
		}

		//烧写和读取ECU有线网络的MAC
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
			fp=fopen("/etc/yuneng/ecu_eth0_mac.conf","w");
			fputs(mac,fp);
			fclose(fp);
			memset(mac,'\0',sizeof(mac));
			sleep(1);
			fp=fopen("/etc/yuneng/ecu_eth0_mac.conf","r");
			fgets(mac,18,fp);
			fclose(fp);
			printf("Send %d\n",send(clientfd,mac,strlen(mac),0));
		}
		if(!strncmp(recvbuff, "get_eth0_mac", 12)){
			memset(mac,'\0',sizeof(mac));
			fp=fopen("/etc/yuneng/ecu_eth0_mac.conf","r");
			fgets(mac,18,fp);
			fclose(fp);
			printf("Send %d\n",send(clientfd,mac,strlen(mac),0));
		}

		//烧写和读取ECU无线网络的MAC
		if(!strncmp(recvbuff, "set_wlan0_mac", 13)){
			mac[0] = recvbuff[14];
			mac[1] = recvbuff[15];
			mac[2] = ':';
			mac[3] = recvbuff[16];
			mac[4] = recvbuff[17];
			mac[5] = ':';
			mac[6] = recvbuff[18];
			mac[7] = recvbuff[19];
			mac[8] = ':';
			mac[9] = recvbuff[20];
			mac[10] = recvbuff[21];
			mac[11] = ':';
			mac[12] = recvbuff[22];
			mac[13] = recvbuff[23];
			mac[14] = ':';
			mac[15] = recvbuff[24];
			mac[16] = recvbuff[25];
			printf("ECU wlan0 MAC address:%s,	length:%d\n",mac,strlen(mac));
			fp=fopen("/etc/yuneng/ecu_wlan0_mac.conf","w");
			fputs(mac,fp);
			fclose(fp);
			memset(mac,'\0',sizeof(mac));
			sleep(1);
			fp=fopen("/etc/yuneng/ecu_wlan0_mac.conf","r");
			fgets(mac,18,fp);
			fclose(fp);
			printf("Send %d\n",send(clientfd,mac,strlen(mac),0));
		}
		if(!strncmp(recvbuff, "get_wlan0_mac", 13)){
			memset(mac,'\0',sizeof(mac));
			fp=fopen("/etc/yuneng/ecu_wlan0_mac.conf","r");
			fgets(mac,18,fp);
			fclose(fp);
			printf("Send %d\n",send(clientfd,mac,strlen(mac),0));
		}

		//设置ECU的本地时间
		if(!strncmp(recvbuff, "set_time", 8)){
			settime(&recvbuff[9]);
			send(clientfd, &recvbuff[9], 14, 0);
		}

		//设置逆变器的ID
		if(!strncmp(recvbuff, "set_inverter_id", 15)){
			row = insertinverter(&recvbuff[16]);
			sendbuff[0] = row / 256;
			sendbuff[1] = row % 256;
			send(clientfd, sendbuff, 2, 0);
		}

		//读取PLC的测试结果
		if(!strncmp(recvbuff, "query_result", 12)){
			if(getrecord(record) > 0)
				send(clientfd, record, strlen(record), 0);
		}

		//清空测试记录
		if(!strncmp(recvbuff, "clear", 5)){
			clearrecord();
			system("rm /home/tmpdb");
			system("rm /home/historical_data.db");
			system("rm /home/record.db");
			send(clientfd, "clearok", 7, 0);
		}

		//串口测试
		if(!strncmp(recvbuff, "serialtest", 10)){
			serialanswer();
		}

		//烧写和读取ECU的地区
		if(!strncmp(recvbuff, "set_area", 8)){
			strncpy(area, &recvbuff[9], sizeof(area));
			fp=fopen("/etc/yuneng/area.conf", "w");
			fputs(area,fp);
			fclose(fp);

			memset(area,'\0',sizeof(area));
			sleep(1);
			fp=fopen("/etc/yuneng/area.conf","r");
			if(fp){
				fgets(area, sizeof(area), fp);
				fclose(fp);
			}
			printf("Send %d\n",send(clientfd,area,strlen(area),0));
		}

		if(!strncmp(recvbuff, "get_area", 8)){
			memset(area,'\0',sizeof(area));
			fp=fopen("/etc/yuneng/area.conf","r");
			if(fp){
				fgets(area, sizeof(area), fp);
				fclose(fp);
			}
			printf("Send %d\n",send(clientfd,area,strlen(area),0));
		}

		//读取ECU软件版本号
		if(!strncmp(recvbuff, "get_version", 11)){
			memset(version, 0, sizeof(version));
			fp = fopen("/etc/yuneng/version.conf", "r");
			if(fp){
				fgets(version, sizeof(version), fp);
				if(10 == version[strlen(version)-1]){
					version[strlen(version)-1] = '\0';
				}
				fclose(fp);
			}
			memset(area, 0, sizeof(area));
			fp = fopen("/etc/yuneng/area.conf", "r");
			if(fp){
				fgets(area, sizeof(area), fp);
				fclose(fp);
			}
			strcat(version, area);
			printf("Send %d\n",send(clientfd, version, strlen(version), 0));
		}

		close(clientfd);
	}
}
