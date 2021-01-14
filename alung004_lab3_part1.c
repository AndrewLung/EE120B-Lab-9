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
	DDRB = 0x00; PORTB = 0xFF;
	DDRC = 0xFF; PORTB = 0x00;
	unsigned char cnt;
	unsigned char tmpA;
	unsigned char tmpB;
	unsigned char bitMask;
    /* Insert your solution below */
    while (1) {
	cnt = 0;
	tmpA = PINA;
	tmpB = PINB;
	bitMask = 0x01;
	for (int i = 0; i < 8; ++i) {
		if (tmpA & bitMask) {
			++cnt;
		}
		bitMask = bitMask << 1;
	}
	bitMask = 0x01;
	for (int i = 0; i < 8; ++i) {
		if (tmpB & bitMask) {
			++cnt;
		}
		bitMask = bitMask << 1;
	}
	PORTC = cnt;
    }
    return 1;
}
