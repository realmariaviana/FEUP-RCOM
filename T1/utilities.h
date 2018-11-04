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
#define HEADER_SIZE 6

#define FILE_SIZE 0
#define FILE_NAME 1

#define DATA 1
#define START 2
#define END 3

#define FLAG 0x7E

#define A_SEND 0x03
#define A_ALT 0x01

#define C_SET 0x03
#define C_UA 0x07
#define C_DISC 0x0B

#define SET_BCC 0x00

#define RR 0x05
#define REJ 0x01
#define RR_ALT 0x85
#define REJ_ALT 0x81

#define WAIT 4000

#define PACKET_SIZE 256
#define PACKET_HEADER_SIZE 4
#define DATA_PACKET_SIZE PACKET_SIZE - PACKET_HEADER_SIZE

#define SFRAMELEN 5

typedef enum {TRANSMITTER, RECEIVER} status;
