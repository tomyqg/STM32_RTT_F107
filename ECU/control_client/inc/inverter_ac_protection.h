#ifndef __INVERTER_AC_PROTECTION_H__
#define __INVERTER_AC_PROTECTION_H__

int set_inverter_ac_protection_5(const char *recvbuffer, char *sendbuffer);
int response_ecu_ac_protection_5(const char *recvbuffer, char *sendbuffer);
int read_inverter_ac_protection_5(const char *recvbuffer, char *sendbuffer);
int set_inverter_ac_protection_13(const char *recvbuffer, char *sendbuffer);
int response_ecu_ac_protection_13(const char *recvbuffer, char *sendbuffer);
int read_inverter_ac_protection_13(const char *recvbuffer, char *sendbuffer);
int set_inverter_ac_protection_17(const char *recvbuffer, char *sendbuffer);
int response_ecu_ac_protection_17(const char *recvbuffer, char *sendbuffer);
int read_inverter_ac_protection_17(const char *recvbuffer, char *sendbuffer);

#endif	/*__INVERTER_AC_PROTECTION_H__*/
