#ifndef __FLASH_IF_H__
#define __FLASH_IF_H__

#include "stm32f10x.h"

uint32_t FLASH_If_Erase_APP2(void);
uint32_t FLASH_IF_FILE_COPY_TO_APP2(char * updateFileName);

#endif /*__FLASH_IF_H__*/
