
#include "checkdata.h"

int ecu_type = 1;	//1:SAA; 2:NA; 3:MX

/*
int get_ecu_type()			//在初始化initinverter()函数调用
{
	FILE *fp;
	char version[256] = {'\0'};
	
	fp = fopen("/etc/yuneng/version.conf", "r");
	if(fp)
	{
		fgets(version, sizeof(version), fp);
		if('\n' == version[strlen(version)-1])
			version[strlen(version)-1] = '\0';
		if(!strncmp(&version[strlen(version)-2], "MX", 2))
			ecu_type = 3;
		else if(!strncmp(&version[strlen(version)-2], "NA", 2))
			ecu_type = 2;
		else
			ecu_type = 1;
		fclose(fp);
	}
	
	return 0;
}
*/

int check_yc200_yc250(struct inverter_info_t *inverter)		//在解析函数的最后调用
{
	if(inverter->dv > 1500)
		inverter->dv = 1500;
	if(inverter->dv < 0)
		inverter->dv = 0;
	if(inverter->di > 150)
		inverter->di = 150;
	if(inverter->di < 0)
		inverter->di = 0;
	if(inverter->op > 300)
		inverter->op = 300;
	if(inverter->op < 0)
		inverter->op = 0;

	
	if(1 == ecu_type)	//SAA,45-55Hz
	{
		if(inverter->gf > 55)
			inverter->gf = 55;
		if(inverter->gf < 0)
			inverter->gf = 0;
	}
	else		//NA/MX,55-65Hz
	{
		if(inverter->gf > 65)
			inverter->gf = 65;
		if(inverter->gf < 0)
			inverter->gf = 0;
	}
	
	if(3 == ecu_type)		//MX,82-155V
	{
		if(inverter->gv > 155)
			inverter->gv = 155;
		if(inverter->gv < 0)
			inverter->gv = 0;
	}
	else if(2 == ecu_type)	//NA,181-298V
	{
		if(inverter->gv > 298)
			inverter->gv = 298;
		if(inverter->gv < 0)
			inverter->gv = 0;
	}
	else					//SAA,149-278V
	{
		if(inverter->gv > 278)
			inverter->gv = 278;
		if(inverter->gv < 0)
			inverter->gv = 0;
	}
	
	if(inverter->it > 150)
		inverter->it = 150;
	if(inverter->it < -50)
		inverter->it = -50;
	
	if(inverter->curgeneration > 0.999999)
		inverter->curgeneration = 0.999999;
	if(inverter->curgeneration < 0)
		inverter->curgeneration = 0;
	
	return 0;
}

int check_yc500(struct inverter_info_t *inverter)		//在解析函数的最后调用
{
	if(inverter->dv > 1500)
		inverter->dv = 1500;
	if(inverter->dvb > 1500)
		inverter->dvb = 1500;
	if(inverter->dv < 0)
		inverter->dv = 0;
	if(inverter->dvb < 0)
		inverter->dvb = 0;
	if(inverter->di > 150)
		inverter->di = 150;
	if(inverter->dib > 150)
		inverter->dib = 150;
	if(inverter->di < 0)
		inverter->di = 0;
	if(inverter->dib < 0)
		inverter->dib = 0;
	if(inverter->op > 300)
		inverter->op = 300;
	if(inverter->opb > 300)
		inverter->opb = 300;
	if(inverter->op < 0)
		inverter->op = 0;
	if(inverter->opb < 0)
		inverter->opb = 0;

	
	if(1 == ecu_type)	//SAA,45-55Hz
	{
		if(inverter->gf > 55)
			inverter->gf = 55;
		if(inverter->gf < 0)
			inverter->gf = 0;
	}
	else		//NA/MX,55-65Hz
	{
		if(inverter->gf > 65)
			inverter->gf = 65;
		if(inverter->gf < 0)
			inverter->gf = 0;
	}
	
	if(3 == ecu_type)		//MX,82-155V
	{
		if(inverter->gv > 155)
			inverter->gv = 155;
		if(inverter->gv < 0)
			inverter->gv = 0;
	}
	else if(2 == ecu_type)	//NA,181-298V
	{
		if(inverter->gv > 298)
			inverter->gv = 298;
		if(inverter->gv < 0)
			inverter->gv = 0;
	}
	else					//SAA,149-278V
	{
		if(inverter->gv > 278)
			inverter->gv = 278;
		if(inverter->gv < 0)
			inverter->gv = 0;
	}
	
	if(inverter->it > 150)
		inverter->it = 150;
	if(inverter->it < -50)
		inverter->it = -50;
	
	if(inverter->curgeneration > 0.999999)
		inverter->curgeneration = 0.999999;
	if(inverter->curgenerationb > 0.999999)
		inverter->curgenerationb = 0.999999;
	if(inverter->curgeneration < 0)
		inverter->curgeneration = 0;
	if(inverter->curgenerationb < 0)
		inverter->curgenerationb = 0;
	
	return 0;
}

int check_yc1000(struct inverter_info_t *inverter)		//在解析函数的最后调用
{
	if(inverter->dv > 1500)
		inverter->dv = 1500;
	if(inverter->dvb > 1500)
		inverter->dvb = 1500;
	if(inverter->dv < 0)
		inverter->dv = 0;
	if(inverter->dvb < 0)
		inverter->dvb = 0;
	if(inverter->di > 150)
		inverter->di = 150;
	if(inverter->dib > 150)
		inverter->dib = 150;
	if(inverter->dic > 150)
		inverter->dic = 150;
	if(inverter->did > 150)
		inverter->did = 150;
	if(inverter->di < 0)
		inverter->di = 0;
	if(inverter->dib < 0)
		inverter->dib = 0;
	if(inverter->dic < 0)
		inverter->dic = 0;
	if(inverter->did < 0)
		inverter->did = 0;
	if(inverter->op > 300)
		inverter->op = 300;
	if(inverter->opb > 300)
		inverter->opb = 300;
	if(inverter->opc > 300)
		inverter->opc = 300;
	if(inverter->opd > 300)
		inverter->opd = 300;
	if(inverter->op < 0)
		inverter->op = 0;
	if(inverter->opb < 0)
		inverter->opb = 0;
	if(inverter->opc < 0)
		inverter->opc = 0;
	if(inverter->opd < 0)
		inverter->opd = 0;


	if(1 == ecu_type)	//SAA,45-55Hz
	{
		if(inverter->gf > 55)
			inverter->gf = 55;
		if(inverter->gf < 0)
			inverter->gf = 0;
	}
	else		//NA/MX,55-65Hz
	{
		if(inverter->gf > 65)
			inverter->gf = 65;
		if(inverter->gf < 0)
			inverter->gf = 0;
	}

	if(3 == ecu_type)		//MX,82-155V
	{
		if(inverter->gv > 155)
			inverter->gv = 155;
		if(inverter->gv < 0)
			inverter->gv = 0;
	}
	else if(2 == ecu_type)	//NA,181-298V
	{
		if(inverter->gv > 298)
			inverter->gv = 298;
		if(inverter->gv < 0)
			inverter->gv = 0;
	}
	else					//SAA,149-278V
	{
		if(inverter->gv > 278)
			inverter->gv = 278;
		if(inverter->gv < 0)
			inverter->gv = 0;
	}

	if(inverter->it > 150)
		inverter->it = 150;
	if(inverter->it < -50)
		inverter->it = -50;

	if(inverter->curgeneration > 0.999999)
		inverter->curgeneration = 0.999999;
	if(inverter->curgenerationb > 0.999999)
		inverter->curgenerationb = 0.999999;
	if(inverter->curgenerationc > 0.999999)
		inverter->curgenerationc = 0.999999;
	if(inverter->curgenerationd > 0.999999)
		inverter->curgenerationd = 0.999999;
	if(inverter->curgeneration < 0)
		inverter->curgeneration = 0;
	if(inverter->curgenerationb < 0)
		inverter->curgenerationb = 0;
	if(inverter->curgenerationc < 0)
		inverter->curgenerationc = 0;
	if(inverter->curgenerationd < 0)
		inverter->curgenerationd = 0;

	return 0;
}

int check_optimizer_YC500(struct inverter_info_t *inverter)
{
	printmsg((char *)"Check Optimizer_YC500");

	if(inverter->output_voltage > 999)
		inverter->output_voltage = 999;

	if(inverter->output_current > 999)
		inverter->output_current = 999;

	if(inverter->output_power > 999)
		inverter->output_power = 999;

	if(inverter->output_energy_optimizer > 9999999)
		inverter->output_energy_optimizer = 9999999;

	if(inverter->temperature > 899)
		inverter->temperature = 899;

	if(inverter->input_voltage_pv1 > 999)
		inverter->input_voltage_pv1 = 999;

	if(inverter->input_current_pv1 > 999)
		inverter->input_current_pv1 = 999;

	if(inverter->input_power_pv1 > 999)
		inverter->input_power_pv1 = 999;

	if(inverter->input_energy_pv1 > 9999999)
		inverter->input_energy_pv1 = 9999999;

	if(inverter->input_voltage_pv2 > 999)
		inverter->input_voltage_pv2 = 999;

	if(inverter->input_current_pv2 > 999)
		inverter->input_current_pv2 = 999;

	if(inverter->input_power_pv2 > 999)
		inverter->input_power_pv2 = 999;

	if(inverter->input_energy_pv2 > 9999999)
		inverter->input_energy_pv2 = 9999999;

	return 0;
}


/*void modify_data(struct inverter_info_t *inverter)	//构造错误的数据，用于测试
{
//	inverter->dv = -1;
//	inverter->di = -1;
	inverter->op = -1;
//	inverter->np = 64.9;
//	inverter->nv = 81;
	inverter->it = -51;
//	inverter->curgeneration = -1;

}

void modify_data_yc500(struct inverter_info_t *inverter)	//构造错误的数据，用于测试
{
//	inverter->dv = -1;
//	inverter->dvb = -1;
//	inverter->di = -1;
//	inverter->dib = -1;
	inverter->op = -1;
	inverter->opb = -1;
//	inverter->np = 64.9;
//	inverter->nv = 81;
	inverter->it = -51;
//	inverter->curgeneration = -1;
//	inverter->curgenerationb = -1;

}*/
