#ifndef __MYSOCKET_H__
#define __MYSOCKET_H__

int client_socket_init(int port, const char *ip, const char *domain);
int send_socket(int sockfd, char *sendbuffer, int size);
int recv_socket(int sockfd, char *recvbuffer, int size, int timeout_s);

#endif	/*__MYSOCKET_H__*/
