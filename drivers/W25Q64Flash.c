/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-02-22 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Include Files                                                            */
/*                                                                           */
/*****************************************************************************/
#include "W25Q64Flash.h"
#include <rtthread.h>
#include <stm32f10x.h>

/*****************************************************************************/
/*                                                                           */
/*  Definitions                                                              */
/*                                                                           */
/*****************************************************************************/
#define W25_RCC                    RCC_APB2Periph_GPIOA
#define CLK_GPIO                   GPIOA
#define CLK_PIN                    (GPIO_Pin_6)

#define DO_GPIO                   GPIOA
#define DO_PIN                    (GPIO_Pin_4)

#define DI_GPIO                   GPIOA
#define DI_PIN                    (GPIO_Pin_5)

#define CS_GPIO                   GPIOA
#define CS_PIN                    (GPIO_Pin_3)

#define W25X_WriteEnable    0x06
#define W25X_WriteDisable   0x04
#define W25X_ReadStatusReg  0x05
#define W25X_ReadData       0x03
#define W25X_PageProgram    0x02
#define W25X_64KBErase      0xD8//0x20-sector-4KB*1024//0x52-1/2.block-32KB*128//0xd8-block-64KB*64//0xc7-all//
#define W25X_4KBErase       0x20//0x20-sector-4KB*1024//0x52-1/2.block-32KB*128//0xd8-block-64KB*64//0xc7-all//
#define W25X_32KBErase      0x52//0x20-sector-4KB*1024//0x52-1/2.block-32KB*128//0xd8-block-64KB*64//0xc7-all//
#define W25X_alKBErase      0xc7
#define FlashTestAdd        0x000000

/*****************************************************************************/
/*                                                                           */
/*  Function Implementations                                                 */
/*                                                                           */
/*****************************************************************************/
void SPI_Send_Byte(U8 out)
{
	U8 i = 0;
	
	GPIO_ResetBits(CS_GPIO, CS_PIN);
	for (i = 0; i < 8; i++)
	{
		if ((out & 0x80) == 0x80)	                                 //check if MSB is high
			GPIO_SetBits(DI_GPIO, DI_PIN);
		else
			GPIO_ResetBits(DI_GPIO, DI_PIN);		                                     //if not, set to low
		__NOP();
		__NOP();
		GPIO_SetBits(CLK_GPIO, CLK_PIN);			                                     //toggle clock high
		out = (out << 1);		                                     //shift 1 place for next bit
		__NOP();
		__NOP();
		__NOP();
	    __NOP();
		__NOP();
		GPIO_ResetBits(CLK_GPIO, CLK_PIN);			                                     //toggle clock low
	}
}

U8 SPI_Get_Byte(void)
{	
	U8 i = 0, in = 0, temp = 0;
	
	GPIO_ResetBits(CS_GPIO, CS_PIN);
	for (i = 0; i < 8; i++)
	{	
		__NOP();
		__NOP();
		in = (in << 1);						                         //shift 1 place to the left or shift in 0
		temp = GPIO_ReadInputDataBit(DO_GPIO, DO_PIN);;						                         //save input
		GPIO_SetBits(CLK_GPIO, CLK_PIN);						                         //toggle clock high
		__NOP();
		__NOP();
		if (temp == 1)						                         //check to see if bit is high
			in |= 0x01;						                         //if high, make bit high
		GPIO_ResetBits(CLK_GPIO, CLK_PIN);						                         //toggle clock low
	}	
	return in;
}

U8 SPI_Read_StatusReg(void)
{	
	U8 byte = 0;
	
	GPIO_ResetBits(CS_GPIO, CS_PIN);							                         //enable device
	SPI_Send_Byte(W25X_ReadStatusReg);		                         //send Read Status Register command
	byte = SPI_Get_Byte();					                         //receive byte
	GPIO_SetBits(CS_GPIO, CS_PIN);							                         //disable device	
	return byte;
}

void SPI_Wait_Busy(void)
{
 	//waste time until not busy WEL & Busy bit all be 1 (0x03).	
	//while(SPI_Read_StatusReg()==0x03)
	while ((SPI_Read_StatusReg()&0x03) == 0x03)
	{
		__NOP();
		__NOP();
		__NOP();
		__NOP();
	}
}


void SPI_Write_Enable(void)
{	
	GPIO_ResetBits(CS_GPIO, CS_PIN);							                         //enable device
	SPI_Send_Byte(W25X_WriteEnable);		                         //send W25X_Write_Enable command
	GPIO_SetBits(CS_GPIO, CS_PIN);							                         //disable device
}

void SPI_Read_nBytes(U32 Dst_Addr, U8 nBytes,U8 *header)//num=1,2
{
	U8 i;

	GPIO_ResetBits(CS_GPIO, CS_PIN);										             //enable device
	SPI_Send_Byte(W25X_ReadData);						             //read command
	SPI_Send_Byte((U8)((Dst_Addr & 0xFFFFFF) >> 16));		     //send 3 address bytes
	SPI_Send_Byte((U8)((Dst_Addr & 0xFFFF) >> 8));
	SPI_Send_Byte((U8)(Dst_Addr & 0xFF));
	for (i = 0; i < nBytes; i++)			                         //read until no_bytes is reached
	{
		*(header+i) = SPI_Get_Byte();					         //receive byte and store at address 80H - FFH
	}
	GPIO_SetBits(CS_GPIO, CS_PIN);									                 //disable device
}

void SPI_Write_nBytes(U32 Dst_Addr, U8 nBytes, U8 *header)
{	
	U8 i, byte;

	GPIO_ResetBits(CS_GPIO, CS_PIN);					                                 //enable device
	SPI_Write_Enable();				                                 //set WEL
	GPIO_ResetBits(CS_GPIO, CS_PIN);
	SPI_Send_Byte(W25X_PageProgram); 		                         //send Byte Program command
	SPI_Send_Byte((U8)((Dst_Addr & 0xFFFFFF) >> 16));	         //send 3 address bytes
	SPI_Send_Byte((U8)((Dst_Addr & 0xFFFF) >> 8));
	SPI_Send_Byte((U8)(Dst_Addr & 0xFF));
	
	for (i = 0; i < nBytes; i++)
	{
		byte = *(header+i);
		SPI_Send_Byte(byte);		                                 //send byte to be programmed
	}	
	GPIO_SetBits(CS_GPIO, CS_PIN);				                                     //disable device
}

void SPI_Erase_Block(U32 Dst_Addr)
{
	GPIO_ResetBits(CS_GPIO, CS_PIN);										             //enable device
	SPI_Write_Enable();									             //set WEL
	GPIO_ResetBits(CS_GPIO, CS_PIN);
	SPI_Send_Byte(W25X_64KBErase);			                         //send Sector Erase command
	SPI_Send_Byte((U8)((Dst_Addr & 0xFFFFFF) >> 16)); 	         //send 3 address bytes
	SPI_Send_Byte((U8)((Dst_Addr & 0xFFFF) >> 8));
	SPI_Send_Byte((U8)(Dst_Addr & 0xFF));
	GPIO_SetBits(CS_GPIO, CS_PIN);					                                 //disable device
}

void SPI_Erase_Half_Block(U32 Dst_Addr)
{
	GPIO_ResetBits(CS_GPIO, CS_PIN);										             //enable device
	SPI_Write_Enable();									             //set WEL
	GPIO_ResetBits(CS_GPIO, CS_PIN);
	SPI_Send_Byte(W25X_32KBErase);			                         //send Sector Erase command
	SPI_Send_Byte((U8)((Dst_Addr & 0xFFFFFF) >> 16)); 	         //send 3 address bytes
	SPI_Send_Byte((U8)((Dst_Addr & 0xFFFF) >> 8));
	SPI_Send_Byte((U8)(Dst_Addr & 0xFF));
	GPIO_SetBits(CS_GPIO, CS_PIN);					                                 //disable device
}

void SPI_Erase_Sector(U32 Dst_Addr)
{
	GPIO_ResetBits(CS_GPIO, CS_PIN);										             //enable device
	SPI_Write_Enable();									             //set WEL
	GPIO_ResetBits(CS_GPIO, CS_PIN);
	SPI_Send_Byte(W25X_4KBErase);			                         //send Sector Erase command
	SPI_Send_Byte((U8)((Dst_Addr & 0xFFFFFF) >> 16)); 	         //send 3 address bytes
	SPI_Send_Byte((U8)((Dst_Addr & 0xFFFF) >> 8));
	SPI_Send_Byte((U8)(Dst_Addr & 0xFF));
	GPIO_SetBits(CS_GPIO, CS_PIN);					                                 //disable device
}

void SPI_Erase_All(void)
{
	GPIO_ResetBits(CS_GPIO, CS_PIN);										             //enable device
	SPI_Write_Enable();									             //set WEL
	GPIO_ResetBits(CS_GPIO, CS_PIN);
	SPI_Send_Byte(W25X_alKBErase);			                         //send Sector Erase command
	GPIO_SetBits(CS_GPIO, CS_PIN);					                                 //disable device
}

void SPI_erase(U32 address,U16 numbers,U8 mode)
{
	U8 i;

	if(mode==1)
	{
		SPI_Erase_All();
		return;
	}
	if(mode==4)
	{
		for(i=0;i<numbers;i++)
		{
		 	SPI_Wait_Busy();                                         //0x010000 32KB  U16
			SPI_Erase_Sector(address);
			address=address+0x1000;                                  //0x001000 4 KB  U16
		}
		return;
	}
	if(mode==32)
	{
		for(i=0;i<numbers;i++)
		{
			SPI_Wait_Busy();
			SPI_Erase_Half_Block(address);
			address=address+0x8000;                                  //0x010000 32KB  U16                                   //0x001000 4 KB  U16
		}
		return;
	}
	if(mode==64)
	{
		for(i=0;i<numbers;i++)
		{
			SPI_Wait_Busy();
			SPI_Erase_Block(address);
			address=address+0x10000;                                 //0x010000 64KB  U16                                   //0x001000 4 KB  U16
		}
		return;
	}
}

void SPI_WriteW25X_Disable(void)
{
	GPIO_ResetBits(CS_GPIO, CS_PIN);							                         //enable device
	SPI_Send_Byte(W25X_WriteDisable);		                         //send W25X_WriteW25X_DIsable command
	GPIO_SetBits(CS_GPIO, CS_PIN);							                         //disable device
}

void SPI_init(void)
{
	   GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(W25_RCC,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = CLK_PIN;
    GPIO_Init(CLK_GPIO, &GPIO_InitStructure);
	
    GPIO_InitStructure.GPIO_Pin   = DI_PIN;
    GPIO_Init(DI_GPIO, &GPIO_InitStructure);
	
	  GPIO_InitStructure.GPIO_Pin   = CS_PIN;
    GPIO_Init(CS_GPIO, &GPIO_InitStructure);
	
	  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = DO_PIN;
    GPIO_Init(DO_GPIO, &GPIO_InitStructure);
	
	GPIO_ResetBits(CLK_GPIO, CLK_PIN);							                         //set clock to low initial state for SPI operation mode 0
//	GPIO_SetBits(CLK_GPIO, CLK_PIN);							                         //set clock to High initial state for SPI operation mode 3	
	GPIO_SetBits(CS_GPIO, CS_PIN);
	SPI_WriteW25X_Disable();
}


#ifdef RT_USING_FINSH
#include <finsh.h>
void FlashReadTest()
{
	unsigned char recvbuff[11];
	SPI_Read_nBytes(0x000000, 10,recvbuff);
	rt_kprintf("read date on address 0x000000:%s\n",recvbuff);
}
void FlashWriteTest()
{
	unsigned char sendbuff[11] = "YuNeng APS";
	SPI_Write_nBytes(0x000000, 10, sendbuff);
	rt_kprintf("write date on address 0x000000:%s\n",sendbuff);
}

void FlashEraseTest()
{
	SPI_Erase_Sector(0x000000);
	rt_kprintf("erase date on address 0x000000\n");
}

FINSH_FUNCTION_EXPORT(FlashReadTest, Flash Read Test.)
FINSH_FUNCTION_EXPORT(FlashWriteTest, Flash Write Test.)
FINSH_FUNCTION_EXPORT(FlashEraseTest, Flash Erase Test.)
#endif
