#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include "parser.h"

typedef struct {
	int controlSocketFd; // file descriptor to control socket
	int dataSocketFd; // file descriptor to data socket
} FTPInfo;

int connectSocket(char* ip, int port);
int initConnection(FTPInfo* ftp, char* ip, int port);
int download(FTPInfo ftp, urlInfo url);
int socketRead(int socketfd, char* repply);
int socketWrite(int socketfd, char* cmd);
int endConnection(FTPInfo ftp);
