#include <stdio.h>
#include <stdlib.h>
#include "resolve.h"
#include <string.h>

extern ecu_info ecu;
int caltype;		//计算方式，NA版和非NA版的区别
extern float energy;

int resolvedata_250(char *inverter_data, struct inverter_info_t *inverter)
{
//	unsigned char temp[2] = {'\0'};
	int seconds;

	inverter->dv=(inverter_data[0]*256+inverter_data[1])*825/4096;
	inverter->di=(inverter_data[2]*256+inverter_data[3])*275/4096;
	inverter->gf=(inverter_data[4]*256+inverter_data[5])/10.0;

	inverter->curacctime = inverter_data[18]*256 + inverter_data[19];		//最近两次上报的累计时间的差值，即两次上报的时间间隔
	inverter->curaccgen = (float)(((inverter_data[13]*256 + inverter_data[14])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (inverter_data[15]*256.0*256.0 + inverter_data[16]*256.0 + inverter_data[17])/1661900.0*220.0/1000.0)/3600.0);

	if((inverter->curaccgen > inverter->preaccgen)&&(inverter->curacctime > inverter->preacctime)){
		seconds = inverter->curacctime - inverter->preacctime;
		inverter->duration = seconds;
		inverter->curgeneration = inverter->curaccgen - inverter->preaccgen;
	}
	else{
		seconds = inverter->curacctime;
		inverter->curgeneration = inverter->curaccgen;
	}

	inverter->preacctime = inverter->curacctime;
	inverter->preaccgen = inverter->curaccgen;

	inverter->output_energy = inverter->curgeneration;				//ZK

	if(inverter->curacctime > 600)
	{
		inverter->op = inverter->curgeneration * 1000.0 * 3600.0 / seconds;
	}
	else
	{
		inverter->op = (int)(inverter->dv*inverter->di/100.0);
	}


	if(inverter->op>260)
		inverter->op = (int)(inverter->dv*inverter->di/100.0);


	if(2 == caltype)
		inverter->gv=(inverter_data[6]*256+inverter_data[7]) / 2.93;
	else if(1 == caltype)
		inverter->gv=(inverter_data[6]*256+inverter_data[7])*352/1024;
	else
		inverter->gv=(inverter_data[6]*256+inverter_data[7])*687/1024;

	inverter->it=((inverter_data[8]*256+inverter_data[9])*330-245760)/4096;

	inverter->status_web[0]=((inverter_data[10]>>7)&0x01)+0x30;		//AC Frequency under Range 1bit
	inverter->status_web[1]=((inverter_data[10]>>6)&0x01)+0x30;		//AC Frequency exceeding Range 1bit
	inverter->status_web[2]=((inverter_data[10]>>5)&0x01)+0x30;		//AC Voltage exceeding Range 1bit
	inverter->status_web[3]=((inverter_data[10]>>4)&0x01)+0x30;		//AC Voltage under Range 1bit
	inverter->status_web[4]=((inverter_data[10]>>3)&0x01)+0x30;		//DC Voltage Too High 1bit
	inverter->status_web[5]=((inverter_data[10]>>2)&0x01)+0x30;		//DC Voltage Too Low 1bit
	inverter->status_web[6]='0';
	inverter->status_web[7]=(inverter_data[10]&0x01)+0x30;			//Over Critical Temperature 1bit
	inverter->status_web[8]='0';
	inverter->status_web[9]='0';
	inverter->status_web[10]='0';
	inverter->status_web[11]=((inverter_data[11]>>4)&0x01)+0x30;		//GFDI
	inverter->status_web[12]=((inverter_data[11]>>3)&0x01)+0x30;		//Remote-shut
	inverter->status_web[13]=((inverter_data[10]>>1)&0x01)+0x30;		//Active anti-island protection
	inverter->status_web[14]=((inverter_data[11]>>7)&0x01)+0x30;		//CP protection
	inverter->status_web[15]=((inverter_data[11]>>1)&0x01)+0x30;		//HV protection
	inverter->status_web[16]=(inverter_data[11]&0x01)+0x30;			//Over zero protection

	inverter->status[0]=inverter->status_web[0];
	inverter->status[1]=inverter->status_web[1];
	inverter->status[2]=inverter->status_web[2];
	inverter->status[3]=inverter->status_web[3];
	inverter->status[4]=inverter->status_web[4];
	inverter->status[5]=inverter->status_web[5];
	inverter->status[6]=inverter->status_web[7];
	inverter->status[7]=inverter->status_web[11];
	inverter->status[8]=inverter->status_web[12];
	inverter->status[9]='0';
	inverter->status[10]='0';

//	inverter->lastreporttime[0] = inverter_data[20];
//	inverter->lastreporttime[1] = inverter_data[21];

	if(inverter->last_gfdi_flag != inverter->status_web[11])
		inverter->gfdi_changed_flag = 1;
	else
		inverter->gfdi_changed_flag = 0;
	inverter->last_gfdi_flag = inverter->status_web[11];
	if(inverter->last_turn_on_off_flag != inverter->status_web[12])
		inverter->turn_on_off_changed_flag = 1;
	else
		inverter->turn_on_off_changed_flag = 0;
	inverter->last_turn_on_off_flag = inverter->status_web[12];

	check_yc200_yc250(inverter);


	/*

	inverter->it = (((data[0] & 0x0F) * 256 + data[1]) * 330 - 245760) / 4096;
	if((0==data[2])&&(0==data[3]))								//防止空数据传过来导致主程序出错
		inverter->gf = 0;
	else
		inverter->gf = 223750.0 / (data[2]*256 + data[3]);
	inverter->curacctime = data[7]*256 + data[8];
	inverter->dv = (((data[9] >> 4) & 0x0F) * 256 + ((data[9] << 4) & 0xF0) | ((data[10] >> 4) & 0x0F)) * 825 / 4096;
	inverter->di = (((data[12] >> 4) & 0x0F) * 256 + ((data[12] << 4) & 0xF0) | ((data[13] >> 4) & 0x0F)) * 275 / 4096;
	inverter->gv = (data[18]*256*256 + data[19]*256 + data[20]) / 26204.64;
	inverter->curaccgen = (float)(((data[27]*256 + data[28])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (data[29]*256.0*256.0 + data[30]*256.0 + data[31])/1661900.0*220.0/1000.0)/3600.0);
	inverter->reactive_power = (data[47] * 65535 + data[48] * 256 + data[49]) / 1712;
	inverter->active_power = (data[56] * 65535 + data[57] * 256 + data[58]) / 3314;
	inverter->cur_output_energy = (data[65] * 65535 + data[66] * 256 + data[67]) / 996952.0;


	if((inverter->cur_output_energy >= inverter->pre_output_energy)&&(inverter->curacctime >= inverter->preacctime))
	{
		inverter->output_energy=inverter->cur_output_energy-inverter->pre_output_energy;
	}
	else
	{
		inverter->output_energy=inverter->cur_output_energy;
	}
	inverter->pre_output_energy=inverter->cur_output_energy;

	if((inverter->curaccgen >= inverter->preaccgen)&&(inverter->curacctime >= inverter->preacctime))
	{
		seconds = inverter->curacctime - inverter->preacctime;
		inverter->curgeneration = inverter->curaccgen - inverter->preaccgen;
	}
	else
	{
		seconds = inverter->curacctime;
		inverter->curgeneration = inverter->curaccgen;
	}

	inverter->preacctime = inverter->curacctime;
	inverter->preaccgen = inverter->curaccgen;



	if(0==seconds)//防止空数据传过来导致主程序出错
	{
		inverter->op = 0;
	}

	if(inverter->curacctime > 600)		//防止早上起来功率爆点，所以在启动10分钟后才采用时间差的功率计算模式，ZK
	{
		inverter->op = inverter->curgeneration * 1000.0 * 3600.0 / seconds;
	}
	else
	{
		inverter->op = (int)(inverter->dv*inverter->di/100.0);
	}

	if(inverter->op>260)
		inverter->op = (int)(inverter->dv*inverter->di/100.0);


	inverter->status_web[0]=(data[6]&0x01)+0x30;			//AC Frequency exceeding Range 1bit
	inverter->status_web[1]=((data[6]>>1)&0x01)+0x30;		//AC Frequency under Range 1bit
	inverter->status_web[2]=((data[6]>>2)&0x01)+0x30;		//AC-A Voltage exceeding Range 1bit
	inverter->status_web[3]=((data[6]>>3)&0x01)+0x30;		//AC-A Voltage under Range 1bit
	inverter->status_web[4]='0';
	inverter->status_web[5]='0';
	inverter->status_web[6]='0';
	inverter->status_web[7]='0';
	inverter->status_web[8]=(data[5]&0x01)+0x30;			//DC-A Voltage Too High 1bit
	inverter->status_web[9]=((data[5]>>1)&0x01)+0x30;		//DC-A Voltage Too Low 1bit
	inverter->status_web[10]='0';
	inverter->status_web[11]='0';
	inverter->status_web[12]='0';
	inverter->status_web[13]='0';
	inverter->status_web[14]='0';
	inverter->status_web[15]='0';
	inverter->status_web[16]=(data[4]&0x01)+0x30;			//Over Critical Temperature 1bit
	inverter->status_web[17]=((data[4]>>1)&0x01)+0x30;		//GFDI
	inverter->status_web[18]=((data[4]>>2)&0x01)+0x30;		//Remote-shut
	inverter->status_web[19]=((data[4]>>3)&0x01)+0x30;		//AC-Disconnect
	inverter->status_web[20]='0';
	inverter->status_web[21]='0';
	inverter->status_web[22]='0';
	inverter->status_web[23]='0';


	inverter->status[0]=inverter->status_web[1];			//交流端频率欠频
	inverter->status[1]=inverter->status_web[0];			//交流端频率过频
	inverter->status[2]=inverter->status_web[2];			//交流端电压过压
	inverter->status[3]=inverter->status_web[3];			//交流端电压欠压
	inverter->status[4]=inverter->status_web[8];			//直流端A路过压
	inverter->status[5]=inverter->status_web[9];			//直流端A路欠压
	inverter->status[6]=inverter->status_web[16];			//温度过高
	inverter->status[7]=inverter->status_web[17];			//GFDI
	inverter->status[8]=inverter->status_web[18];			//远程关闭
	inverter->status[9]=inverter->status_web[19];			//交流关闭
	inverter->status[10] = '0';

	if('1'==inverter->status_web[19])
	{
		if(1==inverter->model)
			inverter->gf = 50.0;
		if(2==inverter->model)
			inverter->gf = 60.0;

		inverter->gv=0;
	}

	if(inverter->last_gfdi_flag != inverter->status_web[17])
		inverter->gfdi_changed_flag = 1;
	else
		inverter->gfdi_changed_flag = 0;
	inverter->last_gfdi_flag = inverter->status_web[17];
	if(inverter->last_turn_on_off_flag != inverter->status_web[18])
		inverter->turn_on_off_changed_flag = 1;
	else
		inverter->turn_on_off_changed_flag = 0;
	inverter->last_turn_on_off_flag = inverter->status_web[18];


	check_yc200_yc250(inverter);			//ZK

	strcpy(inverter->last_report_time , ecu.broadcast_time);
*/
	return 1;
}


int resolvedata_500(char *data, struct inverter_info_t *inverter)
{
//	unsigned char temp[2] = {'\0'};
	int seconds;

	inverter->it = (((data[0] & 0x0F) * 256 + data[1]) * 330 - 245760) / 4096;
	if((0==data[2])&&(0==data[3]))								//防止空数据传过来导致主程序出错
		inverter->gf = 0;
	else
	{
		inverter->gf = 223750.0 / (data[2]*256 + data[3]);
	}
	inverter->curacctime = data[7]*256 + data[8];
	inverter->dv = (((data[9] >> 4) & 0x0F) * 256 + ((data[9] << 4) & 0xF0) | ((data[10] >> 4) & 0x0F)) * 825 / 4096;
	inverter->dvb = ((data[10] & 0x0F) * 256 + data[11]) * 825 / 4096;
	inverter->di = (((data[12] >> 4) & 0x0F) * 256 + ((data[12] << 4) & 0xF0) | ((data[13] >> 4) & 0x0F)) * 275 / 4096;
	inverter->dib = ((data[13] & 0x0F) * 256 + data[14]) * 275 / 4096;
	inverter->gv = (data[18]*256*256 + data[19]*256 + data[20]) / 26204.64;
	inverter->curaccgen = (float)(((data[27]*256 + data[28])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (data[29]*256.0*256.0 + data[30]*256.0 + data[31])/1642397.0*220.0/1000.0)/3600.0);
	inverter->curaccgenb = (float)(((data[32]*256 + data[33])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (data[34]*256.0*256.0 + data[35]*256.0 + data[36])/1642397.0*220.0/1000.0)/3600.0);
	inverter->reactive_power = (data[47] * 65535 + data[48] * 256 + data[49]) / 1712;
	inverter->active_power = (data[56] * 65535 + data[57] * 256 + data[58]) / 3360;
	inverter->cur_output_energy = (data[65] * 65535 + data[66] * 256 + data[67]) / 1009687.0;


	if((inverter->cur_output_energy >= inverter->pre_output_energy)&&(inverter->curacctime >= inverter->preacctime))
	{
		inverter->output_energy=inverter->cur_output_energy-inverter->pre_output_energy;
	}
	else
	{
		inverter->output_energy=inverter->cur_output_energy;
	}
	inverter->pre_output_energy=inverter->cur_output_energy;

	if((inverter->curaccgen >= inverter->preaccgen)&&(inverter->curaccgenb >= inverter->preaccgenb)&&(inverter->curacctime >= inverter->preacctime))
	{
		seconds = inverter->curacctime - inverter->preacctime;
		inverter->curgeneration = inverter->curaccgen - inverter->preaccgen;
		inverter->curgenerationb = inverter->curaccgenb - inverter->preaccgenb;
	}
	else
	{
		seconds = inverter->curacctime;
		inverter->curgeneration = inverter->curaccgen;
		inverter->curgenerationb = inverter->curaccgenb;
	}

	inverter->preacctime = inverter->curacctime;
	inverter->preaccgen = inverter->curaccgen;
	inverter->preaccgenb = inverter->curaccgenb;

	if(0==seconds)//防止空数据传过来导致主程序出错
	{
		inverter->op = 0;
		inverter->opb = 0;
	}


	if(inverter->curacctime > 600)		//防止早上起来功率爆点，所以在启动10分钟后才采用时间差的功率计算模式，ZK
	{
		inverter->op = inverter->curgeneration * 1000.0 * 3600.0 / seconds;
		inverter->opb = inverter->curgenerationb * 1000.0 * 3600.0 / seconds;
	}
	else
	{
		inverter->op = (int)(inverter->dv*inverter->di/100.0);
		inverter->opb = (int)(inverter->dvb*inverter->dib/100.0);
	}


	if(inverter->op>260)
		inverter->op = (int)(inverter->dv*inverter->di/100.0);
	if(inverter->opb>260)
		inverter->opb = (int)(inverter->dvb*inverter->dib/100.0);


	inverter->status_web[0]=(data[6]&0x01)+0x30;			//AC Frequency exceeding Range 1bit
	inverter->status_web[1]=((data[6]>>1)&0x01)+0x30;		//AC Frequency under Range 1bit
	inverter->status_web[2]=((data[6]>>2)&0x01)+0x30;		//AC-A Voltage exceeding Range 1bit
	inverter->status_web[3]=((data[6]>>3)&0x01)+0x30;		//AC-A Voltage under Range 1bit
	inverter->status_web[4]='0';
	inverter->status_web[5]='0';
	inverter->status_web[6]='0';
	inverter->status_web[7]='0';
	inverter->status_web[8]=(data[5]&0x01)+0x30;			//DC-A Voltage Too High 1bit
	inverter->status_web[9]=((data[5]>>1)&0x01)+0x30;		//DC-A Voltage Too Low 1bit
	inverter->status_web[10]=((data[5]>>2)&0x01)+0x30;		//DC-B Voltage Too High 1bit
	inverter->status_web[11]=((data[5]>>3)&0x01)+0x30;		//DC-B Voltage Too Low 1bit
	inverter->status_web[12]='0';
	inverter->status_web[13]='0';
	inverter->status_web[14]='0';
	inverter->status_web[15]='0';
	inverter->status_web[16]=(data[4]&0x01)+0x30;			//Over Critical Temperature 1bit
	inverter->status_web[17]=((data[4]>>1)&0x01)+0x30;		//GFDI
	inverter->status_web[18]=((data[4]>>2)&0x01)+0x30;		//Remote-shut
	inverter->status_web[19]=((data[4]>>3)&0x01)+0x30;		//AC-Disconnect
	inverter->status_web[20]='0';
	inverter->status_web[21]='0';
	inverter->status_web[22]='0';
	inverter->status_web[23]='0';


	inverter->status[0]=inverter->status_web[1];			//交流端频率欠频
	inverter->status[1]=inverter->status_web[0];			//交流端频率过频
	inverter->status[2]=inverter->status_web[2];			//交流端电压过压
	inverter->status[3]=inverter->status_web[3];			//交流端电压欠压
	inverter->status[4]=inverter->status_web[8];			//直流端A路过压
	inverter->status[5]=inverter->status_web[9];			//直流端A路欠压
	inverter->status[6]=inverter->status_web[16];			//温度过高
	inverter->status[7]=inverter->status_web[17];			//GFDI
	inverter->status[8]=inverter->status_web[18];			//远程关闭
	inverter->status[9]=inverter->status_web[19];			//交流关闭
	inverter->status[10] = '0';

	inverter->statusb[0]=inverter->status_web[1];			//交流端频率欠频
	inverter->statusb[1]=inverter->status_web[0];			//交流端频率过频
	inverter->statusb[2]=inverter->status_web[2];			//交流端电压过压
	inverter->statusb[3]=inverter->status_web[3];			//交流端电压欠压
	inverter->statusb[4]=inverter->status_web[10];			//直流端B路过压
	inverter->statusb[5]=inverter->status_web[11];			//直流端B路欠压
	inverter->statusb[6]=inverter->status_web[16];			//温度过高
	inverter->statusb[7]=inverter->status_web[17];			//GFDI
	inverter->statusb[8]=inverter->status_web[18];			//远程关闭
	inverter->statusb[9]=inverter->status_web[19];			//交流关闭
	inverter->statusb[10] = '0';

	if('1'==inverter->status_web[19])
	{
		if(3==inverter->model)
			inverter->gf = 50.0;
		if(4==inverter->model)
			inverter->gf = 60.0;

		inverter->gv=0;
	}

	if(inverter->last_gfdi_flag != inverter->status_web[17])
		inverter->gfdi_changed_flag = 1;
	else
		inverter->gfdi_changed_flag = 0;
	inverter->last_gfdi_flag = inverter->status_web[17];
	if(inverter->last_turn_on_off_flag != inverter->status_web[18])
		inverter->turn_on_off_changed_flag = 1;
	else
		inverter->turn_on_off_changed_flag = 0;
	inverter->last_turn_on_off_flag = inverter->status_web[18];


	check_yc500(inverter);				//ZK

	strcpy(inverter->last_report_time , ecu.broadcast_time);

	return 1;
}

int resolvedata_1000(char *data, struct inverter_info_t *inverter)
{
//	unsigned char temp[2] = {'\0'};
	int seconds;

	inverter->it = (((data[0] & 0x0F) * 256 + data[1]) * 330 - 245760) / 4096;
	if((0==data[2])&&(0==data[3]))//防止空数据传过来导致主程序出错
		inverter->gf = 0;
	else
		inverter->gf = 256000.0 / (data[2]*256 + data[3]);
	inverter->curacctime = data[7]*256 + data[8];
	inverter->dv = (((data[9] >> 4) & 0x0F) * 256 + ((data[9] << 4) & 0xF0) | ((data[10] >> 4) & 0x0F)) * 825 / 4096;
	inverter->dvb = inverter->dvc = inverter->dvd = inverter->dv;
	inverter->di = (((data[12] >> 4) & 0x0F) * 256 + ((data[12] << 4) & 0xF0) | ((data[13] >> 4) & 0x0F)) * 275 / 4096;
	inverter->dib = ((data[13] & 0x0F) * 256 + data[14]) * 275 / 4096;
	inverter->dic = (((data[15] >> 4) & 0x0F) * 256 + ((data[15] << 4) & 0xF0) | ((data[16] >> 4) & 0x0F)) * 275 / 4096;
	inverter->did = ((data[16] & 0x0F) * 256 + data[17]) * 275 / 4096;
	inverter->gv = (data[18]*256*256 + data[19]*256 + data[20]) / 11614.45;
	inverter->gvb = (data[21]*256*256 + data[22]*256 + data[23]) / 11614.45;
	inverter->gvc = (data[24]*256*256 + data[25]*256 + data[26]) / 11614.45;
	inverter->curaccgen = (float)(((data[27]*256 + data[28])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (data[29]*256.0*256.0 + data[30]*256.0 + data[31])/1661900.0*220.0/1000.0)/3600.0);
	inverter->curaccgenb = (float)(((data[32]*256 + data[33])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (data[34]*256.0*256.0 + data[35]*256.0 + data[36])/1661900.0*220.0/1000.0)/3600.0);
	inverter->curaccgenc = (float)(((data[37]*256 + data[38])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (data[39]*256.0*256.0 + data[40]*256.0 + data[41])/1661900.0*220.0/1000.0)/3600.0);
	inverter->curaccgend = (float)(((data[42]*256 + data[43])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (data[44]*256.0*256.0 + data[45]*256.0 + data[46])/1661900.0*220.0/1000.0)/3600.0);
	inverter->reactive_power = (data[47] * 65535 + data[48] * 256 + data[49]) / 1712;
	inverter->reactive_powerb = (data[50] * 65535 + data[51] * 256 + data[52]) / 1712;
	inverter->reactive_powerc = (data[53] * 65535 + data[54] * 256 + data[55]) / 1712;
	inverter->active_power = (data[56] * 65535 + data[57] * 256 + data[58]) / 959.57;
	inverter->active_powerb = (data[59] * 65535 + data[60] * 256 + data[61]) / 959.57;
	inverter->active_powerc = (data[62] * 65535 + data[63] * 256 + data[64]) / 959.57;
	inverter->cur_output_energy = (data[65] * 65535 + data[66] * 256 + data[67]) / 93267.5;
	inverter->cur_output_energyb = (data[68] * 65535 + data[69] * 256 + data[70]) / 93267.5;
	inverter->cur_output_energyc = (data[71] * 65535 + data[72] * 256 + data[73]) / 93267.5;

//	printfloatmsg("DEBUG-1 output_energy", inverter->output_energy);

	if((inverter->cur_output_energy >= inverter->pre_output_energy)&&(inverter->cur_output_energyb >= inverter->pre_output_energyb)&&(inverter->cur_output_energyc >= inverter->pre_output_energyc)&&(inverter->curacctime >= inverter->preacctime))
	{
		inverter->output_energy=inverter->cur_output_energy-inverter->pre_output_energy;
		inverter->output_energyb=inverter->cur_output_energyb-inverter->pre_output_energyb;
		inverter->output_energyc=inverter->cur_output_energyc-inverter->pre_output_energyc;
//		printfloatmsg("DEBUG-2 output_energy", inverter->output_energy);
	}
	else
	{
		inverter->output_energy=inverter->cur_output_energy;
		inverter->output_energyb=inverter->cur_output_energyb;
		inverter->output_energyc=inverter->cur_output_energyc;
		printfloatmsg("DEBUG-3 output_energy", inverter->output_energy);
	}
	inverter->pre_output_energy=inverter->cur_output_energy;
	inverter->pre_output_energyb=inverter->cur_output_energyb;
	inverter->pre_output_energyc=inverter->cur_output_energyc;


	if((inverter->curaccgen > inverter->preaccgen)&&(inverter->curaccgenb > inverter->preaccgenb)&&(inverter->curaccgenc > inverter->preaccgenc)&&(inverter->curaccgend > inverter->preaccgend)&&(inverter->curacctime > inverter->preacctime))
	{
		seconds = inverter->curacctime - inverter->preacctime;
		inverter->curgeneration = inverter->curaccgen - inverter->preaccgen;
		inverter->curgenerationb = inverter->curaccgenb - inverter->preaccgenb;
		inverter->curgenerationc = inverter->curaccgenc - inverter->preaccgenc;
		inverter->curgenerationd = inverter->curaccgend - inverter->preaccgend;
	}
	else
	{
		seconds = inverter->curacctime;
		inverter->curgeneration = inverter->curaccgen;
		inverter->curgenerationb = inverter->curaccgenb;
		inverter->curgenerationc = inverter->curaccgenc;
		inverter->curgenerationd = inverter->curaccgend;
	}

	inverter->preacctime = inverter->curacctime;
	inverter->preaccgen = inverter->curaccgen;
	inverter->preaccgenb = inverter->curaccgenb;
	inverter->preaccgenc = inverter->curaccgenc;
	inverter->preaccgend = inverter->curaccgend;

	if(0==seconds)											//防止空数据传过来导致主程序出错
	{
		inverter->op = 0;
		inverter->opb = 0;
		inverter->opc = 0;
		inverter->opd = 0;
	}

	if(inverter->curacctime > 600)		//防止早上起来功率爆点，所以在启动10分钟后才采用时间差的功率计算模式，ZK
	{
		inverter->op = inverter->curgeneration * 1000.0 * 3600.0 / seconds;
		inverter->opb = inverter->curgenerationb * 1000.0 * 3600.0 / seconds;
		inverter->opc = inverter->curgenerationc * 1000.0 * 3600.0 / seconds;
		inverter->opd = inverter->curgenerationd * 1000.0 * 3600.0 / seconds;
	}
	else
	{
		inverter->op = (int)(inverter->dv*inverter->di/100.0);
		inverter->opb = (int)(inverter->dvb*inverter->dib/100.0);
		inverter->opc = (int)(inverter->dvc*inverter->dic/100.0);
		inverter->opd = (int)(inverter->dvd*inverter->did/100.0);
	}


	if(inverter->op>260)
		inverter->op = (int)(inverter->dv*inverter->di/100.0);
	if(inverter->opb>260)
		inverter->opb = (int)(inverter->dvb*inverter->dib/100.0);
	if(inverter->opc>260)
		inverter->opc = (int)(inverter->dvc*inverter->dic/100.0);
	if(inverter->opd>260)
		inverter->opd = (int)(inverter->dvd*inverter->did/100.0);



	inverter->status_web[0]=(data[6]&0x01)+0x30;			//AC Frequency exceeding Range 1bit
	inverter->status_web[1]=((data[6]>>1)&0x01)+0x30;		//AC Frequency under Range 1bit
	inverter->status_web[2]=((data[6]>>2)&0x01)+0x30;		//AC-A Voltage exceeding Range 1bit
	inverter->status_web[3]=((data[6]>>3)&0x01)+0x30;		//AC-A Voltage under Range 1bit
	inverter->status_web[4]=((data[6]>>4)&0x01)+0x30;		//AC-B Voltage exceeding Range 1bit
	inverter->status_web[5]=((data[6]>>5)&0x01)+0x30;		//AC-B Voltage under Range 1bit
	inverter->status_web[6]=((data[6]>>6)&0x01)+0x30;		//AC-C Voltage exceeding Range 1bit
	inverter->status_web[7]=((data[6]>>7)&0x01)+0x30;		//AC-C Voltage under Range 1bit
	inverter->status_web[8]=(data[5]&0x01)+0x30;			//DC Voltage Too High 1bit
	inverter->status_web[9]=((data[5]>>1)&0x01)+0x30;		//DC Voltage Too Low 1bit
	inverter->status_web[10]='0';
	inverter->status_web[11]='0';
	inverter->status_web[12]='0';
	inverter->status_web[13]='0';
	inverter->status_web[14]='0';
	inverter->status_web[15]='0';
	inverter->status_web[16]=(data[4]&0x01)+0x30;			//Over Critical Temperature 1bit
	inverter->status_web[17]=((data[4]>>1)&0x01)+0x30;		//GFDI
	inverter->status_web[18]=((data[4]>>2)&0x01)+0x30;		//Remote-shut
	inverter->status_web[19]=((data[4]>>3)&0x01)+0x30;		//AC-Disconnect
	inverter->status_web[20]='0';
	inverter->status_web[21]='0';
	inverter->status_web[22]='0';
	inverter->status_web[23]='0';

	inverter->status[0]=inverter->status_web[1];			//交流端频率欠频
	inverter->status[1]=inverter->status_web[0];			//交流端频率过频
	inverter->status[2]=inverter->status_web[2];			//交流端A路电压过压
	inverter->status[3]=inverter->status_web[3];			//交流端A路电压欠压
	inverter->status[4]=inverter->status_web[8];			//直流端过压
	inverter->status[5]=inverter->status_web[9];			//直流端欠压
	inverter->status[6]=inverter->status_web[16];			//温度过高
	inverter->status[7]=inverter->status_web[17];			//GFDI
	inverter->status[8]=inverter->status_web[18];			//远程关闭
	inverter->status[9]=inverter->status_web[19];			//交流关闭
	inverter->status[10] = '0';

	inverter->statusb[0]=inverter->status_web[1];			//交流端频率欠频
	inverter->statusb[1]=inverter->status_web[0];			//交流端频率过频
	inverter->statusb[2]=inverter->status_web[4];			//交流端B路电压过压
	inverter->statusb[3]=inverter->status_web[5];			//交流端B路电压欠压
	inverter->statusb[4]=inverter->status_web[8];			//直流端过压
	inverter->statusb[5]=inverter->status_web[9];			//直流端欠压
	inverter->statusb[6]=inverter->status_web[16];			//温度过高
	inverter->statusb[7]=inverter->status_web[17];			//GFDI
	inverter->statusb[8]=inverter->status_web[18];			//远程关闭
	inverter->statusb[9]=inverter->status_web[19];			//交流关闭
	inverter->statusb[10] = '0';

	inverter->statusc[0]=inverter->status_web[1];			//交流端频率欠频
	inverter->statusc[1]=inverter->status_web[0];			//交流端频率过频
	inverter->statusc[2]=inverter->status_web[6];			//交流端C路电压过压
	inverter->statusc[3]=inverter->status_web[7];			//交流端C路电压欠压
	inverter->statusc[4]=inverter->status_web[8];			//直流端过压
	inverter->statusc[5]=inverter->status_web[9];			//直流端欠压
	inverter->statusc[6]=inverter->status_web[16];			//温度过高
	inverter->statusc[7]=inverter->status_web[17];			//GFDI
	inverter->statusc[8]=inverter->status_web[18];			//远程关闭
	inverter->statusc[9]=inverter->status_web[19];			//交流关闭
	inverter->statusc[10] = '0';

	if('1'==inverter->status_web[19])
	{
		if(5==inverter->model)
			inverter->gf = 50.0;
		if(6==inverter->model)
			inverter->gf = 60.0;

		inverter->gv=0;
		inverter->gvb=0;
		inverter->gvc=0;
	}

	if(inverter->last_gfdi_flag != inverter->status_web[17])
		inverter->gfdi_changed_flag = 1;
	else
		inverter->gfdi_changed_flag = 0;
	inverter->last_gfdi_flag = inverter->status_web[17];
	if(inverter->last_turn_on_off_flag != inverter->status_web[18])
		inverter->turn_on_off_changed_flag = 1;
	else
		inverter->turn_on_off_changed_flag = 0;
	inverter->last_turn_on_off_flag = inverter->status_web[18];


	check_yc1000(inverter);				//ZK

	strcpy(inverter->last_report_time , ecu.broadcast_time);
//	printinverterinfo(inverter);
	return 1;

}

int resolvedata_optimizer_YC500(char *inverter_data, struct inverter_info_t *inverter)
{
	int i;
	/*
	inverter->output_voltage = inverter_data[1]*65535+inverter_data[2]*256+inverter_data[3];
	inverter->input_voltage_pv1 = inverter_data[5]*65535+inverter_data[6]*256+inverter_data[7];
	inverter->input_voltage_pv2 = inverter_data[9]*65535+inverter_data[10]*256+inverter_data[11];

	inverter->output_current = inverter_data[22]*256+inverter_data[23];
	inverter->output_power = inverter_data[25]*65535+inverter_data[26]*256+inverter_data[27];
	inverter->input_power_pv1 = inverter_data[29]*65535+inverter_data[30]*256+inverter_data[31];
	inverter->input_power_pv2 = inverter_data[33]*65535+inverter_data[34]*256+inverter_data[35];
	inverter->input_current_pv1 = (inverter->input_power_pv1/inverter->input_voltage_pv1)*1000;		//因为是mA级别，所以要乘以1000，ZK
	inverter->input_current_pv2 = (inverter->input_power_pv2/inverter->input_voltage_pv2)*1000;
	*/
	inverter->output_voltage = (inverter_data[0]*256+inverter_data[1])/10.0;
	inverter->input_voltage_pv1 = (inverter_data[2]*256+inverter_data[3])/10.0;
	inverter->input_voltage_pv2 = (inverter_data[4]*256+inverter_data[5])/10.0;
	inverter->output_power = (inverter_data[6]*256+inverter_data[7])/10.0;
	inverter->input_power_pv1 = (inverter_data[8]*256+inverter_data[9])/10.0;
	inverter->input_power_pv2 =(inverter_data[10]*256+inverter_data[11])/10.0;
	inverter->output_current = (inverter_data[12]*256+inverter_data[13])/10.0;
	inverter->input_current_pv1=(inverter_data[14]*256+inverter_data[15])/1.0;
	inverter->input_current_pv2=(inverter_data[16]*256+inverter_data[17])/1.0;
	inverter->temperature =(inverter_data[18]*256+inverter_data[19])/1.0;
	inverter->cur_output_energy_optimizer=(inverter_data[20]*256*256*256+inverter_data[21]*65536+inverter_data[22]*256+inverter_data[23])/36000000.0;
	inverter->cur_input_energy_optimizer_pv1=(inverter_data[24]*256*256*256+inverter_data[25]*65536+inverter_data[26]*256+inverter_data[27])/36000000.0;
	inverter->cur_input_energy_optimizer_pv2=(inverter_data[28]*256*256*256+inverter_data[29]*65536+inverter_data[30]*256+inverter_data[31])/36000000.0;
	inverter->turn_on_off_status_optimizer=(inverter_data[34]*256+inverter_data[35]);
	inverter->version=(inverter_data[32]*256+inverter_data[33]);
//	update_inverter_model_version(inverter);
	
	for(i=0;i<24;i++)inverter->status_web[i]='0';
	if(inverter->turn_on_off_status_optimizer==1)
		inverter->status_web[18]='1';
	else if(inverter->turn_on_off_status_optimizer==1)
		inverter->status_web[18]='0';
	if(inverter->last_turn_on_off_flag != inverter->status_web[18])
		inverter->turn_on_off_changed_flag = 1;
	else
		inverter->turn_on_off_changed_flag = 0;
	inverter->last_turn_on_off_flag = inverter->status_web[18];

	printf("\ncur=%f\t",inverter->cur_output_energy_optimizer);printf("cur=%f\t",inverter->cur_input_energy_optimizer_pv1);printf("cur=%f\n",inverter->cur_input_energy_optimizer_pv2);
	printf("pre=%f\t",inverter->pre_output_energy_optimizer);printf("pre=%f\t",inverter->pre_input_energy_optimizer_pv1);printf("pre=%f\n",inverter->pre_input_energy_optimizer_pv2);
	if(inverter->pre_output_energy_optimizer>(inverter->cur_output_energy_optimizer))
		inverter->cur_output_energy_optimizer+=inverter->pre_output_energy_optimizer;
	if(inverter->pre_output_energy_optimizer!=0)
		inverter->output_energy_optimizer=inverter->cur_output_energy_optimizer-inverter->pre_output_energy_optimizer;

	if(inverter->pre_input_energy_optimizer_pv1>(inverter->cur_input_energy_optimizer_pv1))
		inverter->cur_input_energy_optimizer_pv1+=inverter->pre_input_energy_optimizer_pv1;
	if(inverter->pre_input_energy_optimizer_pv1!=0)
		inverter->input_energy_optimizer_pv1=inverter->cur_input_energy_optimizer_pv1-inverter->pre_input_energy_optimizer_pv1;

	if(inverter->pre_input_energy_optimizer_pv2>(inverter->cur_input_energy_optimizer_pv2))
		inverter->cur_input_energy_optimizer_pv2+=inverter->pre_input_energy_optimizer_pv2;
	if(inverter->pre_input_energy_optimizer_pv2!=0)
		inverter->input_energy_optimizer_pv2=inverter->cur_input_energy_optimizer_pv2-inverter->pre_input_energy_optimizer_pv2;
printf("A-B=%f\t",inverter->output_energy_optimizer);printf("A-B=%f\t",inverter->input_energy_optimizer_pv1);printf("A-B=%f\n\n",inverter->input_energy_optimizer_pv2);
	inverter->pre_output_energy_optimizer=(inverter_data[20]*256*256*256+inverter_data[21]*65536+inverter_data[22]*256+inverter_data[23])/36000000.0;
	inverter->pre_input_energy_optimizer_pv1=(inverter_data[24]*256*256*256+inverter_data[25]*65536+inverter_data[26]*256+inverter_data[27])/36000000.0;
	inverter->pre_input_energy_optimizer_pv2=(inverter_data[28]*256*256*256+inverter_data[29]*65536+inverter_data[30]*256+inverter_data[31])/36000000.0;
	printf("pre=%f\n",inverter->pre_output_energy_optimizer);
	printf("cur=%f",inverter->cur_output_energy_optimizer);
	//inverter->input_energy_pv2=(inverter->cur_output_energy)/2.0;
	if(
			((inverter_data[6]*256+inverter_data[7])==4040)
//			||
//			((inverter_data[0]==0xff)&&(inverter_data[1]==0xff))||
//			((inverter_data[2]==0xff)&&(inverter_data[3]==0xff))||
//			((inverter_data[4]==0xff)&&(inverter_data[5]==0xff))||
//			((inverter_data[8]==0xff)&&(inverter_data[9]==0xff))||
//			((inverter_data[10]==0xff)&&(inverter_data[11]==0xff))||
//			(inverter_data[12]==0xff)
			)
	{
		inverter->dataflag=1;
	}

	check_optimizer_YC500(inverter);

	return 1;
}
