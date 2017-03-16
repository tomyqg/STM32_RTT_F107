#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "variation.h"
#include "zigbee.h"
#define MIN_GROUP_NUM 12

extern ecu_info ecu;
extern struct rt_device serial4;	
/*
void setPanidOfInverters(int num, char **ids);
int getShortaddrOfInverters(int num, char **ids);
int getShortaddrOfEachInverter(char *id);

void saveraduistostruct(char *id,int raduis)
{
	FILE *fp;
	fp = fopen("/tmp/radui.txt", "a");
	if(fp)
	{
		fprintf(fp, "%s %X\n", id,raduis);
		fclose(fp);
	}
}

void savezbversion(char *id,int zbversion)
{
    char *zErrMsg = 0;
	char sql[1024]={'\0'};
	sprintf(sql,"UPDATE id SET turned_off_rpt_flag=%d where id='%s'",zbversion,id);
	sqlite3_exec(db, sql, 0, 0, &zErrMsg);perror(zErrMsg);
}

void clear_bind_flag()
{
    char *zErrMsg = 0;
	char sql[1024]={'\0'};
	sprintf(sql,"UPDATE id SET bind_zigbee_flag=NULL");
	sqlite3_exec(db, sql, 0, 0, &zErrMsg);perror(zErrMsg);

}
void change_bind_zigbee_flag(char *id)
{
	char *zErrMsg = 0;
	char sql[1024]={'\0'};
	sprintf(sql,"UPDATE id SET bind_zigbee_flag=1 WHERE id='%s'",id);
	sqlite3_exec(db, sql, 0, 0, &zErrMsg);perror(zErrMsg);
}

int getaddrOldOrNew(char *id)
{
	int i, ret, timestamp;
	int short_addr = 0;
	char recvMsg[256];
	char inverterid[13];
	char command[21] = {0xAA, 0xAA, 0xAA, 0xAA, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0xAE, 0x06};

	command[15]=((id[0]-0x30)*16+(id[1]-0x30));
	command[16]=((id[2]-0x30)*16+(id[3]-0x30));
	command[17]=((id[4]-0x30)*16+(id[5]-0x30));
	command[18]=((id[6]-0x30)*16+(id[7]-0x30));
	command[19]=((id[8]-0x30)*16+(id[9]-0x30));
	command[20]=((id[10]-0x30)*16+(id[11]-0x30));

	//发送上报单台逆变器ID的命令
	clear_zbmodem();
	write(zbmodem, command, 21);
	print2msg("Get each inverter's short address", id);
	printhexmsg("Sent", command, 21);

	//接收
	ret = zigbeeRecvMsg(recvMsg, 5);
	snprintf(inverterid, sizeof(inverterid), "%02x%02x%02x%02x%02x%02x",
			recvMsg[4], recvMsg[5], recvMsg[6], recvMsg[7], recvMsg[8], recvMsg[9]);
	if ((11 == ret)
		&& (0xFF == recvMsg[2])
		&& (0xFF == recvMsg[3])
		&& (0 == strcmp(id, inverterid))) {
		//获取短地址成功
		short_addr = recvMsg[0]*256 + recvMsg[1];
		update_inverter_addr(id, short_addr);
		return 1;
	}
	else if ((11 == ret)
		&& (0 == strcmp(id, inverterid))) {
		saveraduistostruct(inverterid,recvMsg[2]);	//保存路由深度到结构体
		savezbversion(inverterid,recvMsg[3]);
		//暂存绑定标志
		change_bind_zigbee_flag(inverterid);
		return 1;
	}

	return 0;
}
void send11order(char *inverterid,int count)
{
	int i;
	int check=0;
	char sendbuff[23]={0xAA,0xAA,0xAA,0xAA,0x11,0x00,0x00,0xCC,0xCC,0xcc,0x00,0x00,0x00,0x00,0x08,0xbb,0xbb,0xbb,0xbb,0xbb,0xbb,0xdd,0xdd};
	sendbuff[7]=ecu.panid/256;
	sendbuff[8]=ecu.panid%256;
	sendbuff[9]=ecu.channel;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;
	sendbuff[15]=((inverterid[0]-0x30)*16+(inverterid[1]-0x30));
	sendbuff[16]=((inverterid[2]-0x30)*16+(inverterid[3]-0x30));
	sendbuff[17]=((inverterid[4]-0x30)*16+(inverterid[5]-0x30));
	sendbuff[18]=((inverterid[6]-0x30)*16+(inverterid[7]-0x30));
	sendbuff[19]=((inverterid[8]-0x30)*16+(inverterid[9]-0x30));
	sendbuff[20]=((inverterid[10]-0x30)*16+(inverterid[11]-0x30));
	sendbuff[21]=(2*count)/256;
	sendbuff[22]=(2*count)%256;
	write(zbmodem, sendbuff, 21);
	printhexmsg("Ready Change Inverter Panid (11order)", sendbuff, 23);
	sleep(1);
}
void send22order()
{
	int i;
	int check=0;
	char sendbuff[15]={0xAA,0xAA,0xAA,0xAA,0x22,0x00,0x00,0xCC,0xCC,0xcc,0x00,0x00,0x00,0x00,0x00};
	sendbuff[7]=ecu.panid/256;
	sendbuff[8]=ecu.panid%256;
	sendbuff[9]=ecu.channel;
	for(i=4;i<12;i++)
		check=check+sendbuff[i];
	sendbuff[12] = check/256;
	sendbuff[13] = check%256;

	for(i=0;i<3;i++){
		write(zbmodem, sendbuff, 15);
		printhexmsg("Change Panid Now (22order)", sendbuff, 15);
		sleep(1);
	}
}
void getshortadd(char *recvbuff)
{
	char *zErrMsg = 0;
	char curinverterid[13];
	sprintf(curinverterid,"%02x%02x%02x%02x%02x%02x",recvbuff[4],recvbuff[5],recvbuff[6],recvbuff[7],recvbuff[8],recvbuff[9]);
	char sql[1024];
	sprintf(sql,"UPDATE id SET short_address=%d WHERE id='%s'",(recvbuff[0]*256+recvbuff[1]),curinverterid);
	sqlite3_exec(db, sql, 0, 0, &zErrMsg);perror(zErrMsg);
}
*/
/*
//绑定逆变器
void bind_inverters()
{
   char sql[1024] = {'\0'};
   char **azResult;
   char *zErrMsg = 0;
   char recvbuff[1024];
   int i, nrow, ncolumn;

   //0.设置信道
   process_channel();
   zb_change_ecu_panid(); //将ECU的PANID和信道设置成配置文件中的

	//1.绑定已经有短地址的逆变器,如绑定失败，则需要重新获取短地址
	memset(sql, 0, sizeof(sql));
	snprintf(sql, sizeof(sql), "SELECT short_address FROM id WHERE short_address is not null AND bind_zigbee_flag is null");
	sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);
	for (i=1; i<=nrow; i++) {
		//对每个逆变器进行绑定
		if (!zb_off_report_id_and_bind(atoi(azResult[i]))) {
			//绑定失败,重置短地址
			memset(sql, 0, sizeof(sql));
			snprintf(sql, sizeof(sql), "UPDATE id SET short_address=null WHERE short_address=%d", atoi(azResult[i]));
			sqlite3_exec(db, sql, 0, 0, &zErrMsg);
		}
	}
	sqlite3_free_table(azResult);

	//2.查询表中没有短地址的逆变器ID，让其上报短地址，并存入数据库
//	memset(sql, 0, sizeof(sql));
//	snprintf(sql, sizeof(sql), "SELECT id FROM id WHERE short_address is null");
//	sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);
//	setPanidOfInverters(nrow, azResult); //设置PANID
//	if (nrow > MIN_GROUP_NUM) {
//		//让所有逆变器上报三次ID
//		getShortaddrOfInverters(nrow, azResult);
//	}
//	else {
//		//让单台逆变器上报ID
//		for (i=1; i<=nrow; i++) {
//			getShortaddrOfEachInverter(azResult[i]);
//		}
//	}
//	sqlite3_free_table(azResult);

	//***新组网方案
	int j;unsigned short temppanid=ecu.panid;int k;
	sprintf(sql, "SELECT id FROM id WHERE short_address is null");//短地址为空的先找出来
	sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);
	if(nrow>0)
	{printf("aa\n");
		//清空短地址
		//memset(sql,"\0",1024);
		//sprintf(sql,"UPDATE id SET short_address=null,bind_zigbee_flag=null,turned_off_rpt_flag=null");
		//sqlite3_exec(db, sql, 0, 0, &zErrMsg);perror(zErrMsg);

		zb_restore_ecu_panid_0xffff(ecu.channel);
		for(i=0;i<5;i++)
		{printf("bb\n");
			memset(sql, 0, sizeof(sql));
			sprintf(sql, "SELECT id FROM id WHERE short_address is null AND bind_zigbee_flag  is null");
			sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);printf("nrow=%d\n",nrow);
			ecu.panid=0xFFFF;
			if(nrow==0)
				break;
			for(j=0;j<nrow;j++)
			{printf("cc\n");
				zb_change_inverter_channel_one(azResult[j+1],ecu.channel);//所有逆变器设置成0xFFFF
				sleep(3);//zigbeeRecvMsg(recvbuff,5);
			}
			for(j=0;j<nrow;j++)
			{
				for(k=0;k<3;k++){
					if(1==getaddrOldOrNew(azResult[j+1]))
						break;
					sleep(2);
				}
				printf("dd\n");
//					if(-1!=zigbeeRecvMsg(recvbuff,5))
//						getshortadd(recvbuff);
			}
			sqlite3_free_table(azResult);
		}
		for(i=0;i<3;i++)			//新组网
		{
			memset(sql, 0, sizeof(sql));
			sprintf(sql, "SELECT id FROM id WHERE short_address is null");
			sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);printf("nrow=%d\n",nrow);
			ecu.panid=temppanid;
			for(j=0;j<nrow;j++)
			{
				send11order(azResult[j+1],nrow);
				if(-1!=zigbeeRecvMsg(recvbuff,5))
					getshortadd(recvbuff);printf("ee\n");
			}
			sqlite3_free_table(azResult);
		}
					//旧组网

		memset(sql,'\0',1024);
		sprintf(sql, "SELECT id FROM id WHERE bind_zigbee_flag  is null");
		sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);printf("nrow=%d\n",nrow);
		ecu.panid=temppanid;
		for(j=0;j<nrow;j++)
		{
			for(i=0;i<3;i++)
				zb_change_inverter_channel_one(azResult[j+1],ecu.channel);
		}
		sqlite3_free_table(azResult);

		clear_bind_flag();		//清空用于分辨新旧zigbee的绑定标志位
		ecu.panid=0xFFFF;
		send22order();
		sleep(10);
		ecu.panid=temppanid;
		zb_change_ecu_panid();
	}
	sleep(10);
	//3.绑定获取到短地址的逆变器
	memset(sql, 0, sizeof(sql));
	snprintf(sql, sizeof(sql), "SELECT short_address FROM id WHERE short_address is not null AND bind_zigbee_flag is null");
	sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);
	for (i=1; i<=nrow; i++) {
		//对每个逆变器进行绑定
	//	zb_off_report_id_and_bind(atoi(azResult[i]));

	}
	sqlite3_free_table(azResult);
}
/*
void setPanidOfInverters(int num, char **ids)
{
	int i;

	if (num > 0) {
		//设置PANID（前提是在同一信道下）
		zb_restore_ecu_panid_0xffff(ecu.channel); //PANID为0xFFFF（万能发送）,信道为配置文件中的信道
		for (i=1; i<=num; i++) {
			if (NULL != ids[i] && strlen(ids[i])) {
				zb_change_inverter_channel_one(ids[i], ecu.channel);
			}
		}
		zb_change_ecu_panid(); //设置ECU：PANID为ECU的MAC地址后四位，信道为配置文件中的信道
	}
}

int getShortaddrOfInverters(int num, char **ids)
{
	int i, timestamp;
	int count = 0;
	int short_addr = 0;
	char recvMsg[256];
	char tmp_inverterid[256];
	char command[15] = {0xAA, 0xAA, 0xAA, 0xAA, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0xA1, 0x00};

	//发送广播上报逆变器ID的命令
	clear_zbmodem();
	write(zbmodem, command, 15);
	printmsg("Turn on report id limited");
	printhexmsg("Sent", command, 15);

	//限定上报逆变器ID的时间60秒,并将上报上来的短地址存入数据库
	timestamp = time(NULL);
	while ((time(NULL) - timestamp) <= 60) {
		//短地址获取完毕，退出循环
		if (count >= num) break;

		//接收上报上来的逆变器ID
		memset(recvMsg, '\0', sizeof(recvMsg));
		memset(tmp_inverterid, '\0', sizeof(tmp_inverterid));
		if ((11 == zb_get_id(recvMsg)) && (0xFF == recvMsg[2]) && (0xFF == recvMsg[3])) {
			//解析出逆变器ID
			for (i=0; i<6; i++) {
				tmp_inverterid[2*i] = (recvMsg[i+4]>>4) + 0x30;
				tmp_inverterid[2*i+1] = (recvMsg[i+4]&0x0F) + 0x30;
			}
			print2msg("tmp_inverterid", tmp_inverterid);

			//将收到的逆变器ID与数据库中的逆变器ID进行匹配
			for (i=1; i<=num; i++) {
				if (!strcmp(ids[i], tmp_inverterid)) {
					//解析出短地址并存入数据库
					short_addr = recvMsg[0]*256 + recvMsg[1];
					if (1 == update_inverter_addr(tmp_inverterid, short_addr)) {
						//清空存入数据库的逆变器ID，避免重复
						memset(ids[i], '\0', strlen(ids[i]));
						count++;
						printdecmsg("real_count", count);
						printdecmsg("nrow", num);
					}
					break;
				}
			}
		}
	}
	return (num-count);
}

int getShortaddrOfEachInverter(char *id)
{
	int i, ret, timestamp;
	int short_addr = 0;
	char recvMsg[256];
	char inverterid[13];
	char command[21] = {0xAA, 0xAA, 0xAA, 0xAA, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x06};

	command[15]=((id[0]-0x30)*16+(id[1]-0x30));
	command[16]=((id[2]-0x30)*16+(id[3]-0x30));
	command[17]=((id[4]-0x30)*16+(id[5]-0x30));
	command[18]=((id[6]-0x30)*16+(id[7]-0x30));
	command[19]=((id[8]-0x30)*16+(id[9]-0x30));
	command[20]=((id[10]-0x30)*16+(id[11]-0x30));

	//发送上报单台逆变器ID的命令
	clear_zbmodem();
	write(zbmodem, command, 21);
	print2msg("Get each inverter's short address", id);
	printhexmsg("Sent", command, 21);

	//接收
	ret = zigbeeRecvMsg(recvMsg, 5);
	snprintf(inverterid, sizeof(inverterid), "%02x%02x%02x%02x%02x%02x",
			recvMsg[4], recvMsg[5], recvMsg[6], recvMsg[7], recvMsg[8], recvMsg[9]);
	if ((11 == ret)
		&& (0xFF == recvMsg[2])
		&& (0xFF == recvMsg[3])
		&& (0 == strcmp(id, inverterid))) {
		//获取短地址成功
		short_addr = recvMsg[0]*256 + recvMsg[1];
		update_inverter_addr(id, short_addr);
		return 1;
	}
	return 0;
}
*/
