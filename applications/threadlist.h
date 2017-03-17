#ifndef __THREADLIST_H
#define __THREADLIST_H

#define THREAD_PRIORITY_LED               20
#define THREAD_PRIORITY_MAIN	            23
#define THREAD_PRIORITY_NTP								21
#define THREAD_PRIORITY_INIT							10
#define THREAD_PRIORITY_CLIENT						24

#define THREAD_PRIORITY_HC595							22 
#define THREAD_PRIORITY_LCD								22
void tasks_new(void);//创建系统需要的线程
#endif
