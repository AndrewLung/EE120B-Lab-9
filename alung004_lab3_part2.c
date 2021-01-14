/*	Author: lab
 *  Partner(s) Name: Andrew Lung
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	unsigned char tmpA;
	unsigned char output;
    /* Insert your solution below */
    while (1) {
	tmpA = PINA;
	output = 0x00;
	if (tmpA > 0) {
		//PC5
		output = output | 0x20;
	}
	if (tmpA > 2) {
		//PC4
		output = output | 0x10;	
	}
	if (tmpA > 4) {
		//PC3
		output = output | 0x08;
	}
	if (tmpA > 6) {
		//PC2
		output = output | 0x04;
	}
	if (tmpA > 9) {
		//PC1	
		output = output | 0x02;
	}
	if (tmpA > 12) {
		//PC0
		output = output | 0x01;
	}
	if (tmpA <= 4) {
		//PC6	
		output = output | 0x40;
	}
	PORTC = output;
    }
    return 1;
}
