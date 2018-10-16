#include "dataLink.h"

char SET[5] = {FLAG, A_SEND, C_SET, A_SEND ^ C_SET, FLAG};
char UA[5] = {FLAG, A_SEND, C_UA, A_SEND ^ C_UA, FLAG};
char DISC[5] ={FLAG, A_SEND, C_DISC, A_SEND ^ C_DISC, FLAG};

volatile int STOP=FALSE;
int timeOut = FALSE;
int count = 0;

void alarmHandler(int sig){
  timeOut = true;
  count ++;
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

      newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
      newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */


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

      link_layer.mode = mode; //receiver or transmitter

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

      fd = open(link_layer.port, O_RDWR | O_NOCTTY );
      if (fd <0) {perror(link_layer.port); exit(-1); }

      if (setTermios(fd) != 0) {
        printf("dataLink - llopen() - setTermios: error\n");
        return -1;
      }

      link_layer.mode = mode;

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

  do{

    if(write(fd, SET, 5) != 5){
      printf("dataLink - llopen: error writting SET");
      exit(-1);
    }

    timeOut = false;
    alarm(link_layer.timeout);

    while(state != 5 && !timeOut){

      if(read(fd, &c, 1) == -1){
        printf("dataLink - llopen: error reading");
        exit(-1);
      }
        state = stateMachine(c, state, SET);
    }

  } while(timeOut && count < link_layer.transmissions);
}

int llopenReceiver(int fd){
  unsigned char c;
  int state=0;

  while(state!=5){

     if(read(fd, &c, 1) == -1){
       printf("dataLink - llopen: read error");
       exit(-1);
     }

     state = stateMachine(c, state, SET);

   }

   if(write(fd, UA, 5) != 5){
     printf("dataLink - llopen: error writing UA");
     exit(-1);
   }

   return 0;
}

int llclose(int fd){

  if(link_layer.mode == TRANSMITTER)
  if(llcloseTransmitter(fd) <0)
  return -1;

  if(link_layer.mode == RECEIVER)
  if(llcloseReceiver(fd) < 0)
  return -1;

  if ( tcsetattr(fd, TCSANOW, &link_layer.portSettings) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);
  return 0;
}

int llcloseTransmitter(int fd){
  unsigned char c;
  int state = 0;

  signal(SIGALRM, alarmHandler);

  do{

    if(write(fd, DISC, 5) != 5){
      printf("dataLink - llclose: error writting DISC");
      return -1;
    }

    timeOut = false;
    alarm(link_layer.timeout);

    while(state != 5 && !timeOut){

      if(read(fd, &c, 1) == -1){
        printf("dataLink - llclose: error reading");
        return -1;
      }
        state = stateMachine(c, state, DISC);
    }

  } while(timeOut && count < link_layer.transmissions);

  if(write(fd, UA, 5) != 5){
    printf("dataLink - llclose: error writting UA");
  }
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

   if(write(fd, DISC, 5) != 5){
     printf("dataLink - llclose: error writing DISC");
     exit(-1);
   }

   state = 0;

   while(state != 5) {
     if(read(fd, &c, 1) == -1){
       printf("dataLink - llclose: error reading UA");
       return -1;
     }

     state = stateMachine(c, state, UA);
  }
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


  	   fd = open(argv[1], O_RDWR | O_NOCTTY );
      if (fd <0) {perror(argv[1]); exit(-1); }

  	setTermios(fd);
  	llopen(fd,mode);
    llclose(fd);

  	return 0;
  }
