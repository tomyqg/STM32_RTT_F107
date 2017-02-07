/*
 * File      : stm32f1_rtc.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version.
 * 2011-11-26     aozima       implementation time.
 * 2015-07-16     FlyM         rename rtc to stm32f1_rtc. remove finsh export function
 */

#include <rtthread.h>
#include <time.h>
#include <string.h>
#include <stm32f10x.h>

#define DS1302CLK GPIO_Pin_0   //与时钟线相连的芯片的管脚
#define DS1302DAT GPIO_Pin_8   //与数据线相连的芯片的管脚
#define DS1302RST GPIO_Pin_2   //与复位端相连的芯片的管脚

static struct rt_device rtc;

void ds1302_writebyte(unsigned char dat)
{
	unsigned char i;
	unsigned char sda;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_ResetBits(GPIOC,DS1302CLK);		
	rt_thread_delay(1);
	
	GPIO_InitStructure.GPIO_Pin =  DS1302DAT;     
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	rt_thread_delay(1);			
	for(i=0; i<8; i++){
		sda = dat & 0x01;
		if(sda)
		{
			GPIO_SetBits(GPIOC,DS1302DAT);
		}else
		{
			GPIO_ResetBits(GPIOC,DS1302DAT);
		}
		rt_thread_delay(1);			
		GPIO_SetBits(GPIOC,DS1302CLK);	
		rt_thread_delay(1);			
		GPIO_ResetBits(GPIOC,DS1302CLK);	
		rt_thread_delay(1);			
		dat >>= 1;
	}
}

unsigned char ds1302_readbyte(void)
{
	unsigned char i, dat = 0x00;
    GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin =  DS1302DAT;     
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	rt_thread_delay(1);

	for(i=0; i<8; i++){
		dat >>= 1;
		if(1 == GPIO_ReadInputDataBit(GPIOC, DS1302DAT))
			dat |= 0x80;
		GPIO_SetBits(GPIOC,DS1302CLK);	
		rt_thread_delay(1);			
		GPIO_ResetBits(GPIOC,DS1302CLK);	
		rt_thread_delay(1);	
	}

	return dat;
}

unsigned char ds1302_read(unsigned char cmd)
{
	unsigned char data;

	GPIO_ResetBits(GPIOC,DS1302RST);	
	rt_thread_delay(1);	
	GPIO_ResetBits(GPIOC,DS1302CLK);	
	rt_thread_delay(1);	
	GPIO_SetBits(GPIOC,DS1302RST);		
	rt_thread_delay(1);	
	ds1302_writebyte(cmd);
	rt_thread_delay(1);	
	data = ds1302_readbyte();
	rt_thread_delay(1);	
	GPIO_SetBits(GPIOC,DS1302CLK);		
	rt_thread_delay(1);	
	GPIO_ResetBits(GPIOC,DS1302RST);	

	return data;
}

void ds1302_write(unsigned char cmd, unsigned char data)
{
	GPIO_ResetBits(GPIOC,DS1302RST);	
	rt_thread_delay(1);
	GPIO_ResetBits(GPIOC,DS1302CLK);	
	rt_thread_delay(1);
	GPIO_SetBits(GPIOC,DS1302RST);		
	rt_thread_delay(1);

	ds1302_writebyte(cmd);
	ds1302_writebyte(data);
	rt_thread_delay(1);
	GPIO_SetBits(GPIOC,DS1302CLK);	
	rt_thread_delay(1);
	GPIO_ResetBits(GPIOC,DS1302RST);	
}



static rt_err_t rt_rtc_open(rt_device_t dev, rt_uint16_t oflag)
{
    if (dev->rx_indicate != RT_NULL)
    {
        /* Open Interrupt */
    }

    return RT_EOK;
}

static rt_size_t rt_rtc_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    return 0;
}

static rt_err_t rt_rtc_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
    rt_time_t *time;
    RT_ASSERT(dev != RT_NULL);
		
    switch (cmd)
    {
    case RT_DEVICE_CTRL_RTC_GET_TIME:		//获取时间
	{
		time_t now;
    struct tm tm_new;
		unsigned char year, month, day, hour, minute, second;
		time = (rt_time_t *)args;
        /* read device */
		year = ds1302_read(0x8D);
		month = ds1302_read(0x89);
		day = ds1302_read(0x87);
		hour = ds1302_read(0x85);
		minute = ds1302_read(0x83);
		second = ds1302_read(0x81);
		tm_new.tm_year = year;
		tm_new.tm_mon = month;
		tm_new.tm_mday = day;
		tm_new.tm_hour = hour;
		tm_new.tm_min = minute;
		tm_new.tm_sec = second;
		//转换为1970.1.1开始的时间
		now = mktime(&tm_new);
		*time = (rt_time_t)now;
        
	}     
        break;

    case RT_DEVICE_CTRL_RTC_SET_TIME:		//设置时间
    {
			time_t set_time;
			struct tm *p_tm;
			struct tm tm_new;
			unsigned char year, month, day, hour, minute, second;
			time = (rt_time_t *)args;
			//转换为年月日时分秒时间
			set_time = *time;
			p_tm = localtime(&set_time);
			memcpy(&tm_new, p_tm, sizeof(struct tm));
			year = tm_new.tm_year;
			month = tm_new.tm_mon;
			day = tm_new.tm_mday;
			hour = tm_new.tm_hour;
			minute = tm_new.tm_min;
			second = tm_new.tm_sec;
			
			ds1302_write(0x8C, year);
			ds1302_write(0x88, month);
			ds1302_write(0x86, day);
			ds1302_write(0x84, hour);
			ds1302_write(0x82, minute);
			ds1302_write(0x80, second);
		
    }
    break;
    }

    return RT_EOK;
}

/*******************************************************************************
* Function Name  : RTC_Configuration
* Description    : Configures the RTC.
* Input          : None
* Output         : None
* Return         : 0 reday,-1 error.
*******************************************************************************/
int RTC_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/* PC 2,8,为输出*/
	GPIO_InitStructure.GPIO_Pin =  DS1302CLK | DS1302RST;   
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//PA0配置为开漏模式，此模式下可以实现真下的双向IO  
	GPIO_InitStructure.GPIO_Pin =  DS1302DAT;     
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;    
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
    return 0;
}

void rt_hw_rtc_init(void)
{
    rtc.type	= RT_Device_Class_RTC;

    if ( RTC_Configuration() != 0)
    {
        rt_kprintf("rtc configure fail...\r\n");
        return ;
    }


    /* register rtc device */
    rtc.init 	= RT_NULL;
    rtc.open 	= rt_rtc_open;
    rtc.close	= RT_NULL;
    rtc.read 	= rt_rtc_read;
    rtc.write	= RT_NULL;
    rtc.control = rt_rtc_control;

    /* no private */
    rtc.user_data = RT_NULL;

    rt_device_register(&rtc, "rtc", RT_DEVICE_FLAG_RDWR);

    return;
}


