#ifndef __THREADLIST_H
#define __THREADLIST_H

#define THREAD_PRIORITY_LED               20
#define THREAD_PRIORITY_MAIN	            23
#define THREAD_PRIORITY_NTP								21
#define THREAD_PRIORITY_INIT							10
#define THREAD_PRIORITY_CLIENT						25
#define THREAD_PRIORITY_CONTROL_CLIENT		26
#define THREAD_PRIORITY_DHCPRESET					11

void tasks_new(void);//创建系统需要的线程
#endif
