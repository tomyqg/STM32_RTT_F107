/*
 * client.c
 * V1.2.2
 * modified on: 2013-08-13
 * 与EMA异步通信
 * update操作数据库出现上锁时，延时再update
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include <board.h>
#include "checkdata.h"
#include "datetime.h"
#include <dfs_posix.h> 
#include <rtthread.h>
#include "file.h"

ALIGN(RT_ALIGN_SIZE)
extern rt_uint8_t client_stack[ 8192 ];
extern struct rt_thread client_thread;




void client_thread_entry(void* parameter)
{
	printf("client_thread_entry>>>>>>>>>>>>>>..\n");
	while(1)
	{
		
	}
}
