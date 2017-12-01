#ifndef __VARIATION_H__
#define __VARIATION_H__

#define MAXINVERTERCOUNT 100	//最大的逆变器数
#define RECORDLENGTH 150		//子记录的长度
#define RECORDTAIL 100			//大记录的结尾，包括发电量、时间等信息
#define TNUIDLENGTH 7			//逆变器3501ID的长度
#define idLEN 13		//逆变器ID的长度
#define STATUSLENGTH 3			//逆变器当前状态的长度
#define DVLENGTH 5				//直流电压的长度，EMA通信协议中使用
#define DILENGTH 3				//直流电流的长度，EMA通信协议中使用
#define POWERLENGTH 5			//功率长度，EMA通信协议中使用
#define REACTIVEPOWERLENGTH 5	//无功功率长度，EMA通信协议中使用
#define ACTIVEPOWERLENGTH 5		//有功功率长度，EMA通信协议中使用
#define FREQUENCYLENGTH 5		//电网频率长度，EMA通信协议中使用
#define TEMPERATURELENGTH 3		//温度长度，EMA通信协议中使用
#define GRIDVOLTLENGTH 3		//电网电压长度，EMA通信协议中使用
#define CURGENLENGTH 6			//当前一轮发电量的长度，EMA通信协议中使用
#define SYSTEMPOWERLEN 10		//系统功率，EMA通信协议中使用
#define CURSYSGENLEN 10			//系统当前发电量，EMA通信协议中使用
#define LIFETIMEGENLEN 10		//历史发电量，EMA通信协议中使用

#define PROCESS_RESULT_HEAD			51		
#define PROCESS_RESULT_RECORD_LEN			16
#define STATUS_PER_LEN				64

#define INVARTER_STATUS_PER_LEN			16
#define INVARTER_STATUS_PER_OTHER			16

#define INVERTER_PHONE_PER_LEN			27
#define INVERTER_PHONE_PER_OTHER			100


//Client 相关通信参数
#define CLIENT_RECORD_HEAD							20
#define CLIENT_RECORD_ECU_HEAD					78
#define CLIENT_RECORD_INVERTER_LENGTH		104
#define CLIENT_RECORD_OTHER							100

#define CONTROL_RECORD_HEAD							18
#define CONTROL_RECORD_ECU_HEAD					33
#define CONTROL_RECORD_INVERTER_LENGTH	41
#define CONTROL_RECORD_OTHER						100

typedef struct
{
    unsigned int deputy_model:4;				//副机型码（600里1是BB，2是B1）
	unsigned int dataflag:1;					//1表示读到当前数据；0表示读取数据失败
	unsigned int bindflag:1;					//逆变器绑定短地址标志，1表示绑定，0表示未绑定
	unsigned int flag:1;						//id中的flag标志
	unsigned int response_protection_paras_one:1;	//单台设置保护参数是否需要上报标志

	unsigned int last_turn_on_off_flag:1;
	unsigned int turn_on_off_changed_flag:1;		//0 1
	unsigned int last_gfdi_flag:1;
	unsigned int gfdi_changed_flag:1;				//0 1
	unsigned int fill_up_data_flag:3;				//0 1 2 3			//逆变器是否有补数据功能的标志位，1为有功能,2为没有功能，默认0为没有响应或者第一次
	unsigned int updating:1;						//0 1

}status_t;

typedef struct inverter_info_t{
	char id[13];		//逆变器的ID
	unsigned short shortaddr;			//Zigbee的短地址
	
	unsigned char model;					//机型：1是YC250CN,2是YC250NA，3是YC500CN，4是YC500NA，5是YC900CN，6是YC900NA 7是YC600
	status_t inverterstatus;		//位域的各种状态
	int version;				//软件版本号(见zigbee  zb_query_inverter_info函数)	
	unsigned char signalstrength;			//逆变器Zigbee信号强度
	unsigned char raduis;			
	
	float dv;					//直流电压
	float di;					//直流电流
	int op;						//输出功率
	float gf;					//电网频率
	int gv;						//电网电压
	int it;						//机内温度
	/********B路数据*****************/
	float dvb;					//直流电压
	float dib;					//直流电流
	int opb;					//输出功率
	int gvb;					//电网电压
	/********C路数据*****************/
	float dvc;					//直流电压
	float dic;					//直流电流
	int opc;					//输出功率
	int gvc;					//电网电压
	/********D路数据*****************/
	float dvd;					//直流电压
	float did;					//直流电流
	int opd;					//输出功率
	int gvd;					//电网电压


	/***********保护参数**************/
	int protect_voltage_min;			//内围电压保护下限
	int protect_voltage_max;			//内围电压保护上限
	float protect_frequency_min;		//频率保护下限
	float protect_frequency_max;		//频率保护上限
	int recovery_time;					//并网恢复时间/开机时间

	float reactive_power;		//无功功率
	float reactive_powerb;		//B路无功功率
	float reactive_powerc;		//C路无功功率
	float active_power;			//有功功率
	float active_powerb;		//B路有功功率
	float active_powerc;		//C路有功功率
	float output_energy;		//输出电量差值
	float output_energyb;		//B路输出电量差值
	float output_energyc;		//C路输出电量差值
	float pre_output_energy;	//上一轮读到的输出电量
	float pre_output_energyb;	//B路上一轮读到的输出电量
	float pre_output_energyc;	//C路上一轮读到的输出电量
	float cur_output_energy;	//本轮读到的输出电量
	float cur_output_energyb;	//B路本轮读到的输出电量
	float cur_output_energyc;	//C路本轮读到的输出电量

	float curgeneration;		//逆变器当前一轮的电量
	float curgenerationb;		//逆变器B路当前一轮的电量
	float curgenerationc;		//逆变器C路当前一轮的电量
	float curgenerationd;		//逆变器D路当前一轮的电量

	float preaccgen;			//A路上一轮返回的累计电量（逆变器启动后电量不停累计，直到其重启）
	float preaccgenb;			//B路上一轮返回的累计电量（逆变器启动后电量不停累计，直到其重启）
	float preaccgenc;			//C路上一轮返回的累计电量（逆变器启动后电量不停累计，直到其重启）
	float preaccgend;			//D路上一轮返回的累计电量（逆变器启动后电量不停累计，直到其重启）
	float curaccgen;			//A路本轮返回的累计电量（逆变器启动后电量不停累计，直到其重启）
	float curaccgenb;			//B路本轮返回的累计电量（逆变器启动后电量不停累计，直到其重启）
	float curaccgenc;			//C路本轮返回的累计电量（逆变器启动后电量不停累计，直到其重启）
	float curaccgend;			//D路本轮返回的累计电量（逆变器启动后电量不停累计，直到其重启）
	int preacctime;				//上一轮返回的累计时间（逆变器启动后时间不停累计，直到其重启）
	int curacctime;				//本轮返回的累计时间（逆变器启动后时间不停累计，直到其重启）

	char status_web[50];		//存入ECU本地数据库的状态，用于本地页面显示
	char status[12];			//逆变器状态
	char statusb[12];			//逆变器B路状态
	char statusc[12];			//逆变器C路状态
	char statusd[12];			//逆变器D路状态
	char status_ema[64];		//16项参数新加
	char status_send_flag;		//16项参数新加

	char last_report_time[16];	//发送给EMA时的日期和时间，格式：年月日时分秒
	unsigned char no_getdata_num;					//unsigned char(保持上限255)连续没有获取到逆变器数据的次数
	unsigned char disconnect_times;				//unsigned char(保持上限255) 一天中没有与逆变器通信上的所有次数 ZK
	unsigned char zigbee_version;					//unsigned char   zigbee版本号ZK		//turned_off_rpt_flag

	int updating_time;
	
}inverter_info;

typedef struct ecu_info_t{
	char id[13];				//ECU的ID
	//char ccuid[8];				//Zigbee/3501的ID
	unsigned short panid;				//Zigbee的panid
	char channel;				//Zigbee信道
	char ip[20];
	float life_energy;			//系统历史总电量
	float current_energy;		//系统当前一轮电量
	float today_energy;			//当天的发电量
	int system_power;			//系统总功率
	int count;					//系统当前一轮有数据的逆变器数
	int total;					//系统逆变器总数
	int assigned_shortaddr_count;	//已经获取到短地址的逆变器数量

	int type;					//ECU类型:1表示NA，0表示SAA
	int zoneflag;				//修改过时区的标志：1表示修改，0表示未修改
	char MacAddress[7];			//ECU  MAC地址
	char broadcast_time[16];	//发送给EMA时的日期和时间，格式：年月日时分秒
	char had_data_broadcast_time[16];	//发送给EMA时的日期和时间，格式：年月日时分秒
	char last_ema_time[16];		//client 最后一次和client通信时间
//	char inverterid_noget_shortaddr[MAXINVERTERCOUNT][13];	//没有获取到短地址的逆变器ID

	int no_assigned_shortaddr_count;	//没有获取到短地址的逆变器数量
	int flag_ten_clock_getshortaddr;	//每天10点有没有重新获取短地址标志
	int polling_total_times;			//ECU一天之中总的轮询次数 ZK
}ecu_info;


#endif /*__VARIATION_H__*/
