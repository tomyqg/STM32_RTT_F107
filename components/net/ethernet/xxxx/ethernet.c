
/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
/*
 * STM32 Eth Driver for RT-Thread
 * Change Logs:
 * Date           Author       Notes
 * 2009-10-05     Bernard      eth interface driver for STM32F107 CL
 */

#include <rtthread.h>
#include <netif/etharp.h>
#include <lwip/icmp.h>
#include "lwipopts.h" 
#include "stm32_eth.h"
#include <rtdevice.h>
#include <ethernet.h>


#if defined(ETH_RX_DUMP) ||  defined(ETH_TX_DUMP)
static void packet_dump(const char * msg, const struct pbuf* p)
{
    rt_uint32_t i;
    rt_uint8_t *ptr = p->payload;

    STM32_ETH_TRACE("%s %d byte\n", msg, p->tot_len);

    for(i=0; i<p->tot_len; i++)
    {
        if( (i%8) == 0 )
        {
            STM32_ETH_TRACE("  ");
        }
        if( (i%16) == 0 )
        {
            STM32_ETH_TRACE("\r\n");
        }
        STM32_ETH_TRACE("%02x ",*ptr);
        ptr++;
    }
    STM32_ETH_TRACE("\n\n");
}
#endif /* dump */

static ETH_DMADESCTypeDef  DMARxDscrTab[ETH_RXBUFNB], DMATxDscrTab[ETH_TXBUFNB];
static rt_uint8_t Rx_Buff[ETH_RXBUFNB][ETH_MAX_PACKET_SIZE], Tx_Buff[ETH_TXBUFNB][ETH_MAX_PACKET_SIZE];
static struct rt_semaphore tx_buf_free;
 
/* RT-Thread Device Interface */

/*********************************************************接口函数定义*********************************************************/
//初始化ETH MAC层及DMA配置
//返回值:ETH_ERROR,发送失败(0)
//		ETH_SUCCESS,发送成功(1)
u8 ETH_MACDMA_Config(void)
{
	u8 rval;
	ETH_InitTypeDef ETH_InitStructure; 
	 
	ETH_DeInit();  								//AHB总线重启以太网
	ETH_SoftwareReset();  						//软件重启网络
	while (ETH_GetSoftwareResetStatus() == SET);//等待软件重启网络完成 
	ETH_StructInit(&ETH_InitStructure); 	 	//初始化网络为默认值  
	///网络MAC参数设置 
	ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;   			//开启网络自适应功能
	ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;					//关闭反馈
	ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable; 		//关闭重传功能
	ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable; 	//关闭自动去除PDA/CRC功能 
	ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;						//关闭接收所有的帧
	ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;//允许接收所有广播帧
	ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;			//关闭混合模式的地址过滤  
	ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;//对于组播地址使用完美地址过滤   
	ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;	//对单播地址使用完美地址过滤 
#ifdef CHECKSUM_BY_HARDWARE
	ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable; 			//开启ipv4和TCP/UDP/ICMP的帧校验和卸载   
#endif
	//当我们使用帧校验和卸载功能的时候，一定要使能存储转发模式,存储转发模式中要保证整个帧存储在FIFO中,
	//这样MAC能插入/识别出帧校验值,当真校验正确的时候DMA就可以处理帧,否则就丢弃掉该帧
	ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable; //开启丢弃TCP/IP错误帧
	ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;     //开启接收数据的存储转发模式    
	ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;   //开启发送数据的存储转发模式  

	ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;     	//禁止转发错误帧  
	ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;	//不转发过小的好帧 
	ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;  		//打开处理第二帧功能
	ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;  	//开启DMA传输的地址对齐功能
	ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;            			//开启固定突发功能    
	ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;     		//DMA发送的最大突发长度为32个节拍   
	ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;			//DMA接收的最大突发长度为32个节拍
	ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;
	rval=ETH_Init(&ETH_InitStructure,PHY_ADDRESS);		//配置ETH
	if(rval==ETH_SUCCESS)//配置成功
	{
		//ETH_DMAITConfig(ETH_DMA_IT_NIS|ETH_DMA_IT_R,ENABLE);  	//使能以太网接收中断	
	  ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R | ETH_DMA_IT_T, ENABLE);//使能以太网接收发送中断	
	}
	return rval;
}
/* initialize the interface */
rt_err_t rt_stm32_eth_init(rt_device_t dev)
{
	struct rt_stm32_eth *stm32_eth_device;
	u8 rval;
	rval=ETH_MACDMA_Config();
  if(rval==ETH_ERROR)
		return RT_ERROR;
  RT_ASSERT(dev != RT_NULL);
  stm32_eth_device = (struct rt_stm32_eth*)dev;
	/* Initialize Tx Descriptors list: Chain Mode */
	ETH_DMATxDescChainInit(DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);
	/* Initialize Rx Descriptors list: Chain Mode  */
	ETH_DMARxDescChainInit(DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);

	/* MAC address configuration */
	ETH_MACAddressConfig(ETH_MAC_Address0, (u8*)&stm32_eth_device->dev_addr[0]);

	/* Enable MAC and DMA transmission and reception */
	ETH_Start();

  return RT_EOK;
}
rt_err_t rt_stm32_eth_open(rt_device_t dev, rt_uint16_t oflag)
{
    return RT_EOK;
}

rt_err_t rt_stm32_eth_close(rt_device_t dev)
{
    return RT_EOK;
}
rt_size_t rt_stm32_eth_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    rt_set_errno(-RT_ENOSYS);
    return 0;
}
rt_size_t rt_stm32_eth_write (rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
    rt_set_errno(-RT_ENOSYS);
    return 0;
}
rt_err_t rt_stm32_eth_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
		struct rt_stm32_eth *stm32_eth_device;
		RT_ASSERT(dev != RT_NULL);
		stm32_eth_device = (struct rt_stm32_eth*)dev;
    switch(cmd)
    {
    case NIOCTL_GADDR:
        /* get mac address */
        if(args) rt_memcpy(args, stm32_eth_device->dev_addr, 6);
        else return -RT_ERROR;
        break;

    default :
        break;
    }

    return RT_EOK;
}

/* ethernet device interface */
/* transmit packet. */
rt_err_t rt_stm32_eth_tx( rt_device_t dev, struct pbuf* p)
{
    struct pbuf* q;
    rt_uint32_t offset;

    /* get free tx buffer */
    {
        rt_err_t result;
        result = rt_sem_take(&tx_buf_free, 2);
        if (result != RT_EOK) return -RT_ERROR;
    }

    offset = 0;
    for (q = p; q != NULL; q = q->next)
    {
        rt_uint8_t* ptr;
        rt_uint32_t len;

        len = q->len;
        ptr = q->payload;

        /* Copy the frame to be sent into memory pointed by the current ETHERNET DMA Tx descriptor */
        while (len)
        {
            (*(__IO uint8_t *)((DMATxDescToSet->Buffer1Addr) + offset)) = *ptr;

            offset ++;
            ptr ++;
            len --;
        }
    }

#ifdef ETH_TX_DUMP
    packet_dump("TX dump", p);
#endif

    /* Setting the Frame Length: bits[12:0] */
    DMATxDescToSet->ControlBufferSize = (p->tot_len & ETH_DMATxDesc_TBS1);
    /* Setting the last segment and first segment bits (in this case a frame is transmitted in one descriptor) */
    DMATxDescToSet->Status |= ETH_DMATxDesc_LS | ETH_DMATxDesc_FS;
    /* Enable TX Completion Interrupt */
    DMATxDescToSet->Status |= ETH_DMATxDesc_IC;

#ifdef CHECKSUM_BY_HARDWARE
    DMATxDescToSet->Status |= ETH_DMATxDesc_ChecksumTCPUDPICMPFull;
    /* clean ICMP checksum STM32F need */
    {
        struct eth_hdr *ethhdr = (struct eth_hdr *)(DMATxDescToSet->Buffer1Addr);
        /* is IP ? */
        if( ethhdr->type == htons(ETHTYPE_IP) )
        {
            struct ip_hdr *iphdr = (struct ip_hdr *)(DMATxDescToSet->Buffer1Addr + SIZEOF_ETH_HDR);
            /* is ICMP ? */
            if( IPH_PROTO(iphdr) == IP_PROTO_ICMP )
            {
                struct icmp_echo_hdr *iecho = (struct icmp_echo_hdr *)(DMATxDescToSet->Buffer1Addr + SIZEOF_ETH_HDR + sizeof(struct ip_hdr) );
                iecho->chksum = 0;
            }
        }
    }
#endif /* CHECKSUM_BY_HARDWARE */

    /* Set Own bit of the Tx descriptor Status: gives the buffer back to ETHERNET DMA */
    DMATxDescToSet->Status |= ETH_DMATxDesc_OWN;
    /* When Tx Buffer unavailable flag is set: clear it and resume transmission */
    if ((ETH->DMASR & ETH_DMASR_TBUS) != (uint32_t)RESET)
    {
        /* Clear TBUS ETHERNET DMA flag */
        ETH->DMASR = ETH_DMASR_TBUS;
        /* Transmit Poll Demand to resume DMA transmission*/
        ETH->DMATPDR = 0;
    }

    /* Update the ETHERNET DMA global Tx descriptor with next Tx decriptor */
    /* Chained Mode */
    /* Selects the next DMA Tx descriptor list for next buffer to send */
    DMATxDescToSet = (ETH_DMADESCTypeDef*) (DMATxDescToSet->Buffer2NextDescAddr);

    /* Return SUCCESS */
    return RT_EOK;
}

/* reception packet. */
struct pbuf *rt_stm32_eth_rx(rt_device_t dev)
{
    struct pbuf* p;
    rt_uint32_t framelength = 0;

    /* init p pointer */
    p = RT_NULL;

    /* Check if the descriptor is owned by the ETHERNET DMA (when set) or CPU (when reset) */
    if(((DMARxDescToGet->Status & ETH_DMARxDesc_OWN) != (uint32_t)RESET))
        return p;

    if (((DMARxDescToGet->Status & ETH_DMARxDesc_ES) == (uint32_t)RESET) &&
            ((DMARxDescToGet->Status & ETH_DMARxDesc_LS) != (uint32_t)RESET) &&
            ((DMARxDescToGet->Status & ETH_DMARxDesc_FS) != (uint32_t)RESET))
    {
        /* Get the Frame Length of the received packet: substruct 4 bytes of the CRC */
        framelength = ((DMARxDescToGet->Status & ETH_DMARxDesc_FL) >> ETH_DMARXDESC_FRAME_LENGTHSHIFT) - 4;

        /* allocate buffer */
        p = pbuf_alloc(PBUF_LINK, framelength, PBUF_RAM);
        if (p != RT_NULL)
        {
            const char * from;
            struct pbuf* q;

            from = (const char *)(DMARxDescToGet->Buffer1Addr);

            for (q = p; q != RT_NULL; q= q->next)
            {
                /* Copy the received frame into buffer from memory pointed by the current ETHERNET DMA Rx descriptor */
                memcpy(q->payload, from, q->len);
                from += q->len;
            }

#ifdef ETH_RX_DUMP
            packet_dump("RX dump", p);
#endif /* ETH_RX_DUMP */
        }
    }

    /* Set Own bit of the Rx descriptor Status: gives the buffer back to ETHERNET DMA */
    DMARxDescToGet->Status = ETH_DMARxDesc_OWN;

    /* When Rx Buffer unavailable flag is set: clear it and resume reception */
    if ((ETH->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET)
    {
        /* Clear RBUS ETHERNET DMA flag */
        ETH->DMASR = ETH_DMASR_RBUS;
        /* Resume DMA reception */
        ETH->DMARPDR = 0;
    }

    /* Update the ETHERNET DMA global Rx descriptor with next Rx decriptor */
    /* Chained Mode */
    if((DMARxDescToGet->ControlBufferSize & ETH_DMARxDesc_RCH) != (uint32_t)RESET)
    {
        /* Selects the next DMA Rx descriptor list for next buffer to read */
        DMARxDescToGet = (ETH_DMADESCTypeDef*) (DMARxDescToGet->Buffer2NextDescAddr);
    }
    else /* Ring Mode */
    {
        if((DMARxDescToGet->ControlBufferSize & ETH_DMARxDesc_RER) != (uint32_t)RESET)
        {
            /* Selects the first DMA Rx descriptor for next buffer to read: last Rx descriptor was used */
            DMARxDescToGet = (ETH_DMADESCTypeDef*) (ETH->DMARDLAR);
        }
        else
        {
            /* Selects the next DMA Rx descriptor list for next buffer to read */
            DMARxDescToGet = (ETH_DMADESCTypeDef*) ((uint32_t)DMARxDescToGet + 0x10 + ((ETH->DMABMR & ETH_DMABMR_DSL) >> 2));
        }
    }

    return p;
}

rt_err_t rt_hw_ethernet_register(struct rt_stm32_eth *ethernet,
                               const char              *name,
                               rt_uint32_t              flag,
                               void                    *data)
{
    struct rt_device *device;
    RT_ASSERT(ethernet != RT_NULL);
		device= &(ethernet->parent.parent);

    device->init       = rt_stm32_eth_init;
		device->open       = rt_stm32_eth_open;
    device->close      = rt_stm32_eth_close;
    device->read       = rt_stm32_eth_read;
    device->write      = rt_stm32_eth_write;
    device->control    = rt_stm32_eth_control;
    device->user_data  = RT_NULL;

    ethernet->parent.eth_rx     = rt_stm32_eth_rx;
    ethernet->parent.eth_tx     = rt_stm32_eth_tx;

    /* init tx buffer free semaphore */
    rt_sem_init(&tx_buf_free, "tx_buf", ETH_TXBUFNB, RT_IPC_FLAG_FIFO);

    /* register eth device */
    return eth_device_init(&(ethernet->parent),(char*)name);
}
void tx_buf_free_release(void)
{
	 rt_sem_release(&tx_buf_free);
}
#ifdef RT_USING_FINSH
#include <finsh.h>
static void phy_search(void)
{
    int i;
    int value;

    for(i=0; i<32; i++)
    {
        value = ETH_ReadPHYRegister(i, 2);
        rt_kprintf("addr %02d: %04X\n", i, value);
    }
}
FINSH_FUNCTION_EXPORT(phy_search, search phy use MDIO);

static void phy_dump(int addr)
{
    int i;
    int value;

    rt_kprintf("dump phy addr %d\n", addr);

    for(i=0; i<32; i++)
    {
        value = ETH_ReadPHYRegister(addr, i);
        rt_kprintf("reg %02d: %04X\n", i, value);
    }
}
FINSH_FUNCTION_EXPORT(phy_dump, dump PHY register);

static void phy_write(int addr, int reg, int value)
{
    ETH_WritePHYRegister(addr, reg ,value);
}
FINSH_FUNCTION_EXPORT(phy_write, write PHY register);

static void emac_dump(int addr)
{
    int i;
    int value;
    int *p = (int *)ETH;

    rt_kprintf("dump EAMC reg %d\n", addr);

    for(i=0; i<sizeof(ETH_TypeDef)/4; i++)
    {
        value = *p++;
        rt_kprintf("reg %04X: %08X\n", i*4, value);
    }
}
FINSH_FUNCTION_EXPORT(emac_dump, dump EMAC register);

#endif // RT_USING_FINSH
