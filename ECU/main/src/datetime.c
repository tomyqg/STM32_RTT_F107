#include "rtc.h"
#include "datetime.h"
#include <string.h>
#include <rtthread.h>


int get_time(char *sendcommanddatetime, char *sendcommandtime)		//����EMA��¼ʱ��ȡ��ʱ�䣬��ʽ��������ʱ���룬��20120902142835
{
	char datetime[15] = {'\0'};
	unsigned hour, minute;
	apstime(datetime);
	rt_memcpy(sendcommanddatetime,datetime,14);
	hour = ((datetime[8] - 0x30) * 10) + (datetime[9] - 0x30);
	minute = ((datetime[10] - 0x30) * 10) + (datetime[11] - 0x30);
    
	sendcommandtime[0] = hour;
	sendcommandtime[1] = minute;
    
#ifdef DEBUGINFO
	printf("Broadcast time:%s\n", sendcommanddatetime);
#endif

	return hour;
}



void getdate(char date[10])		//��ȡ���ڣ����ڵ��췢��������ʽ�������գ���20120902
{
	char datetime[15] = {'\0'};
	apstime(datetime);
	rt_memcpy(date,datetime,8);
	

}