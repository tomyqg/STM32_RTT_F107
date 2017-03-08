#ifndef __USR_WIFI232_H__
#define __USR_WIFI232_H__

#define WIFI_STATUS  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)

typedef enum 
{
	TYPE_IP,
	TYPE_DOMAIN,
	
}tcp_address_type;

typedef struct 
{
	tcp_address_type address_type;
	union AddressData
  {
		char ip[4];
		char domain[20];
	}address;
	unsigned short port;
}tcp_address_t;



int WiFi_Open(void);
#endif /*__USR_WIFI232_H__ */
