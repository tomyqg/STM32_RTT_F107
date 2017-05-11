#ifndef __USR_WIFI232_H__
#define __USR_WIFI232_H__

#define WIFI_STATUS  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)

typedef enum 
{
	TYPE_IP,			//地址类型为IP
	TYPE_DOMAIN,	//地址类型为域名
	
}tcp_address_type;

typedef struct 
{
	tcp_address_type address_type; //使用地址的类型：IP或者域名 
	union AddressData
  {
		char ip[4];				//如果是IP给该部分赋值
		char domain[20];	//如果是域名给该部分赋值
	}address;
	unsigned short port;	//端口号
}tcp_address_t;


//打开WIFI串口 在使用WIFI的时候必须打开串口
int WiFi_Open(void);		
//清除WIFI串口缓冲区的数据
void clear_WIFI(void);
//发送数据   该发数据格式在串口指令模式
int WiFi_SendData(tcp_address_t address ,char *data ,int length);
//从串口接收数据
int WiFi_RecvData(int timeout,char *data);

// 进入AT模式
int AT(void);


/********************AT模式下相关的命令*********************************/
// 返回原来的模式  
int AT_ENTM(void);
//获取USR模块软件版本号命令
int AT_VER(void);
//模块复位命令
int AT_Z(void);
//模块恢复出厂化命令
int AT_RELD(void);
//配置SOCKET A的相关功能   TCP服务器模式
int AT_NETP(char *IP,int port);
//打开SOCKET B功能
int AT_TCPB_ON(void);
//配置SOCKET B的服务器IP地址
int AT_TCPADDB(char *IP);
//配置SOCKET B的服务器端口号
int AT_TCPPTB(int port);
//打开SOCKET C功能
int AT_TCPC_ON(void);
//配置SOCKET C的服务器IP地址
int AT_TCPADDC(char *IP);
//配置SOCKET C的服务器端口号
int AT_TCPPTC(int port);
//开启AP+STA功能模式
int AT_FAPSTA_ON(void);
//设置WIFI工作模式  STA or AP
int AT_WMODE(char *WMode);
//设置连接路由器SSID
int AT_WSSSID(char *SSSID);
//设置连接路由器KEY
int AT_WSKEY(char *SKEY);

/***********************************************************************/

int SendToSocketB(char *data ,int length);
int SendToSocketC(char *data ,int length);

#endif /*__USR_WIFI232_H__ */
