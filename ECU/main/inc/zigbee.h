#ifndef __ZIGBEE_H__
#define __ZIGBEE_H__
#include "variation.h"

int selectZigbee(int timeout);			//zigbee串口数据检测 返回0 表示串口没有数据  返回1表示串口有数据
int openzigbee(void);
void zigbee_reset(void);
int zb_shortaddr_cmd(int shortaddr, char *buff, int length);		//zigbee 短地址报头
int zb_shortaddr_reply(char *data,int shortaddr,char *id);			//读取逆变器的返回帧,短地址模式
int zb_get_reply(char *data,inverter_info *inverter);			//读取逆变器的返回帧
int zb_get_reply_update_start(char *data,inverter_info *inverter);			//读取逆变器远程更新的Update_start返回帧，ZK，返回响应时间定为10秒
int zb_get_reply_restore(char *data,inverter_info *inverter);			//读取逆变器远程更新失败，还原指令后的返回帧，ZK，因为还原时间比较长，所以单独写一个函数
int zb_get_reply_from_module(char *data);			//读取zigbee模块的返回帧
int zb_get_id(char *data);			//获取逆变器ID
int zb_turnoff_limited_rptid(int short_addr,inverter_info *inverter);			//关闭限定单个逆变器上报ID功能
int zb_turnoff_rptid(int short_addr);			//关闭单个逆变器上报ID功能
int zb_get_inverter_shortaddress_single(inverter_info *inverter);			//获取单台指定逆变器短地址，ZK
int zb_turnon_rtpid(inverter_info *firstinverter);			//开启逆变器自动上报ID
int zb_change_inverter_panid_broadcast(void);	//广播改变逆变器的PANID，ZK
int zb_change_inverter_panid_single(inverter_info *inverter);	//单点改变逆变器的PANID和信道，ZK
int zb_restore_inverter_panid_channel_single_0x8888_0x10(inverter_info *inverter);	//单点还原逆变器的PANID到0X8888和信道0X10，ZK
int zb_change_ecu_panid(void);		//设置ECU的PANID和信道
int zb_restore_ecu_panid_0x8888(void);			//恢复ECU的PANID为0x8888,ZK
int zb_restore_ecu_panid_0xffff(int channel); 		//设置ECU的PANID为0xFFFF,信道为指定信道(注:向逆变器发送设置命令时,需将ECU的PANID设为0xFFFF)
int zb_send_cmd(inverter_info *inverter, char *buff, int length);		//zigbee包头
int zb_broadcast_cmd(char *buff, int length);		//zigbee广播包头
int zb_query_inverter_info(inverter_info *inverter);		//请求逆变器的机型码
int zb_query_data(inverter_info *inverter);		//请求逆变器实时数据
int zb_test_communication(void);		//zigbee测试通信有没有断开
int zb_set_protect_parameter(inverter_info *inverter, char *protect_parameter);		//参数修改CC指令
int zb_query_protect_parameter(inverter_info *inverter, char *protect_data_DA_reply);		//存储参数查询DD指令
int zb_afd_broadcast(void);		//AFD广播指令
int zb_turnon_inverter_broadcast(void);		//开机指令广播,OK
int zb_boot_single(inverter_info *inverter);		//开机指令单播,OK
int zb_shutdown_broadcast(void);		//关机指令广播,OK
int zb_shutdown_single(inverter_info *inverter);		//关机指令单播,OK
int zb_boot_waitingtime_single(inverter_info *inverter);		//开机等待时间启动控制单播,OK
int zb_clear_gfdi_broadcast(void);		//清除GFDI广播,OK
int zb_clear_gfdi(inverter_info *inverter);		//清除GFDI,OK
int zb_ipp_broadcast(void);		//IPP广播
int zb_ipp_single(inverter_info *inverter);		//IPP单播,暂时不用,ZK
int zb_frequency_protectime_broadcast(void);		//欠频保护时间广播
int zb_frequency_protectime_single(inverter_info *inverter);		//欠频保护时间单播
int zb_voltage_protectime_broadcast(void);		//欠压保护时间广播
int zb_voltage_protectime_single(inverter_info *inverter);		//欠压保护时间单播
int process_gfdi(inverter_info *firstinverter);
int compare_protect_data(char *set_protect_data, char *actual_protect_data);				//比较逆变器返回的预设值和页面上输入的预设值
int resolve_presetdata(inverter_info *inverter, char * protect_data_result);	//?????????????
int process_protect_data(inverter_info *firstinverter);
int process_turn_on_off(inverter_info *firstinverter);
int process_quick_boot(inverter_info *firstinverter);
int process_ipp(inverter_info *firstinverter);
int process_all(inverter_info *firstinverter);
int getalldata(inverter_info *firstinverter);		//获取每个逆变器的数据
int get_inverter_shortaddress(inverter_info *firstinverter);		//获取没有数据的逆变器的短地址
int bind_nodata_inverter(inverter_info *firstinverter);		//绑定没有数据的逆变器,并且获取短地址
int zb_change_inverter_channel_one(char *inverter_id, int channel);
int zb_change_channel(int num, char **ids);
int zb_reset_channel(int num, char **ids);
int zb_off_report_id_and_bind(int short_addr);
int zigbeeRecvMsg(char *data, int timeout_sec);
#endif /*__ZIGBEE_H__*/
