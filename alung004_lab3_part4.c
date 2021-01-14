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
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	unsigned char tmpA;
	unsigned char upperNibble;
	unsigned char lowerNibble;
    /* Insert your solution below */
    while (1) {
	tmpA = PINA;
	lowerNibble = (tmpA >> 4) & 0x0F;
	upperNibble = (tmpA << 4) & 0xF0;
	PORTB = lowerNibble;
	PORTC = upperNibble;
    }
    return 1;
}
