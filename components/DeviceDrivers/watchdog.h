/*
 * File      : watchdog.h
 * Change Logs:
 * Date           Author       		Notes
 * 2017-05-17     shengfeng dong  first version.
 */

#ifndef  __WATCHDOG_H__
#define  __WATCHDOG_H__

void rt_hw_watchdog_init(void);

void kickwatchdog(void);


#endif /* __WATCHDOG_H__ */
