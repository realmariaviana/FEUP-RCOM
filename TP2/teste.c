#include "teste.h"

unsigned char seqNum = 0;
int fDes;

int createControlPacket(char* filename, unsigned long filesize, unsigned char control_byte, unsigned char * packet){

  packet[0] = control_byte;
  packet[1] = FILE_SIZE;
  packet[2] = sizeof(filesize);
  packet[3] = (filesize >> 24) & 0xFF;
  packet[4] = (filesize >> 16) & 0xFF;
  packet[5] = (filesize >> 8) & 0xFF;
  packet[6] = filesize & 0xFF;
  packet[7] = FILE_NAME;
  packet[8] = strlen(filename) + 1;
  int i=0;
  for(; i< strlen(filename) + 1; i++){
    packet[i+9] = filename[i];
  }

  return i+9;
}

int createDataPacket(unsigned char* data, int dataSize, unsigned char* packet){
  packet[0] = DATA;
  packet[1] = seqNum;
  packet[2] = dataSize/256;
  packet[3] = dataSize%256;
  int i = 0;
  for(; i < dataSize; i++){
      packet[i+4] = data[i];
  }
  return i+4;
}

int receivePacket(unsigned char *packet){
  switch(packet[0]){
    case 1:
      recieveDataPacket(packet);
      break;
    case 2:
      receiveControlPacket(packet, START);
      break;
    case 3:
      receiveControlPacket(packet, END);
      break;
    default:
      break;
  }
  return 0;
}

void recieveDataPacket(unsigned char *packet){
  unsigned char l1, l2;
  int k;
  l1 = packet[3];
  l2 = packet[2];
  k = 256*l2 + l1;
  unsigned char d[k];
  int i = 0;
  for(; i < k; i++){
    d[i] = packet[i+3];
  }

  write(fDes, d, k);
}

void receiveControlPacket(unsigned char *packet, unsigned char control_byte){
  unsigned char filenameSize = packet[8];
  unsigned char filename[filenameSize];
  int i = 0;
  for(; i < filenameSize; i++){
    filename[i] = packet[i+9];
  }
  if(control_byte == START){
    fDes = open(filename, O_APPEND | O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
  } else if(control_byte == END){
    close(fDes);
  }
}

void set_connection(char * port, char * stat){

  if(strcmp(port,COM1_PORT)==0)
  app.port = COM1;

  if(strcmp(port,COM2_PORT)== 0)
  app.port =COM2;

  if(strcmp(stat,"T")==0){
    app.mode = TRANSMITTER;
  }

  if(strcmp(stat,"R")== 0){
    app.mode = RECEIVER;
  }

  app.fileDescriptor = llopen(app.port,app.mode);

  if(app.fileDescriptor< 0){
    printf("app - set_connection(): invalid file descriptor\n");
    exit(-1);
  }


}


int main(int argc, char** argv){

  if ( (argc < 4) ||
       ((strcmp(COM1_PORT, argv[1])!=0) && (strcmp(COM2_PORT, argv[1])!=0)) ||
       ((strcmp("T", argv[2]) !=0) &&  (strcmp("R", argv[2]) !=0))) {

    printf("Usage:\tnserial SerialPort ComunicationMode\n\tex: nserial /dev/ttyS1 R penguin.gif\n");
    exit(1);
  }

  unsigned char* filename = argv[3];

  initDataLinkStruct(TRANSMISSIONS, TIMEOUT, BAUDRATE);

  set_connection(argv[1],argv[2]);

  struct stat d;
  unsigned char* packet;
  int rejCounter = 0;
  stat(filename, &d);
  unsigned long filesize = d.st_size;

  if(app.mode == TRANSMITTER){
    int packetsize = createControlPacket(filename, filesize, START, packet);
    llwrite(app.fileDescriptor, packet, packetsize, rejCounter);
  }

  llclose(app.fileDescriptor);
  return 0;
}
