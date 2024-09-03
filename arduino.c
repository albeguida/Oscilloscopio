#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <uart.h>
#include <utils.h>
#include <adc.h>  
#define BAUD 19200
#define MYUBRR (F_CPU/16/BAUD-1)
#define BUFFER_SIZE 128
#define PIN_MASK 0x0F // last four bit of PORT B (50-53)

uint8_t mode[20];
uint8_t frequency[20];
uint8_t channels[20];
uint8_t buffer[BUFFER_SIZE];

volatile uint8_t previous_pins;
volatile uint8_t current_pins;
volatile uint8_t int_occurred=0;
volatile uint16_t int_count=0;
// interrupt routine for position PCINT0
// DEBUG from SO_repository

ISR(PCINT0_vect) {
  previous_pins=current_pins;
  current_pins=PINB&PIN_MASK;
  int_occurred=1;
  int_count=1;
}


void trigger_wait(void){
  DDRB &= ~PIN_MASK; //set PIN_MASK pins as input
  PORTB |= PIN_MASK; //enable pull up resistors
  PCICR |= (1 << PCIE0); // set interrupt on change, looking up PCMSK0
  PCMSK0 |= PIN_MASK;   // set PCINT0 to trigger an interrupt on state change 
  //int_occurred=0;
  sei();
  while(1){

    if (int_occurred) {
      int_occurred=0;
      break;
    }
    sleep_cpu();
  }
  return;
}

// our interrupt routine installed in 
// interrupt vector position
// corresponding to output compare
// of timer 5

// channel bitmask, it is set in the main
uint8_t bitmask;
uint32_t bytes_written = 0;
uint32_t n_channels;
// ISR for timer 5
ISR(TIMER5_COMPA_vect) {
  if(mode[0] == '1'){ // continuous sampling
    for(int i = 0; i < 8; i++) {
      if(bitmask & (1 << i)) {
        uint8_t value = ADC_read(i);
        UART_putChar(value); 
      } 
    }
  }else if(bytes_written + n_channels < BUFFER_SIZE / 4){ // buffered sampling
    // we read all the channels
    for(int i = 0; i < 8; i++) {
      if(bitmask & (1 << i)) {
        uint8_t value = ADC_read(i);
        buffer[bytes_written] = value;
        bytes_written++;
      
      }
    }
  }else { // time to send the buffer
    UART_putBuffer(buffer, bytes_written);
    bytes_written = 0;
    for(int i = 0; i < 8; i++) {
      if(bitmask & (1 << i)) {
        uint8_t value = ADC_read(i);
        buffer[bytes_written+i] = value;
        bytes_written++;
      
      }
    }

  }
}
// timer 5 init
void timer_init(const int timer_duration_ms){
  TCCR5A = 0;
  TCCR5B = (1 << WGM52) | (1 << CS50) | (1 << CS52); 
  uint16_t ocrval=(uint16_t)(15.62*timer_duration_ms);

  OCR5A = ocrval;
  cli(); 
  TIMSK5 |= (1 << OCIE5A);  
  sei(); 
} 

int main(void) {
    // init UART

    UART_init();
    // get configuration parameters and ACKs
    while(UART_getString(mode) < 1);
    UART_putString(mode); 

    while(UART_getString(frequency) < 1);
    UART_putString(frequency); 

    while(UART_getString(channels) < 1);
    UART_putString(channels);
    // aspetto che l'utente disturba uno dei pin 50-53 --> funge da trigger
    if(mode[0] == '0') trigger_wait();
    // channels bitmask
    bitmask = atoi((const char *)channels);
    n_channels = count_channels(bitmask);
    // init ADC and timer
    set_to_zero(buffer);
    ADC_init();
    timer_init((1/((uint32_t)atoi((char*)frequency)))*1000);
    while(1) {
      sleep_cpu(); // sleep until interrupt
    }

    return 0;
}
