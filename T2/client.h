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

int initConnection(FTPInfo * ftp, char * ip, int port);
int connectSocket(char * ip, int port);
void login(FTPInfo ftp, urlInfo url);
void passiveMode(FTPInfo ftp, char * ip, int * port);
void retrieve(FTPInfo ftp, urlInfo url);
int readMessage(int socketfd, char * repply);
int sendMessage(int socketfd, char * cmd);
int download(FTPInfo ftp, urlInfo url);
int endConnection(FTPInfo ftp);
