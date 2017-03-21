#include <stdio.h>
#include <stdlib.h>
#include "resolve.h"
#include <string.h>

extern ecu_info ecu;
int caltype;		//计算方式，NA版和非NA版的区别
extern float energy;

float sqrt(float number) {
    float new_guess;
    float last_guess;
    
    if (number < 0) {
        printf("Cannot compute the square root of a negative number!\n");
        return -1;
    }
    
    new_guess = 1;
    do {
        last_guess = new_guess;
        new_guess = (last_guess + number / last_guess) / 2;
        printf("%.15e\n", new_guess);
    } while (new_guess != last_guess);
    
    return new_guess;
}

void yc600_status(struct inverter_info_t *inverter)
{
	memset(inverter->status_ema, '\0', sizeof(inverter->status_ema));
	inverter->status_send_flag=0;

	if(('1' == inverter->status_web[0]) || ('1' == inverter->status_web[1]) || ('1' == inverter->status_web[2]) ||
			('1' == inverter->status_web[3]) || ('1' == inverter->status_web[4]) || ('1' == inverter->status_web[5]) ||
			('1' == inverter->status_web[6]) || ('1' == inverter->status_web[7]) || ('1' == inverter->status_web[16]) ||
			('1' == inverter->status_web[17]) || ('1' == inverter->status_web[18])|| ('1' == inverter->status_web[19]) ||
			('1' == inverter->status_web[21]) || ('1' == inverter->status_web[22])){
		strcat(inverter->status_ema, inverter->id);
		strcat(inverter->status_ema, "04");
		if(('1' == inverter->status_web[0]) || ('1' == inverter->status_web[1]) ||  ('1' == inverter->status_web[19]) ||
			('1' == inverter->status_web[21]) || ('1' == inverter->status_web[22]) || ('1' == inverter->status_web[23]) || ('1' == inverter->status_web[24]))
			strcat(inverter->status_ema, "1");		//AC??
		else
			strcat(inverter->status_ema, "0");
		if('1' == inverter->status_web[16])			//????
			strcat(inverter->status_ema, "1");
		else
			strcat(inverter->status_ema, "0");		//????????
		if('1' == inverter->status_web[17])
			strcat(inverter->status_ema, "1");
		else
			strcat(inverter->status_ema, "0");
		if('1' == inverter->status_web[18])			//??????
			strcat(inverter->status_ema, "1");
		else
			strcat(inverter->status_ema, "0");

		inverter->status_ema[18] = inverter->status_web[0];
		inverter->status_ema[19] = inverter->status_web[1];
		inverter->status_ema[20] = inverter->status_web[19];
		inverter->status_ema[21] = inverter->status_web[22];
		inverter->status_ema[22] = inverter->status_web[21];
		inverter->status_ema[23] = inverter->status_web[16];
		inverter->status_ema[24] = inverter->status_web[17];
		inverter->status_ema[25] = inverter->status_web[18];
		inverter->status_ema[26] = '0';
		inverter->status_ema[27] = inverter->status_web[23];
		inverter->status_ema[28] = inverter->status_web[24];
		strcat(inverter->status_ema, "END");

		inverter->status_send_flag=1;
	}
}

void yc1000_status(struct inverter_info_t *inverter)
{
	memset(inverter->status_ema, '\0', sizeof(inverter->status_ema));
	inverter->status_send_flag=0;

	if(('1' == inverter->status_web[0]) || ('1' == inverter->status_web[1]) || ('1' == inverter->status_web[2]) ||
			('1' == inverter->status_web[3]) || ('1' == inverter->status_web[4]) || ('1' == inverter->status_web[5]) ||
			('1' == inverter->status_web[6]) || ('1' == inverter->status_web[7]) || ('1' == inverter->status_web[16]) ||
			('1' == inverter->status_web[17]) || ('1' == inverter->status_web[18])|| ('1' == inverter->status_web[19]) ||
			('1' == inverter->status_web[21]) || ('1' == inverter->status_web[22])){
		strcat(inverter->status_ema, inverter->id);
		strcat(inverter->status_ema, "04");
		if(('1' == inverter->status_web[0]) || ('1' == inverter->status_web[1]) || ('1' == inverter->status_web[2]) ||
				('1' == inverter->status_web[3]) || ('1' == inverter->status_web[4]) || ('1' == inverter->status_web[5]) ||
				('1' == inverter->status_web[6]) || ('1' == inverter->status_web[7]) || ('1' == inverter->status_web[19]) ||
				('1' == inverter->status_web[21]) || ('1' == inverter->status_web[22]))
			strcat(inverter->status_ema, "1");		//AC??
		else
			strcat(inverter->status_ema, "0");
		if('1' == inverter->status_web[16])			//????
			strcat(inverter->status_ema, "1");
		else
			strcat(inverter->status_ema, "0");		//????????
		if('1' == inverter->status_web[17])
			strcat(inverter->status_ema, "1");
		else
			strcat(inverter->status_ema, "0");
		if('1' == inverter->status_web[18])			//??????
			strcat(inverter->status_ema, "1");
		else
			strcat(inverter->status_ema, "0");

		inverter->status_ema[18] = inverter->status_web[0];
		inverter->status_ema[19] = inverter->status_web[1];
		inverter->status_ema[20] = inverter->status_web[19];
		inverter->status_ema[21] = inverter->status_web[22];
		inverter->status_ema[22] = inverter->status_web[21];
		inverter->status_ema[23] = inverter->status_web[16];
		inverter->status_ema[24] = inverter->status_web[17];
		inverter->status_ema[25] = inverter->status_web[18];
		inverter->status_ema[26] = '0';
		inverter->status_ema[27] = 'A';
		inverter->status_ema[28] = inverter->status_web[2];
		inverter->status_ema[29] = inverter->status_web[3];
		inverter->status_ema[30] = 'B';
		inverter->status_ema[31] = inverter->status_web[4];
		inverter->status_ema[32] = inverter->status_web[5];
		inverter->status_ema[33] = 'C';
		inverter->status_ema[34] = inverter->status_web[6];
		inverter->status_ema[35] = inverter->status_web[7];
		strcat(inverter->status_ema, "END");

		inverter->status_send_flag=1;
	}
}



int resolvedata_250(char *data, struct inverter_info_t *inverter)
{
	int seconds = 0;
	inverter->it = (((data[0] & 0x0F) * 256 + data[1]) * 330 - 245760) / 4096;
	if((0==data[2])&&(0==data[3]))								//???????????????
		inverter->gf = 0;
	else
		inverter->gf = 223750.0 / (data[2]*256 + data[3]);
	inverter->curacctime = data[7]*256 + data[8];
	inverter->dv = (((data[9] >> 4) & 0x0F) * 256 + ((data[9] << 4) & 0xF0) | ((data[10] >> 4) & 0x0F)) * 825 / 4096;
	inverter->di = (((data[12] >> 4) & 0x0F) * 256 + ((data[12] << 4) & 0xF0) | ((data[13] >> 4) & 0x0F)) * 275 / 4096;
	inverter->gv = (data[18]*256*256 + data[19]*256 + data[20]) / 26204.64;
	inverter->curaccgen = (float)(((data[27]*256 + data[28])/1661900.0*220.0/1000.0*256.0*256.0*256.0 + (data[29]*256.0*256.0 + data[30]*256.0 + data[31])/1661900.0*220.0/1000.0)/3600.0);
	inverter->reactive_power = (data[47] * 65536 + data[48] * 256 + data[49]) / 1712;
	inverter->active_power = (data[56] * 65536 + data[57] * 256 + data[58]) / 3314;
	inverter->cur_output_energy = (data[65] * 65536 + data[66] * 256 + data[67]) / 996952.0;


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



	if(0==seconds)//???????????????
	{
		inverter->op = 0;
	}

	if(inverter->curacctime > 600)		//??????????,?????10????????????????,ZK
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


	inverter->status[0]=inverter->status_web[1];			//???????
	inverter->status[1]=inverter->status_web[0];			//???????
	inverter->status[2]=inverter->status_web[2];			//???????
	inverter->status[3]=inverter->status_web[3];			//???????
	inverter->status[4]=inverter->status_web[8];			//???A???
	inverter->status[5]=inverter->status_web[9];			//???A???
	inverter->status[6]=inverter->status_web[16];			//????
	inverter->status[7]=inverter->status_web[17];			//GFDI
	inverter->status[8]=inverter->status_web[18];			//????
	inverter->status[9]=inverter->status_web[19];			//????
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

	return 1;
}


int resolvedata_500(char *data, struct inverter_info_t *inverter)
{
	int  seconds;

	inverter->it = (((data[0] & 0x0F) * 256 + data[1]) * 330 - 245760) / 4096;
	if((0==data[2])&&(0==data[3]))								//???????????????
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
	inverter->reactive_power = (data[47] * 65536 + data[48] * 256 + data[49]) / 1712;
	inverter->active_power = (data[56] * 65536 + data[57] * 256 + data[58]) / 3360;
	inverter->cur_output_energy = (data[65] * 65536 + data[66] * 256 + data[67]) / 1009687.0;


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

	if(0==seconds)//???????????????
	{
		inverter->op = 0;
		inverter->opb = 0;
	}


	if(inverter->curacctime > 600)		//??????????,?????10????????????????,ZK
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


	inverter->status[0]=inverter->status_web[1];			//???????
	inverter->status[1]=inverter->status_web[0];			//???????
	inverter->status[2]=inverter->status_web[2];			//???????
	inverter->status[3]=inverter->status_web[3];			//???????
	inverter->status[4]=inverter->status_web[8];			//???A???
	inverter->status[5]=inverter->status_web[9];			//???A???
	inverter->status[6]=inverter->status_web[16];			//????
	inverter->status[7]=inverter->status_web[17];			//GFDI
	inverter->status[8]=inverter->status_web[18];			//????
	inverter->status[9]=inverter->status_web[19];			//????
	inverter->status[10] = '0';

	inverter->statusb[0]=inverter->status_web[1];			//???????
	inverter->statusb[1]=inverter->status_web[0];			//???????
	inverter->statusb[2]=inverter->status_web[2];			//???????
	inverter->statusb[3]=inverter->status_web[3];			//???????
	inverter->statusb[4]=inverter->status_web[10];			//???B???
	inverter->statusb[5]=inverter->status_web[11];			//???B???
	inverter->statusb[6]=inverter->status_web[16];			//????
	inverter->statusb[7]=inverter->status_web[17];			//GFDI
	inverter->statusb[8]=inverter->status_web[18];			//????
	inverter->statusb[9]=inverter->status_web[19];			//????
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
	int  seconds;

	inverter->it = (((data[0] & 0x0F) * 256 + data[1]) * 330 - 245760) / 4096;
	if((0==data[2])&&(0==data[3]))//???????????????
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
	inverter->reactive_power = (data[47] * 65536 + data[48] * 256 + data[49]) / 1712;
	inverter->reactive_powerb = (data[50] * 65536 + data[51] * 256 + data[52]) / 1712;
	inverter->reactive_powerc = (data[53] * 65536 + data[54] * 256 + data[55]) / 1712;
	inverter->active_power = (data[56] * 65536 + data[57] * 256 + data[58]) / 959.57;
	inverter->active_powerb = (data[59] * 65536 + data[60] * 256 + data[61]) / 959.57;
	inverter->active_powerc = (data[62] * 65536 + data[63] * 256 + data[64]) / 959.57;
	inverter->cur_output_energy = (data[65] * 65536 + data[66] * 256 + data[67]) / 93267.5;
	inverter->cur_output_energyb = (data[68] * 65536 + data[69] * 256 + data[70]) / 93267.5;
	inverter->cur_output_energyc = (data[71] * 65536 + data[72] * 256 + data[73]) / 93267.5;

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
		printfloatmsg("main","DEBUG-3 output_energy", inverter->output_energy);
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

	if(0==seconds)											//???????????????
	{
		inverter->op = 0;
		inverter->opb = 0;
		inverter->opc = 0;
		inverter->opd = 0;
	}

	if(inverter->curacctime > 600)		//??????????,?????10????????????????,ZK
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
	inverter->status_web[21]=((data[4]>>5)&0x01)+0x30;		//Active anti-island protection
	inverter->status_web[22]=((data[4]>>6)&0x01)+0x30;		//CP protection
	inverter->status_web[23]='0';
	inverter->status_web[24]='0';

	inverter->status[0]=inverter->status_web[1];			//???????
	inverter->status[1]=inverter->status_web[0];			//???????
	inverter->status[2]=inverter->status_web[2];			//???A?????
	inverter->status[3]=inverter->status_web[3];			//???A?????
	inverter->status[4]=inverter->status_web[8];			//?????
	inverter->status[5]=inverter->status_web[9];			//?????
	inverter->status[6]=inverter->status_web[16];			//????
	inverter->status[7]=inverter->status_web[17];			//GFDI
	inverter->status[8]=inverter->status_web[18];			//????
	inverter->status[9]=inverter->status_web[19];			//????
	inverter->status[10] = '0';

	inverter->statusb[0]=inverter->status_web[1];			//???????
	inverter->statusb[1]=inverter->status_web[0];			//???????
	inverter->statusb[2]=inverter->status_web[4];			//???B?????
	inverter->statusb[3]=inverter->status_web[5];			//???B?????
	inverter->statusb[4]=inverter->status_web[8];			//?????
	inverter->statusb[5]=inverter->status_web[9];			//?????
	inverter->statusb[6]=inverter->status_web[16];			//????
	inverter->statusb[7]=inverter->status_web[17];			//GFDI
	inverter->statusb[8]=inverter->status_web[18];			//????
	inverter->statusb[9]=inverter->status_web[19];			//????
	inverter->statusb[10] = '0';

	inverter->statusc[0]=inverter->status_web[1];			//???????
	inverter->statusc[1]=inverter->status_web[0];			//???????
	inverter->statusc[2]=inverter->status_web[6];			//???C?????
	inverter->statusc[3]=inverter->status_web[7];			//???C?????
	inverter->statusc[4]=inverter->status_web[8];			//?????
	inverter->statusc[5]=inverter->status_web[9];			//?????
	inverter->statusc[6]=inverter->status_web[16];			//????
	inverter->statusc[7]=inverter->status_web[17];			//GFDI
	inverter->statusc[8]=inverter->status_web[18];			//????
	inverter->statusc[9]=inverter->status_web[19];			//????
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

	yc1000_status(inverter);

	strcpy(inverter->last_report_time , ecu.broadcast_time);
//	printinverterinfo(inverter);
	return 1;

}

int resolvedata_600(char *data, struct inverter_info_t *inverter)
{
	int i, seconds=0;
	float factor1 = 0;
	float factor2 = 0;
	if(data[37]==0x1F)
	{
		factor1 = 1;
		factor2 = 0;
	}
	else if((data[37]>=11)&&(data[37]<=20))
	{
		factor1 = (data[37]-11)/10.0;
		factor2 = sqrt(1-(factor1*factor1));
	}
	else if((data[37]>0)&&(data[37]<11))
	{
		factor2 = (data[37]-1)/10.0;
		factor1 = sqrt(1-(factor2*factor2));
	}
	inverter->it = ((data[0]*256+data[1])*330-245760)/4096;
	if((0==data[2])&&(0==data[3])&&(0==data[4]))								//???????????????
		inverter->gf = 0;
	else
	{
		inverter->gf =50000000.0/(data[2]*256*256+data[3]*256+data[4]);
	}
	inverter->curacctime = data[7]*256 + data[8];


	inverter->dv = ((data[17]<<4) + (data[16] & 0xF0)) * 82.5 / 4096.0;
	inverter->dvb = ((data[14]<<4) + (data[13] & 0xF0)) * 82.5 / 4096.0;
	inverter->di = ((data[16] & 0x0F) * 256 + data[15]) * 27.5 / 4096.0;
	inverter->dib = ((data[13] & 0x0F) * 256 + data[12]) * 27.5 / 4096.0;
	inverter->gv = (data[18]*256+data[19])/1.3277;
	inverter->curaccgen = ((data[27]*256*10.095)*220+(data[28]*10.095)*220+(data[29]*256*256+data[30]*256+data[31])/1661900.0*220.0)/3600.0/1000.0;
	//inverter->curaccgen = (data[27]*256*10.095)*220+(data[28]*10.095)+(data[29]/1661900*220*256*256)+(data[30]/1661900*220*256)+(data[31]/1661900*220);

	inverter->curaccgenb = ((data[32]*256*10.095)*220+(data[33]*10.095)*220+(data[34]*256*256+data[35]*256+data[36])/1661900.0*220.0)/3600.0/1000.0;
	inverter->reactive_power = (factor2)*0.94*(inverter->dv*inverter->di+inverter->dvb*inverter->dib);
	inverter->active_power = (factor1)*0.94*(inverter->dv*inverter->di+inverter->dvb*inverter->dib);
	inverter->cur_output_energy = (inverter->curaccgen+inverter->curaccgenb)*0.94*data[37];


//	if((inverter->cur_output_energy >= inverter->pre_output_energy)&&(inverter->curacctime >= inverter->preacctime))
//	{
//		inverter->output_energy=inverter->cur_output_energy-inverter->pre_output_energy;
//	}
//	else
//	{
//		inverter->output_energy=inverter->cur_output_energy;
//	}
//	inverter->pre_output_energy=inverter->cur_output_energy;

	if((inverter->curaccgen >= inverter->preaccgen)&&(inverter->curaccgenb >= inverter->preaccgenb)&&(inverter->curacctime >= inverter->preacctime))
	{	//printf("%d\n",__LINE__);
		seconds = inverter->curacctime - inverter->preacctime;
		inverter->curgeneration = inverter->curaccgen - inverter->preaccgen;
		inverter->curgenerationb = inverter->curaccgenb - inverter->preaccgenb;
	}
	else
	{//printf("%d\n",__LINE__);
		seconds = inverter->curacctime;
		inverter->curgeneration = inverter->curaccgen;
		inverter->curgenerationb = inverter->curaccgenb;
	}
	//printf("prtm=%d\n",inverter->preacctime);
	inverter->preacctime = inverter->curacctime;
	inverter->preaccgen = inverter->curaccgen;
	inverter->preaccgenb = inverter->curaccgenb;

	if(0==seconds)//???????????????
	{//printf("%d\n",__LINE__);
		inverter->op = 0;
		inverter->opb = 0;
	}


	if(inverter->curacctime > 600)		//??????????,?????10????????????????,ZK
	{//printf("%d\n",__LINE__);printf("%f %d\n",inverter->curgeneration,seconds);
		inverter->op = inverter->curgeneration * 1000.0 * 3600.0 / seconds;
		inverter->opb = inverter->curgenerationb * 1000.0 * 3600.0 / seconds;
	}
	else
	{//printf("%d\n",__LINE__);
		inverter->op = (int)(inverter->dv*inverter->di);
		inverter->opb = (int)(inverter->dvb*inverter->dib);
	}


	if(inverter->op>360)
		inverter->op = (int)(inverter->dv*inverter->di);
	if(inverter->opb>360)
		inverter->opb = (int)(inverter->dvb*inverter->dib);

	printf("tm=%d dv=%f  di=%f  op=%d  gv=%d curaccgen=%f reactive_power=%f active_power=%f cur_output_energy=%f\n",inverter->curacctime,inverter->dv,inverter->di,inverter->op,inverter->gv,inverter->curaccgen,inverter->reactive_power,inverter->active_power,inverter->cur_output_energy);
	printf("sm=%d dvb=%f dib=%f opb=%d gv=%d curaccgenb=%f reactive_power=%f active_power=%f cur_output_energy=%f\n",seconds,inverter->dvb,inverter->dib,inverter->opb,inverter->gv,inverter->curaccgenb,inverter->reactive_power,inverter->active_power,inverter->cur_output_energy);
	printf("prtm=%d\n",inverter->preacctime);

	for(i=0;i<25;i++)
		inverter->status_web[i] = 0x30;
	inverter->status_web[0]=((data[25]>>6)&0x01)+0x30;		//AC Frequency exceeding Range 1bit"??????"
	inverter->status_web[1]=((data[25]>>7)&0x01)+0x30;			//AC Frequency under Range 1bit"??????";
	inverter->status_web[2]='0';//((data[25]>>5)&0x01)+0x30;		//AC-A Voltage exceeding Range 1bit"A???????
	inverter->status_web[3]='0';//((data[25]>>4)&0x01)+0x30;		//AC-A Voltage under Range 1bit"A???????
	inverter->status_web[4]='0';//((data[25]>>5)&0x01)+0x30;		//B???????
	inverter->status_web[5]='0';//((data[25]>>4)&0x01)+0x30;		//B???????"
	inverter->status_web[6]='0';//((data[25]>>5)&0x01)+0x30;		//"C???????"
	inverter->status_web[7]='0';//((data[25]>>4)&0x01)+0x30;		//"C???????"

	inverter->status_web[8]=((data[25]>>1)&0x01)+0x30;			//DC-A Voltage Too High 1bitA???????
	inverter->status_web[9]=((data[26]>>7)&0x01)+0x30;		//DC-A Voltage Too Low 1bitA???????
	inverter->status_web[10]=((data[26]>>6)&0x01)+0x30;		//DC-B Voltage Too High 1bitB???????
	inverter->status_web[11]=((data[26]>>5)&0x01)+0x30;		//DC-B Voltage Too Low 1bitB???????
//	inverter->status_web[12]=((data[24]>>4)&0x01)+0x30;		//
//	inverter->status_web[13]=((data[24]>>5)&0x01)+0x30;		//
//	inverter->status_web[14]=((data[24]>>6)&0x01)+0x30;		//
//	inverter->status_web[15]=data[48]+0x30;				//gfdi??
	inverter->status_web[16]=((data[25])&0x01)+0x30;			//Over Critical Temperature 1bit??????
	inverter->status_web[17]=((data[26]>>4)&0x01)+0x30;		//GFDI"GFDI??"
	inverter->status_web[18]=((data[26]>>3)&0x01)+0x30;		//Remote-shut"????"
	inverter->status_web[19]=((data[26])&0x01)+0x30;		//AC-Disconnect"????
	inverter->status_web[21]=((data[25]>>3)&0x01)+0x30;		//"??????
	inverter->status_web[22]=((data[25]>>2)&0x01)+0x30;		//"CP??
	inverter->status_web[23]=((data[25]>>5)&0x01)+0x30;		//??????
	inverter->status_web[24]=((data[25]>>4)&0x01)+0x30;		//??????


	inverter->status[0]=inverter->status_web[1];			//???????
	inverter->status[1]=inverter->status_web[0];			//???????
	inverter->status[2]=inverter->status_web[2];			//???????
	inverter->status[3]=inverter->status_web[3];			//???????
	inverter->status[4]=inverter->status_web[8];			//???A???
	inverter->status[5]=inverter->status_web[9];			//???A???
	inverter->status[6]=inverter->status_web[16];			//????
	inverter->status[7]=inverter->status_web[17];			//GFDI
	inverter->status[8]=inverter->status_web[18];			//????
	inverter->status[9]=inverter->status_web[19];			//????
	inverter->status[10] = '0';

	inverter->statusb[0]=inverter->status_web[1];			//???????
	inverter->statusb[1]=inverter->status_web[0];			//???????
	inverter->statusb[2]=inverter->status_web[2];			//???????
	inverter->statusb[3]=inverter->status_web[3];			//???????
	inverter->statusb[4]=inverter->status_web[10];			//???B???
	inverter->statusb[5]=inverter->status_web[11];			//???B???
	inverter->statusb[6]=inverter->status_web[16];			//????
	inverter->statusb[7]=inverter->status_web[17];		//GFDI
	inverter->statusb[8]=inverter->status_web[18];			//????
	inverter->statusb[9]=inverter->status_web[19];			//????
	inverter->statusb[10] = '0';

	if('1'==inverter->status_web[17])
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

	yc600_status(inverter);

	strcpy(inverter->last_report_time , ecu.broadcast_time);

	return 1;

}


