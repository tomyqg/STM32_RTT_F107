#ifndef __DEBUG_H__
#define __DEBUG_H__
#include "variation.h"



extern void printmsg(char * funname,char *msg);		//打印字符串
extern void print2msg(char * funname,char *msg1, char *msg2);		//打印字符串
extern void printdecmsg(char * funname,char *msg, int data);		//打印整形数据
extern void printhexdatamsg(char * funname,char *msg, int data);		//打印16进制数据,ZK
extern void printfloatmsg(char * funname,char *msg, float data);		//打印实数
extern void printhexmsg(char * funname,char *msg, char *data, int size);		//打印十六进制数据
extern void printecuinfo(ecu_info *ecu);
extern void printinverterinfo(inverter_info *inverter);

#endif /*__DEBUG_H__*/
