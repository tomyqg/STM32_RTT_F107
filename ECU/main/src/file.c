#include "file.h"
#include "checkdata.h"
#include <dfs_posix.h> 
#include <stdio.h>

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
