#include "dataLink.h"

  unsigned char SET[5] = {FLAG, A_SEND, C_SET, A_SEND ^ C_SET, FLAG};
unsigned char UA[5] = {FLAG, A_SEND, C_UA, A_SEND ^ C_UA, FLAG};
unsigned char DISC[5] ={FLAG, A_SEND, C_DISC, A_SEND ^ C_DISC, FLAG};

volatile int STOP=FALSE;
int timeOut = FALSE;
int count = 0;

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

int stateMachine(unsigned char c, int state, char * msg){
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

    link_layer.oldTermios = oldtio; //saves old termios structure to undo change in the end

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
        perror("tcgetattr");
        exit(-1);
      }

      bzero(&newtio, sizeof(newtio));
      newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
      newtio.c_iflag = IGNPAR;
      newtio.c_oflag = 0;

      /* set input mode (non-canonical, no echo,...) */
      newtio.c_lflag = 0;
      if(link_layer.mode == TRANSMITTER){
      newtio.c_cc[VTIME]    = 5;   /* inter-character timer unused */
      newtio.c_cc[VMIN]     = 0;  /* blocking read until 5 chars received */
    }   else {
      newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
      newtio.c_cc[VMIN]     = 1;
    }


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
        printf("data_link - llopen(): invalid port!\n");
        return -1;
      }
          if(link_layer.mode == TRANSMITTER)
        	   fd = open(link_layer.port, O_RDWR | O_NOCTTY | O_NONBLOCK);
          else
          fd = open(link_layer.port, O_RDWR | O_NOCTTY);

            if (fd <0) {perror(link_layer.port);printf("ll open errror FD: %d\n", fd); exit(-1);   }

      if (setTermios(fd) != 0) {
        printf("dataLink - llopen() - setTermios: error\n");
        return -1;
      }

      if(mode == TRANSMITTER)
      if(llopenTransmitter(fd) <0)
      return -1;

      if(mode == RECEIVER)
      if(llopenReceiver(fd) < 0)
      return -1;

      link_layer.sequenceNumber = 0;

      return fd;
  }

int llopenTransmitter(int fd){
  unsigned char c;
  int state = 0;

  signal(SIGALRM, alarmHandler);
  sleep(1);
  do{

    if(write(fd, SET, 5) != 5){
      printf("dataLink - llopen: error writting SET");
      exit(-1);
    }
    printf("SET sent\n");
    timeOut = false;
    alarm(link_layer.timeout);
    sleep(1);
    while(state != 5 && !timeOut){

      if(read(fd, &c, 1) == -1){
        printf("dataLink - llopen: error reading");
        exit(-1);
      }
        state = stateMachine(c, state, UA);
    }
    printf("UA RECEIVED\n");

  } while(timeOut && count < link_layer.transmissions);
}

int llopenReceiver(int fd){
  unsigned char c;
  int state=0;
  //sleep(1);
  while(state!=5){
     if(read(fd, &c, 1) == -1){
       printf("dataLink - llopen: read error");
       exit(-1);
     }

     state = stateMachine(c, state, SET);

   }
   printf("SET RECEIVED\n");

   if(write(fd, UA, 5) != 5){
     printf("dataLink - llopen: error writing UA");
     exit(-1);
   }
   printf("UA SENT\n");
   return 0;
}

int llwrite(int fd, char * packet, int length, int * rejCounter){
  int frameLength;

  unsigned char *frame = createIFrame(&frameLength, packet, length);

  do{

    if(writePacket(fd, frame, frameLength) < 0){
      printf("llwrite: error sending packet\n");
      return -1;
    }

    timeOut = false;
    alarm(link_layer.timeout);



  }while(timeOut && count < link_layer.transmissions);

  return -1;
}

unsigned char createIFrame(int *frameLength, char *packet, int packetLength){
  unsigned char *stuffPacket = stuff(packet, &packetLength);
  *frameLength = packetLength + 5; //packetLength + 5 flags
  unsigned char *frame = (unsigned char *)malloc(*frameLength * sizeof(char));

  frame[0] = FLAG;
  frame[1] = A_SEND;
  frame[2] = link_layer.sequenceNumber << 6; // PQ 6???????????????
  frame[3] = frame[1] ^ frame[2];

  memcpy(frame + 4, stuffPacket, packetLength); //copies stuffed packet to frame
  frame[*frameLength-1] = FLAG; //frame[4] = FLAG;

  return *frame;
}

int writePacket(int fd, unsigned char* buffer, int bufLength){
  int charTotal = 0;
  int chars = 0;

  while(charTotal < bufLength){
    usleep(WAIT * 1000);
    chars = write(fd, buffer, bufLength);

    if(chars <= 0) {
      printf("error writing\n");
      return -1;
    }

    charTotal += chars;
  }

  return 0;
}

int readPacket(int fd, unsigned char* frame, int frameLength){
return 0;
}

bool frameSCorrect(unsigned char *response, int responseLength, unsigned char C){
  if(responseLength < 5) return false;

  if(response[0] == (unsigned char)FLAG &&
   response[1] == (unsigned char)A_SEND &&
   response[3] == (unsigned char)(response[1]^response[2]) &&
   response[4] == (unsigned char)FLAG &&
   ((C == RR  && response[2] == (unsigned char)(!link_layer.sequenceNumber << 7 | C)) ||
   (C == REJ && response[2] == (unsigned char)(link_layer.sequenceNumber << 7 | C))))
   return true;
   else
   return false;
}

unsigned char *createSFrame(char ctrlByte){
  unsigned char *answer = (unsigned char *)malloc(SFRAMELEN * sizeof(char));

  answer[0] = FLAG;
  answer[1] =  A_SEND;
  answer[2] = (link_layer.sequenceNumber << 7) | ctrlByte; //PORQUE 7?????????????
  answer[3] = answer[1] ^ answer[2];
  answer[4] = FLAG;

  return answer;
}

bool frameICorrect(unsigned char * frame){
  if(frame[0] == FLAG && frame[1] == A_SEND && frame[3] == (frame[1] ^ frame[2])) // WTFFFFFFFFFF
    return true;
    else
      return false;
}

unsigned char *stuff(char *packet, int *packetLength){

  unsigned char* stuffed = (unsigned char *)malloc(2 * sizeof(unsigned char*));

  unsigned char BCC2 = 0;
  int i = 0, j = 0;

  //Calcular o BCC2
  for(i = 0; i < *packetLength; i++){
    BCC2 ^= packet[i];
  }

  //QUE É ESTA MERDAAA????????????
  if(link_layer.wrongPackets > 0){
    BCC2 = 0;
    link_layer.wrongPackets--;
  }

  packet[*packetLength] = BCC2;
  *packetLength = *packetLength + 1;

  //Fazer stuff
  for(i = 0; i < *packetLength; i++){

    if(packet[i] == FLAG || packet[i] == ESC){
      stuffed[j] = ESC;
      stuffed[++j] = packet[i] ^ BYTE_STUFF;
    } else{
      stuffed[j] = packet[i];
      j++;
    }
  }

  *packetLength = j;

  return stuffed;

}

unsigned char *destuff(char *packet, int *packetLength){
  unsigned char* destuffed = (unsigned char*)malloc(((*packetLength)) * sizeof(char));

  int i = 0, j = 0;

  for (; i < *packetLength; i++){
    if(packet[i] == ESC){

      destuffed[i] = packet[i + 1] ^ BYTE_STUFF;
      i++;

    } else{

      destuffed[j] = packet[i];
      j++;

    }
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
      printf("dataLink - llclose: error writting DISC");
      return -1;
    }
    printf("DISC SENT!\n");

    timeOut = false;
    alarm(link_layer.timeout);
  sleep(1);
    while(state != 5 && !timeOut){

      if(read(fd, &c, 1) == -1){
        printf("dataLink - llclose: error reading");
        return -1;
      }
        state = stateMachine(c, state, DISC);
    }
    printf("RECEIVED DISC\n");

  } while(timeOut && count < link_layer.transmissions);

  if(write(fd, UA, 5) != 5){
    printf("dataLink - llclose: error writting UA");
  }

  printf("UA SENT\n");

  sleep(1);
  return 0;
}

int llcloseReceiver(int fd){
  unsigned char c;
  int state = 0;

  while(state != 5){

     if(read(fd, &c, 1) == -1){
       printf("dataLink - llclose: error reading DISC");
       return -1;
     }

     state = stateMachine(c, state, DISC);

   }

   printf("DISC RECEIVED!\n");

   if(write(fd, DISC, 5) != 5){
     printf("dataLink - llclose: error writing DISC");
     exit(-1);
   }

   printf("DISC SENT!\n");

   state = 0;

   while(state != 5) {
     if(read(fd, &c, 1) == -1){
       printf("dataLink - llclose: error reading UA");
       return -1;
     }

     state = stateMachine(c, state, UA);
  }
  printf("UA RECEIVED\n");

   return 0;
}

int main(int argc, char** argv)
  {

      int fd,c, res,mode;
      struct termios oldtio,newtio;
      unsigned char buf[5];
      int i, sum = 0, speed = 0;

  	 if ( (argc < 3) ||
    	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
    	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
  	}

  	 if(strcmp(argv[2],"T")==0){
  		printf("MODE: TRANSMITER\n");
  		mode=0;
  }
  	else if(strcmp(argv[2],"R")==0){
  	printf("MODE: RECEIVER\n");
  		mode=1;
  }
  	else
  	printf("Usage: T or R to transitter/receiver mode");

    initDataLinkStruct(TRANSMISSIONS, TIMEOUT, BAUDRATE);

  	fd = llopen(0,mode);
    llclose(fd);

  	return 0;
  }
