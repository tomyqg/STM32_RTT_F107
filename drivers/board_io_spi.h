/*--------------File Info---------------------------------------------------------------------------------
** File Name:           board_io_spi.h
** Last modified Date:  2017-02-08
** Last Version:        V1.00
** Description:         IO模拟spi主机的驱动程序
**--------------------------------------------------------------------------------------------------------
** Created By:          shengfeng dong
** Created date:        2017-02-08
** Version:             V1.00
** Descriptions:        
**--------------------------------------------------------------------------------------------------------
** Modified by:         
** Modified date:       
** Version:             
** Description:         
*********************************************************************************************************/
#ifndef __BOARD_IO_SPI_H__
#define __BOARD_IO_SPI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <rtthread.h>
#include "spi.h"
#include <rtdevice.h>



/*********************************************************************************************************
**  驱动配置
*********************************************************************************************************/
/* SPI driver select. */
#define RT_USING_IO_SPI1

/*********************************************************************************************************
stm32下的spi特性结构
*********************************************************************************************************/
struct stm32_io_spi_bus
{
  struct rt_spi_bus parent;
};


/*********************************************************************************************************
**  片选信号结构声明
*********************************************************************************************************/
struct stm32_spi_cs
{
	rt_uint32_t gpio_base;
	rt_uint16_t gpio_pin;
};


/*********************************************************************************************************
**  实现的外部函数声明
*********************************************************************************************************/
void rt_hw_io_spi_init(void);


#ifdef __cplusplus
    }
#endif      // __cplusplus
    
#endif      // __BOARD_IO_SPI_H__
/*********************************************************************************************************
END FILE
*********************************************************************************************************/
