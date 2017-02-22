/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-02-22 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/
#ifndef __W25Q64FLASH_H__
#define __W25Q64FLASH_H__

/*****************************************************************************/
/*                                                                           */
/*  Definitions                                                              */
/*                                                                           */
/*****************************************************************************/
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;

/*****************************************************************************/
/*                                                                           */
/*  Function Declarations                                                    */
/*                                                                           */
/*****************************************************************************/
void SPI_Wait_Busy(void);
void SPI_Write_Enable(void);
void SPI_Read_nBytes(U32 Dst_Addr, U8 nBytes,U8 *header);//num=1,2
void SPI_Write_nBytes(U32 Dst_Addr, U8 nBytes, U8 *header);
void SPI_Erase_Block(U32 Dst_Addr);
void SPI_Erase_Half_Block(U32 Dst_Addr);
void SPI_Erase_Sector(U32 Dst_Addr);
void SPI_Erase_All(void);
void SPI_erase(U32 address,U16 numbers,U8 mode);
void SPI_WriteW25X_Disable(void);
void SPI_init(void);
#endif /*__W25Q64FLASH_H__*/
