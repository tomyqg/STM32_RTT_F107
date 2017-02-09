/*--------------File Info---------------------------------------------------------------------------------
** File Name:           board_i2c.c
** Last modified Date:  2017-02-09
** Last Version:        v1.00
** Description:         IO模拟i2c接口的驱动程序
**--------------------------------------------------------------------------------------------------------
** Created By:          ShengFeng Dong
** Created date:        2017-02-09
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
#include "stm32f10x.h"
#include "board_i2c.h"

#define GPIOx_BSRR_SET 		((uint32_t)0x0014)
#define GPIOx_BSRR_RESET 	((uint32_t)0x0010)
#define GPIOx_IDR 			((uint32_t)0x0008)


#ifdef RT_USING_I2C_BITOPS
/*********************************************************************************************************
** SPI硬件连接配置
*********************************************************************************************************/
#ifdef RT_USING_I2C1
// use PB6、PB7
#define I2C1_GPIO_PERIPHERAL    RCC_APB2Periph_GPIOB
#define I2C1_GPIO_SCL_BASE      GPIOB_BASE
#define I2C1_GPIO_SDA_BASE      GPIOB_BASE
#define I2C1_GPIO_SCL_PIN       GPIO_Pin_6
#define I2C1_GPIO_SDA_PIN       GPIO_Pin_7

#define I2C1_SCL_H()            (HWREG16(I2C1_GPIO_SCL_BASE + GPIOx_BSRR_SET) = I2C1_GPIO_SCL_PIN) 
#define I2C1_SCL_L()            (HWREG16(I2C1_GPIO_SCL_BASE + GPIOx_BSRR_RESET) = I2C1_GPIO_SCL_PIN)
#define I2C1_SDA_H()            (HWREG16(I2C1_GPIO_SDA_BASE + GPIOx_BSRR_SET) = I2C1_GPIO_SDA_PIN) 
#define I2C1_SDA_L()            (HWREG16(I2C1_GPIO_SDA_BASE + GPIOx_BSRR_RESET) = I2C1_GPIO_SDA_PIN)
#define I2C1_SCL_GET()          !!(HWREG32(I2C1_GPIO_SCL_BASE + GPIOx_IDR) & I2C1_GPIO_SCL_PIN)
#define I2C1_SDA_GET()          !!(HWREG32(I2C1_GPIO_SDA_BASE + GPIOx_IDR) & I2C1_GPIO_SDA_PIN)

#endif

#ifdef RT_USING_I2C1
/*********************************************************************************************************
** Function name:       rt_hw_i2c_init
** Descriptions:        i2c驱动初始化
** Input parameters:    None 无
** Output parameters:   None 无
** Returned value:      None 无
*********************************************************************************************************/
static struct rt_i2c_bus_device i2c_device0;

static void gpio0_set_sda(void *data, rt_int32_t state)
{
    if (state)
    {
        I2C1_SDA_H();
    }
    else
    {
        I2C1_SDA_L();
    }
}

static void gpio0_set_scl(void *data, rt_int32_t state)
{
    if (state)
    {
        I2C1_SCL_H();
    }
    else
    {
        I2C1_SCL_L(); 
    }
}

static rt_int32_t gpio0_get_sda(void *data)
{
    return I2C1_SDA_GET();
}

static rt_int32_t gpio0_get_scl(void *data)
{
    return I2C1_SCL_GET();
}

static const struct rt_i2c_bit_ops bit_ops0 =
{
    RT_NULL,
    gpio0_set_sda,
    gpio0_set_scl,
    gpio0_get_sda,
    gpio0_get_scl,

    rt_hw_us_delay,

    10,
    100
};
#endif /* endif of RT_USING_I2C1 */

/*********************************************************************************************************
** Function name:       rt_hw_i2c_init
** Descriptions:        i2c驱动初始化
** Input parameters:    None 无
** Output parameters:   None 无
** Returned value:      None 无
*********************************************************************************************************/
void rt_hw_i2c_init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
#ifdef RT_USING_I2C1
  RCC_APB2PeriphClockCmd(I2C1_GPIO_PERIPHERAL, ENABLE);	  

  // scl
  GPIO_InitStructure.GPIO_Pin = I2C1_GPIO_SCL_PIN;				 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 		 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_SetBits(GPIOB, I2C1_GPIO_SCL_PIN);

  // sda
  GPIO_InitStructure.GPIO_Pin = I2C1_GPIO_SDA_PIN;				 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 		 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_SetBits(GPIOB, I2C1_GPIO_SDA_PIN);
  
  rt_memset((void *)&i2c_device0, 0, sizeof(struct rt_i2c_bus_device));
  i2c_device0.priv = (void *)&bit_ops0;
  rt_i2c_bit_add_bus(&i2c_device0, "i2c1");
#endif
}
#endif   /* end of RT_USING_I2C_BITOPS */
/*********************************************************************************************************
END FILE
*********************************************************************************************************/
