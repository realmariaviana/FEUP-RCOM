#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define BAUDRATE B38400
#define TIMEOUT 3
#define TRANSMISSIONS 3

#define COM1 0
#define COM2 1
#define COM1_PORT "/dev/ttyS0"
#define COM2_PORT "/dev/ttyS1"

#define FALSE 0
#define TRUE 1

#define ESC 0x7D
#define BYTE_STUFF 0x20

#define FLAG 0x7E

#define A_SEND 0x03

#define C_SET 0x03
#define C_UA 0x07
#define C_DISC 0x0B

#define SET_BCC 0x00

#define RR 0x05
#define REJ 0x01

#define WAIT 4000

#define SFRAMELEN 5

typedef enum {TRANSMITTER, RECEIVER} status;
