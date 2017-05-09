#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "remote_control_protocol.h"
#include "debug.h"
#include "myfile.h"
#include "rtthread.h"

/*********************************************************************
setpropa表格字段：
parameter_name,parameter_value,set_flag             primary key(parameter_name)

setpropi表格字段：
id,parameter_name, parameter_value,set_flag         primary key(id, parameter_name)
**********************************************************************/

#define NUM 17

extern rt_mutex_t record_data_lock;

static const char pro_name[NUM][32] = {
		"under_voltage_fast",
		"over_voltage_fast",
		"under_voltage_slow",
		"over_voltage_slow",
		"under_frequency_fast",
		"over_frequency_fast",
		"under_frequency_slow",
		"over_frequency_slow",
		"voltage_triptime_fast",
		"voltage_triptime_slow",
		"frequency_triptime_fast",
		"frequency_triptime_slow",
		"grid_recovery_time",
		"regulated_dc_working_point",
		"under_voltage_stage_2",
		"voltage_3_clearance_time",
		"start_time",
};
static float pro_value[NUM] = {0};
static int pro_flag[NUM] = {0};

/* 从协议消息中解析出交流保护参数
 *
 * name   : 交流保护参数名称
 * s      : 消息字符串指针
 * len    : 交流保护参数所占字节
 * decimal: 交流保护参数小数位数
 *
 */
int get_ac_protection(const char *name, const char *s, int len, int decimal)
{
	int i, j, value;

	for(i=0; i<NUM; i++)
	{
		if(!strcmp(name, pro_name[i])){
			value = msg_get_int(s, len);
			if(value >= 0){
				pro_value[i] = (float)value;
				pro_flag[i] = 1;
			}
			else{
				return -1;
			}
			for(j=0; j<decimal; j++){
				pro_value[i] /= 10;
			}
			break;
		}
	}
	return 0;
}

/* 从数据库中查询ECU级别的交流保护参数 */
int query_ecu_ac_protection()
{
	get_protection_from_file(pro_name,pro_value,pro_flag,NUM);
	return 0;
}

/* 根据参数名获取缓存的参数值,并格式化为整数（去掉小数点） */
int format_ecu_ac_protection(const char *name, int decimal)
{
	int i, j;
	float temp = 0.0;

	for (i=0; i<NUM; i++) {
		if (!strcmp(name, pro_name[i])) {
			if (pro_flag[i] != 1) {
				return -1;
			}
			temp = pro_value[i];
			for (j=0; j<decimal; j++) {
				temp *= 10;
			}
			return (int)temp;
		}
	}
	return -1;
}

/* 在消息末尾拼接电压频率上下限及功率设置范围 */
int msgcat_parameter_range(char *msg)
{
	msgcat_s(msg, 30, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
	return 0;
}

/* 查询ECU级别的最大功率 */
int query_ecu_maxpower()
{
	int maxpower = -1;

	return maxpower;
}

/* 设置所有逆变器的交流保护参数 */
int save_ac_protection_all()
{
	int i, err_count = 0;
	char str[100];

	unlink("/home/data/setpropa");
	for(i=0; i<NUM; i++)
	{
		if(pro_flag[i] == 1){
		
			//如果存在该逆变器数据则删除该记录
			//delete_line("/home/data/setpropa","/home/data/setpropa.t",(char *)pro_name[i],strlen(pro_name[i]));
			sprintf(str,"%s,%.2f,1\n",pro_name[i], pro_value[i]);
			//插入数据
			if(-1 == insert_line("/home/data/setpropa",str))
			{
				err_count++;
			}
	
		}
	}

	return err_count;
}

/* 设置指定逆变器的交流保护参数 */
int save_ac_protection_num(const char *msg, int num)
{

	int i, j, err_count = 0;
	char inverter_id[13] = {'\0'};
	char str[100];
	
	unlink("/home/data/setpropi");
	for(i=0; i<num; i++)
	{
		//获取一台逆变器的ID号
		strncpy(inverter_id, &msg[i*12], 12);

		for(j=0; j<NUM; j++)
		{
			if(pro_flag[j] == 1){
				//如果存在该逆变器数据则删除该记录
				//delete_line("/home/data/setpropi","/home/data/setpropi.t",(char *)pro_name[j],strlen(pro_name[j]));
				sprintf(str,"'%s', '%s', %f, 1\n",inverter_id, pro_name[j], pro_value[j]);
				//插入数据
				if(-1 == insert_line("/home/data/setpropi",str))
				{
					err_count++;
				}
			}
		}
	}
	return err_count;
}

/*【A109】EMA设置逆变器的交流保护参数（5项）*/
int set_inverter_ac_protection_5(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	char timestamp[15] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	memset(pro_value, 0, sizeof(pro_value));
	memset(pro_flag, 0, sizeof(pro_flag));

	//时间戳
	strncpy(timestamp, &recvbuffer[30], 14);
	//内内围电压下限
	if(get_ac_protection("under_voltage_slow", &recvbuffer[47], 3, 0) < 0) ack_flag = FORMAT_ERROR;
	//内围电压上限
	if(get_ac_protection("over_voltage_slow", &recvbuffer[50], 3, 0) < 0) ack_flag = FORMAT_ERROR;
	//内围频率下限
	if(get_ac_protection("under_frequency_slow", &recvbuffer[53], 3, 1) < 0) ack_flag = FORMAT_ERROR;
	//内围频率上限
	if(get_ac_protection("over_frequency_slow", &recvbuffer[56], 3, 1) < 0) ack_flag = FORMAT_ERROR;
	//并网恢复时间
	if(get_ac_protection("grid_recovery_time", &recvbuffer[59], 5, 0) < 0) ack_flag = FORMAT_ERROR;

	//保存到数据库
	if(SUCCESS == ack_flag){
		if(save_ac_protection_all() > 0)
			ack_flag = DB_ERROR;
	}

	//拼接应答消息
	msg_ACK(sendbuffer, "A109", timestamp, ack_flag);
	rt_mutex_release(record_data_lock);
	return 113;
}

/*【A113】ECU上报(ECU级别的)交流保护参数（5项）*/
int response_ecu_ac_protection_5(const char *recvbuffer, char *sendbuffer)
{
	char ecuid[13] = {'\0'};
	char timestamp[15] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	//获取参数
	file_get_one(ecuid, sizeof(ecuid), "/yuneng/ecuid.con");
	strncpy(timestamp, &recvbuffer[34], 14);
	memset(pro_value, 0, sizeof(pro_value));
	memset(pro_flag, 0, sizeof(pro_flag));
	query_ecu_ac_protection(); //查询ECU级别的交流保护参数

	//拼接信息
	msg_Header(sendbuffer, "A113");
	msgcat_s(sendbuffer, 12, ecuid);
	msgcat_s(sendbuffer, 14, timestamp);
	msgcat_s(sendbuffer, 3, "END");
	msgcat_d(sendbuffer, 3, format_ecu_ac_protection("under_voltage_slow", 0));
	msgcat_d(sendbuffer, 3, format_ecu_ac_protection("over_voltage_slow", 0));
	msgcat_d(sendbuffer, 3, format_ecu_ac_protection("under_frequency_slow", 1));
	msgcat_d(sendbuffer, 3, format_ecu_ac_protection("over_frequency_slow", 1));
	msgcat_d(sendbuffer, 5, format_ecu_ac_protection("grid_recovery_time", 0));
	msgcat_parameter_range(sendbuffer);
	msgcat_d(sendbuffer, 3, query_ecu_maxpower());
	msgcat_s(sendbuffer, 3, "END");
	rt_mutex_release(record_data_lock);
	return 0;
}

/*【A114】EMA读取逆变器的交流保护参数（5项）*/
int read_inverter_ac_protection_5(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	char timestamp[15] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	//读取逆变器交流保护参数
	if(file_set_one("2", "/tmp/presdata.con")){
		ack_flag = FILE_ERROR;
	}

	//获取时间戳
	strncpy(timestamp, &recvbuffer[34], 14);

	//拼接应答消息
	msg_ACK(sendbuffer, "A114", timestamp, ack_flag);
	rt_mutex_release(record_data_lock);
	return 0;
}

/*【A122】EMA设置逆变器的交流保护参数（13项）*/
int set_inverter_ac_protection_13(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	int inverter_num = 0;
	char timestamp[15] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	memset(pro_value, 0, sizeof(pro_value));
	memset(pro_flag, 0, sizeof(pro_flag));

	//时间戳
	strncpy(timestamp, &recvbuffer[30], 14);
	//逆变器数量
	inverter_num = msg_get_int(&recvbuffer[44], 3);
	//内内围电压下限
	get_ac_protection("under_voltage_slow", &recvbuffer[50], 3, 0);
	//内围电压上限
	get_ac_protection("over_voltage_slow", &recvbuffer[53], 3, 0);
	//内围频率下限
	get_ac_protection("under_frequency_slow", &recvbuffer[56], 3, 1);
	//内围频率上限
	get_ac_protection("over_frequency_slow", &recvbuffer[59], 3, 1);
	//并网恢复时间
	get_ac_protection("grid_recovery_time", &recvbuffer[62], 5, 0);
	//外围电压下限
	get_ac_protection("under_voltage_fast", &recvbuffer[67], 3, 0);
	//外围电压上限
	get_ac_protection("over_voltage_fast", &recvbuffer[70], 3, 0);
	//外围频率下限
	get_ac_protection("under_frequency_fast", &recvbuffer[73], 3, 1);
	//外围频率上限
	get_ac_protection("over_frequency_fast", &recvbuffer[76], 3, 1);
	//外围电压延迟保护时间
	get_ac_protection("voltage_triptime_fast", &recvbuffer[79], 2, 2);
	//内围电压延迟保护时间
	get_ac_protection("voltage_triptime_slow", &recvbuffer[81], 4, 2);
	//外围频率延迟保护时间
	get_ac_protection("frequency_triptime_fast", &recvbuffer[85], 2, 2);
	//内围频率延迟保护时间
	get_ac_protection("frequency_triptime_slow", &recvbuffer[87], 4, 2);

	if(inverter_num == 0)
	{
		if(save_ac_protection_all() > 0)
			ack_flag = DB_ERROR;
		//拼接应答消息
		msg_ACK(sendbuffer, "A122", timestamp, ack_flag);
		rt_mutex_release(record_data_lock);
		return 120;
	}
	else
	{
		if(!msg_num_check(&recvbuffer[94], inverter_num, 12, 0)){
			ack_flag = FORMAT_ERROR;
		}
		else{
			if(save_ac_protection_num(&recvbuffer[94], inverter_num) > 0)
				ack_flag = DB_ERROR;
		}
		//拼接应答消息
		msg_ACK(sendbuffer, "A122", timestamp, ack_flag);
		rt_mutex_release(record_data_lock);
		return 0;
	}
}

/*【A120】ECU上报(ECU级别的)交流保护参数（13项）*/
int response_ecu_ac_protection_13(const char *recvbuffer, char *sendbuffer)
{
	char ecuid[13] = {'\0'};
	char timestamp[15] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	//获取参数
	file_get_one(ecuid, sizeof(ecuid), "/yuneng/ecuid.con");
	strncpy(timestamp, &recvbuffer[34], 14);
	memset(pro_value, 0, sizeof(pro_value));
	memset(pro_flag, 0, sizeof(pro_flag));
	query_ecu_ac_protection(); //查询ECU级别的交流保护参数

	//拼接信息
	msg_Header(sendbuffer, "A120");
	msgcat_s(sendbuffer, 12, ecuid);
	msgcat_s(sendbuffer, 14, timestamp);
	msgcat_s(sendbuffer, 3, "END");
	msgcat_d(sendbuffer, 3, format_ecu_ac_protection("under_voltage_slow", 0));
	msgcat_d(sendbuffer, 3, format_ecu_ac_protection("over_voltage_slow", 0));
	msgcat_d(sendbuffer, 3, format_ecu_ac_protection("under_frequency_slow", 1));
	msgcat_d(sendbuffer, 3, format_ecu_ac_protection("over_frequency_slow", 1));
	msgcat_d(sendbuffer, 5, format_ecu_ac_protection("grid_recovery_time", 0));
	msgcat_parameter_range(sendbuffer);
	msgcat_d(sendbuffer, 3, query_ecu_maxpower());
	msgcat_d(sendbuffer, 3, format_ecu_ac_protection("under_voltage_fast", 0));
	msgcat_d(sendbuffer, 3, format_ecu_ac_protection("over_voltage_fast", 0));
	msgcat_d(sendbuffer, 3, format_ecu_ac_protection("under_frequency_fast", 1));
	msgcat_d(sendbuffer, 3, format_ecu_ac_protection("over_frequency_fast", 1));
	msgcat_d(sendbuffer, 2, format_ecu_ac_protection("voltage_triptime_fast", 2));
	msgcat_d(sendbuffer, 4, format_ecu_ac_protection("voltage_triptime_slow", 2));
	msgcat_d(sendbuffer, 2, format_ecu_ac_protection("frequency_triptime_fast", 2));
	msgcat_d(sendbuffer, 4, format_ecu_ac_protection("frequency_triptime_slow", 2));
	msgcat_s(sendbuffer, 3, "END");
	rt_mutex_release(record_data_lock);
	return 0;
}

/*【A121】EMA读取逆变器的交流保护参数（13项）*/
int read_inverter_ac_protection_13(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	char timestamp[15] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	//读取逆变器交流保护参数
	if(file_set_one("2", "/tmp/presdata.con")){
		ack_flag = FILE_ERROR;
	}

	//获取时间戳
	strncpy(timestamp, &recvbuffer[34], 14);

	//拼接应答消息
	msg_ACK(sendbuffer, "A121", timestamp, ack_flag);
	rt_mutex_release(record_data_lock);
	return 0;
}

/*【A130】ECU上报(ECU级别的)交流保护参数（17项）*/
int response_ecu_ac_protection_17(const char *recvbuffer, char *sendbuffer)
{
	char ecuid[13] = {'\0'};
	char timestamp[15] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	//获取参数
	file_get_one(ecuid, sizeof(ecuid), "/yuneng/ecuid.con");
	strncpy(timestamp, &recvbuffer[34], 14);
	memset(pro_value, 0, sizeof(pro_value));
	memset(pro_flag, 0, sizeof(pro_flag));
	query_ecu_ac_protection(); //查询ECU级别的交流保护参数

	//拼接信息
	msg_Header(sendbuffer, "A130");
	msgcat_s(sendbuffer, 12, ecuid);
	msgcat_s(sendbuffer, 14, timestamp);
	msgcat_s(sendbuffer, 3, "END");
	msgcat_d(sendbuffer, 3, format_ecu_ac_protection("under_voltage_slow", 0));
	msgcat_d(sendbuffer, 3, format_ecu_ac_protection("over_voltage_slow", 0));
	msgcat_d(sendbuffer, 3, format_ecu_ac_protection("under_frequency_slow", 1));
	msgcat_d(sendbuffer, 3, format_ecu_ac_protection("over_frequency_slow", 1));
	msgcat_d(sendbuffer, 5, format_ecu_ac_protection("grid_recovery_time", 0));
	msgcat_parameter_range(sendbuffer);
	msgcat_d(sendbuffer, 3, query_ecu_maxpower());
	msgcat_d(sendbuffer, 3, format_ecu_ac_protection("under_voltage_fast", 0));
	msgcat_d(sendbuffer, 3, format_ecu_ac_protection("over_voltage_fast", 0));
	msgcat_d(sendbuffer, 3, format_ecu_ac_protection("under_frequency_fast", 1));
	msgcat_d(sendbuffer, 3, format_ecu_ac_protection("over_frequency_fast", 1));
	msgcat_d(sendbuffer, 6, format_ecu_ac_protection("voltage_triptime_fast", 2));
	msgcat_d(sendbuffer, 6, format_ecu_ac_protection("voltage_triptime_slow", 2));
	msgcat_d(sendbuffer, 6, format_ecu_ac_protection("frequency_triptime_fast", 2));
	msgcat_d(sendbuffer, 6, format_ecu_ac_protection("frequency_triptime_slow", 2));
	msgcat_d(sendbuffer, 3, format_ecu_ac_protection("regulated_dc_working_point", 1));
	msgcat_d(sendbuffer, 3, format_ecu_ac_protection("under_voltage_stage_2", 0));
	msgcat_d(sendbuffer, 6, format_ecu_ac_protection("voltage_3_clearance_time", 2));
	msgcat_d(sendbuffer, 5, format_ecu_ac_protection("start_time", 0));
	msgcat_s(sendbuffer, 3, "END");
	rt_mutex_release(record_data_lock);
	return 0;
}

/*【A132】EMA设置逆变器的交流保护参数（17项）*/
int set_inverter_ac_protection_17(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	int inverter_num = 0;
	char timestamp[15] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	memset(pro_value, 0, sizeof(pro_value));
	memset(pro_flag, 0, sizeof(pro_flag));

	//时间戳
	strncpy(timestamp, &recvbuffer[30], 14);
	//逆变器数量
	inverter_num = msg_get_int(&recvbuffer[44], 3);
	//内内围电压下限
	get_ac_protection("under_voltage_slow", &recvbuffer[50], 3, 0);
	//内围电压上限
	get_ac_protection("over_voltage_slow", &recvbuffer[53], 3, 0);
	//内围频率下限
	get_ac_protection("under_frequency_slow", &recvbuffer[56], 3, 1);
	//内围频率上限
	get_ac_protection("over_frequency_slow", &recvbuffer[59], 3, 1);
	//并网恢复时间
	get_ac_protection("grid_recovery_time", &recvbuffer[62], 5, 0);
	//外围电压下限
	get_ac_protection("under_voltage_fast", &recvbuffer[67], 3, 0);
	//外围电压上限
	get_ac_protection("over_voltage_fast", &recvbuffer[70], 3, 0);
	//外围频率下限
	get_ac_protection("under_frequency_fast", &recvbuffer[73], 3, 1);
	//外围频率上限
	get_ac_protection("over_frequency_fast", &recvbuffer[76], 3, 1);
	//外围电压延迟保护时间
	get_ac_protection("voltage_triptime_fast", &recvbuffer[79], 6, 2);
	//内围电压延迟保护时间
	get_ac_protection("voltage_triptime_slow", &recvbuffer[85], 6, 2);
	//外围频率延迟保护时间
	get_ac_protection("frequency_triptime_fast", &recvbuffer[91], 6, 2);
	//内围频率延迟保护时间
	get_ac_protection("frequency_triptime_slow", &recvbuffer[97], 6, 2);
	//直流稳压电压
	get_ac_protection("regulated_dc_working_point", &recvbuffer[103], 3, 1);
	//内围电压下限
	get_ac_protection("under_voltage_stage_2", &recvbuffer[106], 3, 0);
	//内内围电压延迟保护时间
	get_ac_protection("voltage_3_clearance_time", &recvbuffer[109], 6, 2);
	//直流启动时间
	get_ac_protection("start_time", &recvbuffer[115], 5, 0);

	if(inverter_num == 0)
	{
		if(save_ac_protection_all() > 0)
			ack_flag = DB_ERROR;
		//拼接应答消息
		msg_ACK(sendbuffer, "A132", timestamp, ack_flag);
		rt_mutex_release(record_data_lock);
		return 130;
	}
	else
	{
		if(!msg_num_check(&recvbuffer[123], inverter_num, 12, 0)){
			ack_flag = FORMAT_ERROR;
		}
		else{
			if(save_ac_protection_num(&recvbuffer[123], inverter_num) > 0)
				ack_flag = DB_ERROR;
		}
		//拼接应答消息
		msg_ACK(sendbuffer, "A132", timestamp, ack_flag);
		rt_mutex_release(record_data_lock);
		return 0;
	}
}

/*【A131】EMA读取逆变器的交流保护参数（17项）*/
int read_inverter_ac_protection_17(const char *recvbuffer, char *sendbuffer)
{
	int ack_flag = SUCCESS;
	char timestamp[15] = {'\0'};
	rt_err_t result = rt_mutex_take(record_data_lock, RT_WAITING_FOREVER);
	//读取逆变器交流保护参数
	if(file_set_one("2", "/tmp/presdata.con")){
		ack_flag = FILE_ERROR;
	}

	//获取时间戳
	strncpy(timestamp, &recvbuffer[34], 14);

	//拼接应答消息
	msg_ACK(sendbuffer, "A131", timestamp, ack_flag);
	rt_mutex_release(record_data_lock);
	return 0;
}
