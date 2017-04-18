#ifndef __THFTPAPI_H__
#define __THFTPAPI_H__
//
//  THFTPAPI.h
//  MyFTP
//

 
//连接服务器  OK
int ftp_connect( char *host, int port, char *user, char *pwd );
//断开服务器  OK
int ftp_quit( int c_sock);
 
//设置表示类型 OK
int ftp_type( int c_sock, char mode );
 
//改变工作目录 OK
int ftp_cwd( int c_sock, char *path );
//回到上一层目录  OK
int ftp_cdup( int c_sock );
//创建目录  OK
int ftp_mkd( int c_sock, char *path );
//列表
int ftp_list( int c_sock, char *path, void **data, unsigned long long *data_len);
 
//下载文件
int ftp_retrfile( int c_sock, char *s, char *d ,unsigned long long *stor_size, int *stop);
//上传文件
int ftp_storfile( int c_sock, char *s, char *d ,unsigned long long *stor_size, int *stop);
 
//修改文件名&移动目录 OK
int ftp_renamefile( int c_sock, char *s, char *d );
//删除文件  OK
int ftp_deletefile( int c_sock, char *s );
//删除目录  OK
int ftp_deletefolder( int c_sock, char *s );

//下载文件
int ftpgetfile(char *host, int port, char *user, char *pwd,char *remotefile,char *localfile);
	
//上传文件
int ftpputfile(char *host, int port, char *user, char *pwd,char *remotefile,char *localfile);


#endif /*__THFTPAPI_H__*/
