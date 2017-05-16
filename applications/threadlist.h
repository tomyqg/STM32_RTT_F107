#ifndef __THREADLIST_H
#define __THREADLIST_H

#include <rtthread.h>



#define WIFI_USE 

typedef enum THREADTYPE {
	TYPE_LED = 1,
	TYPE_LANRST = 2,
  TYPE_UPDATE = 3,
	TYPE_IDWRITE = 4,
  TYPE_MAIN = 5,
  TYPE_CLIENT = 6,
  TYPE_CONTROL_CLIENT = 7,
	TYPE_NTP = 8
}threadType;
//每个线程的优先级   宏打开表示线程打开
//初始化线程
#define THREAD_PRIORITY_INIT							10
//LAN8720A检测复位线程
#define THREAD_PRIORITY_LAN8720_RST				17
//LED 闪烁线程
#define THREAD_PRIORITY_LED               20

//测试相关线程
//#define THREAD_PRIORITY_WIFI_TEST					21
//#define THREAD_PRIORITY_NET_TEST					22
//#define THREAD_PRIORITY_ZIGBEE_TEST				22

//更新线程
//#define THREAD_PRIORITY_UPDATE						22

//#define THREAD_PRIORITY_NTP								21
//ID Write线程
//#define THREAD_PRIORITY_IDWRITE						23
//数据采集主线程
//#define THREAD_PRIORITY_MAIN	            24
//远程控制线程
//#define THREAD_PRIORITY_CONTROL_CLIENT		26
//数据上传线程
//#define THREAD_PRIORITY_CLIENT						27

//每个线程在开机后的启动时间,单位S
#define START_TIME_UPDATE									0
#define START_TIME_NTP										10
#define START_TIME_IDWRITE								5
#define START_TIME_MAIN										20
#define START_TIME_CONTROL_CLIENT					60
#define START_TIME_CLIENT									120
void tasks_new(void);//创建系统需要的线程

void restartThread(threadType type);

#endif
