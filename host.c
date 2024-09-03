#include "serial_linux.h"
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
#include "gnuplot.h"
//#include <iostream>
#include <stdlib.h>

#define BUFFER_SIZE 1024

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

  // bitmask dei canali
  uint8_t bitmask = atoi(channels);

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

  // Conto quanti canali sono stati selezionati
  int n_channels = count_channels(bitmask);
  // creo data.txt per salvare le letture dal canale / ne elimino, se già esistente, il contenuto
  fclose(fopen("data.txt", "w"));
  printf("data.txt azzerato\n");
  // apro data.txt in append 
  FILE* data_file = fopen("data.txt", "a"); 
  printf("data.txt aperto\n");
  // scrivo in data.txt i dati ricevuti dalla seriale
  char buffer[BUFFER_SIZE]; 
  int sample_counter = 0;
  if(mode == "1"){
    while (1) {
      // scrivo nella prima colonna il numero di sample (formato gnuplot) --> work in progress per la correlazione con il tempo
      fprintf(data_file, "%d ", sample_counter);
      printf("sample_counter: %d\n", sample_counter);
      
      for (int i = 0; i < n_channels; i++) {
        //leggo 1 byte alla volta che corrisponde al valore del canale
          ssize_t bytes_read = read(fd, buffer, 1); 
          printf("Buffer : %s\n",buffer); //DEBUG
          if (bytes_read > 0) {
              // scrivo il valore del canale nel file
              if(i == n_channels - 1){
                fprintf(data_file, "%d", buffer[0]); 
              }else{
                fprintf(data_file, "%d ", buffer[0]); 
              }
          }
      }
      sample_counter++;
      fprintf(data_file, "\n"); 
      fflush(data_file); 
      // lancio gnuplot per visualizzare i dati
      if(sample_counter == 1){ // altrimenti errore in quanto data.txt è ancora vuoto
        gnuplot_start(bitmask);
      }
    }
  }else{
    //buffered mode
    while(1){
      // leggo i dati dalla seriale a blocchi
      ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE); 
      //printf("Buffer : %s\n",buffer); //DEBUG
      if (bytes_read > 0) {
        // scrivo i dati nel file
        for (int i = 0; i < bytes_read; i+=n_channels) {
          fprintf(data_file, "%d ", sample_counter);
          for(int j = 0; j < n_channels; j++){
            // scrivo il valore del canale nel file
            sample_counter++;
            sprintf(buffer, "%d", buffer[i+j]);
            if(i == bytes_read - 1){
              fprintf(data_file, "%d", buffer[i+j]); 
            }else{
              fprintf(data_file, "%d ", buffer[i+j]); 
            }
            if(sample_counter == 1){ // altrimenti errore in quanto data.txt è ancora vuoto
              gnuplot_start(bitmask);
            }
          }
          fprintf(data_file, "\n"); 
          fflush(data_file);
        }
         
      }
    }
  }
  close(fd);
  fclose(data_file);
  
  return 0;


}

  

  

