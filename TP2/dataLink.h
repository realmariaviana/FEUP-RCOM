#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include "utilities.h"

typedef struct {
  char port[20]; //dispositivo /dev/ttySx, x = 0, 1
  int baudRate; //velocidade de transmissao
  unsigned int sequenceNumber; //numero da sequencia de trama: 0 ou 1
  unsigned int timeout; //valor do temporizador: 1s
  unsigned int transmissions; //numero de tentativas em caso de falha
  status mode; //receiver or transmitter
  struct termios oldTermios;
  int wrongPackets;
}linkLayer;

linkLayer link_layer;

void initDataLinkStruct(int transmissions, int timeOut, int baudRate);
void alarmHandler(int sig);
int stateMachine(unsigned char c, int state, char * msg);
int setTermios(int fd);
int llopen(int port, status mode);
int llopenTransmitter(int fd);
int llopenReceiver(int fd);
int llwrite(int fd, char * packet, int length, int * rejCounter);
int llread(int fd, unsigned char *packet);
unsigned char *createIFrame(int *frameLength, char *packet, int packetLength);
int writePacket(int fd, unsigned char* buffer, int bufLength);
int readPacket(int fd, unsigned char* frame, int *frameLength);
bool frameSCorrect(unsigned char *response, int responseLength, unsigned char C);
bool frameICorrect(unsigned char * frame);
unsigned char getBCC2(const unsigned char* buf, unsigned int size);
bool valid_sequence_number(char ctrlByte);
unsigned char *createSFrame(char ctrlByte);
unsigned char *stuff(char *packet, int *packetLength);
unsigned char *destuff(char *packet, int *packetLength);
int llclose(int fd);
int llcloseTransmitter(int fd);
int llcloseReceiver(int fd);
