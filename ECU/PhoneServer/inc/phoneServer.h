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


void Phone_GetBaseInfo(unsigned char * ID,int Data_Len,const char *recvbuffer); 				//获取基本信息请求
void Phone_GetGenerationData(unsigned char * ID,int Data_Len,const char *recvbuffer); 	//获取逆变器发电量数据
void Phone_GetPowerCurve(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//获取功率曲线
void Phone_GetGenerationCurve(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//获取发电量曲线
void Phone_RegisterID(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//逆变器ID注册
void Phone_SetTime(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//ECU时间设置
void Phone_SetWiredNetwork(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//有线网络设置
void Phone_SetWIFI(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//无线网络连接
void Phone_SearchWIFIStatus(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//无线网络连接状态
void Phone_SetWIFIPasswd(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//AP密码设置
void Phone_GetIDInfo(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//获取ID信息
void Phone_GetTime(unsigned char * ID,int Data_Len,const char *recvbuffer); 			//获取时间


void phone_server_thread_entry(void* parameter);

#endif /*__PHONESERVER_H__*/
