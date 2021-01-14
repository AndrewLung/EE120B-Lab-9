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
	DDRD = 0x00; PORTD = 0xFF;
	DDRB = 0xF7; PORTB = 0x01;
	unsigned short concatInput;
	unsigned char output;
	unsigned char tmpB;
    /* Insert your solution below */
    while (1) {
	output = 0;
	tmpB = PINB;
	concatInput = tmpB | (PIND << 1);
	if (concatInput >= 70) {
		output = output | 0x02;	
	}
	if ((concatInput > 5) && (concatInput < 70)) {
		output = output | 0x04;	
	}
	PORTB = output;
    }
    return 1;
}
