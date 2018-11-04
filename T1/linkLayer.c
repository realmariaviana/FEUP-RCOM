#include "linkLayer.h"

unsigned char SET[5] = {FLAG, A_SEND, C_SET, A_SEND ^ C_SET, FLAG};
unsigned char UA[5] = {FLAG, A_SEND, C_UA, A_SEND ^ C_UA, FLAG};
unsigned char DISC[5] ={FLAG, A_SEND, C_DISC, A_SEND ^ C_DISC, FLAG};
unsigned char UA_ALT[5] = {FLAG, A_ALT, C_UA, A_ALT ^ C_UA, FLAG};
unsigned char DISC_ALT[5] ={FLAG, A_ALT, C_DISC, A_ALT ^ C_DISC, FLAG};

unsigned char RR0[5] = {FLAG, A_SEND, RR, A_SEND^RR, FLAG};
unsigned char RR1[5] = {FLAG, A_SEND, RR_ALT, A_SEND^RR_ALT, FLAG};
unsigned char REJ0[5] = {FLAG, A_SEND, REJ, A_SEND^REJ, FLAG};
unsigned char REJ1[5] = {FLAG, A_SEND, REJ_ALT, A_SEND^REJ_ALT, FLAG};

bool timeOut = false;
int count = 0;
bool ignore = false;

void initDataLinkStruct(int transmissions, int timeOut, int baudRate){

  link_layer.transmissions = transmissions;
  link_layer.timeout = timeOut;
  link_layer.baudRate = baudRate;
}

void alarmHandler(int sig){
  timeOut = true;
  count ++;
  printf("TIMED OUT\n");
}

int stateMachine(unsigned char c, int state, unsigned char * msg){
  switch (state) {

    case 0:
      if(c == msg[0])
        return 1;
    break;

    case 1:
      if(c == msg[1])
        return 2;

      if(c != msg[0])
        return 0;
    break;

    case 2:
      if(c == msg[2])
        return 3;

      if(c!= msg[0])
        return 0;
      else
        return 1;
    break;

    case 3:
      if( c == (msg[2]^msg[1]))
        return 4;

      if(c!= msg[0])
        return 0;
      if( c== msg[0])
        return 1;
    break;

    case 4:
      if(c != msg[0])
        return 0;
      else
        return 5;
    break;

    default:
    break;
  }
  return 0;
}

int setTermios(int fd){

    struct termios oldtio, newtio;

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
      }

      link_layer.oldTermios = oldtio; //saves old termios structure to undo change in the end

      bzero(&newtio, sizeof(newtio));
      newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
      newtio.c_iflag = IGNPAR;
      newtio.c_oflag = 0;

      /* set input mode (non-canonical, no echo,...) */
      newtio.c_lflag = 0;
      //if(link_layer.mode == TRANSMITTER){
      newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
      newtio.c_cc[VMIN]     = 0;  /* blocking read until 5 chars received */
    // }   else {
    //   newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    //   newtio.c_cc[VMIN]     = 0;
    // }


    /*
      VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
      leitura do(s) pr�ximo(s) caracter(es)
    */


      tcflush(fd, TCIOFLUSH);

      if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
        perror("tcsetattr");
        exit(-1);
      }

  	return 0;

  }

int llopen(int port, status mode){
      int fd;
      link_layer.mode = mode;

      switch (port) {
        case COM1:
        strcpy(link_layer.port, COM1_PORT);
        break;

        case COM2:
        strcpy(link_layer.port, COM2_PORT);
        break;

        default:
        printf("linkLayer - llopen(): invalid port\n");
        return -1;
      }

      fd = open(link_layer.port, O_RDWR | O_NOCTTY);

      if (fd <0) {
        perror(link_layer.port);
        exit(-1);
      }

      if (setTermios(fd) != 0) {
        printf("linkLayer - llopen() - setTermios: error\n");
        return -1;
      }

      if(mode == TRANSMITTER){
        if(llopenTransmitter(fd) <0)
        return -1;
      }

      if(mode == RECEIVER){
        if(llopenReceiver(fd) < 0)
        return -1;
      }


	  link_layer.sequenceNumber = 0;

      printf("linkLayer - llopen() - SUCCESS!\n");

      return fd;
  }

int llopenTransmitter(int fd){
  unsigned char c;
  int state = 0;

  signal(SIGALRM, alarmHandler);

  do{

    if(write(fd, SET, 5) != 5){
      printf("linkLayer - llopen: error writting SET\n");
      exit(-1);
    }

    printf("SET sent\n");

    timeOut = false;
    alarm(link_layer.timeout);

    while(state != 5 && !timeOut){

      if(read(fd, &c, 1) == -1){
        printf("linkLayer - llopen: error reading\n");
        exit(-1);
      }

        state = stateMachine(c, state, UA);
    }
    printf("UA RECEIVED\n");

  } while(timeOut && count < link_layer.transmissions);

  if (count == link_layer.transmissions)
      return -1;
    else
      return 0;
}

int llopenReceiver(int fd){
  unsigned char c;
  int state = 0;

  while(state!=5){
     if(read(fd, &c, 1) == -1){
       printf("linkLayer - llopen: read error\n");
       exit(-1);
     }

     state = stateMachine(c, state, SET);

   }
   printf("SET RECEIVED\n");

   if(write(fd, UA, 5) != 5){
     printf("linkLayer - llopen: error writing UA\n");
     exit(-1);
   }

   printf("UA SENT\n");

   return 0;
}

int llwrite(int fd,unsigned char * packet, int length, int * rejCounter){
  int frameLength;
  unsigned char *frame = createIFrame(&frameLength, packet, length);
  count = 0;
  int state;

  do{
    if(writePacket(fd, frame, frameLength) < 0){
      printf("llwrite: error sending packet\n");
      return -1;
    }

    timeOut = false;
    alarm(link_layer.timeout);

    state=0;
    unsigned char c, C;
    while(state!=5 && !timeOut){
      if(read(fd, &c, 1) == -1) {
        printf("linkLayer - llopen: read error\n");
        exit(-1);
      }

      switch (state) {
      case 0:
        if(c == FLAG)
          state = 1;
        break;

      case 1:
        if(c == A_SEND)
          state = 2;
        else if(c != FLAG)
          state = 0;
        break;

      case 2:
        if(c == RR || c == RR_ALT || c == REJ || c == REJ_ALT){
          state = 3;
          C = c;
        }
        else if(c == FLAG)
          state = 1;
        else
          state = 0;
        break;

      case 3:
        if( c == (A_SEND^C))
          state = 4;
        else if(c == FLAG)
          state = 1;
        else
          state = 0;
        break;

      case 4:
        if(c == FLAG)
          state = 5;
        else {
          state = 0;
        }
        break;

      default:
        break;
      }
    }

    if ((link_layer.sequenceNumber && C == RR) || (!link_layer.sequenceNumber && C == RR_ALT)) {
      printf("Recebeu RR %x, trama = %d\n", C, link_layer.sequenceNumber);
      link_layer.sequenceNumber ^= 1;
      alarm(0);
    }
    else if ((link_layer.sequenceNumber && C == REJ) || (!link_layer.sequenceNumber && C == REJ_ALT)){
      printf("Recebeu REJ %x, trama = %d\n", C, link_layer.sequenceNumber);
      timeOut = true;
      alarm(0);
    }
  }while(timeOut && count < link_layer.transmissions);

  if (count == link_layer.transmissions)
    return -1;

  return 0;
}

int llread(int fd, unsigned char *packet, int *packetSize){

  unsigned char c;
  int state=0;
  int res;

  while(state!=5){
    if((res = read(fd, &c, 1)) == -1){
      printf("linkLayer - llopen: read error\n");
      return -1;
    }
    else if(res == 0 && state == 4)
      break;

    //printf("state: %d\n", state);

    switch (state) {
      case 0:
        if(c == FLAG)
          state = 1;
        break;

      case 1:
        if(c == A_SEND)
          state = 2;
        else if(c != FLAG)
          state = 0;
        break;

      case 2:
        if(c == (link_layer.sequenceNumber << 6))
          state = 3;
        else if(c == FLAG)
          state = 1;
        else
          state = 0;
        break;

      case 3:
        if( c == (A_SEND^(link_layer.sequenceNumber << 6)))
          state = 4;
        else if(c == FLAG)
          state = 1;
        else
          state = 0;
        break;

      case 4:
        if(c == FLAG)
          state = 5;
        else {
          packet[(*packetSize)++] = c;
        }
        break;

      default:
        break;
    }
  }

  printf("saiu da state machine\n");
  //printf("antes destuff - %d\n", *packetSize);

  unsigned char* destuffed;
  destuffed = destuff(packet, packetSize);
  memcpy(packet, destuffed, *packetSize);

  //printf("depois do destuff: %d\n", *packetSize);
  // int i = 0;
  // for(; i < *packetSize; i++){
  //   printf("%x\n", packet[i]);
  // }


  if (correctBCC2(destuffed, *packetSize) == 0){
    ignore = false;
    if (link_layer.sequenceNumber)
      write(fd, RR0, 5);
    else
      write(fd, RR1, 5);

    printf("Enviou RR, sequenceNumber:%d\n", link_layer.sequenceNumber);

    link_layer.sequenceNumber ^= 1;
  }
  else {
    ignore = true;

    if (link_layer.sequenceNumber)
      write(fd, REJ0, 5);
    else
      write(fd, REJ1, 5);

    printf("Enviou REJ, sequenceNumber:%d\n", link_layer.sequenceNumber);
  }
  return ignore;
}

unsigned char *createIFrame(int *frameLength,unsigned char *packet, int packetLength){
  unsigned char *stuffPacket = stuff(packet, &packetLength);
  *frameLength = packetLength + 5; //packetLength + 5 flags
  unsigned char *frame = (unsigned char *)malloc(*frameLength * sizeof(unsigned char));

  frame[0] = FLAG;
  frame[1] = A_SEND;
  frame[2] = link_layer.sequenceNumber << 6;
  frame[3] = frame[1] ^ frame[2];

  memcpy(frame + 4, stuffPacket, packetLength); //copies stuffed packet to frame
  frame[*frameLength-1] = FLAG; //frame[4] = FLAG;

  return frame;
}

int writePacket(int fd, unsigned char* buffer, int bufLength){
  int chars;

  chars = write(fd, buffer, bufLength);

  if(chars < 0) {
    printf("error writing\n");
    return -1;
  }
  else
    printf("Enviou I com %d bytes\n", chars);

  return 0;
}

unsigned char correctBCC2(const unsigned char* buf, unsigned int size) {
	unsigned char BCC2 = buf[0];

	unsigned int i;
	for (i = 1; i < size; ++i)
		BCC2 ^= buf[i];

	return BCC2;
}

unsigned char *stuff(unsigned char *packet, int *packetLength){

  unsigned char* stuffed = (unsigned char *)malloc(2 * (*packetLength));

  unsigned char BCC2 = 0;
  int i = 0, j = 0;

  //Calcular o BCC2
  for(i = 0; i < *packetLength; i++){
    BCC2 ^= packet[i];
  }

  packet[*packetLength] = BCC2;
  *packetLength = *packetLength + 1;

  //Fazer stuff
  for(i = 0; i < *packetLength; i++){

     if(packet[i] == FLAG || packet[i] == ESC){
      stuffed[j] = ESC;
      stuffed[++j] = packet[i] ^ BYTE_STUFF;
      j++;
    }

    else{
      stuffed[j] = packet[i];
      j++;
    }
  }

  *packetLength = j;

  return stuffed;

}

unsigned char *destuff(unsigned char *packet, int *packetLength){

  unsigned char* destuffed = (unsigned char*)malloc(((*packetLength)));

  int i = 0, j = 0;

  for (; i < *packetLength; i++){
    if(packet[i] == ESC){
      destuffed[j] = packet[i + 1] ^ BYTE_STUFF;
      i++;
    } else
      destuffed[j] = packet[i];

      j++;
  }

  *packetLength = j;

  return destuffed;
}

int llclose(int fd){
  if(link_layer.mode == TRANSMITTER)
  if(llcloseTransmitter(fd) <0)
  return -1;

  if(link_layer.mode == RECEIVER)
  if(llcloseReceiver(fd) < 0)
  return -1;

  if ( tcsetattr(fd, TCSANOW, &link_layer.oldTermios) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);
  return 0;
}

int llcloseTransmitter(int fd){
  unsigned char c;
  int state = 0;

  do{

    if(write(fd, DISC, 5) != 5){
      printf("linkLayer - llclose: error writting DISC\n");
      return -1;
    }
    printf("DISC SENT!\n");

    timeOut = false;
    alarm(link_layer.timeout);

    while(state != 5 && !timeOut){

      if(read(fd, &c, 1) == -1){
        printf("linkLayer - llclose: error reading\n");
        return -1;
      }
        state = stateMachine(c, state, DISC_ALT);
    }
    printf("RECEIVED DISC\n");

  } while(timeOut && count < link_layer.transmissions);

  if(write(fd, UA_ALT, 5) != 5){
    printf("linkLayer - llclose: error writting UA\n");
  }

  printf("UA SENT\n");


  return 0;
}

int llcloseReceiver(int fd){
  unsigned char c;
  unsigned char d;
  int state = 0;

  while(state != 5){

     if(read(fd, &c, 1) == -1){
       printf("linkLayer - llclose: error reading DISC\n");
       return -1;
     }

     state = stateMachine(c, state, DISC);

   }

   printf("DISC RECEIVED!\n");

   if(write(fd, DISC_ALT, 5) != 5){
     printf("linkLayer - llclose: error writing DISC\n");
     exit(-1);
   }

   printf("DISC SENT!\n");

   state = 0;

   while(state != 5) {
     if(read(fd, &d, 1) == -1){
       printf("linkLayer - llclose: error reading UA\n");
       return -1;
     }
     state = stateMachine(d, state, UA_ALT);
  }
  printf("UA RECEIVED\n");

   return 0;
}
