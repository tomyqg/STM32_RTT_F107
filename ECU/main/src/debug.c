#define DEBUG
#include "variation.h"
#include <rtdef.h>
#include <rtthread.h>
#include <stdio.h>

void printmsg(char * funname,char *msg)		//打印字符串
{
#ifdef DEBUG
	printf("%s==>%s!\n",funname, msg);
#endif
}

void print2msg(char * funname,char *msg1, char *msg2)		//打印字符串
{
#ifdef DEBUG
	printf("%s==>%s: %s!\n",funname, msg1, msg2);
#endif
}


void printdecmsg(char * funname,char *msg, int data)		//打印整形数据
{
#ifdef DEBUG
	printf("%s==>%s: %d!\n",funname, msg, data);
#endif
}

void printhexdatamsg(char * funname,char *msg, int data)		//打印16进制数据,ZK
{
#ifdef DEBUG
	printf("%s==>%s: %X!\n",funname, msg, data);
#endif
}

void printfloatmsg(char * funname,char *msg, float data)		//打印实数
{
#ifdef DEBUG
	printf("%s==>%s: %f!\n",funname, msg, data);
#endif
}

void printhexmsg(char * funname,char *msg, char *data, int size)		//打印十六进制数据
{
#ifdef DEBUG
	int i;

	printf("%s==>%s: ",funname, msg);
	for(i=0; i<size; i++)
		printf("%02X, ", data[i]);
	printf("\n");
#endif
}

void printecuinfo(ecu_info *ecu)
{
	print2msg("main","ECU ID", ecu->id);
	printhexdatamsg("main","PANID", ecu->panid);
	printhexdatamsg("main","Channel", ecu->channel);
	print2msg("main","IP", ecu->ip);
	printfloatmsg("main","Lifetime energy", ecu->life_energy);
	printfloatmsg("main","Current energy", ecu->current_energy);
	printdecmsg("main","System power", ecu->system_power);
	printdecmsg("main","Total", ecu->total);
	printdecmsg("main","Current count", ecu->count);
	printdecmsg("main","Type", ecu->type);
	printdecmsg("main","Zoneflag", ecu->zoneflag);
}

void printinverterinfo(inverter_info *inverter)
{
	printdecmsg("main","Inverter it", inverter->it);
	printfloatmsg("main","Inverter gf", inverter->gf);
	printdecmsg("main","Inverter curacctimet", inverter->curacctime);
	printfloatmsg("main","Inverter dv", inverter->dv);
	printfloatmsg("main","Inverter dvb", inverter->dvb);
	printfloatmsg("main","Inverter di", inverter->di);
	printfloatmsg("main","Inverter dib", inverter->dib);
	printfloatmsg("main","Inverter dic", inverter->dic);
	printfloatmsg("main","Inverter did", inverter->did);
	printdecmsg("main","Inverter gv", inverter->gv);
	printdecmsg("main","Inverter gvb", inverter->gvb);
	printdecmsg("main","Inverter gvc", inverter->gvc);
	printfloatmsg("main","Inverter curaccgen", inverter->curaccgen);
	printfloatmsg("main","Inverter curaccgenb", inverter->curaccgenb);
	printfloatmsg("main","Inverter curaccgenc", inverter->curaccgenc);
	printfloatmsg("main","Inverter curaccgend", inverter->curaccgend);
	printfloatmsg("main","Inverter reactive_power", inverter->reactive_power);
	printfloatmsg("main","Inverter reactive_powerb", inverter->reactive_powerb);
	printfloatmsg("main","Inverter reactive_powerc", inverter->reactive_powerc);
	printfloatmsg("main","Inverter active_power", inverter->active_power);
	printfloatmsg("main","Inverter active_powerb", inverter->active_powerb);
	printfloatmsg("main","Inverter active_powerc", inverter->active_powerc);
	printfloatmsg("main","Inverter output_energy", inverter->output_energy);
	printfloatmsg("main","Inverter output_energyb", inverter->cur_output_energyb);
	printfloatmsg("main","Inverter output_energyc", inverter->cur_output_energyc);
	printmsg("main","=================================================");
	printfloatmsg("main","preaccgen", inverter->preaccgen);
	printfloatmsg("main","preaccgenb", inverter->preaccgenb);
	printfloatmsg("main","preaccgenc", inverter->preaccgenc);
	printfloatmsg("main","preaccgend", inverter->preaccgend);
	printfloatmsg("main","pre_output_energy", inverter->pre_output_energy);
	printfloatmsg("main","pre_output_energyb", inverter->pre_output_energyb);
	printfloatmsg("main","pre_output_energyc", inverter->pre_output_energyc);
	printdecmsg("main","preacctime", inverter->preacctime);
	print2msg("main","last_report_time",inverter->last_report_time);
	printdecmsg("main","Inverter op", inverter->op);
	printdecmsg("main","Inverter opb", inverter->opb);
	printdecmsg("main","Inverter opc", inverter->opc);
	printmsg("main","=================================================");
}
