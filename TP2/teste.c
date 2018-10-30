#include "teste.h"

unsigned char seqNum = 0;
int fDes;

int main(int argc, char** argv){

  if ( (argc < 4) ||
       ((strcmp(COM1_PORT, argv[1])!=0) && (strcmp(COM2_PORT, argv[1])!=0)) ||
       ((strcmp("T", argv[2]) !=0) &&  (strcmp("R", argv[2]) !=0))) {

    printf("Usage:\tnserial SerialPort ComunicationMode\n\tex: nserial /dev/ttyS1 R penguin.gif\n");
    exit(1);
  }

  char *filename = argv[3];

  initDataLinkStruct(TRANSMISSIONS, TIMEOUT, BAUDRATE);

  set_connection(argv[1],argv[2]);

  if(app.mode == TRANSMITTER){
      transmitterMode(filename);
  }
  else if (app.mode == RECEIVER) {
      receiverMode(filename);
  }

  llclose(app.fileDescriptor);
  return 0;
}

int transmitterMode(char* fileName) {
    int file;
    struct stat data;
    long fileSize;

    if ((file = open(fileName, O_RDONLY)) == -1) {
        perror("Error while opening the file");
        return 0;
    }

    stat((const char *)fileName, &data); //get the file metadata
    fileSize = data.st_size; //gets file size in bytes
    //unsigned char *fileData = (unsigned char *)malloc(fileSize);

    int rejCounter = 0;
    unsigned char startPacket[PACKET_SIZE];
    int packetSize = createControlPacket(fileName, fileSize, START, startPacket);
    llwrite(app.fileDescriptor, startPacket, packetSize, &rejCounter);

    unsigned char msg[DATA_PACKET_SIZE];
    unsigned char packet[PACKET_SIZE];

    while ((packetSize = read(file, msg, DATA_PACKET_SIZE)) != 0) {
      createDataPacket(msg, packetSize, packet);
      llwrite(app.fileDescriptor, startPacket, packetSize, &rejCounter);
    }

    unsigned char endPacket[PACKET_SIZE];
    packetSize = createControlPacket(fileName, fileSize, END, endPacket);
    llwrite(app.fileDescriptor, endPacket, packetSize, &rejCounter);
    return 0;
}

int receiverMode(char* filename) {
    int packetSize = 0;
    unsigned char startPacket[(PACKET_SIZE+5)*2];
    llread(app.fileDescriptor, startPacket, &packetSize);
    receiveControlPacket(startPacket, START);

    unsigned char dataPacket[DATA_PACKET_SIZE];

    int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT );

    while(true){
      packetSize = 0;
      llread(app.fileDescriptor, dataPacket, &packetSize);
      receiveDataPacket(dataPacket, fd);
    }
    close(fd);

    packetSize = 0;
    unsigned char endPacket[(PACKET_SIZE+5)*2];
    llread(app.fileDescriptor, endPacket, &packetSize);
    receiveControlPacket(endPacket, END);

    return 0;
}

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
  packet[1] = seqNum++;
  packet[2] = dataSize/256;
  packet[3] = dataSize%256;
  int i = 0;
  for(; i < dataSize; i++){
      packet[i+4] = data[i];
  }
  return i+4;
}

void receiveDataPacket(unsigned char *packet, int fd){
  unsigned char l1, l2;
  int k;
  l1 = packet[3];
  l2 = packet[2];
  k = 256*l2 + l1;
  unsigned char d[k];
  int i = 0;
  for(; i < k; i++){
    d[i] = packet[i+4];
  }

  write(fd, d, k);
}

void receiveControlPacket(unsigned char *packet, unsigned char control_byte){
  // int fileSizeLength = (int) packet[2];
  unsigned long fileSize = 0;

  fileSize += packet[6];
  fileSize += packet[5] << 8;
  fileSize += packet[4] << 16;
  fileSize += packet[3] << 24;
  printf("fileSize: %ld\n", fileSize);

  int filenameSize = (int) packet[8];
  unsigned char filename[filenameSize];
  int i = 0;
  for(; i < filenameSize; i++){
    filename[i] = packet[i+9];
  }

  printf("filename: %s\n", filename);
}

// int receivePacket(unsigned char *packet){
//   switch(packet[0]){
//     case 1:
//       receiveDataPacket(packet);
//       break;
//     case 2:
//       receiveControlPacket(packet, START);
//       break;
//     case 3:
//       receiveControlPacket(packet, END);
//       break;
//     default:
//       break;
//   }
//   return 0;
// }

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
