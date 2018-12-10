#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include "parser.h"
#include "client.h"

#define PORT 21


int main(int argc, char** argv){

  if(argc != 2){
    fprintf(stderr, "Usage: %s <address>\n", argv[0]);
    exit(1);
  }

  urlInfo url;
  ftpInfo ftp;

  if(parseUrl(argv[1], &url) != 0){
    fprintf(stderr, "Invalid URL\n");
    exit(1);
  }

  printf("user:%s\n", url.user);
  printf("pass:%s\n", url.password);
  printf("ip:%s\n", url.ip);
  printf("path:%s\n", url.filePath);
  printf("file_name:%s\n", url.fileName);
  printf("host:%s\n", url.host);
  printf("\n\n");

  if(initConnection(&ftp,url.ip,PORT) !=0){
    fprintf(stderr, "Error opening control connection\n");
    exit(1);
  }

  // login(ftp,url);
  //
  // char ip_address[MAX_SIZE];
  // int port;
  //
  // passiveMode(ftp, ip_address, &port);
  //
  // if ((ftp.data_socket_fd = initSocket(ip_address,port))<0){
  //   fprintf(stderr, "Error opening data connection\n");
  //   exit(1);
  // }
  //
  // retrieve(ftp,url);
  download(ftp,url);
  endConnection(ftp);

  return 0;

  }
