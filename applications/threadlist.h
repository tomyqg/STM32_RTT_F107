#ifndef __THREADLIST_H
#define __THREADLIST_H

#include <rtthread.h>

typedef enum THREADTYPE {
	TYPE_LED = 1,
	TYPE_LANRST = 2,
  TYPE_UPDATE = 3,
  TYPE_MAIN = 4,
  TYPE_CLIENT = 5,
  TYPE_CONTROL_CLIENT = 6
}threadType;

#define THREAD_PRIORITY_INIT							10
#define THREAD_PRIORITY_LED               20
#define THREAD_PRIORITY_LAN8720_RST				21
//#define THREAD_PRIORITY_UPDATE						22
//#define THREAD_PRIORITY_MAIN	            24
//#define THREAD_PRIORITY_CONTROL_CLIENT		26
//#define THREAD_PRIORITY_CLIENT						27


void tasks_new(void);//创建系统需要的线程

void restartThread(threadType type);

#endif
