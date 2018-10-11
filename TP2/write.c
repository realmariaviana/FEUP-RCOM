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
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07

int STOP=0;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    unsigned char buf[5];
    int i, sum = 0, speed = 0;
	  unsigned char SETUP[5];

	SETUP[0]=FLAG;
	SETUP[1]=A;
	SETUP[2]=C_SET;
	SETUP[3]=A^C_SET;
	SETUP[4]=FLAG;

    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

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


    write(fd, SETUP, 5);
	//RECEIVE UA
    unsigned int state=0;
	i = 0;
	sleep(1);


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
          if(buf[0] == A)
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
          if(buf[0]==(A^C_UA)){//bcc received
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

  
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
    }



    close(fd);
    return 0;
}
