#include "client.h"

int initConnection(FTPInfo* ftp, char* ip, int port){
  int socketfd;

  if ((socketfd = connectSocket(ip, port)) < 0) {
		printf("Error connecting socket.\n");
		return 1;
	}

	ftp->controlSocketFd = socketfd;
  ftp->dataSocketFd = 0;

  return 0;

}

int connectSocket(char * ip, int port){
  int	socketfd;
  struct	sockaddr_in server_addr;

  /*server address handling*/
  bzero((char*)&server_addr,sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(ip);	/*32 bit Internet address network byte ordered*/
  server_addr.sin_port = htons(port);		/*server TCP port must be network byte ordered */

  /*open an TCP socket*/
  if ((socketfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
        perror("socket()");
          return -1;
      }
  /*connect to the server*/
      if(connect(socketfd,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0){
          perror("connect()");
          return -1;
        }
  return socketfd;
}

void login(FTPInfo ftp, urlInfo url){
  char user[MAX_SIZE];
  char password[MAX_SIZE];

  readMessage(ftp.controlSocketFd, NULL);

  sprintf(user, "USER %s\r\n", url.user);
  printf(">%s",user);

  sendMessage(ftp.controlSocketFd,user);
  readMessage(ftp.controlSocketFd,NULL);

  sprintf(password, "PASS %s\r\n", url.password);
  printf(">%s",password);

  sendMessage(ftp.controlSocketFd, password);
  if(readMessage(ftp.controlSocketFd,NULL) !=0){
    fprintf(stderr, "Wrong credentials. Exiting...\n");
    exit(1);
  }
}

void passiveMode(FTPInfo ftp, char * ip_adress, int * port){
  char repply[MAX_SIZE];

  sendMessage(ftp.controlSocketFd, "PASV\r\n");
  if(readMessage(ftp.controlSocketFd,repply) !=0){
    fprintf(stderr, "Error entering passive mode. Exiting...\n");
    exit(1);
  }

  int values[6];
  char* data = strchr(repply, '(');
  sscanf(data, "(%d, %d, %d, %d, %d, %d)", &values[0],&values[1],&values[2],&values[3],&values[4],&values[5]);
  sprintf(ip_adress, "%d.%d.%d.%d", values[0],values[1],values[2],values[3]);
  *port = values[4]*256+values[5];
}

void retrieve(FTPInfo ftp, urlInfo url){
  char cmd[MAX_SIZE];

  sprintf(cmd, "RETR %s%s\r\n", url.filePath, url.fileName);
  printf(">%s",cmd);
  sendMessage(ftp.controlSocketFd, cmd);

  if(readMessage(ftp.controlSocketFd,NULL) != 0){
    fprintf(stderr, "Error retrieving file. Exiting...\n");
    exit(1);
  }
}

int readMessage(int socketfd, char * reply){
  FILE* fd = fdopen(socketfd, "r");
  int allocated = 0;

  if(reply == NULL){
    reply = (char*) malloc(sizeof(char) * MAX_SIZE);
    allocated = 1;
  }

  do {
    memset(reply, 0, MAX_SIZE);
    reply = fgets(reply, MAX_SIZE, fd);

    if(reply == NULL){
      printf("ERROR: Cannot read the message.\n");
			return -1;
    }

    printf("<%s", reply);
  } while (!('1' <= reply[0] && reply[0] <= '5') || reply[3] != ' ');

  char r0 = reply[0];

  if(allocated)
    free(reply);

  return (r0>'4');
}

int sendMessage(int socketfd, char * cmd){
    int ret = write(socketfd, cmd, strlen(cmd));
    return ret;
}

int download(FTPInfo ftp, urlInfo url){
  FILE* dest_file;
  if(!(dest_file = fopen(url.fileName, "w"))) {
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

    if ((bytes = fwrite(buf, bytes, 1, dest_file)) < 0) {
      fprintf(stderr, "Error, cannot write data in file.\n");
      return 1;
    }
  }

  fclose(dest_file);

  printf("Finished downloading file\n");

  return 0;
}


int endConnection(FTPInfo ftp){
  printf("Closing connection\n");
  sendMessage(ftp.controlSocketFd,"QUITTING\r\n");

  if(readMessage(ftp.controlSocketFd,NULL) != 0){
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
