#include <util/delay.h>
#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>

#define BAUD 19600
#define MYUBRR (F_CPU/16/BAUD-1)


void ADC_init(void) {
    // V_ref=AVcc
    ADMUX |= (1<<REFS0);
    //prescaler 128, enable ADC 
    ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);    
}

uint8_t ADC_read(uint8_t ch) {
    // channel must be between 0 and 7
    ch &= 0b00000111;  // AND with 7
    // set ADLAR = 1 to use 8 bits
    ADMUX = (ADMUX & 0xF0)|ch| (1<<ADLAR);
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


