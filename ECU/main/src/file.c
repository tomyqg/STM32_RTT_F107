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
#include "dfs_fs.h"
#include "rthw.h"
#include "myfile.h"
#include "variation.h"


/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern rt_mutex_t record_data_lock;
extern inverter_info inverter[MAXINVERTERCOUNT];
extern ecu_info ecu;

int ecu_type;	//1:SAA; 2:NA; 3:MX

#define EPSILON 0.000000001

//day_tab[0]   平年每月日期数    day_tab[1]   瑞年每月日期数   
int day_tab[2][12]={{31,28,31,30,31,30,31,31,30,31,30,31},{31,29,31,30,31,30,31,31,30,31,30,31}}; 

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

int get_Passwd(char *PassWD)
{
	int fd;
	fd = open("/YUNENG/PASSWD.CON", O_RDONLY, 0);
	if (fd >= 0)
	{
		read(fd, PassWD, 100);
		close(fd);
		return 0;
	}else
	{
		return -1;
	}

}

int set_Passwd(char *PassWD,int length)
{
	int fd;
	fd = open("/YUNENG/PASSWD.CON", O_WRONLY, 0);
	if (fd >= 0)
	{
		write(fd, PassWD, length);
		close(fd);
		return 0;
	}else
	{
		return -1;
	}


}

//返回0表示DHCP模式  返回1表示静态IP模式
int get_DHCP_Status(void)
{
	int fd;
	fd = open("/yuneng/staticIP.con", O_RDONLY, 0);
	if (fd >= 0)
	{
		close(fd);
		return 1;
	}else
	{
		return 0;
	}

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
//读取DRM开关函数   返回值为1：表示功能打开   返回值为-1表示功能关闭
int DRMFunction(void)
{
	int fd;
	char buff[2] = {'\0'};
	
	fd = open("/YUNENG/DRM.CON", O_RDONLY, 0);
	if(fd >= 0)
	{
		memset(buff, '\0', sizeof(buff));
		read(fd, buff, 1);
		close(fd);
		if(buff[0] == '1')
		{
			return 1;
		}
	}
	
	return -1;
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

void save_system_power(int system_power, char *date_time)
{
	char dir[50] = "/home/record/power/";
	char file[9];
	char sendbuff[50] = {'\0'};
	char date_time_tmp[14] = {'\0'};
	rt_err_t result;
	int fd;
	if(system_power == 0) return;
	
	memcpy(date_time_tmp,date_time,14);
	memcpy(file,&date_time[0],6);
	file[6] = '\0';
	sprintf(dir,"%s%s.dat",dir,file);
	date_time_tmp[12] = '\0';
	print2msg(ECU_DBG_MAIN,"save_system_power DIR",dir);
	result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		fd = open(dir, O_WRONLY | O_APPEND | O_CREAT,0);
		if (fd >= 0)
		{		
			sprintf(sendbuff,"%s,%d\n",date_time_tmp,system_power);
			//print2msg(ECU_DBG_MAIN,"save_system_power",sendbuff);
			write(fd,sendbuff,strlen(sendbuff));
			close(fd);
		}
	}
	rt_mutex_release(record_data_lock);
	
}

//计算两个月前的月份
int calculate_earliest_2_month_ago(char *date,int *earliest_data)
{
	char year_s[5] = {'\0'};
	char month_s[3] = {'\0'};
	int year = 0,month = 0;	//year为年份 month为月份 day为日期  flag 为瑞年判断标志 count表示上个月好需要补的天数 number_of_days:上个月的总天数
	
	memcpy(year_s,date,4);
	year_s[4] = '\0';
	year = atoi(year_s);

	memcpy(month_s,&date[4],2);
	month_s[2] = '\0';
	month = atoi(month_s);
	
	if(month >= 3)
	{
		month -= 2;
		*earliest_data = (year * 100 + month);
		//printf("calculate_earliest_2_month_ago:%d %d    %d \n",year,month,*earliest_data);
		return 0;
	}else if(month == 2)
	{
		month = 12;
		year = year - 1;
		*earliest_data = (year * 100 + month);
		//printf("calculate_earliest_2_month_ago:%d %d    %d \n",year,month,*earliest_data);
		return 0;
	}else if(month == 1)
	{
		month = 11;
		year = year - 1;
		*earliest_data = (year * 100 + month);
		//printf("calculate_earliest_2_month_ago:%d %d    %d \n",year,month,*earliest_data);
		return 0;
	}
	
	return -1;
}


//删除两个月之前的数据
void delete_system_power_2_month_ago(char *date_time)
{
	DIR *dirp;
	char dir[30] = "/home/record/power";
	struct dirent *d;
	char path[100];
	int earliest_data,file_data;
	char fileTime[20] = {'\0'};

	/* 打开dir目录*/
	dirp = opendir("/home/record/power");
	if(dirp == RT_NULL)
	{
		printmsg(ECU_DBG_CLIENT,"delete_system_power_2_month_ago open directory error");
	}
	else
	{
		calculate_earliest_2_month_ago(date_time,&earliest_data);
		//printf("calculate_earliest_2_month_ago:::::%d\n",earliest_data);
		/* 读取dir目录*/
		while ((d = readdir(dirp)) != RT_NULL)
		{
		
			
			memcpy(fileTime,d->d_name,6);
			fileTime[6] = '\0';
			file_data = atoi(fileTime);
			if(file_data <= earliest_data)
			{
				sprintf(path,"%s/%s",dir,d->d_name);
				unlink(path);
			}

			
		}
		/* 关闭目录 */
		closedir(dirp);
	}


}

//读取某日的功率曲线参数   日期   报文
int read_system_power(char *date_time, char *power_buff,int *length)
{
	char dir[30] = "/home/record/power";
	char path[100];
	char buff[100]={'\0'};
	char date_tmp[9] = {'\0'};
	int power = 0;
	FILE *fp;
	//rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);

	memset(path,0,100);
	memset(buff,0,100);
	memcpy(date_tmp,date_time,8);
	date_tmp[6] = '\0';
	sprintf(path,"%s/%s.dat",dir,date_tmp);

	*length = 0;
	fp = fopen(path, "r");
	if(fp)
	{
		while(NULL != fgets(buff,100,fp))  //读取一行数据
		{	//第8个字节为，  且时间相同
			if((buff[12] == ',') && (!memcmp(buff,date_time,8)))
			{
				power = (int)atoi(&buff[13]);
				power_buff[(*length)++] = (buff[8]-'0') * 16 + (buff[9]-'0');
				power_buff[(*length)++] = (buff[10]-'0') * 16 + (buff[11]-'0');
				power_buff[(*length)++] = power/256;
				power_buff[(*length)++] = power%256;
			}
				
		}
		fclose(fp);
		//rt_mutex_release(record_data_lock);
		return 0;
	}

	//rt_mutex_release(record_data_lock);

	return -1;

	
}


int search_daily_energy(char *date,float *daily_energy)	
{
	char dir[30] = "/home/record/energy";
	char path[100];
	char buff[100]={'\0'};
	char date_tmp[9] = {'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);

	memset(path,0,100);
	memset(buff,0,100);
	memcpy(date_tmp,date,8);
	date_tmp[6] = '\0';
	sprintf(path,"%s/%s.dat",dir,date_tmp);
	
	fp = fopen(path, "r");
	if(fp)
	{
		while(NULL != fgets(buff,100,fp))  //读取一行数据
		{	//第8个字节为，  且时间相同
			if((buff[8] == ',') && (!memcmp(buff,date,8)))
			{
				*daily_energy = (float)atof(&buff[9]);
				fclose(fp);
				rt_mutex_release(record_data_lock);
				return 0;
			}
				
		}
	
		fclose(fp);
	}

	rt_mutex_release(record_data_lock);

	return -1;
}


void update_daily_energy(float current_energy, char *date_time)
{
	char dir[50] = "/home/record/energy/";
	char file[9];
	char sendbuff[50] = {'\0'};
	char date_time_tmp[14] = {'\0'};
	rt_err_t result;
	float energy_tmp = 0;
	int fd;
	//当前一轮发电量为0 不更新发电量
	if(current_energy <= EPSILON && current_energy >= -EPSILON) return;
	
	memcpy(date_time_tmp,date_time,14);
	memcpy(file,&date_time[0],6);
	file[6] = '\0';
	sprintf(dir,"%s%s.dat",dir,file);
	date_time_tmp[8] = '\0';	//存储时间为年月日 例如:20170718

	//检测是否已经存在该时间点的数据
	if (0 == search_daily_energy(date_time_tmp,&energy_tmp))
	{
		energy_tmp = current_energy + energy_tmp;
		delete_line(dir,"/home/record/energy/1.tmp",date_time,8);
	}
	
	print2msg(ECU_DBG_MAIN,"update_daily_energy DIR",dir);
	result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		fd = open(dir, O_WRONLY | O_APPEND | O_CREAT,0);
		if (fd >= 0)
		{	
			ecu.today_energy = energy_tmp;
			sprintf(sendbuff,"%s,%f\n",date_time_tmp,energy_tmp);
			//print2msg(ECU_DBG_MAIN,"update_daily_energy",sendbuff);
			write(fd,sendbuff,strlen(sendbuff));
			close(fd);
		}
	}
	rt_mutex_release(record_data_lock);
	
}

int leap(int year) 
{ 
    if(year%4==0 && year%100!=0 || year%400==0) 
        return 1; 
    else 
        return 0; 
} 

//计算一周中最早一天的时间		返回值：0表示最早一天在当月   1：表示最早一天在上个月
int calculate_earliest_week(char *date,int *earliest_data)
{
	char year_s[5] = {'\0'};
	char month_s[3] = {'\0'};
	char day_s[3] = {'\0'};
	int year = 0,month = 0,day = 0,flag = 0,count = 0;	//year为年份 month为月份 day为日期  flag 为瑞年判断标志 count表示上个月好需要补的天数 number_of_days:上个月的总天数
	
	memcpy(year_s,date,4);
	year_s[4] = '\0';
	year = atoi(year_s);

	memcpy(month_s,&date[4],2);
	month_s[2] = '\0';
	month = atoi(month_s);
	
	memcpy(day_s,&date[6],2);
	day_s[2] = '\0';
	day = atoi(day_s);
	
	if(day > 7)
	{
		*earliest_data = (year * 10000 + month * 100 + day) - 6;
		return 0;
	}else
	{
		count = 7 - day;
		//判断上个月是几月
		month = month - 1;
		if(month == 0)
		{
			year = year - 1;
			month = 12;
		}	
		//计算是否是闰年
		flag = leap(year);
		day = day_tab[flag][month-1]+1-count;
		*earliest_data = (year * 10000 + month * 100 + day);	
		
		return 1;
	}

}

//读取最近一周的发电量    按每日计量
int read_weekly_energy(char *date_time, char *power_buff,int *length)
{
	char dir[30] = "/home/record/energy";
	char path[100];
	char buff[100]={'\0'};
	char date_tmp[9] = {'\0'};
	int earliest_date = 0,compare_time = 0,flag = 0;
	char energy_tmp[20] = {'\0'};
	int energy = 0;
	FILE *fp;

	memset(path,0,100);
	memset(buff,0,100);

	*length = 0;
	//计算前七天中最早的一天   如果flag为0  表示最早一天在当月  如果为1表示在上个月
	flag = calculate_earliest_week(date_time,&earliest_date);
	
	if(flag == 1)
	{
		sprintf(date_tmp,"%d",earliest_date);
		date_tmp[6] = '\0';
		//组件文件目录
		sprintf(path,"%s/%s.dat",dir,date_tmp);
		//打开文件
		print2msg(ECU_DBG_OTHER,"path",path);
		fp = fopen(path, "r");
		if(fp)
		{
			while(NULL != fgets(buff,100,fp))  //读取一行数据
			{	
				//将时间转换为int型   然后进行比较
				memcpy(date_tmp,buff,8);
				date_tmp[8] = '\0';
				compare_time = atoi(date_tmp);
				//printf("compare_time %d     earliest_date %d\n",compare_time,earliest_date);
				if(compare_time >= earliest_date)
				{
					memcpy(energy_tmp,&buff[9],(strlen(buff)-9));
					energy = (int)(atof(energy_tmp)*100);
					print2msg(ECU_DBG_OTHER,"buff",buff);
					printdecmsg(ECU_DBG_OTHER,"energy",energy);
					power_buff[(*length)++] = (date_tmp[0]-'0')*16 + (date_tmp[1]-'0');
					power_buff[(*length)++] = (date_tmp[2]-'0')*16 + (date_tmp[3]-'0');
					power_buff[(*length)++] = (date_tmp[4]-'0')*16 + (date_tmp[5]-'0');
					power_buff[(*length)++] = (date_tmp[6]-'0')*16 + (date_tmp[7]-'0');
					power_buff[(*length)++] = energy/256;
					power_buff[(*length)++] = energy%256;
				}
					
			}
			fclose(fp);
		}
	}
	
	
	memcpy(date_tmp,date_time,8);
	date_tmp[6] = '\0';
	sprintf(path,"%s/%s.dat",dir,date_tmp);

	print2msg(ECU_DBG_OTHER,"path",path);
	fp = fopen(path, "r");
	if(fp)
	{
		while(NULL != fgets(buff,100,fp))  //读取一行数据
		{	
			//将时间转换为int型   然后进行比较
			memcpy(date_tmp,buff,8);
			date_tmp[8] = '\0';
			compare_time = atoi(date_tmp);
			//printf("compare_time %d     earliest_date %d\n",compare_time,earliest_date);
			if(compare_time >= earliest_date)
			{
				memcpy(energy_tmp,&buff[9],(strlen(buff)-9));
				energy = (int)(atof(energy_tmp)*100);
				print2msg(ECU_DBG_OTHER,"buff",buff);
				printdecmsg(ECU_DBG_OTHER,"energy",energy);
				power_buff[(*length)++] = (date_tmp[0]-'0')*16 + (date_tmp[1]-'0');
				power_buff[(*length)++] = (date_tmp[2]-'0')*16 + (date_tmp[3]-'0');
				power_buff[(*length)++] = (date_tmp[4]-'0')*16 + (date_tmp[5]-'0');
				power_buff[(*length)++] = (date_tmp[6]-'0')*16 + (date_tmp[7]-'0');
				power_buff[(*length)++] = energy/256;
				power_buff[(*length)++] = energy%256;
			}
				
		}
		fclose(fp);
	}
	return 0;

}

//计算一个月中最早一天的时间
int calculate_earliest_month(char *date,int *earliest_data)
{
	char year_s[5] = {'\0'};
	char month_s[3] = {'\0'};
	char day_s[3] = {'\0'};
	int year = 0,month = 0,day = 0;	//year为年份 month为月份 day为日期  flag 为瑞年判断标志 count表示上个月好需要补的天数 number_of_days:上个月的总天数
	
	memcpy(year_s,date,4);
	year_s[4] = '\0';
	year = atoi(year_s);

	memcpy(month_s,&date[4],2);
	month_s[2] = '\0';
	month = atoi(month_s);
	
	memcpy(day_s,&date[6],2);
	day_s[2] = '\0';
	day = atoi(day_s);
	
	//判断是否大于28号  
	if(day >= 28)
	{
		day = 1;
		*earliest_data = (year * 10000 + month * 100 + day);
		//printf("calculate_earliest_month:%d %d  %d    %d \n",year,month,day,*earliest_data);
		return 0;
	}else
	{
		//如果小于28号，取上个月的该天的后一天 
		day = day + 1;
		month = month - 1;
		if(month == 0)
		{
			year = year - 1;
			month = 12;
		}
		*earliest_data = (year * 10000 + month * 100 + day);	
		//printf("calculate_earliest_month:%d %d  %d    %d \n",year,month,day,*earliest_data);
		return 1;
	}
	
}

//读取最近一个月的发电量    按每日计量
int read_monthly_energy(char *date_time, char *power_buff,int *length)
{
	char dir[30] = "/home/record/energy";
	char path[100];
	char buff[100]={'\0'};
	char date_tmp[9] = {'\0'};
	int earliest_date = 0,compare_time = 0,flag = 0;
	char energy_tmp[20] = {'\0'};
	int energy = 0;
	FILE *fp;

	memset(path,0,100);
	memset(buff,0,100);

	*length = 0;
	//计算前七天中最早的一天   如果flag为0  表示最早一天在当月  如果为1表示在上个月
	flag = calculate_earliest_month(date_time,&earliest_date);
	
	if(flag == 1)
	{
		sprintf(date_tmp,"%d",earliest_date);
		date_tmp[6] = '\0';
		//组件文件目录
		sprintf(path,"%s/%s.dat",dir,date_tmp);
		//打开文件
		//printf("path:%s\n",path);
		fp = fopen(path, "r");
		if(fp)
		{
			while(NULL != fgets(buff,100,fp))  //读取一行数据
			{	
				//将时间转换为int型   然后进行比较
				memcpy(date_tmp,buff,8);
				date_tmp[8] = '\0';
				compare_time = atoi(date_tmp);
				//printf("compare_time %d     earliest_date %d\n",compare_time,earliest_date);
				if(compare_time >= earliest_date)
				{
					memcpy(energy_tmp,&buff[9],(strlen(buff)-9));
					energy = (int)(atof(energy_tmp)*100);
					//printf("buff:%s\n energy:%d\n",buff,energy);
					power_buff[(*length)++] = (date_tmp[0]-'0')*16 + (date_tmp[1]-'0');
					power_buff[(*length)++] = (date_tmp[2]-'0')*16 + (date_tmp[3]-'0');
					power_buff[(*length)++] = (date_tmp[4]-'0')*16 + (date_tmp[5]-'0');
					power_buff[(*length)++] = (date_tmp[6]-'0')*16 + (date_tmp[7]-'0');
					power_buff[(*length)++] = energy/256;
					power_buff[(*length)++] = energy%256;
				}
					
			}
			fclose(fp);
		}
	}
	
	
	memcpy(date_tmp,date_time,8);
	date_tmp[6] = '\0';
	sprintf(path,"%s/%s.dat",dir,date_tmp);

	//printf("path:%s\n",path);
	fp = fopen(path, "r");
	if(fp)
	{
		while(NULL != fgets(buff,100,fp))  //读取一行数据
		{	
			//将时间转换为int型   然后进行比较
			memcpy(date_tmp,buff,8);
			date_tmp[8] = '\0';
			compare_time = atoi(date_tmp);
			//printf("compare_time %d     earliest_date %d\n",compare_time,earliest_date);
			if(compare_time >= earliest_date)
			{
				memcpy(energy_tmp,&buff[9],(strlen(buff)-9));
				energy = (int)(atof(energy_tmp)*100);
				//printf("buff:%s\n energy:%d\n",buff,energy);
				power_buff[(*length)++] = (date_tmp[0]-'0')*16 + (date_tmp[1]-'0');
				power_buff[(*length)++] = (date_tmp[2]-'0')*16 + (date_tmp[3]-'0');
				power_buff[(*length)++] = (date_tmp[4]-'0')*16 + (date_tmp[5]-'0');
				power_buff[(*length)++] = (date_tmp[6]-'0')*16 + (date_tmp[7]-'0');
				power_buff[(*length)++] = energy/256;
				power_buff[(*length)++] = energy%256;
			}
				
		}
		fclose(fp);
	}
	return 0;
}

//计算一年中最早一个月的时间
int calculate_earliest_year(char *date,int *earliest_data)
{
	char year_s[5] = {'\0'};
	char month_s[3] = {'\0'};
	int year = 0,month = 0;	//year为年份 month为月份 day为日期  flag 为瑞年判断标志 count表示上个月好需要补的天数 number_of_days:上个月的总天数
	
	memcpy(year_s,date,4);
	year_s[4] = '\0';
	year = atoi(year_s);

	memcpy(month_s,&date[4],2);
	month_s[2] = '\0';
	month = atoi(month_s);
	
	if(month == 12)
	{
		month = 1;
		*earliest_data = (year * 100 + month);
		//printf("calculate_earliest_month:%d %d    %d \n",year,month,*earliest_data);
		return 0;
	}else
	{
		month = month + 1;
		year = year - 1;
		*earliest_data = (year * 100 + month);
		//printf("calculate_earliest_month:%d %d    %d \n",year,month,*earliest_data);
		return 1;
	}
	
}

//读取最近一年的发电量    以每月计量
int read_yearly_energy(char *date_time, char *power_buff,int *length)
{
	char dir[30] = "/home/record/energy";
	char path[100];
	char buff[100]={'\0'};
	char date_tmp[9] = {'\0'};
	int earliest_date = 0,compare_time = 0;
	char energy_tmp[20] = {'\0'};
	int energy = 0;
	FILE *fp;

	memset(path,0,100);
	memset(buff,0,100);

	*length = 0;
	//计算前七天中最早的一天   如果flag为0  表示最早一天在当月  如果为1表示在上个月
	calculate_earliest_year(date_time,&earliest_date);
	
	sprintf(path,"%s/year.dat",dir);

	fp = fopen(path, "r");
	if(fp)
	{
		while(NULL != fgets(buff,100,fp))  //读取一行数据
		{	
			//将时间转换为int型   然后进行比较
			memcpy(date_tmp,buff,6);
			date_tmp[6] = '\0';
			compare_time = atoi(date_tmp);
			//printf("compare_time %d     earliest_date %d\n",compare_time,earliest_date);
			if(compare_time >= earliest_date)
			{
				memcpy(energy_tmp,&buff[7],(strlen(buff)-7));
				energy = (int)(atof(energy_tmp)*100);
				//printf("buff:%s\n energy:%d\n",buff,energy);
				power_buff[(*length)++] = (date_tmp[0]-'0')*16 + (date_tmp[1]-'0');
				power_buff[(*length)++] = (date_tmp[2]-'0')*16 + (date_tmp[3]-'0');
				power_buff[(*length)++] = (date_tmp[4]-'0')*16 + (date_tmp[5]-'0');
				power_buff[(*length)++] = 0x01;
				power_buff[(*length)++] = energy/256;
				power_buff[(*length)++] = energy%256;
			}
				
		}
		fclose(fp);
	}
	return 0;
}


int search_monthly_energy(char *date,float *daily_energy)	
{
	char dir[30] = "/home/record/energy";
	char path[100];
	char buff[100]={'\0'};
	char date_tmp[9] = {'\0'};
	FILE *fp;
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);

	memset(path,0,100);
	memset(buff,0,100);
	memcpy(date_tmp,date,6);
	date_tmp[4] = '\0';
	sprintf(path,"%s/year.dat",dir);
	
	fp = fopen(path, "r");
	if(fp)
	{
		while(NULL != fgets(buff,100,fp))  //读取一行数据
		{	//第8个字节为，  且时间相同
			if((buff[6] == ',') && (!memcmp(buff,date,6)))
			{
				*daily_energy = (float)atof(&buff[7]);
				fclose(fp);
				rt_mutex_release(record_data_lock);
				return 0;
			}
				
		}
	
		fclose(fp);
	}

	rt_mutex_release(record_data_lock);

	return -1;
}

void update_monthly_energy(float current_energy, char *date_time)
{
	char dir[50] = "/home/record/energy/";
	char file[9];
	char sendbuff[50] = {'\0'};
	char date_time_tmp[14] = {'\0'};
	rt_err_t result;
	float energy_tmp = 0;
	int fd;
	//当前一轮发电量为0 不更新发电量
	if(current_energy <= EPSILON && current_energy >= -EPSILON) return;
	
	memcpy(date_time_tmp,date_time,14);
	memcpy(file,&date_time[0],4);
	file[4] = '\0';
	sprintf(dir,"%syear.dat",dir);
	date_time_tmp[6] = '\0';	//存储时间为年月日 例如:20170718

	//检测是否已经存在该时间点的数据
	if (0 == search_monthly_energy(date_time_tmp,&energy_tmp))
	{
		energy_tmp = current_energy + energy_tmp;
		delete_line(dir,"/home/record/energy/2.tmp",date_time,6);
	}
	
	print2msg(ECU_DBG_MAIN,"update_monthly_energy DIR",dir);
	result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		fd = open(dir, O_WRONLY | O_APPEND | O_CREAT,0);
		if (fd >= 0)
		{		
			sprintf(sendbuff,"%s,%f\n",date_time_tmp,energy_tmp);
			//print2msg(ECU_DBG_MAIN,"update_daily_energy",sendbuff);
			write(fd,sendbuff,strlen(sendbuff));
			close(fd);
		}
	}
	rt_mutex_release(record_data_lock);
	
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
#if ECU_JLINK_DEBUG
			SEGGER_RTT_printf(0,"ECU MAC: %x %x %x %x %x %x\n",dev_addr[0],dev_addr[1],dev_addr[2],dev_addr[3],dev_addr[4],dev_addr[5]);
#endif
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
	rt_hw_ms_delay(20);
	mkdir("/tmp",0x777);
	rt_hw_ms_delay(20);
	mkdir("/yuneng",0x777);
	rt_hw_ms_delay(20);
	mkdir("/home/data",0x777);
	rt_hw_ms_delay(20);
	mkdir("/home/record",0x777);
	rt_hw_ms_delay(20);
	mkdir("/home/data/proc_res",0x777);
	rt_hw_ms_delay(20);
	mkdir("/home/data/iprocres",0x777);
	rt_hw_ms_delay(20);
	echo("/home/data/ltpower","0.000000");
	rt_hw_ms_delay(20);
	mkdir("/home/record/data",0x777);
	rt_hw_ms_delay(20);
	mkdir("/home/record/inversta",0x777);
	rt_hw_ms_delay(20);
	mkdir("/home/record/power",0x777);
	mkdir("/home/record/energy",0x777);
	rt_hw_ms_delay(20);
	echo("/yuneng/area.con","NA");
	rt_hw_ms_delay(20);
	echo("/yuneng/channel.con","0x10");
	rt_hw_ms_delay(20);
	echo("/yuneng/limiteid.con","1");
	rt_hw_ms_delay(20);	
	echo("/yuneng/control.con","Timeout=10\nReport_Interval=15\nDomain=ecu.apsema.com\nIP=60.190.131.190\nPort1=8997\nPort2=8997\n");
	//echo("/yuneng/control.con","Timeout=15\nReport_Interval=15\nDomain=eee.apsema.com\nIP=60.190.131.190\nPort1=8997\nPort2=8997\n");
	rt_hw_ms_delay(20);
	echo("/yuneng/vernum.con","2\n");
	rt_hw_ms_delay(20);
	echo("/yuneng/ftpadd.con", "IP=60.190.131.190\nPort=9219\nuser=zhyf\npassword=yuneng\n");
	//echo("/yuneng/ftpadd.con", "IP=192.168.1.103\nPort=21\nuser=admin\npassword=admin\n");
	rt_hw_ms_delay(20);
	echo("/yuneng/datacent.con","Domain=ecu.apsema.com\nIP=60.190.131.190\nPort1=8995\nPort2=8996\n");
	//echo("/yuneng/datacent.con","Domain=eee.apsema.com\nIP=139.168.200.158\nPort1=8093\nPort2=8093\n");
	rt_hw_ms_delay(20);
	echo("/home/data/power","");
	rt_hw_ms_delay(20);
	echo("/yuneng/timezone.con","Etc/GMT-8\n");
	rt_hw_ms_delay(20);
	mkdir("/ftp",0x777);
	echo("/yuneng/A118.con","1");
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

//检查目录中时间最早的文件,存在返回1，不存在返回0
//dir为目录的名称（传入参数）    oldfile为最早文件的文件名（传出参数）
static int checkOldFile(char *dir,char *oldFile)
{
	DIR *dirp;
	struct dirent *d;
	char path[100] , fullpath[100] = {'\0'};
	int fileDate = 0,temp = 0;
	char tempDate[9] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	if(result == RT_EOK)
	{
		/* 打开dir目录*/
		dirp = opendir(dir);
		
		if(dirp == RT_NULL)
		{
			printmsg(ECU_DBG_OTHER,"check Old File open directory error");
		}
		else
		{
			/* 读取dir目录*/
			while ((d = readdir(dirp)) != RT_NULL)
			{
				
				memcpy(tempDate,d->d_name,8);
				tempDate[8] = '\0';
				if(((temp = atoi(tempDate)) < fileDate) || (fileDate == 0))
				{
					fileDate = temp;
					memset(path,0,100);
					strcpy(path,d->d_name);
				}
				
			}
			if(fileDate != 0)
			{
				sprintf(fullpath,"%s/%s",dir,path);
				strcpy(oldFile,fullpath);
				closedir(dirp);
				return 1;
			}
			/* 关闭目录 */
			closedir(dirp);
		}
	}
	rt_mutex_release(record_data_lock);
	return 0;
}


//检索整个文件系统，判断剩余空间存储量，如果剩余可存储空间过小，则检索相应的目录，并进行相应的删除操作
int optimizeFileSystem(void)
{
  int result;
  long long cap;
  struct statfs buffer;
	char oldFile[100] = {'\0'};

  result = dfs_statfs("/", &buffer);
  if (result != 0)
  {
      printmsg(ECU_DBG_OTHER,"dfs_statfs failed.\n");
      return -1;
  }
  cap = buffer.f_bsize * buffer.f_bfree / 1024;
	
	printdecmsg(ECU_DBG_MAIN,"disk free size",(unsigned long)cap);
	//当flash芯片所剩下的容量小于40KB的时候进行一些必要的文件删除操作。
	if (cap < 40) 
	{
		//删除最前面一天的ECU级别处理结果数据    如果该目录下存在文件的话
		if(1 == checkOldFile("/home/data/proc_res",oldFile))
		{
			unlink(oldFile);
		}
		
		//删除最前面一天的逆变器级别处理结果数据  如果该目录下存在文件的话
		memset(oldFile,0x00,100);
		if(1 == checkOldFile("/home/data/iprocres",oldFile))
		{
			unlink(oldFile);
		}
		
		//删除最前面一天的逆变器状态数据 如果该目录下存在文件的话
		memset(oldFile,0x00,100);
		if(1 == checkOldFile("/home/record/inversta",oldFile))
		{
			unlink(oldFile);
		}
		
		//删除最前面一天的发电量数据   如果该目录下存在文件的话
		memset(oldFile,0x00,100);
		if(1 == checkOldFile("/home/record/data",oldFile))
		{
			unlink(oldFile);
		}	
		
	}
	
	return 0;
		
}


#ifdef RT_USING_FINSH
#include <finsh.h>

FINSH_FUNCTION_EXPORT(echo, eg:echo("/test","test"));
#if 0
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
#endif

int cal(char * date)
{
	int length = 0;
	char power_buff[1024] = { '\0' };
	//calculate_earliest_week(date,&earliest_data);
	read_yearly_energy(date, power_buff,&length);
	//printf("length:%d \n",length);
	//calculate_earliest_month(date,&earliest_data);
	//calculate_earliest_year(date,&earliest_data);
	return 0;
}
FINSH_FUNCTION_EXPORT(cal, eg:cal("20170721"));

int initsystem(char *ecuid,char *mac)
{
	char fileecuid[13];	
	mkdir("/home",0x777);
	rt_hw_ms_delay(20);
	mkdir("/tmp",0x777);
	rt_hw_ms_delay(20);
	mkdir("/yuneng",0x777);
	rt_hw_ms_delay(20);
	mkdir("/home/data",0x777);
	rt_hw_ms_delay(20);
	mkdir("/home/record",0x777);
	rt_hw_ms_delay(20);
	mkdir("/home/data/proc_res",0x777);
	rt_hw_ms_delay(20);
	mkdir("/home/data/iprocres",0x777);
	rt_hw_ms_delay(20);
	echo("/home/data/ltpower","0.000000");
	rt_hw_ms_delay(20);
	mkdir("/home/record/data",0x777);
	rt_hw_ms_delay(20);
	mkdir("/home/record/inversta",0x777);
	rt_hw_ms_delay(20);
	memcpy(fileecuid,ecuid,12);
	rt_hw_ms_delay(20);
	fileecuid[12] = '\n';
	echo("/yuneng/ecuid.con",fileecuid);
	rt_hw_ms_delay(20);
	echo("/yuneng/area.con","NA");
	rt_hw_ms_delay(20);
	mkdir("/home/record/power",0x777);
	mkdir("/home/record/energy",0x777);
	echo("/yuneng/ecumac.con",mac);
	rt_hw_ms_delay(20);
	echo("/yuneng/channel.con","0x10");
	rt_hw_ms_delay(20);
	echo("/yuneng/limiteid.con","1");
	rt_hw_ms_delay(20);
	echo("/yuneng/control.con","Timeout=10\nReport_Interval=15\nDomain=ecu.apsema.com\nIP=60.190.131.190\nPort1=8997\nPort2=8997\n");
	//echo("/yuneng/control.con","Timeout=15\nReport_Interval=5\nDomain=111.apsema.com\nIP=139.168.200.158\nPort1=8997\nPort2=8997\n");
	rt_hw_ms_delay(20);
	echo("/yuneng/vernum.con","2\n");
	rt_hw_ms_delay(20);
	echo("/yuneng/ftpadd.con", "IP=60.190.131.190\nPort=9219\nuser=zhyf\npassword=yuneng\n");
	//echo("/yuneng/ftpadd.con", "IP=192.168.1.103\nPort=21\nuser=admin\npassword=admin\n");
	rt_hw_ms_delay(20);
	echo("/yuneng/datacent.con","Domain=ecu.apsema.com\nIP=60.190.131.190\nPort1=8995\nPort2=8996\n");
	//echo("/yuneng/datacent.con","Domain=eee.apsema.com\nIP=139.168.200.158\nPort1=8093\nPort2=8093\n");
	rt_hw_ms_delay(20);
	echo("/home/data/power","");
	rt_hw_ms_delay(20);
	echo("/yuneng/timezone.con","Etc/GMT-8\n");
	rt_hw_ms_delay(20);
	mkdir("/ftp",0x777);
	echo("/yuneng/A118.con","1");
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
