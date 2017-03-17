#ifndef __DATETIME_H__
#define __DATETIME_H__


int get_time(char *sendcommanddatetime, char *sendcommandtime);
void getdate(char date[10]);
int acquire_time(void);
int compareTime(int durabletime ,int thistime,int reportinterval);
int get_hour(void);

void getcurrenttime(char db_time[]);

#endif /*__DATETIME_H__*/
