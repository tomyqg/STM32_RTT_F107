#ifndef __INVERTER_IRD_H__
#define __INVERTER_IRD_H__

int read_inverter_ird(const char *recvbuffer, char *sendbuffer);
int set_inverter_ird(const char *recvbuffer, char *sendbuffer);

#endif	/*__INVERTER_IRD_H__*/
