#include <rtthread.h>
#include <board.h>

#ifdef RT_USING_LWIP
#include "stm32_eth.h"
#endif /* RT_USING_LWIP */

#ifdef RT_USING_SPI
#include "board_io_spi.h"

#if defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT)
#include "spi_flash_w25qxx.h"

#endif /* RT_USING_DFS */

/*
 * SPI1_MOSI: PA5
 * SPI1_MISO: PA4
 * SPI1_SCK : PA6
 *
 * CS0: PA3
*/
static void rt_hw_spi_init(void)
{
#ifdef RT_USING_SPI1
    /* register spi bus */
    {
        rt_hw_io_spi_init();
    }

    /* attach cs */
    {
        static struct rt_spi_device spi_device;
        extern struct stm32_spi_cs  spi_cs;

        GPIO_InitTypeDef GPIO_InitStructure;

        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

        /* spi10: PA3 */
        spi_cs.gpio_base = GPIOA_BASE;
        spi_cs.gpio_pin = GPIO_Pin_3;
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
        GPIO_SetBits(GPIOA, GPIO_Pin_3);
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        rt_spi_bus_attach_device(&spi_device, "spi10", "iospi1", (void*)&spi_cs);
    }
#endif /* RT_USING_SPI1 */
}
#endif /* RT_USING_SPI */


void rt_platform_init(void)
{
#ifdef RT_USING_SPI
    rt_hw_spi_init();

#if defined(RT_USING_DFS) && defined(RT_USING_DFS_ELMFAT)
    /* init sdcard driver */
    {
			w25qxx_init("flash","spi10");
    }
#endif /* RT_USING_DFS && RT_USING_DFS_ELMFAT */

#endif // RT_USING_SPI

#ifdef RT_USING_LWIP
    /* initialize eth interface */
    rt_hw_stm32_eth_init();
#endif /* RT_USING_LWIP */

}
