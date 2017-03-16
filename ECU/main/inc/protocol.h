#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__
#include "variation.h"

//把所有逆变器的数据按照ECU和EMA的通信协议转换，见协议
int protocol_APS18(struct inverter_info_t *firstinverter, char *sendcommanddatetime);

/* 逆变器异常状态A123 */
int protocol_status(struct inverter_info_t *firstinverter, char *datetime);
#endif  /*__PROTOCOL_H__*/
