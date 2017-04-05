#include <rtdevice.h>
#include <stm32f10x_rcc.h>
#include <eth@stm32.h>
#include <ethernet.h>
#include "file.h"
struct rt_stm32_eth Lan8720A;
/* interrupt service routine for ETH */
void ETH_IRQHandler(void)
{
    rt_uint32_t status;

    /* enter interrupt */
    rt_interrupt_enter();

    /* get DMA IT status */
    status = ETH->DMASR;

    if ( (status & ETH_DMA_IT_R) != (u32)RESET ) /* packet receiption */
    {
        /* a frame has been received */
        eth_device_ready(&(Lan8720A.parent));
			
        ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
    }

    if ( (status & ETH_DMA_IT_T) != (u32)RESET ) /* packet transmission */
    {
				tx_buf_free_release();
        ETH_DMAClearITPendingBit(ETH_DMA_IT_T);
    }

    /* Clear received IT */
    if ((status & ETH_DMA_IT_NIS) != (u32)RESET)
        ETH->DMASR = (u32)ETH_DMA_IT_NIS;
    if ((status & ETH_DMA_IT_AIS) != (u32)RESET)
        ETH->DMASR = (u32)ETH_DMA_IT_AIS;
    if ((status & ETH_DMA_IT_RO) != (u32)RESET)
        ETH->DMASR = (u32)ETH_DMA_IT_RO;

    if ((status & ETH_DMA_IT_RBU) != (u32)RESET)
    {
        ETH_ResumeDMAReception();
        ETH->DMASR = (u32)ETH_DMA_IT_RBU;
    }

    if ((status & ETH_DMA_IT_TBU) != (u32)RESET)
    {
        ETH_ResumeDMATransmission();
        ETH->DMASR = (u32)ETH_DMA_IT_TBU;
    }

    /* leave interrupt */
    rt_interrupt_leave();
}
/**
  * @brief  Configures RCC.
  * @param  None
  * @retval None
  */
static void RCC_Configuration(void)
{
	 /* 使能网口时钟 */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ETH_MAC | RCC_AHBPeriph_ETH_MAC_Tx |
                        RCC_AHBPeriph_ETH_MAC_Rx, ENABLE);

  /*使能用到的gpio时钟 */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |
                         RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_RMII);//MAC和PHY之间使用RMII接口
}

/**
  * @brief  Configures the different GPIO ports.
  * @param  None
  * @retval None
  */
static void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

	/* MDC */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* MDIO */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	
	/* TX_EN */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* TXD0 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* TXD1 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	 
  /* RX_DV/CRS_DV */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* RXD0 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* RXD1 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
 
  /* Reset */
	/*
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB, GPIO_Pin_10);//硬件复位LAN8720 	
 
  GPIO_SetBits(GPIOB, GPIO_Pin_10);//复位结束
	*/
}

/**
  * @brief  Configures the ADC.
  * @param  None
  * @retval None
  */


/**
  * @brief  Configures the nested vectored interrupt controller.
  * @param  None
  * @retval None
  */
static void ETH_NVIC_Configuration(void)
{
  NVIC_InitTypeDef   NVIC_InitStructure;

  /* Set the Vector Table base location at 0x08000000 */
  //NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);

  /* 2 bit for pre-emption priority, 2 bits for subpriority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
  
  /* Enable the Ethernet global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);    
  
}

rt_err_t rt_hw_stm32_eth_init()
{

    RCC_Configuration();
    GPIO_Configuration();
    ETH_NVIC_Configuration();
		RT_ASSERT((&Lan8720A)!= RT_NULL);
	  rt_memset(&(Lan8720A),0, sizeof(struct rt_stm32_eth));
		get_mac(Lan8720A.dev_addr);
		/*
    // OUI 00-80-E1 STMICROELECTRONICS
    Lan8720A.dev_addr[0] = 0x00;
    Lan8720A.dev_addr[1] = 0x80;
    Lan8720A.dev_addr[2] = 0xE1;
    // generate MAC addr from 96bit unique ID (only for test)
    Lan8720A.dev_addr[3] = *(rt_uint8_t*)(0x1FFFF7E8+7);
    Lan8720A.dev_addr[4] = *(rt_uint8_t*)(0x1FFFF7E8+8);
    Lan8720A.dev_addr[5] = *(rt_uint8_t*)(0x1FFFF7E8+9);
		*/
		return rt_hw_ethernet_register(&Lan8720A,"e0",RT_NULL,RT_NULL);
}

