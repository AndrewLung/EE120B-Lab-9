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
	DDRA = 0x00; PORTA = 0xFF; 
	DDRC = 0xFF; PORTC = 0x00; 
	unsigned char tmpA = 0x00; 
	unsigned char cntavail = 0x00;
while(1) {
		cntavail = 0;
		tmpA = PINA;
		if (tmpA & 0x01) {
			cntavail++;
		}
		if (tmpA & 0x02) {
			cntavail++;
		}
		if (tmpA & 0x04) {
			cntavail++;
		}
		if (tmpA & 0x08) {
			cntavail++;
		}
		PORTC = ((tmpA & 0x70) |  (4 - cntavail));
///*
		if (cntavail == 4) {
			PORTC = (PORTC & 0x0F) | 0x80;
		}
//*/
	}
	return 0;
}

