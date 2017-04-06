#include <stdio.h>
#include <string.h>

#include "myfile.h"
#include "datetime.h"
#include <dfs_posix.h> 
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
		printmsg("control_client",(char *)filename);
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
		if(count >= num)return 0;
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


int save_to_process_result(int cmd_id, char *savebuffer)
{	char dir[50] = "/home/data/proc_res";
	char file[9];
	int fd;
	char time[20];
	getcurrenttime(time);
	memcpy(file,&time[0],8);
	file[8] = '\0';
	sprintf(dir,"%s%s.dat",dir,file);
	printf("%s\n",dir);
	fd = open(dir, O_WRONLY | O_APPEND | O_CREAT,0);
	if (fd >= 0)
	{		
		sprintf(savebuffer,"%s,%3d,1\n",savebuffer,cmd_id);
		printf("%s",savebuffer);
		write(fd,savebuffer,strlen(savebuffer));
		close(fd);
	}
	return 0;
	
}

