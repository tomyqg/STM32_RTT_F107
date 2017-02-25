/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-02-22 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/
#ifndef FLASH_24L256_H_
#define FLASH_24L256_H_
/*****************************************************************************/
/*                                                                           */
/*  Function Declarations                                                    */
/*                                                                           */
/*****************************************************************************/
void EEPROM_Init(void);
void I2CDelay(unsigned int n);
void I2CStart(void);
void I2CStop(void);
void I2CSendByte(unsigned char ucWRData);
void I2CReceiveACK(void);
unsigned char I2CReceiveByte(void);
void I2CAcknowledge(void);
void Write_24L512_Byte(unsigned int ucWRAddress,unsigned char uiWRData);		//单字节读取写入
void Write_24L512_nByte(unsigned int ucWRAddress,unsigned char Counter,unsigned char *Data_Ptr);//多字节数据读写入
unsigned char Read_24L512_Byte(unsigned int ucRDAddress);//单字节数据读取
void Read_24L512_nByte(unsigned int ucRDAddress,unsigned char Counter, unsigned char *Data_Ptr);//多字节数据读取

#endif /* FLASH_24L256_H_ */
