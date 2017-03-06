#ifndef __KEY_H
#define __KEY_H	 
 
#define KEY_Reset  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9)//?áè?°′?ü0

void KEY_Init(void);//IO初始化
void EXTIX_Init(void);//外部中断初始化   
#endif
