#include <stdio.h>
#include <string.h>
#include "myfile.h"
#include "file.h"
#include "datetime.h"
#include <dfs_posix.h> 
#include "debug.h"
#include <stdlib.h>

void delete_newline(char *s)
{
	if(10 == s[strlen(s)-1])
		s[strlen(s)-1] = '\0';
}

/* 获取(存储单个参数的)配置文件中的值 */
char *file_get_one(char *s, int count, const char *filename)
{
	
	FILE *fp;
	
	fp = fopen(filename, "r");
	if(fp == NULL){
		print2msg("control_client",(char *)filename,"open error!");
		return NULL;
	}
	fgets(s, count, fp);
	if (10 == s[strlen(s) - 1])
		s[strlen(s) - 1] = '\0';
	fclose(fp);
	return s;
}

/* 将单个参数值写入配置文件中 */
int file_set_one(const char *s, const char *filename)
{
	FILE *fp;
	fp = fopen(filename, "w");
	if(fp == NULL){
		printmsg("control_client",(char *)filename);
		return -1;
	}
	if(EOF == fputs(s, fp)){
		fclose(fp);
		return -1;
	}
	fclose(fp);
	return 0;
}

/* 获取(存储多个参数的)配置文件中的值 */
int file_get_array(MyArray *array, int num, const char *filename)
{
	FILE *fp;
	int count = 0;
	char buffer[128] = {'\0'};

	memset(array, 0 ,sizeof(MyArray)*num);
	fp = fopen(filename, "r");
	if(fp == NULL){
		printmsg("control_client",(char *)filename);
		return -1;
	}
	while(!feof(fp))
	{
		if(count >= num)
		{
			fclose(fp);
			return 0;
		}
		memset(buffer, 0 ,sizeof(buffer));
		fgets(buffer, 128, fp);
		if(!strlen(buffer))continue;

		strncpy(array[count].name, buffer, strcspn(buffer, "="));
		strncpy(array[count].value, &buffer[strlen(array[count].name)+1], 64);
		delete_newline(array[count].value);
		count++;
	}
	fclose(fp);
	return 0;
}

int file_set_array(const MyArray *array, int num, const char *filename)
{
	FILE *fp;
	int i;

	fp = fopen(filename, "w");
	if(fp == NULL){
		printmsg("control_client",(char *)filename);
		return -1;
	}
	for(i=0; i<num; i++){
		fprintf(fp, "%s=%s\n", array[i].name, array[i].value);
	}
	fclose(fp);
	return 0;
}

//清除文件
int clear_file(char *filename)
{
	FILE *fp;
	fp = fopen(filename, "w");
	if(fp == NULL){
		printmsg("control_client",(char *)filename);
		return -1;
	}
	fclose(fp);
	return 0;
}

//删除文件以行
int delete_line(char* filename,char* temfilename,char* compareData,int len)
{
	FILE *fin,*ftp;
  char data[512];
  fin=fopen(filename,"r");//读打开原文件123.txt
	if(fin == NULL)
	{
		printf("Open the file %s failure...\n",filename);
    return -1;
	}
	
  ftp=fopen(temfilename,"w");
	if( ftp==NULL){
		printf("Open the file %s failure...\n",temfilename);
		fclose(fin);
    return -1;
  }
  while(fgets(data,512,fin))//从原文件读取一行
	{
		if(memcmp(data,compareData,len))
		{
			printf("%s\n",data);
			fputs(data,ftp);//不是则将这一行写入临时文件
		}
	}
  fclose(fin);
  fclose(ftp);
  remove(filename);//删除原文件
  rename(temfilename,filename);//将临时文件名改为原文件名
  return 0;
	
}


int get_num_from_id(char inverter_ids[MAXINVERTERCOUNT][13])
{
	int num=0,i,sameflag;
	FILE *fp;
	char data[200];
	fp = fopen("/home/data/id", "r");
	if(fp)
	{
		while(NULL != fgets(data,200,fp))
		{
			//与前面几行比较 如果当前行存在数据，表示存在一个逆变器
			sameflag = 0;
			for(i = 0;i<num;i++)
			{
				if(!memcmp(data,inverter_ids[i],12))
					sameflag = 1;
			}
			if(sameflag == 1)
			{
				continue;
			}
			memcpy(inverter_ids[num],data,12);
			inverter_ids[num][12] = '\0';
			num++;
		}
		fclose(fp);
	}	
	
	
	return num;
}

int insert_line(char * filename,char *str)
{
	int fd;
	fd = open(filename, O_WRONLY | O_APPEND | O_CREAT,0);
	if (fd >= 0)
	{		

		write(fd,str,strlen(str));
		close(fd);
	}
	
	return search_line(filename,str,strlen(str));
	
}

int search_line(char* filename,char* compareData,int len)
{
	FILE *fin;
  char data[50];
  fin=fopen(filename,"r");
	if(fin == NULL)
	{
		printf("Open the file %s failure1...\n",filename);
    return -1;
	}
	
  while(fgets(data,50,fin))//从原文件读取一行
	{
		if(!memcmp(data,compareData,len))
		{
			//存在相同行，关闭文件   然后返回1  表示存在该行
			fclose(fin);
			return 1;
		}
	}
  fclose(fin);
  return -1;
}


int get_protection_from_file(const char pro_name[][32],float *pro_value,int *pro_flag,int num)
{
	FILE *fp;
	char list[3][32];
	char data[200];
	int j = 0;
	fp = fopen("/home/data/setpropa", "r");
	if(fp)
	{
		while(NULL != fgets(data,200,fp))
		{
			memset(list,0,sizeof(list));
			splitString(data,list);
			for(j=0; j<num; j++){
				if(!memcmp(list[0], pro_name[j],strlen(list[0]))){
					pro_value[j] = atof(list[1]);
					pro_flag[j] = 1;
					break;
				}
			}
		}
		fclose(fp);
	}
	return -1;
}

//返回1 便是寻找到行   返回-1表示未寻找到
int read_line(char* filename,char *linedata,char* compareData,int len)
{
	FILE *fin;
  fin=fopen(filename,"r");
	if(fin == NULL)
	{
		printf("Open the file %s failure1...\n",filename);
    return -1;
	}
	
  while(fgets(linedata,100,fin))//从原文件读取一行
	{
		if(!memcmp(linedata,compareData,len))
		{
			//存在相同行，关闭文件   然后返回1  表示存在该行
			fclose(fin);
			return 1;
		}
	}
  fclose(fin);
  return -1;
}

int read_line_end(char* filename,char *linedata,char* compareData,int len)
{
		FILE *fin;
  fin=fopen(filename,"r");
	if(fin == NULL)
	{
		printf("Open the file %s failure1...\n",filename);
    return -1;
	}
	
  while(fgets(linedata,100,fin))//从原文件读取一行
	{
		if(!memcmp(&linedata[strlen(linedata)-len-1],compareData,len))
		{
			//存在相同行，关闭文件   然后返回1  表示存在该行
			fclose(fin);
			return 1;
		}
	}
  fclose(fin);
  return -1;
}


#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(delete_line, eg:delete_line("/home/data/id","/home/data/id_tmp","201703150001",12));
#endif


