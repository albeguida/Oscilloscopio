#include <avr/io.h>
#include <stdint.h>
#include "adc.h"
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
