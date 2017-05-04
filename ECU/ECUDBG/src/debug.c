#include "variation.h"
#include <rtdef.h>
#include <rtthread.h>
#include <stdio.h>
#include "debug.h"

char funname[7][20] = {
		"update",
		"idwrite",
		"main",
		"client",
		"control",
		"ntp",
		"other"
};

void printmsg(DebugType type,char *msg)		//打印字符串
{
#if ECU_DEBUG
	switch(type)
	{
		case ECU_DBG_UPDATE:
			#if ECU_DEBUG_UPDATE
				printf("%s==>%s!\n",funname[type], msg);
			#endif
			break;
		case ECU_DBG_IDWRITE:
			#if ECU_DEBUG_IDWRITE
				printf("%s==>%s!\n",funname[type], msg);
			#endif
			break;
		case ECU_DBG_MAIN:
			#if ECU_DEBUG_MAIN
				printf("%s==>%s!\n",funname[type], msg);
			#endif
			break;
		case ECU_DBG_CLIENT:
			#if ECU_DEBUG_CLIENT
				printf("%s==>%s!\n",funname[type], msg);
			#endif
			break;
		case ECU_DBG_CONTROL_CLIENT:
			#if ECU_DEBUG_CONTROL_CLIENT
				printf("%s==>%s!\n",funname[type], msg);
			#endif
			break;
		case ECU_DBG_NTP:
			#if ECU_DEBUG_NTP
				printf("%s==>%s!\n",funname[type], msg);
			#endif
			break;
		case ECU_DBG_OTHER:
			#if ECU_DEBUG_OTHER
				printf("%s==>%s!\n",funname[type], msg);
			#endif
			break;
	}

#endif
}

void print2msg(DebugType type,char *msg1, char *msg2)		//打印字符串
{
#if ECU_DEBUG
	switch(type)
	{
		case ECU_DBG_UPDATE:
			#if ECU_DEBUG_UPDATE
				printf("%s==>%s: %s!\n",funname[type], msg1, msg2);
			#endif
			break;
		case ECU_DBG_IDWRITE:
			#if ECU_DEBUG_IDWRITE
				printf("%s==>%s: %s!\n",funname[type], msg1, msg2);
			#endif
			break;
		case ECU_DBG_MAIN:
			#if ECU_DEBUG_MAIN
				printf("%s==>%s: %s!\n",funname[type], msg1, msg2);
			#endif
			break;
		case ECU_DBG_CLIENT:
			#if ECU_DEBUG_CLIENT
				printf("%s==>%s: %s!\n",funname[type], msg1, msg2);
			#endif
			break;
		case ECU_DBG_CONTROL_CLIENT:
			#if ECU_DEBUG_CONTROL_CLIENT
				printf("%s==>%s: %s!\n",funname[type], msg1, msg2);
			#endif
			break;
		case ECU_DBG_NTP:
			#if ECU_DEBUG_NTP
				printf("%s==>%s: %s!\n",funname[type], msg1, msg2);
			#endif
			break;
		case ECU_DBG_OTHER:
			#if ECU_DEBUG_OTHER
				printf("%s==>%s: %s!\n",funname[type], msg1, msg2);
			#endif
			break;
	}
	
#endif
}

void printdecmsg(DebugType type,char *msg, int data)		//打印整形数据
{
#if ECU_DEBUG
	switch(type)
	{
		case ECU_DBG_UPDATE:
			#if ECU_DEBUG_UPDATE
				printf("%s==>%s: %d!\n",funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_IDWRITE:
			#if ECU_DEBUG_IDWRITE
				printf("%s==>%s: %d!\n",funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_MAIN:
			#if ECU_DEBUG_MAIN
				printf("%s==>%s: %d!\n",funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_CLIENT:
			#if ECU_DEBUG_CLIENT
				printf("%s==>%s: %d!\n",funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_CONTROL_CLIENT:
			#if ECU_DEBUG_CONTROL_CLIENT
				printf("%s==>%s: %d!\n",funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_NTP:
			#if ECU_DEBUG_NTP
				printf("%s==>%s: %d!\n",funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_OTHER:
			#if ECU_DEBUG_OTHER
				printf("%s==>%s: %d!\n",funname[type], msg, data);
			#endif
			break;
	}	
#endif
}

void printhexdatamsg(DebugType type,char *msg, int data)		//打印16进制数据,ZK
{
#if ECU_DEBUG
	switch(type)
	{
		case ECU_DBG_UPDATE:
			#if ECU_DEBUG_UPDATE
				printf("%s==>%s: %X!\n",funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_IDWRITE:
			#if ECU_DEBUG_IDWRITE
				printf("%s==>%s: %X!\n",funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_MAIN:
			#if ECU_DEBUG_MAIN
				printf("%s==>%s: %X!\n",funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_CLIENT:
			#if ECU_DEBUG_CLIENT
				printf("%s==>%s: %X!\n",funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_CONTROL_CLIENT:
			#if ECU_DEBUG_CONTROL_CLIENT
				printf("%s==>%s: %X!\n",funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_NTP:
			#if ECU_DEBUG_NTP
				printf("%s==>%s: %X!\n",funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_OTHER:
			#if ECU_DEBUG_OTHER
				printf("%s==>%s: %X!\n",funname[type], msg, data);
			#endif
			break;
	}
#endif
}

void printfloatmsg(DebugType type,char *msg, float data)		//打印实数
{
#if ECU_DEBUG
	switch(type)
	{
		case ECU_DBG_UPDATE:
			#if ECU_DEBUG_UPDATE
				printf("%s==>%s: %f!\n",funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_IDWRITE:
			#if ECU_DEBUG_IDWRITE
				printf("%s==>%s: %f!\n",funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_MAIN:
			#if ECU_DEBUG_MAIN
				printf("%s==>%s: %f!\n",funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_CLIENT:
			#if ECU_DEBUG_CLIENT
				printf("%s==>%s: %f!\n",funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_CONTROL_CLIENT:
			#if ECU_DEBUG_CONTROL_CLIENT
				printf("%s==>%s: %f!\n",funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_NTP:
			#if ECU_DEBUG_NTP
				printf("%s==>%s: %f!\n",funname[type], msg, data);
			#endif
			break;
		case ECU_DBG_OTHER:
			#if ECU_DEBUG_OTHER
				printf("%s==>%s: %f!\n",funname[type], msg, data);
			#endif
			break;
	}	
	
#endif
}

void printhexmsg(DebugType type,char *msg, char *data, int size)		//打印十六进制数据
{
#if ECU_DEBUG
	int i;
	switch(type)
	{
		case ECU_DBG_UPDATE:
			#if ECU_DEBUG_UPDATE
				printf("%s==>%s: ",funname[type], msg);
				for(i=0; i<size; i++)
					printf("%02X, ", data[i]);
				printf("\n");
			#endif
			break;
		case ECU_DBG_IDWRITE:
			#if ECU_DEBUG_IDWRITE
				printf("%s==>%s: ",funname[type], msg);
				for(i=0; i<size; i++)
					printf("%02X, ", data[i]);
				printf("\n");
			#endif
			break;
		case ECU_DBG_MAIN:
			#if ECU_DEBUG_MAIN
				printf("%s==>%s: ",funname[type], msg);
				for(i=0; i<size; i++)
					printf("%02X, ", data[i]);
				printf("\n");
			#endif
			break;
		case ECU_DBG_CLIENT:
			#if ECU_DEBUG_CLIENT
				printf("%s==>%s: ",funname[type], msg);
				for(i=0; i<size; i++)
					printf("%02X, ", data[i]);
				printf("\n");
			#endif
			break;
		case ECU_DBG_CONTROL_CLIENT:
			#if ECU_DEBUG_CONTROL_CLIENT
				printf("%s==>%s: ",funname[type], msg);
				for(i=0; i<size; i++)
					printf("%02X, ", data[i]);
				printf("\n");
			#endif
			break;
		case ECU_DBG_NTP:
			#if ECU_DEBUG_NTP
				printf("%s==>%s: ",funname[type], msg);
				for(i=0; i<size; i++)
					printf("%02X, ", data[i]);
				printf("\n");
			#endif
			break;
		case ECU_DBG_OTHER:
			#if ECU_DEBUG_OTHER
				printf("%s==>%s: ",funname[type], msg);
				for(i=0; i<size; i++)
					printf("%02X, ", data[i]);
				printf("\n");
			#endif
			break;
	}
#endif
}


void printecuinfo(ecu_info *ecu)
{
	print2msg(ECU_DBG_MAIN,"ECU ID", ecu->id);
	printhexdatamsg(ECU_DBG_MAIN,"PANID", ecu->panid);
	printhexdatamsg(ECU_DBG_MAIN,"Channel", ecu->channel);
	print2msg(ECU_DBG_MAIN,"IP", ecu->ip);
	printfloatmsg(ECU_DBG_MAIN,"Lifetime energy", ecu->life_energy);
	printfloatmsg(ECU_DBG_MAIN,"Current energy", ecu->current_energy);
	printdecmsg(ECU_DBG_MAIN,"System power", ecu->system_power);
	printdecmsg(ECU_DBG_MAIN,"Total", ecu->total);
	printdecmsg(ECU_DBG_MAIN,"Current count", ecu->count);
	printdecmsg(ECU_DBG_MAIN,"Type", ecu->type);
	printdecmsg(ECU_DBG_MAIN,"Zoneflag", ecu->zoneflag);
}

void printinverterinfo(inverter_info *inverter)
{
	printdecmsg(ECU_DBG_MAIN,"Inverter it", inverter->it);
	printfloatmsg(ECU_DBG_MAIN,"Inverter gf", inverter->gf);
	printdecmsg(ECU_DBG_MAIN,"Inverter curacctimet", inverter->curacctime);
	printfloatmsg(ECU_DBG_MAIN,"Inverter dv", inverter->dv);
	printfloatmsg(ECU_DBG_MAIN,"Inverter dvb", inverter->dvb);
	printfloatmsg(ECU_DBG_MAIN,"Inverter di", inverter->di);
	printfloatmsg(ECU_DBG_MAIN,"Inverter dib", inverter->dib);
	printfloatmsg(ECU_DBG_MAIN,"Inverter dic", inverter->dic);
	printfloatmsg(ECU_DBG_MAIN,"Inverter did", inverter->did);
	printdecmsg(ECU_DBG_MAIN,"Inverter gv", inverter->gv);
	printdecmsg(ECU_DBG_MAIN,"Inverter gvb", inverter->gvb);
	printdecmsg(ECU_DBG_MAIN,"Inverter gvc", inverter->gvc);
	printfloatmsg(ECU_DBG_MAIN,"Inverter curaccgen", inverter->curaccgen);
	printfloatmsg(ECU_DBG_MAIN,"Inverter curaccgenb", inverter->curaccgenb);
	printfloatmsg(ECU_DBG_MAIN,"Inverter curaccgenc", inverter->curaccgenc);
	printfloatmsg(ECU_DBG_MAIN,"Inverter curaccgend", inverter->curaccgend);
	printfloatmsg(ECU_DBG_MAIN,"Inverter reactive_power", inverter->reactive_power);
	printfloatmsg(ECU_DBG_MAIN,"Inverter reactive_powerb", inverter->reactive_powerb);
	printfloatmsg(ECU_DBG_MAIN,"Inverter reactive_powerc", inverter->reactive_powerc);
	printfloatmsg(ECU_DBG_MAIN,"Inverter active_power", inverter->active_power);
	printfloatmsg(ECU_DBG_MAIN,"Inverter active_powerb", inverter->active_powerb);
	printfloatmsg(ECU_DBG_MAIN,"Inverter active_powerc", inverter->active_powerc);
	printfloatmsg(ECU_DBG_MAIN,"Inverter output_energy", inverter->output_energy);
	printfloatmsg(ECU_DBG_MAIN,"Inverter output_energyb", inverter->cur_output_energyb);
	printfloatmsg(ECU_DBG_MAIN,"Inverter output_energyc", inverter->cur_output_energyc);
	printmsg(ECU_DBG_MAIN,"=================================================");
	printfloatmsg(ECU_DBG_MAIN,"preaccgen", inverter->preaccgen);
	printfloatmsg(ECU_DBG_MAIN,"preaccgenb", inverter->preaccgenb);
	printfloatmsg(ECU_DBG_MAIN,"preaccgenc", inverter->preaccgenc);
	printfloatmsg(ECU_DBG_MAIN,"preaccgend", inverter->preaccgend);
	printfloatmsg(ECU_DBG_MAIN,"pre_output_energy", inverter->pre_output_energy);
	printfloatmsg(ECU_DBG_MAIN,"pre_output_energyb", inverter->pre_output_energyb);
	printfloatmsg(ECU_DBG_MAIN,"pre_output_energyc", inverter->pre_output_energyc);
	printdecmsg(ECU_DBG_MAIN,"preacctime", inverter->preacctime);
	print2msg(ECU_DBG_MAIN,"last_report_time",inverter->last_report_time);
	printdecmsg(ECU_DBG_MAIN,"Inverter op", inverter->op);
	printdecmsg(ECU_DBG_MAIN,"Inverter opb", inverter->opb);
	printdecmsg(ECU_DBG_MAIN,"Inverter opc", inverter->opc);
	printmsg(ECU_DBG_MAIN,"=================================================");
}
