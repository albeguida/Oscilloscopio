#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#define BAUD 19200
#define MYUBRR (F_CPU/16/BAUD-1)
#define BUFFER_SIZE 1024
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
void ADC_init(void) {
    // V_ref=AVcc
    ADMUX |= (1<<REFS0) | (1<<ADLAR);
    //prescaler 128, enable ADC 
    ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);    
}

uint8_t ADC_read(uint8_t ch) {
    // channel must be between 0 and 7
    ch &= 0b00000111;  // AND with 7
    // set ADLAR = 1 to use 8 bits
    ADMUX = (ADMUX & 0xF0)|ch;
    ADCSRA |= (1<<ADSC);
    // wait for conversion
    while(ADCSRA & (1<<ADSC));
    // clear ADIF, write 1
    ADCSRA|=(1<<ADIF);
    // since 8 bit precision is used just return ADCH
    return ADCH;
}

void UART_init(void){
  // Set baud rate
  UBRR0H = (uint8_t)(MYUBRR>>8);
  UBRR0L = (uint8_t)MYUBRR;

  UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); /* 8-bit data */ 
  UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);   /* Enable RX and TX */  

}

void UART_putChar(uint8_t c){
  // wait for transmission completed, looping on status bit
  while ( !(UCSR0A & (1<<UDRE0)) );

  // Start transmission
  UDR0 = c;
}

uint8_t UART_getChar(void){
  // Wait for incoming data, looping on status bit

  while ( !(UCSR0A & (1<<RXC0)) );

  // Return the data
  return UDR0;
    
}

// reads a string until the first newline or 0
// returns the size read
uint8_t UART_getString(uint8_t* buf){
  uint8_t* b0=buf; //beginning of buffer
  while(1){
    uint8_t c=UART_getChar();
    *buf=c;
    ++buf;
    // reading a 0 terminates the string
    if (c==0)
      return buf-b0;
    // reading a \n  or a \r return results
    // in forcedly terminating the string
    if(c=='\n'||c=='\r'){
      *buf=0;
      ++buf;
      return buf-b0;
    }
  }
}

void UART_putString(uint8_t* buf){
  while(*buf){
    UART_putChar(*buf);
    ++buf;
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
  char str[5]; 
  if(mode[0] == '1'){ // continuous sampling
    for(int i = 0; i < 8; i++) {
      if(bitmask & (1 << i)) {
        uint8_t value = ADC_read(i);
        itoa(value, str, 10); // convert value to string, base 10
        UART_putString((uint8_t*)str);
      }
    }
  }else if(bytes_written + n_channels < BUFFER_SIZE){ // buffered sampling
    // we read all the channels
    for(int i = 0; i < 8; i++) {
      if(bitmask & (1 << i)) {
        uint8_t value = ADC_read(i);
        buffer[bytes_written] = value;
        bytes_written++;
      
      }
    }
  }else { // time to send the buffer
    UART_putString(buffer);
    bytes_written = 0;
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
