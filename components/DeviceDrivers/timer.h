#ifndef __TIMER_H
#define __TIMER_H
#include "stm32f10x.h"


extern signed char COMM_Timeout_Event;
extern signed char WIFI_RST_Event;

//WIFI串口接收超时中断
void TIM3_Int_Init(u16 arr,u16 psc);
void TIM3_Int_Deinit(void); 

//生产串口接收超时中断
void TIM4_Int_Init(u16 arr,u16 psc);
void TIM4_Int_Deinit(void); 

//1S定时器 
void TIM2_Int_Init(u16 arr,u16 psc);	//	TIM2_Int_Init(4999,7199);    1S产生一个中断
void TIM2_Int_Deinit(void); 
void TIM2_Refresh(void);

void switchLed(char Switch);


#endif
