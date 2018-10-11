/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define FLAG 0x7E
#define A_SEND 0x03
#define C_SET 0x03
#define SET_BCC 0x00
#define C_UA 0x07
#define C_DISC 0x0B
#define TRANSMITTER 0
#define RECEIVER 1

char SET[5] = {FLAG, A_SEND, C_SET, A_SEND ^ C_SET, FLAG};
char UA[5] = {FLAG, A_SEND, C_UA, A_SEND ^ C_UA, FLAG};
char DISC[5] ={FLAG, A_SEND, C_DISC, A_SEND ^ C_DISC, FLAG};
struct termios oldtio, newtio;

volatile int STOP=FALSE;


int setTermios(int fd){



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



int llopenTransmitter(int fd){

 write(fd, SET, 5);
	//RECEIVE UA
    unsigned int state=0;
	int i = 0;
	int res;
	sleep(1);
	unsigned char buf[5];


    while (STOP==FALSE) {       /* loop for input */
      res = read(fd,&buf,1);
		
     
      switch (state) {
        case 0:
          if(buf[0] == FLAG){
            state = 1;//flag received
       }
          else{
            state = 0;
		}
          break;
        case 1:
          if(buf[0] == A_SEND)
            state=2;//address received
          else if(buf[0]==FLAG)
            state=1;
          else{
            state=0;
		}
          break;
        case 2:
          if(buf[0]==C_UA){//control UA received
            state=3;
		
}
          else if(buf[0]==FLAG)
            state=1;
          else
            state=0;
break;
        case 3:
          if(buf[0]==(A_SEND^C_UA)){//bcc received
            state=4;
}
          else if(buf[0]==FLAG)
            state=1;
          else
            state=0;
break;
        case 4:
          if(buf[0]==FLAG){//bcc received
			STOP=TRUE;
          }
          else
            state=0;
break;
      }
i++;
    }

    //printf("%c\n", buf[0]);
    printf("UA received\n");


    return 0;

}

int llopenReceiver(int fd){
 unsigned char c;
  int state=0;

  while(state!=5){

    read(fd,&c,1);

    switch(state){
      //recebe flag
      case 0:
	printf("0\n");
        if(c==FLAG)
          state=1;
          break;
      //recebe A
      case 1:
	printf("1\n");
        if(c==A_SEND)
          state=2;
        else
          {
            if(c==FLAG)
              state=1;
            else
              state = 0;
          }
      break;
      //recebe C
      case 2:
	printf("2\n");
        if(c==C_SET)
 
         state=3;
        else{
          if(c==FLAG)
            state=1;
          else
            state = 0;
        }
      break;
      //recebe BCC
      case 3:
	printf("3\n");
        if(c==SET_BCC)
          state = 4;
        else
          state=0;
      break;
      //recebe FLAG final
      case 4:
	printf("4\n");
        if(c==FLAG){
	  printf("Recebeu SET\n");
    printf("%d\n", UA[0]);
    printf("%d\n", UA[1]);
    printf("%d\n", UA[2]);
    printf("%d\n", UA[3]);
    printf("%d\n", UA[4]);
	  write(fd,UA,5);
	  printf("Enviou UA\n");
	  state=5;
	}
        else
          state = 0;
      break;
		
    } 
  }

}

int llopen(int porta, int mode){

	int fd;

	
  if(mode == TRANSMITTER)
  if(llopenTransmitter(fd) <0)
  return -1;


  if(mode == RECEIVER)
  if(llopenReceiver(fd) < 0)
	return -1;

tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);

	return 0;
}

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    unsigned char buf[5];
    int i, sum = 0, speed = 0;

	 if ( (argc < 3) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
	}

	   fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }
	
	setTermios(fd);
	llopen(fd,argv[2]);
}

