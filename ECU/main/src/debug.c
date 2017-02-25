#define DEBUG
#include "variation.h"
#include <rtdef.h>
#include <rtthread.h>

void printmsg(char *msg)		//打印字符串
{
#ifdef DEBUG
	rt_kprintf("main==>%s!\n", msg);
#endif
}

void print2msg(char *msg1, char *msg2)		//打印字符串
{
#ifdef DEBUG
	rt_kprintf("main==>%s: %s!\n", msg1, msg2);
#endif
}


void printdecmsg(char *msg, int data)		//打印整形数据
{
#ifdef DEBUG
	rt_kprintf("main==>%s: %d!\n", msg, data);
#endif
}

void printhexdatamsg(char *msg, int data)		//打印16进制数据,ZK
{
#ifdef DEBUG
	rt_kprintf("main==>%s: %X!\n", msg, data);
#endif
}

void printfloatmsg(char *msg, float data)		//打印实数
{
#ifdef DEBUG
	rt_kprintf("main==>%s: %d!\n", msg, data);
#endif
}

void printhexmsg(char *msg, char *data, int size)		//打印十六进制数据
{
#ifdef DEBUG
	int i;

	rt_kprintf("main==>%s: ", msg);
	for(i=0; i<size; i++)
		rt_kprintf("%02X, ", data[i]);
	rt_kprintf("\n");
#endif
}

void printecuinfo(ecu_info *ecu)
{
	print2msg("ECU ID", ecu->id);
	printhexdatamsg("PANID", ecu->panid);
	printhexdatamsg("Channel", ecu->channel);
	print2msg("IP", ecu->ip);
	printfloatmsg("Lifetime energy", ecu->life_energy);
	printfloatmsg("Current energy", ecu->current_energy);
	printdecmsg("System power", ecu->system_power);
	printdecmsg("Total", ecu->total);
	printdecmsg("Current count", ecu->count);
	printdecmsg("Type", ecu->type);
	printdecmsg("Zoneflag", ecu->zoneflag);
}

void printinverterinfo(inverter_info *inverter)
{
	printdecmsg("Inverter it", inverter->it);
	printfloatmsg("Inverter gf", inverter->gf);
	printdecmsg("Inverter curacctimet", inverter->curacctime);
	printfloatmsg("Inverter dv", inverter->dv);
	printfloatmsg("Inverter dvb", inverter->dvb);
	printfloatmsg("Inverter di", inverter->di);
	printfloatmsg("Inverter dib", inverter->dib);
	printfloatmsg("Inverter dic", inverter->dic);
	printfloatmsg("Inverter did", inverter->did);
	printdecmsg("Inverter gv", inverter->gv);
	printdecmsg("Inverter gvb", inverter->gvb);
	printdecmsg("Inverter gvc", inverter->gvc);
	printfloatmsg("Inverter curaccgen", inverter->curaccgen);
	printfloatmsg("Inverter curaccgenb", inverter->curaccgenb);
	printfloatmsg("Inverter curaccgenc", inverter->curaccgenc);
	printfloatmsg("Inverter curaccgend", inverter->curaccgend);
	printfloatmsg("Inverter reactive_power", inverter->reactive_power);
	printfloatmsg("Inverter reactive_powerb", inverter->reactive_powerb);
	printfloatmsg("Inverter reactive_powerc", inverter->reactive_powerc);
	printfloatmsg("Inverter active_power", inverter->active_power);
	printfloatmsg("Inverter active_powerb", inverter->active_powerb);
	printfloatmsg("Inverter active_powerc", inverter->active_powerc);
	printfloatmsg("Inverter output_energy", inverter->output_energy);
	printfloatmsg("Inverter output_energyb", inverter->cur_output_energyb);
	printfloatmsg("Inverter output_energyc", inverter->cur_output_energyc);
	printmsg("=================================================");
	printfloatmsg("preaccgen", inverter->preaccgen);
	printfloatmsg("preaccgenb", inverter->preaccgenb);
	printfloatmsg("preaccgenc", inverter->preaccgenc);
	printfloatmsg("preaccgend", inverter->preaccgend);
	printfloatmsg("pre_output_energy", inverter->pre_output_energy);
	printfloatmsg("pre_output_energyb", inverter->pre_output_energyb);
	printfloatmsg("pre_output_energyc", inverter->pre_output_energyc);
	printdecmsg("preacctime", inverter->preacctime);
	print2msg("last_report_time",inverter->last_report_time);
	printdecmsg("Inverter op", inverter->op);
	printdecmsg("Inverter opb", inverter->opb);
	printdecmsg("Inverter opc", inverter->opc);
	printmsg("=================================================");
}
