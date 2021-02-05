/* Author: Andrew Lung
 * Partner(s) Name (if applicable):
 * Lab Section: 22
 * Assignment: Lab #7  Exercise #1
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 *
 *  Demo Link: https://www.youtube.com/watch?v=TjG_kjzWkoI&feature=youtu.be
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;              // Initialize outputs
	DDRD = 0xFF; PORTD = 0x00;
	ADC_init();
   while(1) {
	unsigned short my_short = ADC;
	unsigned char lower_bits = (char)my_short;
	unsigned char upper_bits = (char)(my_short >> 8);
	PORTB = lower_bits;
	PORTD = upper_bits;
   }
   return 0;
}
