#include "applicationLayer.h"


// int main(int argc, char** argv)
//   {
//       int fd;
//       status mode;
//
//   	 if ( (argc < 3) ||
//     	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
//     	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
//         printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
//         exit(1);
//   	}
//
//   	 if(strcmp(argv[2],"T")==0){
//   		printf("MODE: TRANSMITER\n");
//   		mode=0;
//   }
//   	else if(strcmp(argv[2],"R")==0){
//   	printf("MODE: RECEIVER\n");
//   		mode=1;
//   }
//   	else
//   	printf("Usage: T or R to transitter/receiver mode\n");
//
//     initDataLinkStruct(TRANSMISSIONS, TIMEOUT, BAUDRATE);
//
//   	fd = llopen(0,mode);
//     llclose(fd);
//
//
//   	return 0;
//   }
