#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "gnuplot.h"

void gnuplot_start(uint8_t bitmask){
// preparo il comando da lanciare sulla shell, 
    // passo i canali selezionati come parametri al file data.p di gnuplot
    // così da lanciare i grafici corrispondenti
    char  gnuplot_params[256] = "gnuplot -c data.p data.txt";
    char n_col[20];
    int n_channels = count_channels(bitmask);
    sprintf(n_col, " %d", n_channels);
    // appendo il numero di canali/colonne al comando
    strcat(gnuplot_params, n_col); 
    for (int i = 0; i < 8; i++) {
      if (bitmask & (1 << i)) {
        char channel_param[5]; 
        sprintf(channel_param, " %d", i);
        //appendo i canali selezionati al comando
        strcat(gnuplot_params, channel_param); 
      }
    }
    // lancio gnuplot tramite fork, così da non bloccare il main
    pid_t pid = fork();
    if(pid == 0){
      printf("avvio gnuplot\n");
      system(gnuplot_params);
      exit(0);
    }else if(pid > 0){
      printf("parent process\n");
    }else if(pid < 0){
      printf("fork failed\n");
    }
}
int count_channels(uint8_t bitmask) {
  int n_channels = 0;
  for (int i = 0; i < 8; i++) {
    if (bitmask & (1 << i)) {
      n_channels++;
    }
  }
  return n_channels;
}