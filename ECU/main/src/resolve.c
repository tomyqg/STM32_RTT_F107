/*****************************************************************************/
/*  File      : resolve.c                                                    */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-03-05 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Include Files                                                            */
/*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "resolve.h"
#include "debug.h"
#include <string.h>
#include "SEGGER_RTT.h"

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern ecu_info ecu;
int caltype;		//���㷽ʽ��NA��ͷ�NA�������
extern float energy;
extern int ecu_type;

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/
float sqrt(float number) {
    float new_guess;
    float last_guess;
    
    if (number < 0) {
        printmsg(ECU_DBG_OTHER,"Cannot compute the square root of a negative number");
        return -1;
    }
    
    new_guess = 1;
    do {
        last_guess = new_guess;
        new_guess = (last_guess + number / last_guess) / 2;
    } while (new_guess != last_guess);
    
    return new_guess;
}



int resolvedata_1000(char *data, struct inverter_info_t *inverter)
{

	int i,seconds;
	char temp[2] = {'\0'};
	inverter->it = (((data[0] & 0x0F) * 256 + data[1]) * 330 - 245760) / 4096;
	if((0==data[2])&&(0==data[3]))//��ֹ�����ݴ������������������
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

	if(((inverter->curacctime - inverter->preacctime)<14400)&&(((inverter->curaccgen - inverter->preaccgen)>1)||((inverter->curaccgenb - inverter->preaccgenb)>1)||((inverter->curaccgenc - inverter->preaccgenc)>1)))
	{
		inverter->inverterstatus.dataflag=0;
		return 0;
	}
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
		//printfloatmsg("DEBUG-3 output_energy", inverter->output_energy);
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

	if(0==seconds)											//��ֹ�����ݴ������������������
	{
		inverter->op = 0;
		inverter->opb = 0;
		inverter->opc = 0;
		inverter->opd = 0;
	}

	if(inverter->curacctime > 600)		//��ֹ�����������ʱ��㣬����������10���Ӻ�Ų���ʱ���Ĺ��ʼ���ģʽ��ZK
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


	for(i=0;i<37;i++)
		inverter->status_web[i] = 0x30;
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

	inverter->status[0]=inverter->status_web[1];			//交流端频率欠�?
	inverter->status[1]=inverter->status_web[0];			//交流端频率过�?
	inverter->status[2]=inverter->status_web[2];			//交流端A路电压过�?
	inverter->status[3]=inverter->status_web[3];			//交流端A路电压欠�?
	inverter->status[4]=inverter->status_web[8];			//直流端过�?
	inverter->status[5]=inverter->status_web[9];			//直流端欠�?
	inverter->status[6]=inverter->status_web[16];			//温度过高
	inverter->status[7]=inverter->status_web[17];			//GFDI
	inverter->status[8]=inverter->status_web[18];			//远程关闭
	inverter->status[9]=inverter->status_web[19];			//交流关闭
	inverter->status[10] = '0';

	inverter->statusb[0]=inverter->status_web[1];			//交流端频率欠�?
	inverter->statusb[1]=inverter->status_web[0];			//交流端频率过�?
	inverter->statusb[2]=inverter->status_web[4];			//交流端B路电压过�?
	inverter->statusb[3]=inverter->status_web[5];			//交流端B路电压欠�?
	inverter->statusb[4]=inverter->status_web[8];			//直流端过�?
	inverter->statusb[5]=inverter->status_web[9];			//直流端欠�?
	inverter->statusb[6]=inverter->status_web[16];			//温度过高
	inverter->statusb[7]=inverter->status_web[17];			//GFDI
	inverter->statusb[8]=inverter->status_web[18];			//远程关闭
	inverter->statusb[9]=inverter->status_web[19];			//交流关闭
	inverter->statusb[10] = '0';

	inverter->statusc[0]=inverter->status_web[1];			//交流端频率欠�?
	inverter->statusc[1]=inverter->status_web[0];			//交流端频率过�?
	inverter->statusc[2]=inverter->status_web[6];			//交流端C路电压过�?
	inverter->statusc[3]=inverter->status_web[7];			//交流端C路电压欠�?
	inverter->statusc[4]=inverter->status_web[8];			//直流端过�?
	inverter->statusc[5]=inverter->status_web[9];			//直流端欠�?
	inverter->statusc[6]=inverter->status_web[16];			//温度过高
	inverter->statusc[7]=inverter->status_web[17];			//GFDI
	inverter->statusc[8]=inverter->status_web[18];			//远程关闭
	inverter->statusc[9]=inverter->status_web[19];			//交流关闭
	inverter->statusc[10] = '0';

	if('1'==inverter->status_web[19])
	{
		if(1==ecu_type)
			inverter->gf = 50.0;
		else
			inverter->gf = 60.0;


		inverter->gv=0;
		inverter->gvb=0;
		inverter->gvc=0;
	}

	temp[0] = inverter->status_web[17];
	temp[1] = '\0';
	if(inverter->inverterstatus.last_gfdi_flag != atoi(temp))
		inverter->inverterstatus.gfdi_changed_flag = 1;
	else
		inverter->inverterstatus.gfdi_changed_flag = 0;
	inverter->inverterstatus.last_gfdi_flag = (inverter->status_web[17]=='1')?1:0;
	
	temp[0] = inverter->status_web[18];
	temp[1] = '\0';
	if(inverter->inverterstatus.last_turn_on_off_flag != atoi(temp))
		inverter->inverterstatus.turn_on_off_changed_flag = 1;
	else
		inverter->inverterstatus.turn_on_off_changed_flag = 0;
	
	inverter->inverterstatus.last_turn_on_off_flag = (inverter->status_web[18]=='1')?1:0;


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
	char temp[2] = {'\0'};
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
	if((0==data[2])&&(0==data[3])&&(0==data[4]))								//防止空数据传过来导致主程序出�?
		inverter->gf = 0;
	else
	{
		inverter->gf =50000000.0/(data[2]*256*256+data[3]*256+data[4]);
	}
	inverter->curacctime = data[7]*256 + data[8];


	inverter->dvb = ((data[17]<<4) + (data[16] & 0xF0)) * 82.5 / 4096.0;
	inverter->dv = ((data[14]<<4) + (data[13] & 0xF0)) * 82.5 / 4096.0;
	inverter->dib = ((data[16] & 0x0F) * 256 + data[15]) * 27.5 / 4096.0;
	inverter->di = ((data[13] & 0x0F) * 256 + data[12]) * 27.5 / 4096.0;
	inverter->gv = (data[18]*256+data[19])/1.3277;
	inverter->curaccgenb = ((data[27]*256*10.095)*220+(data[28]*10.095)*220+(data[29]*256*256+data[30]*256+data[31])/1661900.0*220.0)/3600.0/1000.0;
	//inverter->curaccgen = (data[27]*256*10.095)*220+(data[28]*10.095)+(data[29]/1661900*220*256*256)+(data[30]/1661900*220*256)+(data[31]/1661900*220);

	inverter->curaccgen = ((data[32]*256*10.095)*220+(data[33]*10.095)*220+(data[34]*256*256+data[35]*256+data[36])/1661900.0*220.0)/3600.0/1000.0;
	inverter->reactive_power = (factor2)*0.94*(inverter->dv*inverter->di+inverter->dvb*inverter->dib);
	inverter->active_power = (factor1)*0.94*(inverter->dv*inverter->di+inverter->dvb*inverter->dib);
	inverter->cur_output_energy = (inverter->curaccgen+inverter->curaccgenb)*0.94*data[37];


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
	//printf("prtm=%d\n",inverter->preacctime);
	inverter->preacctime = inverter->curacctime;
	inverter->preaccgen = inverter->curaccgen;
	inverter->preaccgenb = inverter->curaccgenb;

	if(0==seconds)//��ֹ�����ݴ������������������
	{
		inverter->op = 0;
		inverter->opb = 0;
	}


	if(inverter->curacctime > 600)		//��ֹ�����������ʱ��㣬����������10���Ӻ�Ų���ʱ���Ĺ��ʼ���ģʽ��ZK
	{
		inverter->op = inverter->curgeneration * 1000.0 * 3600.0 / seconds;
		inverter->opb = inverter->curgenerationb * 1000.0 * 3600.0 / seconds;
	}
	else
	{
		inverter->op = (int)(inverter->dv*inverter->di);
		inverter->opb = (int)(inverter->dvb*inverter->dib);
	}


	if(inverter->op>360)
		inverter->op = (int)(inverter->dv*inverter->di);
	if(inverter->opb>360)
		inverter->opb = (int)(inverter->dvb*inverter->dib);
#if ECU_DEBUG
#if ECU_DEBUG_MAIN
	printf("tm=%d dv=%f  di=%f  op=%d  gv=%d curaccgen=%f reactive_power=%f active_power=%f cur_output_energy=%f\n",inverter->curacctime,inverter->dv,inverter->di,inverter->op,inverter->gv,inverter->curaccgen,inverter->reactive_power,inverter->active_power,inverter->cur_output_energy);
	printf("sm=%d dvb=%f dib=%f opb=%d gv=%d curaccgenb=%f reactive_power=%f active_power=%f cur_output_energy=%f\n",seconds,inverter->dvb,inverter->dib,inverter->opb,inverter->gv,inverter->curaccgenb,inverter->reactive_power,inverter->active_power,inverter->cur_output_energy);
	printf("prtm=%d\n",inverter->preacctime);
#endif
#endif
	for(i=0;i<37;i++)
		inverter->status_web[i] = 0x30;
	inverter->status_web[0]=((data[25]>>6)&0x01)+0x30;		//AC Frequency exceeding Range 1bit"交流频率太高"
	inverter->status_web[1]=((data[25]>>7)&0x01)+0x30;			//AC Frequency under Range 1bit"交流频率太低";
	inverter->status_web[2]='0';//((data[25]>>5)&0x01)+0x30;		//AC-A Voltage exceeding Range 1bit"A路交流电压太�?
	inverter->status_web[3]='0';//((data[25]>>4)&0x01)+0x30;		//AC-A Voltage under Range 1bit"A路交流电压太�?
	inverter->status_web[4]='0';//((data[25]>>5)&0x01)+0x30;		//B路交流电压太�?
	inverter->status_web[5]='0';//((data[25]>>4)&0x01)+0x30;		//B路交流电压太�?
	inverter->status_web[6]='0';//((data[25]>>5)&0x01)+0x30;		//"C路交流电压太�?
	inverter->status_web[7]='0';//((data[25]>>4)&0x01)+0x30;		//"C路交流电压太�?

	inverter->status_web[8]=((data[25]>>1)&0x01)+0x30;			//DC-A Voltage Too High 1bitA路直流电压太�?
	inverter->status_web[9]=((data[26]>>7)&0x01)+0x30;		//DC-A Voltage Too Low 1bitA路直流电压太�?
	inverter->status_web[10]=((data[26]>>6)&0x01)+0x30;		//DC-B Voltage Too High 1bitB路直流电压太�?
	inverter->status_web[11]=((data[26]>>5)&0x01)+0x30;		//DC-B Voltage Too Low 1bitB路直流电压太�?
//	inverter->status_web[12]=((data[24]>>4)&0x01)+0x30;		//
//	inverter->status_web[13]=((data[24]>>5)&0x01)+0x30;		//
//	inverter->status_web[14]=((data[24]>>6)&0x01)+0x30;		//
//	inverter->status_web[15]=data[48]+0x30;				//gfdi状�?
	inverter->status_web[16]=((data[25])&0x01)+0x30;			//Over Critical Temperature 1bit超出温度范围
	inverter->status_web[17]=((data[26]>>4)&0x01)+0x30;		//GFDI"GFDI锁住"
	inverter->status_web[18]=((data[26]>>3)&0x01)+0x30;		//Remote-shut"远程关闭"
	inverter->status_web[19]=((data[26])&0x01)+0x30;		//AC-Disconnect"交流断开
	inverter->status_web[21]=((data[25]>>3)&0x01)+0x30;		//"主动孤岛保护
	inverter->status_web[22]=((data[25]>>2)&0x01)+0x30;		//"CP保护
	inverter->status_web[23]=((data[25]>>5)&0x01)+0x30;		//交流电压太高
	inverter->status_web[24]=((data[25]>>4)&0x01)+0x30;		//交流电压太低


	inverter->status[0]=inverter->status_web[1];			//交流端频率欠�?
	inverter->status[1]=inverter->status_web[0];			//交流端频率过�?
	inverter->status[2]=inverter->status_web[2];			//交流端电压过�?
	inverter->status[3]=inverter->status_web[3];			//交流端电压欠�?
	inverter->status[4]=inverter->status_web[8];			//直流端A路过�?
	inverter->status[5]=inverter->status_web[9];			//直流端A路欠�?
	inverter->status[6]=inverter->status_web[16];			//温度过高
	inverter->status[7]=inverter->status_web[17];			//GFDI
	inverter->status[8]=inverter->status_web[18];			//远程关闭
	inverter->status[9]=inverter->status_web[19];			//交流关闭
	inverter->status[10] = '0';

	inverter->statusb[0]=inverter->status_web[1];			//交流端频率欠�?
	inverter->statusb[1]=inverter->status_web[0];			//交流端频率过�?
	inverter->statusb[2]=inverter->status_web[2];			//交流端电压过�?
	inverter->statusb[3]=inverter->status_web[3];			//交流端电压欠�?
	inverter->statusb[4]=inverter->status_web[10];			//直流端B路过�?
	inverter->statusb[5]=inverter->status_web[11];			//直流端B路欠�?
	inverter->statusb[6]=inverter->status_web[16];			//温度过高
	inverter->statusb[7]=inverter->status_web[17];		//GFDI
	inverter->statusb[8]=inverter->status_web[18];			//远程关闭
	inverter->statusb[9]=inverter->status_web[19];			//交流关闭
	inverter->statusb[10] = '0';

	if('1'==inverter->status_web[19])
	{
		if(3==inverter->model)
			inverter->gf = 50.0;
		if(4==inverter->model)
			inverter->gf = 60.0;
		if(7==inverter->model)
		{
			if(ecu_type==1)
				inverter->gf = 50;
			else
				inverter->gf = 60;
		}
	
		inverter->gv=0;
	}

	temp[0] = inverter->status_web[17];
	temp[1] = '\0';
	if(inverter->inverterstatus.last_gfdi_flag != atoi(temp))
		inverter->inverterstatus.gfdi_changed_flag = 1;
	else
		inverter->inverterstatus.gfdi_changed_flag = 0;
	inverter->inverterstatus.last_gfdi_flag = (inverter->status_web[17]=='1')?1:0;
	
	temp[0] = inverter->status_web[18];
	temp[1] = '\0';
	if(inverter->inverterstatus.last_turn_on_off_flag != atoi(temp))
		inverter->inverterstatus.turn_on_off_changed_flag = 1;
	else
		inverter->inverterstatus.turn_on_off_changed_flag = 0;
	
	inverter->inverterstatus.last_turn_on_off_flag = (inverter->status_web[18]=='1')?1:0;


	check_yc500(inverter);				//ZK

	yc600_status(inverter);

	strcpy(inverter->last_report_time , ecu.broadcast_time);

	return 1;

}

int resolvedata_600_new(char *data, struct inverter_info_t *inverter)
{
	char temp[2] = {'\0'};
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
	inverter->it = ((data[0]*256+data[1])*3300/4092-757)/2.85;
	if((0==data[2])&&(0==data[3])&&(0==data[4]))								//防止空数据传过来导致主程序出�?
		inverter->gf = 0;
	else
	{
		inverter->gf =50000000.0/(data[2]*256*256+data[3]*256+data[4]);
	}
	inverter->curacctime = data[7]*256 + data[8];


	inverter->dvb = ((data[17]<<4) + (data[16] & 0xF0)) * 82.5 / 4096.0;
	inverter->dv = ((data[14]<<4) + (data[13] & 0xF0)) * 82.5 / 4096.0;
	inverter->dib = ((data[16] & 0x0F) * 256 + data[15]) * 27.5 / 4096.0;
	inverter->di = ((data[13] & 0x0F) * 256 + data[12]) * 27.5 / 4096.0;
	inverter->gv = (data[18]*256+data[19])/1.33;
	inverter->curaccgenb = ((data[27]*256*10.095)*220+(data[28]*10.095)*220+(data[29]*256*256+data[30]*256+data[31])/1661900.0*220.0)/3600.0/1000.0;
	//inverter->curaccgen = (data[27]*256*10.095)*220+(data[28]*10.095)+(data[29]/1661900*220*256*256)+(data[30]/1661900*220*256)+(data[31]/1661900*220);

	inverter->curaccgen = ((data[32]*256*10.095)*220+(data[33]*10.095)*220+(data[34]*256*256+data[35]*256+data[36])/1661900.0*220.0)/3600.0/1000.0;
	inverter->reactive_power = (factor2)*0.94*(inverter->dv*inverter->di+inverter->dvb*inverter->dib);
	inverter->active_power = (factor1)*0.94*(inverter->dv*inverter->di+inverter->dvb*inverter->dib);
	inverter->cur_output_energy = (inverter->curaccgen+inverter->curaccgenb)*0.94*data[37];


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

	if(0==seconds)//防止空数据传过来导致主程序出�?
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

	for(i=0;i<37;i++)
		inverter->status_web[i] = 0x30;
	inverter->status_web[0]=((data[25]>>6)&0x01)+0x30;		//AC Frequency exceeding Range 1bit"交流频率太高"
	inverter->status_web[1]=((data[25]>>7)&0x01)+0x30;			//AC Frequency under Range 1bit"交流频率太低";
	inverter->status_web[2]='0';//((data[25]>>5)&0x01)+0x30;		//AC-A Voltage exceeding Range 1bit"A路交流电压太�?
	inverter->status_web[3]='0';//((data[25]>>4)&0x01)+0x30;		//AC-A Voltage under Range 1bit"A路交流电压太�?
	inverter->status_web[4]='0';//((data[25]>>5)&0x01)+0x30;		//B路交流电压太�?
	inverter->status_web[5]='0';//((data[25]>>4)&0x01)+0x30;		//B路交流电压太�?
	inverter->status_web[6]='0';//((data[25]>>5)&0x01)+0x30;		//"C路交流电压太�?
	inverter->status_web[7]='0';//((data[25]>>4)&0x01)+0x30;		//"C路交流电压太�?

	inverter->status_web[8]=((data[25]>>1)&0x01)+0x30;			//DC-A Voltage Too High 1bitA路直流电压太�?
	inverter->status_web[9]=((data[26]>>7)&0x01)+0x30;		//DC-A Voltage Too Low 1bitA路直流电压太�?
	inverter->status_web[10]=((data[26]>>6)&0x01)+0x30;		//DC-B Voltage Too High 1bitB路直流电压太�?
	inverter->status_web[11]=((data[26]>>5)&0x01)+0x30;		//DC-B Voltage Too Low 1bitB路直流电压太�?
//	inverter->status_web[12]=((data[24]>>4)&0x01)+0x30;		//
//	inverter->status_web[13]=((data[24]>>5)&0x01)+0x30;		//
//	inverter->status_web[14]=((data[24]>>6)&0x01)+0x30;		//
//	inverter->status_web[15]=data[48]+0x30;				//gfdi状�?
	inverter->status_web[16]=((data[25])&0x01)+0x30;			//Over Critical Temperature 1bit超出温度范围
	inverter->status_web[17]=((data[26]>>4)&0x01)+0x30;		//GFDI"GFDI锁住"
	inverter->status_web[18]=((data[26]>>3)&0x01)+0x30;		//Remote-shut"远程关闭"
	inverter->status_web[19]=((data[26])&0x01)+0x30;		//AC-Disconnect"交流断开
	inverter->status_web[21]=((data[25]>>3)&0x01)+0x30;		//"主动孤岛保护
	inverter->status_web[22]=((data[25]>>2)&0x01)+0x30;		//"CP保护
	inverter->status_web[23]=((data[25]>>5)&0x01)+0x30;		//交流电压太高
	inverter->status_web[24]=((data[25]>>4)&0x01)+0x30;		//交流电压太低

	inverter->status_web[25]=((data[24]>>2)&0x01)+0x30;		//十分钟平均保�?
	inverter->status_web[26]=((data[24]>>1)&0x01)+0x30;		//母线欠压保护
	inverter->status_web[27]=((data[24])&0x01)+0x30;		//母线过压保护
	inverter->status_web[28]=((data[26]>>1)&0x01)+0x30;		//继电器故障保�?
	inverter->status_web[29]='0';							//预留
	inverter->status_web[30]='0';
	inverter->status_web[31]='0';
	inverter->status_web[32]='0';
	inverter->status_web[33]='0';
	inverter->status_web[34]='0';
	inverter->status_web[35]='0';
	inverter->status_web[36]='0';


	inverter->status[0]=inverter->status_web[1];			//交流端频率欠�?
	inverter->status[1]=inverter->status_web[0];			//交流端频率过�?
	inverter->status[2]=inverter->status_web[23];			//交流端电压过�?
	inverter->status[3]=inverter->status_web[24];			//交流端电压欠�?
	inverter->status[4]=inverter->status_web[8];			//直流端A路过�?
	inverter->status[5]=inverter->status_web[9];			//直流端A路欠�?
	inverter->status[6]=inverter->status_web[16];			//温度过高
	inverter->status[7]=inverter->status_web[17];			//GFDI
	inverter->status[8]=inverter->status_web[18];			//远程关闭
	inverter->status[9]=inverter->status_web[19];			//交流关闭
	inverter->status[10] = '0';

	inverter->statusb[0]=inverter->status_web[1];			//交流端频率欠�?
	inverter->statusb[1]=inverter->status_web[0];			//交流端频率过�?
	inverter->statusb[2]=inverter->status_web[23];			//交流端电压过�?
	inverter->statusb[3]=inverter->status_web[24];			//交流端电压欠�?
	inverter->statusb[4]=inverter->status_web[10];			//直流端B路过�?
	inverter->statusb[5]=inverter->status_web[11];			//直流端B路欠�?
	inverter->statusb[6]=inverter->status_web[16];			//温度过高
	inverter->statusb[7]=inverter->status_web[17];		//GFDI
	inverter->statusb[8]=inverter->status_web[18];			//远程关闭
	inverter->statusb[9]=inverter->status_web[19];			//交流关闭
	inverter->statusb[10] = '0';

	if('1'==inverter->status_web[19])
	{
		if(3==inverter->model)
			inverter->gf = 50.0;
		if(4==inverter->model)
			inverter->gf = 60.0;
		if(7==inverter->model)
		{
			if(ecu_type==1)
				inverter->gf = 50;
			else
				inverter->gf = 60;
		}

		inverter->gv=0;
	}

	temp[0] = inverter->status_web[17];
	temp[1] = '\0';
	if(inverter->inverterstatus.last_gfdi_flag != atoi(temp))
		inverter->inverterstatus.gfdi_changed_flag = 1;
	else
		inverter->inverterstatus.gfdi_changed_flag = 0;
	inverter->inverterstatus.last_gfdi_flag = (inverter->status_web[17]=='1')?1:0;
	
	temp[0] = inverter->status_web[18];
	temp[1] = '\0';
	if(inverter->inverterstatus.last_turn_on_off_flag != atoi(temp))
		inverter->inverterstatus.turn_on_off_changed_flag = 1;
	else
		inverter->inverterstatus.turn_on_off_changed_flag = 0;
	
	inverter->inverterstatus.last_turn_on_off_flag = (inverter->status_web[18]=='1')?1:0;


	check_yc500(inverter);				//ZK

	yc600_status(inverter);

	strcpy(inverter->last_report_time , ecu.broadcast_time);

	return 1;

}


int yc600_status(struct inverter_info_t *inverter)
{
	memset(inverter->status_ema, '\0', sizeof(inverter->status_ema));
	inverter->status_send_flag=0;

	if(('1' == inverter->status_web[0]) || ('1' == inverter->status_web[1]) || ('1' == inverter->status_web[16]) ||
			('1' == inverter->status_web[17]) || ('1' == inverter->status_web[18]) || ('1' == inverter->status_web[19]) ||
			('1' == inverter->status_web[21]) || ('1' == inverter->status_web[22]) ||
			('1' == inverter->status_web[23]) || ('1' == inverter->status_web[24]) || ('1' == inverter->status_web[25]) ||
			('1' == inverter->status_web[26]) || ('1' == inverter->status_web[27]) || ('1' == inverter->status_web[28])){
		strcat(inverter->status_ema, inverter->id);
		strcat(inverter->status_ema, "07");
		if(('1' == inverter->status_web[0]) || ('1' == inverter->status_web[1]) ||  ('1' == inverter->status_web[19]) ||
			('1' == inverter->status_web[21]) || ('1' == inverter->status_web[22]) || ('1' == inverter->status_web[23]) || ('1' == inverter->status_web[24]))
			strcat(inverter->status_ema, "1");		//AC保护
		else
			strcat(inverter->status_ema, "0");
		if('1' == inverter->status_web[16])			//温度保护
			strcat(inverter->status_ema, "1");
		else
			strcat(inverter->status_ema, "0");		//组件接地故障保护
		if(('1' == inverter->status_web[17])||('1' == inverter->status_web[28]))
			strcat(inverter->status_ema, "1");
		else
			strcat(inverter->status_ema, "0");
		if('1' == inverter->status_web[18])			//控制状态保�?
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

		inverter->status_ema[29] = inverter->status_web[25];
		inverter->status_ema[30] = inverter->status_web[26];
		inverter->status_ema[31] = inverter->status_web[27];
		inverter->status_ema[32] = inverter->status_web[28];
		inverter->status_ema[33] = '0';
		inverter->status_ema[34] = '0';
		inverter->status_ema[35] = '0';
		inverter->status_ema[36] = '0';
		inverter->status_ema[37] = '0';
		inverter->status_ema[38] = '0';
		inverter->status_ema[39] = '0';
		inverter->status_ema[40] = '0';
		strcat(inverter->status_ema, "END");

		inverter->status_send_flag=1;
	}
	return 0;
}


int yc1000_status(struct inverter_info_t *inverter)
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
			strcat(inverter->status_ema, "1");		//AC保护
		else
			strcat(inverter->status_ema, "0");
		if('1' == inverter->status_web[16])			//温度保护
			strcat(inverter->status_ema, "1");
		else
			strcat(inverter->status_ema, "0");		//组件接地故障保护
		if('1' == inverter->status_web[17])
			strcat(inverter->status_ema, "1");
		else
			strcat(inverter->status_ema, "0");
		if('1' == inverter->status_web[18])			//控制状态保�?
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
	return 0;
}

