#include "file.h"
#include "checkdata.h"
#include <dfs_posix.h> 
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern inverter_info inverter[MAXINVERTERCOUNT];

int ecu_type;	//1:SAA; 2:NA; 3:MX

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
		//printf("ecu_type:%d  %s\n",ecu_type,version);
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
		
		//printf("get_ecuid:  %s\n",ecuid);
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

int splitString(char *data,char splitdata[20][13])
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

	int i;
	inverter_info *inverter = firstinverter;
	char list[20][13];
	char data[200];
	FILE *fp;
	fp = fopen("/home/data/id", "r");
	if(fp)
	{
		while(NULL != fgets(data,200,fp))
		{
			printf("%s\n",data);
			memset(list,0,sizeof(list));
			splitString(data,list);
			strcpy(inverter->id, list[0]);
			if(NULL==list[1])
			{
				inverter->shortaddr = 0;		//Î´»ñÈ¡µ½¶ÌµØÖ·µÄÄæ±äÆ÷¸³ÖµÎª0.ZK
			}
			else
			{
				inverter->shortaddr = atoi(list[1]);
			}
			if(NULL==list[2])
			{
				inverter->model = 0;		//Î´»ñµÃ»úÐÍÂëµÄÄæ±äÆ÷¸³ÖµÎª0.ZK
			}
			else
			{
				inverter->model = atoi(list[2]);
			}

			if(NULL==list[3])
			{
				inverter->bindflag = 0;		//Î´°ó¶¨µÄÄæ±äÆ÷°Ñ±êÖ¾Î»¸³ÖµÎª0.ZK
			}
			else
			{
				inverter->bindflag = atoi(list[3]);
			}

			if(NULL==list[4])
			{
				inverter->zigbee_version = 0;		//Ã»ÓÐ»ñÈ¡µ½zigbee°æ±¾ºÅµÄÄæ±äÆ÷¸³ÖµÎª0.ZK
			}
			else
			{
				inverter->zigbee_version = atoi(list[4]);
			}
			inverter++;
		}

	}
	/*
	for(i=1;i<=nrow;i++)
	{
		strcpy(inverter->id, azResult[i*ncolumn]);
		if(NULL==azResult[i*ncolumn+1])
		{
			inverter->shortaddr = 0;		//??????????€??€?????????€??€??0.ZK
		}
		else
		{
			inverter->shortaddr = atoi(azResult[i*ncolumn+1]);
		}

		if(NULL==azResult[i*ncolumn+2])
		{
			inverter->model = 0;		//???????????????????????€??0.ZK
		}
		else
		{
			inverter->model = atoi(azResult[i*ncolumn+2]);
		}

		if(NULL==azResult[i*ncolumn+3])
		{
			inverter->bindflag = 0;		//?????????????????????0.ZK
		}
		else
		{
			inverter->bindflag = atoi(azResult[i*ncolumn+3]);
		}

		if(NULL==azResult[i*ncolumn+4])
		{
			inverter->zigbee_version = 0;		//????????IGBEE???????????????0.ZK
		}
		else
		{
			inverter->zigbee_version = atoi(azResult[i*ncolumn+4]);
		}
		inverter++;
	}

	inverter = firstinverter;
	printmsg("--------------");
	for(i=1; i<=nrow; i++, inverter++)
		printdecmsg(inverter->id, inverter->shortaddr);
	printmsg("--------------");
	printdecmsg("total", nrow);
	sqlite3_free_table( azResult );

	strcpy(sql,"SELECT id FROM id WHERE short_address IS NULL");
	sqlite3_get_table( db , sql , &azResult , &nrow2 , &ncolumn2 , &zErrMsg );

	printmsg("--------------");
	for(i=1; i<=nrow2; i++)
		printmsg(azResult[i*ncolumn2]);
	printmsg("--------------");
	printdecmsg("no_shortaddr", nrow2);
	sqlite3_free_table( azResult );

	return nrow;
	*/
}



#ifdef RT_USING_FINSH
#include <finsh.h>
void splitSt(char * str)
{
	int i = 0 , num;
	char list[20][13];
	num = splitString(str,list);
	for(i = 0;i<num;i++)
	{
		printf("%s ",list[i]);
		if(strlen(list[i]) == 0)
		{
			printf("NULL ");
		}
	}
	printf("num:%d\n",num);
   
	
}
FINSH_FUNCTION_EXPORT(splitSt, eg:splitSt());

void testfile()
{
	get_id_from_file(inverter);
}
FINSH_FUNCTION_EXPORT(testfile, eg:testfile());

#endif
