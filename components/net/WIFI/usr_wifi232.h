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



int WiFi_Open(void);		//打开WIFI串口
int WiFi_SendData(tcp_address_t address ,char *data ,int length);	//发送数据
int WiFi_RecvData(int timeout,char *data);//接收数据
#endif /*__USR_WIFI232_H__ */
