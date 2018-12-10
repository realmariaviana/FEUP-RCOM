#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

typedef struct {
	int controlSocketFd; // file descriptor to control socket
	int dataSocketFd; // file descriptor to data socket
} FTPInfo;

int connectSocket(char* ip, int port);
int initConnection(ftpInfo* ftp, char* ip, int port);
int download(ftpInfo ftp, urlInfo url);
int endConnection(ftpInfo ftp);
