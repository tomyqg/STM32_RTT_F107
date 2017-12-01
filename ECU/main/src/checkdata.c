/*****************************************************************************/
/*  File      : checkdata.c                                                  */
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
#include "checkdata.h"
#include <dfs_posix.h> 

/*****************************************************************************/
/*  Variable Declarations                                                    */
/*****************************************************************************/
extern int ecu_type;	//1:SAA; 2:NA; 3:MX

/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/

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
	if(inverter->op > 500)
		inverter->op = 500;
	if(inverter->opb > 500)
		inverter->opb = 500;
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

