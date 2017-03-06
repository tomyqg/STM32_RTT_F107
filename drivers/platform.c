#include <rtthread.h>
#include <Lan8720A.h> 
#include <board.h>
#include <key.h>

#ifdef RT_USING_LWIP
#include "eth@stm32.h"
#endif /* RT_USING_LWIP */

void rt_platform_init(void)
{
#ifdef RT_USING_LWIP
    /* initialize eth interface */
    rt_hw_stm32_eth_init();
#endif /* RT_USING_LWIP */

	EXTIX_Init();		 	//外部中断初始化
}
