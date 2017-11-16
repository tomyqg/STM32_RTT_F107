#ifndef __PHONESERVER_H__
#define	__PHONESERVER_H__
/*****************************************************************************/
/* File      : phoneServer.h                                                 */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-05-19 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/

//获取基本信息
void Phone_GetBaseInfo(unsigned char * ID,int Data_Len,const char *recvbuffer); 				//获取基本信息请求
//获取发电量数据
void Phone_GetGenerationData(unsigned char * ID,int Data_Len,const char *recvbuffer); 	//获取逆变器发电量数据
//获取功率曲线
void Phone_GetPowerCurve(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//获取功率曲线
//获取发电量曲线
void Phone_GetGenerationCurve(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//获取发电量曲线
//注册ID
void Phone_RegisterID(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//逆变器ID注册
//设置时间
void Phone_SetTime(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//ECU时间设置
//设置有线网络
void Phone_SetWiredNetwork(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//有线网络设置
//获取硬件信息
void Phone_GetECUHardwareStatus(unsigned char * ID,int Data_Len,const char *recvbuffer);
//设置WIFI AP 密码
void Phone_SetWIFIPasswd(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//AP密码设置
//获取ID信息
void Phone_GetIDInfo(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//获取ID信息
//获取时间
void Phone_GetTime(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//获取时间
//获取Flash空间
void Phone_FlashSize(unsigned char * ID,int Data_Len,const char *recvbuffer);
//获取有线网络信息
void Phone_GetWiredNetwork(unsigned char * ID,int Data_Len,const char *recvbuffer);			//有线网络设置

void Phone_SetChannel(unsigned char * ID,int Data_Len,const char *recvbuffer);

void Phone_GetShortAddrInfo(unsigned char * ID,int Data_Len,const char *recvbuffer);
//Phone Server线程
void phone_server_thread_entry(void* parameter);

#endif /*__PHONESERVER_H__*/
