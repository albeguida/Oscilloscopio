#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

void led_blink(){
  const uint8_t mask=(1<<7);
  // we configure the pin as output
  DDRB |= mask;
  int k=0;
  while(1){
    printf("led %d\n", (k&1));
    if (k&1)
      PORTB=mask;
    else
      PORTB=0;
    _delay_ms(1000); // from delay.h, wait 1 sec
    ++k;
  }
}
void set_to_zero(uint8_t* buf){
  for(int i=0; i<20; i++){
    buf[i]=0;
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
