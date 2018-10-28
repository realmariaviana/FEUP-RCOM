#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include "dataLink.h"

typedef struct  {
  int port;
  int fileDescriptor;/*Descritor correspondente à porta série*/
  status mode;/*TRANSMITTER | RECEIVER*/
}applicationLayer;

applicationLayer app;

int createControlPacket(char* filename, unsigned long filesize, unsigned char control_byte, unsigned char * packet);
int createDataPacket(unsigned char* data, int dataSize, unsigned char* packet);
int receivePacket(unsigned char *packet);
void recieveDataPacket(unsigned char *packet);
void recieveControlPacket(unsigned char *packet, unsigned char control_byte);
