/*****************************************************************************/
/*  File      : file.c                                                       */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-03-05 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include "file.h"
#include "checkdata.h"
#include <dfs_posix.h> 
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "datetime.h"
#include "SEGGER_RTT.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern rt_mutex_t record_data_lock;
extern inverter_info inverter[MAXINVERTERCOUNT];

int ecu_type;	//1:SAA; 2:NA; 3:MX

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
int fileopen(const char *file, int flags, int mode)
{
	return open(file,flags,mode);
}

int fileclose(int fd)
{
	return close(fd);
}

int fileWrite(int fd,char* buf,int len)
{
	return write( fd, buf, len );
}

int fileRead(int fd,char* buf,int len)
{
	return read( fd, buf, len );
}


int get_ecu_type()			
{
	int fd;
	char version[13] = {'\0'};
	fd = open("/YUNENG/AREA.CON", O_RDONLY, 0);
	if (fd >= 0)
	{
		read(fd, version, 10);
		if('\n' == version[strlen(version)-1])
			version[strlen(version)-1] = '\0';
		if(!strncmp(&version[strlen(version)-2], "MX", 2))
			ecu_type = 3;
		else if(!strncmp(&version[strlen(version)-2], "NA", 2))
			ecu_type = 2;
		else
			ecu_type = 1;
		//printdecmsg(ECU_DBG_MAIN,"ecu_type",ecu_type);
		//print2msg(ECU_DBG_MAIN,"version",version);
		close(fd);
	}
	
	return 0;
}

void get_ecuid(char *ecuid)
{
	int fd;
	fd = open("/YUNENG/ECUID.CON", O_RDONLY, 0);
	if (fd >= 0)
	{
		read(fd, ecuid, 13);
		if('\n' == ecuid[strlen(ecuid)-1])
			ecuid[strlen(ecuid)-1] = '\0';
		
		//print2msg(ECU_DBG_OTHER,"get_ecuid",ecuid);
		close(fd);
	}

}

unsigned short get_panid(void)
{
	int fd;
	unsigned short ret = 0;
	char buff[17] = {'\0'};
	fd = open("/YUNENG/ECUMAC.CON", O_RDONLY, 0);
	if (fd >= 0)
	{
		memset(buff, '\0', sizeof(buff));
		read(fd, buff, 17);
		close(fd);
		if((buff[12]>='0') && (buff[12]<='9'))
			buff[12] -= 0x30;
		if((buff[12]>='A') && (buff[12]<='F'))
			buff[12] -= 0x37;
		if((buff[13]>='0') && (buff[13]<='9'))
			buff[13] -= 0x30;
		if((buff[13]>='A') && (buff[13]<='F'))
			buff[13] -= 0x37;
		if((buff[15]>='0') && (buff[15]<='9'))
			buff[15] -= 0x30;
		if((buff[15]>='A') && (buff[15]<='F'))
			buff[15] -= 0x37;
		if((buff[16]>='0') && (buff[16]<='9'))
			buff[16] -= 0x30;
		if((buff[16]>='A') && (buff[16]<='F'))
			buff[16] -= 0x37;
		ret = ((buff[12]) * 16 + (buff[13])) * 256 + (buff[15]) * 16 + (buff[16]);
	}
	return ret;
}
char get_channel(void)
{
	int fd;
	char ret = 0;
	char buff[5] = {'\0'};
	fd = open("/YUNENG/CHANNEL.CON", O_RDONLY, 0);
	if (fd >= 0)
	{
		memset(buff, '\0', sizeof(buff));
		read(fd, buff, 5);
		close(fd);
		if((buff[2]>='0') && (buff[2]<='9'))
			buff[2] -= 0x30;
		if((buff[2]>='A') && (buff[2]<='F'))
			buff[2] -= 0x37;
		if((buff[2]>='a') && (buff[2]<='f'))
			buff[2] -= 0x57;
		if((buff[3]>='0') && (buff[3]<='9'))
			buff[3] -= 0x30;
		if((buff[3]>='A') && (buff[3]<='F'))
			buff[3] -= 0x37;
		if((buff[3]>='a') && (buff[3]<='f'))
			buff[3] -= 0x57;
		ret = (buff[2]*16+buff[3]);
	} else {
		fd = open("/YUNENG/CHANNEL.CON", O_WRONLY | O_CREAT | O_TRUNC, 0);
		if (fd >= 0) {
			write(fd, "0x10", 5);
			close(fd);
			ret = 0x10;
		}
	}
	return ret;
}

float get_lifetime_power(void)
{
	int fd;
	float lifetime_power = 0.0;
	char buff[20] = {'\0'};
	fd = open("/HOME/DATA/LTPOWER", O_RDONLY, 0);
	if (fd >= 0)
	{
		memset(buff, '\0', sizeof(buff));
		read(fd, buff, 20);
		close(fd);
		
		if('\n' == buff[strlen(buff)-1])
			buff[strlen(buff)-1] = '\0';
		lifetime_power = (float)atof(buff);
	}		
	return lifetime_power;
}

void update_life_energy(float lifetime_power)
{
	int fd;
	char buff[20] = {'\0'};
	fd = open("/HOME/DATA/LTPOWER", O_WRONLY | O_CREAT | O_TRUNC, 0);
	if (fd >= 0) {
		sprintf(buff, "%f", lifetime_power);
		write(fd, buff, 20);
		close(fd);
	}
}
void updateID(void)
{
	FILE *fp;
	int i;
	inverter_info *curinverter = inverter;
	//更新/home/data/id数据
	fp = fopen("/home/data/id","w");
	if(fp)
	{
		curinverter = inverter;
		for(i=0; (i<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); i++, curinverter++)			//有效逆变器轮训
		{
			fprintf(fp,"%s,%d,%d,%d,%d,%d,%d\n",curinverter->id,curinverter->shortaddr,curinverter->model,curinverter->version,curinverter->bindflag,curinverter->zigbee_version,curinverter->flag);
			
		}
		fclose(fp);
	}
}


int splitString(char *data,char splitdata[][32])
{
	int i,j = 0,k=0;

	for(i=0;i<strlen(data);++i){
		
		if(data[i] == ',') {
			splitdata[j][k] = 0;
			++j;
			k = 0; 
		}
		else{
			splitdata[j][k] = data[i];
			++k;
		}
	}
	return j+1;

}

int get_id_from_file(inverter_info *firstinverter)
{

	int i,j,sameflag;
	inverter_info *inverter = firstinverter;
	inverter_info *curinverter = firstinverter;
	char list[20][32];
	char data[200];
	int num =0;
	FILE *fp;
	fp = fopen("/home/data/id", "r");
	if(fp)
	{
		while(NULL != fgets(data,200,fp))
		{
			print2msg(ECU_DBG_MAIN,"ID",data);
			memset(list,0,sizeof(list));
			splitString(data,list);
			//判断是否存在该逆变器
			curinverter = firstinverter;
			sameflag=0;
			for(j=0; (j<MAXINVERTERCOUNT)&&(12==strlen(curinverter->id)); j++)	
			{
				if(!memcmp(list[0],curinverter->id,12))
					sameflag = 1;
				curinverter++;
			}
			if(sameflag == 1)
			{
				continue;
			}
			
			strcpy(inverter->id, list[0]);
			
			if(0==strlen(list[1]))
			{
				inverter->shortaddr = 0;		//未获取到短地址的逆变器赋值为0.ZK
			}
			else
			{
				inverter->shortaddr = atoi(list[1]);
			}
			if(0==strlen(list[2]))
			{
				inverter->model = 0;		//未获得机型码的逆变器赋值为0.ZK
			}
			else
			{
				inverter->model = atoi(list[2]);
			}
			
			if(0==strlen(list[3]))
			{
				inverter->version = 0;		//软件版本，没有标志为0
			}
			else
			{
				inverter->version = atoi(list[3]);
			}
			
			if(0==strlen(list[4]))
			{
				inverter->bindflag = 0;		//未绑定的逆变器把标志位赋值为0.ZK
			}
			else
			{
				inverter->bindflag = atoi(list[4]);
			}

			if(0==strlen(list[5]))
			{
				inverter->zigbee_version = 0;		//没有获取到zigbee版本号的逆变器赋值为0.ZK
			}
			else
			{
				inverter->zigbee_version = atoi(list[5]);
			}
			if(0==strlen(list[6]))
			{
				inverter->flag = 0;		
			}
			else
			{
				inverter->flag = atoi(list[6]);
			}
			
			inverter++;
			num++;
		}
		fclose(fp);
	}


	inverter = firstinverter;
	printmsg(ECU_DBG_MAIN,"--------------");
	for(i=1; i<=num; i++, inverter++)
		printdecmsg(ECU_DBG_MAIN,inverter->id, inverter->shortaddr);
	printmsg(ECU_DBG_MAIN,"--------------");
	printdecmsg(ECU_DBG_MAIN,"total", num);


	inverter = firstinverter;
	printmsg(ECU_DBG_MAIN,"--------------");
	for(i=1; i<=num; i++,inverter++)
	{
		if(inverter->shortaddr == 0)
		{
			printmsg(ECU_DBG_MAIN,inverter->id);
		}
	}
	return num;
}

int save_process_result(int item, char *result)
{
	char dir[50] = "/home/data/proc_res/";
	char file[9];
	int fd;
	char time[20];
	getcurrenttime(time);
	memcpy(file,&time[0],8);
	file[8] = '\0';
	sprintf(dir,"%s%s.dat",dir,file);
	print2msg(ECU_DBG_MAIN,"save_process_result DIR",dir);
	fd = open(dir, O_WRONLY | O_APPEND | O_CREAT,0);
	if (fd >= 0)
	{		
		sprintf(result,"%s,%3d,1\n",result,item);
		print2msg(ECU_DBG_MAIN,"result",result);
		write(fd,result,strlen(result));
		close(fd);
	}
	return 0;

}

int save_inverter_parameters_result(inverter_info *inverter, int item, char *inverter_result)
{
	char dir[50] = "/home/data/iprocres/";
	char file[9];
	int fd;
	char time[20];
	getcurrenttime(time);
	memcpy(file,&time[0],8);
	file[8] = '\0';
	sprintf(dir,"%s%s.dat",dir,file);
	print2msg(ECU_DBG_MAIN,"save_inverter_parameters_result dir",dir);
	fd = open(dir, O_WRONLY | O_APPEND | O_CREAT,0);
	if (fd >= 0)
	{		
		sprintf(inverter_result,"%s,%s,%3d,1\n",inverter_result,inverter->id,item);
		print2msg(ECU_DBG_MAIN,"inverter_result",inverter_result);
		write(fd,inverter_result,strlen(inverter_result));
		close(fd);
	}
	return 0;

}

int save_inverter_parameters_result2(char *id, int item, char *inverter_result)
{
	char dir[50] = "/home/data/iprocres/";
	char file[9];
	int fd;
	char time[20];
	getcurrenttime(time);
	memcpy(file,&time[0],8);
	file[8] = '\0';
	sprintf(dir,"%s%s.dat",dir,file);
	print2msg(ECU_DBG_MAIN,"save_inverter_parameters_result2 DIR",dir);
	fd = open(dir, O_WRONLY | O_APPEND | O_CREAT,0);
	if (fd >= 0)
	{		
		sprintf(inverter_result,"%s,%s %3d,1\n",inverter_result,id,item);
		print2msg(ECU_DBG_MAIN,"inverter_result",inverter_result);
		write(fd,inverter_result,strlen(inverter_result));
		close(fd);
	}
	return 0;

}

void save_record(char sendbuff[], char *date_time)
{
	char dir[50] = "/home/record/data/";
	char file[9];
	rt_err_t result;
	int fd;
	memcpy(file,&date_time[0],8);
	file[8] = '\0';
	sprintf(dir,"%s%s.dat",dir,file);
	print2msg(ECU_DBG_MAIN,"save_record DIR",dir);
	result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		fd = open(dir, O_WRONLY | O_APPEND | O_CREAT,0);
		if (fd >= 0)
		{		
			sprintf(sendbuff,"%s,%s,1\n",sendbuff,date_time);
			//print2msg(ECU_DBG_MAIN,"save_record",sendbuff);
			write(fd,sendbuff,strlen(sendbuff));
			close(fd);
		}
	}
	rt_mutex_release(record_data_lock);
	
}

int save_status(char *result, char *date_time)
{
	char dir[50] = "/home/record/inversta/";
	char file[9];
	int fd;
	memcpy(file,&date_time[0],8);
	file[8] = '\0';
	sprintf(dir,"%s%s.dat",dir,file);
	print2msg(ECU_DBG_MAIN,"save_status DIR",dir);
	fd = open(dir, O_WRONLY | O_APPEND | O_CREAT,0);
	if (fd >= 0)
	{		
		sprintf(result,"%s,%s,1\n",result,date_time);
		print2msg(ECU_DBG_MAIN,"result",result);
		write(fd,result,strlen(result));
		close(fd);
	}
	return 0;
}


void echo(const char* filename,const char* string)
{
	int fd;
	int length;
	if((filename == NULL) ||(string == NULL))
	{
		printmsg(ECU_DBG_OTHER,"para error");
		return ;
	}

	fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0);
	if (fd < 0)
	{
		printmsg(ECU_DBG_OTHER,"open file for write failed");
		return;
	}
	length = write(fd, string, strlen(string));
	if (length != strlen(string))
	{
		printmsg(ECU_DBG_OTHER,"check: read file failed");
		close(fd);
		return;
	}
	close(fd);
}

//将两个字节的字符串转换为16进制数
int strtohex(char str[2])
{
	int ret=0;
	
	((str[0]-'A') >= 0)? (ret =ret+(str[0]-'A'+10)*16 ):(ret = ret+(str[0]-'0')*16);
	((str[1]-'A') >= 0)? (ret =ret+(str[1]-'A'+10) ):(ret = ret+(str[1]-'0'));
	return ret;
}

void get_mac(rt_uint8_t  dev_addr[6])
{
	FILE *fp;
	char macstr[18] = {'\0'};
	fp = fopen("/yuneng/ecumac.con","r");
	if(fp)
	{
		//读取mac地址
		if(NULL != fgets(macstr,18,fp))
		{
			dev_addr[0]=strtohex(&macstr[0]);
			dev_addr[1]=strtohex(&macstr[3]);
			dev_addr[2]=strtohex(&macstr[6]);
			dev_addr[3]=strtohex(&macstr[9]);
			dev_addr[4]=strtohex(&macstr[12]);
			dev_addr[5]=strtohex(&macstr[15]);
			SEGGER_RTT_printf(0,"ECU MAC: %x %x %x %x %x %x\n",dev_addr[0],dev_addr[1],dev_addr[2],dev_addr[3],dev_addr[4],dev_addr[5]);
			fclose(fp);
			return;
		}
		fclose(fp);		
	}
	dev_addr[0]=0x00;;
	dev_addr[1]=0x80;;
	dev_addr[2]=0xE1;
	dev_addr[3]=*(rt_uint8_t*)(0x1FFFF7E8+7);
	dev_addr[4]=*(rt_uint8_t*)(0x1FFFF7E8+8);
	dev_addr[5]=*(rt_uint8_t*)(0x1FFFF7E8+9);
	return;
}

void addInverter(char *inverter_id)
{
	int fd;
	char buff[50];
	fd = open("/home/data/id", O_WRONLY | O_APPEND | O_CREAT,0);
	if (fd >= 0)
	{		
		sprintf(buff,"%s,,,,,,\n",inverter_id);
		write(fd,buff,strlen(buff));
		close(fd);
	}
	echo("/yuneng/limiteid.con","1");
}

void initPath(void)
{
	mkdir("/home",0x777);
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	mkdir("/tmp",0x777);
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	mkdir("/yuneng",0x777);
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	mkdir("/home/data",0x777);
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	mkdir("/home/record",0x777);
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	mkdir("/home/data/proc_res",0x777);
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	mkdir("/home/data/iprocres",0x777);
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	echo("/home/data/ltpower","0.000000");
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	mkdir("/home/record/data",0x777);
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	mkdir("/home/record/inversta",0x777);
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	echo("/yuneng/area.con","SAA");
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	echo("/yuneng/channel.con","0x10");
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	echo("/yuneng/limiteid.con","1");
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	echo("/yuneng/control.con","Timeout=10\nReport_Interval=15\nDomain=eee.apsema.com\nIP=60.190.131.190\nPort1=8997\nPort2=8997\n");
	//echo("/yuneng/control.con","Timeout=10\nReport_Interval=1\nDomain=eee.apsema.com\nIP=192.168.1.100\nPort1=8997\nPort2=8997\n");
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	echo("/yuneng/vernum.con","2\n");
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	echo("/yuneng/ftpadd.con", "IP=60.190.131.190\nPort=9219\nuser=zhyf\npassword=yuneng\n");
	//echo("/yuneng/ftpadd.con", "IP=192.168.1.103\nPort=21\nuser=admin\npassword=admin\n");
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	echo("/yuneng/datacent.con","Domain=111.apsema.com\nIP=139.168.200.158\nPort1=8093\nPort2=8093\n");
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	echo("/home/data/power","");
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	echo("/yuneng/timezone.con","Etc/GMT+8\n");
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	mkdir("/ftp",0x777);
}

int getTimeZone()
{
	int timeZone = 8;
	char s[10] = { '\0' };
	FILE *fp;
	
	fp = fopen("/yuneng/timezone.con", "r");
	if(fp == NULL){
		print2msg(ECU_DBG_NTP,"/yuneng/timezone.con","open error!");
		return timeZone;
	}
	fgets(s, 10, fp);
	//print2msg(ECU_DBG_NTP,"getTimeZone",s);
	//s[strlen(s) - 1] = '\0';
	fclose(fp);
	
	timeZone = atoi(&s[7]);
	if(timeZone >= 12 || timeZone <= -12)
	{
		return 8;
	}
	return timeZone;
	
}

#ifdef RT_USING_FINSH
#include <finsh.h>

FINSH_FUNCTION_EXPORT(echo, eg:echo("/test","test"));

void splitSt(char * str)
{
	int i = 0 , num;
	char list[20][32];
	num = splitString(str,list);
	for(i = 0;i<num;i++)
	{
		printmsg(ECU_DBG_OTHER,list[i]);
		if(strlen(list[i]) == 0)
		{
			printmsg(ECU_DBG_OTHER,"NULL");
		}
	}
	printdecmsg(ECU_DBG_OTHER,"num",num);
   
}
FINSH_FUNCTION_EXPORT(splitSt, eg:splitSt());

void testfile()
{
	get_id_from_file(inverter);
}
FINSH_FUNCTION_EXPORT(testfile, eg:testfile());


void rm_dir(char* dir)
{
	DIR *dirp;
	struct dirent *d;
	/* 打开dir目录*/
	dirp = opendir(dir);
	
	if(dirp == RT_NULL)
	{
		printmsg(ECU_DBG_OTHER,"open directory error!");
	}
	else
	{
		/* 读取目录*/
		while ((d = readdir(dirp)) != RT_NULL)
		{
			char buff[100];
			print2msg(ECU_DBG_OTHER,"delete", d->d_name);
			sprintf(buff,"%s/%s",dir,d->d_name);
			unlink(buff);
		}
		/* 关闭目录 */
		closedir(dirp);
	}
}
FINSH_FUNCTION_EXPORT(rm_dir, eg:rm_dir("/home/record/data"));

int initsystem(char *ecuid,char *mac)
{
	char fileecuid[13];	
	mkdir("/home",0x777);
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	mkdir("/tmp",0x777);
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	mkdir("/yuneng",0x777);
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	mkdir("/home/data",0x777);
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	mkdir("/home/record",0x777);
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	mkdir("/home/data/proc_res",0x777);
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	mkdir("/home/data/iprocres",0x777);
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	echo("/home/data/ltpower","0.000000");
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	mkdir("/home/record/data",0x777);
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	mkdir("/home/record/inversta",0x777);
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	memcpy(fileecuid,ecuid,12);
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	fileecuid[12] = '\n';
	echo("/yuneng/ecuid.con",fileecuid);
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	echo("/yuneng/area.con","SAA");
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	echo("/yuneng/ecumac.con",mac);
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	echo("/yuneng/channel.con","0x10");
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	echo("/yuneng/limiteid.con","1");
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	//echo("/yuneng/control.con","Timeout=10\nReport_Interval=15\nDomain=eee.apsema.com\nIP=60.190.131.190\nPort1=8997\nPort2=8997\n");
	echo("/yuneng/control.con","Timeout=10\nReport_Interval=1\nDomain=111.apsema.com\nIP=139.168.200.158\nPort1=8997\nPort2=8997\n");
	//echo("/yuneng/control.con","Timeout=10\nReport_Interval=1\nDomain=eee.apsema.com\nIP=192.168.1.100\nPort1=8997\nPort2=8997\n");
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	echo("/yuneng/vernum.con","2\n");
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	echo("/yuneng/ftpadd.con", "IP=60.190.131.190\nPort=9219\nuser=zhyf\npassword=yuneng\n");
	//echo("/yuneng/ftpadd.con", "IP=192.168.1.103\nPort=21\nuser=admin\npassword=admin\n");
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	echo("/yuneng/datacent.con","Domain=eee.apsema.com\nIP=139.168.200.158\nPort1=8093\nPort2=8093\n");
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	echo("/home/data/power","");
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	echo("/yuneng/timezone.con","Etc/GMT+8\n");
	rt_thread_delay(RT_TICK_PER_SECOND/50);
	mkdir("/ftp",0x777);
	
	return 0;
}
FINSH_FUNCTION_EXPORT(initsystem, eg:initsystem("123456789012","80:97:1B:00:72:1C"));

void changecontrol(char * IP,char *Domain,int nReport_Interval,int port1,int port2)
{
	char str[300]={'\0'};
	
	sprintf(str,"Timeout=10\nReport_Interval=%d\nDomain=%s\nIP=%s\nPort1=%d\nPort2=%d\n",nReport_Interval,Domain,IP,port1,port2);
	
	echo("/yuneng/control.con",str);
}	
FINSH_FUNCTION_EXPORT(changecontrol, eg:changecontrol("60.190.131.190","eee.apsema.com",15,8997,8997));

void changFTPadd(char * IP,int Port,char *user,char *password)
{
	char str[300]={'\0'};
	
	sprintf(str,"IP=%s\nPort=%d\nuser=%s\npassword=%d\n",IP,Port,user,password);
	
	echo("/yuneng/FTPadd.con",str);
}	
FINSH_FUNCTION_EXPORT(changFTPadd, eg:changFTPadd("60.190.131.190",9219,"zhyf",yuneng));

void changdatacent(char * IP,char *Domain,int port1,int port2)
{
	char str[300]={'\0'};
	
	sprintf(str,"Domain=%s\nIP=%s\nPort1=%d\nPort2=%d\n",Domain,IP,port1,port2);
	
	echo("/yuneng/datacent.con",str);
}	
FINSH_FUNCTION_EXPORT(changdatacent, eg:changdatacent("139.168.200.158","111.apsema.com",8093,8093));




FINSH_FUNCTION_EXPORT(addInverter, eg:addInverter("201703150001"));

#endif
