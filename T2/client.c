#include "client.h"

int initConnection(FTPInfo* ftp, char* ip, int port){
  int socketfd;

  if ((socketfd = connectSocket(ip, port)) < 0) {
		printf("ERROR: Cannot connect socket.\n");
		return 1;
	}

	ftp->controlSocketFd = socketfd;
  ftp->dataSocketFd = 0;

  return 0;
}

int connectSocket(char* ip, int port){

  int	socketfd;
  struct sockaddr_in server_addr;

  /*server address handling*/
  bzero((char*)&server_addr,sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip_address);	/*32 bit Internet address network byte ordered*/
  server_addr.sin_port = htons(port);		/*server TCP port must be network byte ordered */

  /*open an TCP socket*/
  if ((socketfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
        perror("socket()");
          return -1;
      }
  /*connect to the server*/
      if(connect(socketfd,(struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
          perror("connect()");
    return -1;
  }

  return socketfd;
}

int download(ftpInfo ftp, urlInfo url){
  FILE* file;

  if(!(file = fopen(url.fileName, "w"))) {
		printf("Error opening file %s.\n",url.fileName);
		return 1;
	}

  char buf[1024];
  int bytes;

  while ((bytes = read(ftp.dataSocketFd, buf, sizeof(buf)))) {
    if (bytes < 0) {
      fprintf(stderr, "Error, nothing was received from data socket fd.\n");
      return 1;
    }

    if ((bytes = fwrite(buf, bytes, 1, file)) < 0) {
      fprintf(stderr, "Error, cannot write data in file.\n");
      return 1;
    }
  }

  fclose(file);
  close(ftp.dataSocketFd);
  printf("Finished downloading file\n");

  return 0;
}

int endConnection(ftpInfo ftp){

  printf("Closing connection\n");
  socketWrite(ftp.controlSocketFd,"QUIT\r\n");

  if(socketRead(ftp.controlSocketFd,NULL) != 0){
    fprintf(stderr, "Error closing connection. Closing...\n");
    close(ftp.dataSocketFd);
    close(ftp.controlSocketFd);
    exit(1);
  }

  close(ftp.dataSocketFd);
  close(ftp.controlSocketFd);

  printf("Connection ended\n");

  return 0;
}
