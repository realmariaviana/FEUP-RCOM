#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include "utils.h"

typedef struct {
  char port[20]; //dispositivo /dev/ttySx, x = 0, 1
  int baudRate; //velocidade de transmissao
  unsigned int sequenceNumber; //numero da sequencia de trama: 0 ou 1
  unsigned int timeout; //valor do temporizador: 1s
  unsigned int transmissions; //numero de tentativas em caso de falha
  status mode; //receiver or transmitter
  struct termios portSettings;
  int wrongPackets;
}linkLayer;

linkLayer link_layer;

void alarmHandler(int sig);
int stateMachine(unsigned char c, int state, char * msg);
int setTermios(int fd);
int llopen(int port, status mode);
int llopenTransmitter(int fd);
int llopenReceiver(int fd);
int llclose(int fd);
int llcloseTransmitter(int fd);
int llcloseReceiver(int fd);
