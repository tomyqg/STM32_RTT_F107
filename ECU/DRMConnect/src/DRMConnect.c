#include "DRMConnect.h"
#include "stm32f10x.h"
#include "rtthread.h"
#include "file.h"
#include "stdio.h"
#include "debug.h"


#define DRM0_IN1_RCC                    RCC_APB2Periph_GPIOB
#define DRM0_IN1_GPIO                   GPIOB
#define DRM0_IN1_PIN                    (GPIO_Pin_8)

#define DRM0_IN2_RCC                    RCC_APB2Periph_GPIOB
#define DRM0_IN2_GPIO                   GPIOB
#define DRM0_IN2_PIN                    (GPIO_Pin_5)

#define DRM0_IN1					GPIO_ReadInputDataBit(DRM0_IN1_GPIO,DRM0_IN1_PIN)
#define DRM0_IN2					GPIO_ReadInputDataBit(DRM0_IN2_GPIO,DRM0_IN2_PIN)

void rt_hw_DRM_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(DRM0_IN1_RCC,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = DRM0_IN1_PIN;
    GPIO_Init(DRM0_IN1_GPIO, &GPIO_InitStructure);
		
		RCC_APB2PeriphClockCmd(DRM0_IN2_RCC,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = DRM0_IN2_PIN;
    GPIO_Init(DRM0_IN2_GPIO, &GPIO_InitStructure);
}


/*****************************************************************************/
/*  Function Implementations                                                 */
/*****************************************************************************/

/*****************************************************************************/
/* Function Description:                                                     */
/*****************************************************************************/
/*   DRM Connect Application program entry                                   */
/*****************************************************************************/
/* Parameters:                                                               */
/*****************************************************************************/
/*   parameter  unused                                                       */
/*****************************************************************************/
/* Return Values:                                                            */
/*****************************************************************************/
/*   void                                                                    */
/*****************************************************************************/
void DRM_Connect_thread_entry(void* parameter)
{
	int status = 2;   //当前设置开关机的状态  防止重复写入文件
	FILE *fp = NULL;
	rt_hw_DRM_init();
	
	//判断是否带有该功能  如果功能未打开：退出该线程  如果功能打开：运行改线程
	if(-1 == DRMFunction()) 	
	{
		printmsg(ECU_DBG_OTHER,"DRM Function Close!\n");
		return;
	}
	
	while(1)
	{
		//printf("DRM1: %d   DRM2: %d\n",DRM0_IN1,DRM0_IN2);
		
		if((DRM0_IN1 != DRM0_IN2) && (status !=1))
		{
			status = 1;
			fp = fopen("/tmp/connect.con","w");
			fprintf(fp,"connect all");
			fclose(fp);
			fp = NULL;
			printmsg(ECU_DBG_OTHER,"DRM connect all!\n");
		}
		
		if((DRM0_IN1 == DRM0_IN2) && (status != 0))
		{
			status = 0;
			
			fp = fopen("/tmp/connect.con","w");
			fprintf(fp,"disconnect all");
			fclose(fp);
			fp = NULL;
			printmsg(ECU_DBG_OTHER,"DRM disconnect all!\n");
		}	
		
		rt_thread_delay(RT_TICK_PER_SECOND);
	}
	
}
