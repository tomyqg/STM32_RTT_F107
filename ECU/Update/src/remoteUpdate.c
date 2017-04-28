#include "remoteUpdate.h"
#include <rtthread.h>
#include "thftpapi.h"
#include "flash_if.h"
#include "dfs_posix.h"
#include "rthw.h"
#include "myfile.h"
#include "version.h"
#include "threadlist.h"

#define UPDATE_PATH_SUFFIX "ecu-r-m3.bin"
#define UPDATE_PATH "/FTP/ecu.bin"


extern rt_mutex_t record_data_lock; 

int updateECU(void)
{
	int ret = 0;
	char IPFTPadd[50] = {'\0'};
	char remote_path[100] = {'\0'};
	int port=0;
	char user[20]={'\0'};
	char password[20]={'\0'};
	rt_thread_delay(RT_TICK_PER_SECOND * START_TIME_UPDATE);
	getFTPConf(IPFTPadd,&port,user,password);

	printf("FTPIP:%s\nport:%d\nuser:%s\npassword:%s\n ",IPFTPadd,port,user,password);
	
	//获取服务器IP地址
	sprintf(remote_path,"/ECU_R_M3/V%s.%s/%s",MAJORVERSION,MINORVERSION,UPDATE_PATH_SUFFIX);
	rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	ret=ftpgetfile(IPFTPadd, port, user, password,remote_path,UPDATE_PATH);
	if(!ret)
	{
		//获取到文件，进行更新
		FLASH_Unlock();
		FLASH_If_Erase_APP2();
		FLASH_IF_FILE_COPY_TO_APP2(UPDATE_PATH);
		unlink(UPDATE_PATH);
		reboot();
	}else
	{
		unlink(UPDATE_PATH);
	}
	rt_mutex_release(record_data_lock);	
	return ret;
}

void remote_update_thread_entry(void* parameter)
{
	int i = 0;
	while(1)
	{
		for(i = 0;i<3;i++)
		{
			if(-1 != updateECU())
				break;
		}
		//rt_thread_delay(RT_TICK_PER_SECOND*10);		
		rt_thread_delay(RT_TICK_PER_SECOND*86400);		
	}	

}
