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

#define FLAG 0x7E

#define A_SEND 0x03

#define C_SET 0x03
#define C_UA 0x07
#define C_DISC 0x0B

#define SET_BCC 0x00

typedef enum {TRANSMITTER, RECEIVER} status;
