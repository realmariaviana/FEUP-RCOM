#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>
#include <strings.h>
#include <termios.h>
#include <fcntl.h>

#define MAX_SIZE 256

typedef struct{
  char  user[MAX_SIZE];
  char  password[MAX_SIZE];
  char  host[MAX_SIZE];
  char  filePath[MAX_SIZE];
  char  fileName[MAX_SIZE];
  char  ip[MAX_SIZE];
} urlInfo;

int userPassword(urlInfo * infoStruct, char * completeUrl);
int parseUrl(char completeUrl[], urlInfo * infoStruct);
int getIp(urlInfo * infoStruct);
