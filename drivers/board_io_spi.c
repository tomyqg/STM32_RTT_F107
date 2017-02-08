/*--------------File Info---------------------------------------------------------------------------------
** File Name:           board_io_spi.c
** Last modified Date:  2015-04-25
** Last Version:        v1.00
** Description:         IO模拟spi主机的驱动程序
**--------------------------------------------------------------------------------------------------------
** Created By:          shengfeng dong
** Created date:        2015-04-25
** Version:             v1.00
** Descriptions:        
**--------------------------------------------------------------------------------------------------------
** Modified by:         
** Modified date:       
** Version:             
** Description:         
*********************************************************************************************************/
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <drivers/serial.h>
#include "rtreg.h"
#include "spi.h"
#include "board_io_spi.h"
#include "stm32f10x.h"

struct stm32_spi_cs  spi_cs;


#define GPIOx_BSRR_SET 		((uint32_t)0x0014)
#define GPIOx_BSRR_RESET 	((uint32_t)0x0010)
#define GPIOx_IDR 			((uint32_t)0x0008)
/*********************************************************************************************************
** SPI硬件连接配置
*********************************************************************************************************/
#ifdef RT_USING_IO_SPI1
// use PA5、PA6、PB5
#define IO_SPI1_GPIO_PERIPHERAL   RCC_APB2Periph_GPIOA
#define IO_SPI1_CLK_GPIO_BASE     GPIOA_BASE
#define IO_SPI1_CLK_GPIO_PIN      GPIO_Pin_6
#define IO_SPI1_MOSI_GPIO_BASE    GPIOA_BASE
#define IO_SPI1_MOSI_GPIO_PIN     GPIO_Pin_5
#define IO_SPI1_MISO_GPIO_BASE    GPIOA_BASE
#define IO_SPI1_MISO_GPIO_PIN     GPIO_Pin_4

#define IO_SPI1_CLK_H()      (HWREG16(IO_SPI1_CLK_GPIO_BASE + GPIOx_BSRR_SET) = IO_SPI1_CLK_GPIO_PIN)     //  将时钟线CLK置高
#define IO_SPI1_CLK_L()      (HWREG16(IO_SPI1_CLK_GPIO_BASE + GPIOx_BSRR_RESET) = IO_SPI1_CLK_GPIO_PIN)   //  将时钟线CLK置低
#define IO_SPI1_MOSI_H()     (HWREG16(IO_SPI1_MOSI_GPIO_BASE + GPIOx_BSRR_SET) = IO_SPI1_MOSI_GPIO_PIN)   //  将时钟线MOSI置高
#define IO_SPI1_MOSI_L()     (HWREG16(IO_SPI1_MOSI_GPIO_BASE + GPIOx_BSRR_RESET) = IO_SPI1_MOSI_GPIO_PIN) //  将时钟线MOSI置低

// MISO为输入引脚
#define IO_SPI1_MISO_STATE() (HWREG32(IO_SPI1_MISO_GPIO_BASE + GPIOx_IDR) & IO_SPI1_MISO_GPIO_PIN)        	  //  读MISO线的状态
#endif

/*********************************************************************************************************
** Function name:       configure
** Descriptions:        SPI配置
** Input parameters:    device: 	SPI设备描述符
** 			configure: 	配置参数
** Output parameters:   None 无
** Returned value:      None 无
*********************************************************************************************************/
static rt_err_t configure(struct rt_spi_device* device, struct rt_spi_configuration* configuration)
{
  return RT_EOK;
};

/*********************************************************************************************************
** Function name:       configure
** Descriptions:        SPI配置
** Input parameters:    device: 	SPI设备描述符
** 						configure: 	配置参数
** Output parameters:   None 无
** Returned value:      None 无
*********************************************************************************************************/
static rt_uint32_t xfer(struct rt_spi_device* device, struct rt_spi_message* message)
{
  //struct rt_spi_configuration * config = &device->config;
  struct stm32_spi_cs * stm32_spi_cs = &spi_cs;
  rt_uint32_t size = message->length;
  
  /* take CS */
  if(message->cs_take)
  {
    HWREG32(stm32_spi_cs->gpio_base + GPIOx_BSRR_RESET) = stm32_spi_cs->gpio_pin;
  }
  {
    const rt_uint8_t * send_ptr = message->send_buf;
    rt_uint8_t * recv_ptr = message->recv_buf;
    rt_uint8_t i;
    while(size--) {
      rt_uint8_t data = 0xFF;
      rt_uint8_t u8Rtn = 0;
      if(send_ptr != RT_NULL) {
        data = *send_ptr++;
      }
      
      rt_hw_us_delay(200);
      // 循环移出一个字节的数据
      for(i=0; i<8; i++)
      {
        IO_SPI1_CLK_L();
        if((data & 0x80) != 0)  {
          IO_SPI1_MOSI_H();
        } else {
          IO_SPI1_MOSI_L();
        }
        data <<= 1;
        u8Rtn <<= 1;
        if(IO_SPI1_MISO_STATE() != 0)  {
          u8Rtn |= 0x01;
        }
        rt_hw_us_delay(490);
        IO_SPI1_CLK_H();
        
        rt_hw_us_delay(500);
      }
      
      if(recv_ptr != RT_NULL) {
        *recv_ptr++ = u8Rtn;
      }
    }
  }
  
  /* release CS */
  if(message->cs_release)
  {
    HWREG32(stm32_spi_cs->gpio_base + GPIOx_BSRR_SET) = stm32_spi_cs->gpio_pin;
  }
  
  return message->length;
};

/*********************************************************************************************************
定义驱动注册结构
*********************************************************************************************************/
static struct rt_spi_ops stm32_io_spi1_ops =
{
  configure,
  xfer
};

#ifdef RT_USING_IO_SPI1
static struct stm32_io_spi_bus stm32_io_spi1_bus;
#endif /* #ifdef RT_USING_IO_SPI1 */
/*********************************************************************************************************
** Function name:       rt_hw_io_spi_init
** Descriptions:        SPI驱动初始化
** Input parameters:    None 无
** Output parameters:   None 无
** Returned value:      None 无
*********************************************************************************************************/
void rt_hw_io_spi_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
  struct stm32_io_spi_bus * stm32_spi;
  
#ifdef RT_USING_IO_SPI1
	RCC_APB2PeriphClockCmd(IO_SPI1_GPIO_PERIPHERAL, ENABLE);	
  
  // clk
   GPIO_InitStructure.GPIO_Pin = IO_SPI1_CLK_GPIO_PIN;				 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 

  // mosi
	GPIO_InitStructure.GPIO_Pin = IO_SPI1_MOSI_GPIO_PIN;				 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 
  // miso
	GPIO_InitStructure.GPIO_Pin =  IO_SPI1_MISO_GPIO_PIN;     
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
#endif
  
#ifdef RT_USING_IO_SPI1
  stm32_spi = &stm32_io_spi1_bus;
  rt_spi_bus_register(&stm32_spi->parent, "iospi1", &stm32_io_spi1_ops);
#endif
}

/*********************************************************************************************************
END FILE
*********************************************************************************************************/
