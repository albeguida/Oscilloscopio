#include "host.h"
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
//#include <iostream>
#include <stdlib.h>
int serial_set_interface_attribs(int fd, int speed, int parity) {
  struct termios tty;
  memset (&tty, 0, sizeof tty);
  if (tcgetattr (fd, &tty) != 0) {
    printf ("error %d from tcgetattr", errno);
    return -1;
  }
  switch (speed){
  case 19200:
    speed=B19200;
    break;
  case 57600:
    speed=B57600;
    break;
  case 115200:
    speed=B115200;
    break;
  case 230400:
    speed=B230400;
    break;
  case 576000:
    speed=B576000;
    break;
  case 921600:
    speed=B921600;
    break;
  default:
    printf("cannot sed baudrate %d\n", speed);
    return -1;
  }
  cfsetospeed (&tty, speed);
  cfsetispeed (&tty, speed);
  cfmakeraw(&tty);
  // enable reading
  tty.c_cflag &= ~(PARENB | PARODD);               // shut off parity
  tty.c_cflag |= parity;
  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;      // 8-bit chars

  if (tcsetattr (fd, TCSANOW, &tty) != 0) {
    printf ("error %d from tcsetattr", errno);
    return -1;
  }
  tcflush(fd, TCIOFLUSH);
  return 0;
}

void serial_set_blocking(int fd, int should_block) {
  struct termios tty;
  memset (&tty, 0, sizeof tty);
  if (tcgetattr (fd, &tty) != 0) {
      printf ("error %d from tggetattr", errno);
      return;
  }

  tty.c_cc[VMIN]  = should_block ? 1 : 0;
  tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

  if (tcsetattr (fd, TCSANOW, &tty) != 0)
    printf ("error %d setting term attributes", errno);
}

int serial_open(const char* name) {
  int fd = open (name, O_RDWR | O_NOCTTY | O_SYNC );
  if (fd < 0) {
    printf ("error %d opening serial, fd %d\n", errno, fd);
  }
  return fd;
}

int main(int argc, const char** argv) {
  if (argc < 7) { // Adjusted for additional arguments
    printf("Usage: serial_linux <serial_file> <baudrate> <read=1, write=0> <mode> <frequency> <channels>\n");
    return 1;
  }


  const char* serial_device = argv[1];
  int baudrate = atoi(argv[2]);
  int read_or_write = atoi(argv[3]);

  const char * mode = argv[4]; 
  const char * frequency = argv[5]; 
  const char * channels = argv[6]; 

  int fd = serial_open(serial_device);
  serial_set_interface_attribs(fd, baudrate, 0);
  serial_set_blocking(fd, 1);

  /*Protocollo di configurazione
  1)Invio la modalità di campionamento: 1 --> continuos mode 0 --> buffered mode
  2)Invio la frequenza di campionamento
  3)Invio 8 bit corrisponenti ai canali da campionare
  */

  char ack[10]; 

  // wait for avr
  sleep(2); 

  printf("invio mode: %s\n", mode);
  while(write(fd, mode, strlen(mode)+1) < 1); 
  printf("mode inviata\n");
  memset(ack, 0, sizeof(ack)); // pulisco il buffer
  read(fd, ack, sizeof(ack)); 
  printf("ACK mode: %s\n", ack);

  printf("invio frequency: %s\n", frequency);
  while(write(fd, frequency, strlen(frequency)+1) < 1); 
  printf("frequenza inviata\n");
  memset(ack, 0, sizeof(ack)); 
  read(fd, ack, sizeof(ack)); 
  printf("ACK frequenza: %s\n", ack);

  printf("invio channels: %s\n", channels);
  while(write(fd, channels, strlen(channels)+1) < 1); 
  printf("channels inviati\n");
  memset(ack, 0, sizeof(ack)); 
  read(fd, ack, sizeof(ack)); 
  printf("ACK channels: %s\n", ack);

  return 0;
  
}


  

  

