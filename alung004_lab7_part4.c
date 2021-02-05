/* Author: Andrew Lung
 * Partner(s) Name (if applicable):
 * Lab Section: 22
 * Assignment: Lab #7  Exercise #4
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 *
 *  Demo Link: https://www.youtube.com/watch?v=mgcFCHo2d0M&feature=youtu.be
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

// min: 35
// max: 127
unsigned short max = 127;
unsigned char min = 35;

void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;              // Initialize outputs
	//DDRD = 0xFF; PORTD = 0x00;
	ADC_init();
	unsigned short interval = (max - min) / 8;
   while(1) {
	unsigned short my_short = ADC;
	unsigned char tmpB;
	if (my_short >= min) {	
		tmpB = 1;
	}
	if (my_short >= min + interval){
		tmpB = 0x03;	
	}
	if (my_short >= min + (2*interval)) {
		tmpB = 0x07;	
	}
	if (my_short >= min + (3*interval)) {
		tmpB = 0x0F;	
	}
	if (my_short >= min + (4*interval)) {
		tmpB = 0x1F;	
	}
	if (my_short >= min + (5*interval)) {
		tmpB = 0x3F;	
	}
	if (my_short >= min + (6*interval)) {
		tmpB = 0x7F;	
	}
	if (my_short >= min + (7*interval)) {
		tmpB = 0xFF;	
	}
	PORTB = tmpB;
   }
   return 0;
}
