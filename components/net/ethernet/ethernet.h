#ifndef __ETHERNET_H
#define __ETHERNET_H
#include <rtthread.h>
#include <netif/ethernetif.h>

//#define ETH_DEBUG
//#define ETH_RX_DUMP
//#define ETH_TX_DUMP

#ifdef ETH_DEBUG
#define STM32_ETH_TRACE	        rt_kprintf
#else
#define STM32_ETH_TRACE(...)
#endif /* ETH_DEBUG */
#define MAX_ADDR_LEN 				6
#define ETH_RXBUFNB        	4
#define ETH_TXBUFNB        	2

struct rt_stm32_eth
{
    /* inherit from ethernet device */
    struct eth_device parent;

    /* interface address info. */
    rt_uint8_t  dev_addr[MAX_ADDR_LEN];			/* hw address	*/
};

//extern struct rt_stm32_eth stm32_eth_device;
//extern struct rt_semaphore tx_buf_free;
//rt_err_t rt_stm32_eth_init(rt_device_t dev);
//rt_err_t rt_stm32_eth_open(rt_device_t dev, rt_uint16_t oflag);
//rt_err_t rt_stm32_eth_close(rt_device_t dev);
//rt_size_t rt_stm32_eth_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size);
//rt_size_t rt_stm32_eth_write (rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size);
//rt_err_t rt_stm32_eth_control(rt_device_t dev, rt_uint8_t cmd, void *args);
//struct pbuf *rt_stm32_eth_rx(rt_device_t dev);
//rt_err_t rt_stm32_eth_tx( rt_device_t dev, struct pbuf* p);

//rt_err_t eth_device_init(struct eth_device * dev, char *name);
rt_err_t rt_hw_ethernet_register(struct rt_stm32_eth *ethernet,
                               const char              *name,
                               rt_uint32_t              flag,
                               void                    *data);
void tx_buf_free_release(void);
#endif

