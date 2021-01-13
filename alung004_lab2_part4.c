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
	DDRB = 0x00; PORTB = 0xFF;
	DDRC = 0x00; PORTC = 0xFF;
	DDRD = 0xFF; PORTD = 0x00; 
	unsigned char tmpA;
	unsigned char tmpB;
	unsigned char tmpC;
	unsigned char tmpD = 0x00;
	unsigned short totWeight = 0x00; 
	while(1) {
		tmpA = PINA;
		tmpB = PINB;
		tmpC = PINC;
		tmpD = 0;
		totWeight = tmpA + tmpB + tmpC;
		if (totWeight > 140) {
			tmpD = tmpD | 0x01;
		}
		//avoid underflow
		if (tmpC > tmpA) {
			if ((tmpC - tmpA) > 80) {
				tmpD = tmpD | 0x02;
			}
		}
		else if (tmpA > tmpC) {
			if ((tmpA - tmpC) > 80) {
				tmpD = tmpD | 0x02;
			}		
		}
		totWeight = totWeight >> 2;
		tmpD = tmpD | totWeight;
		PORTD = tmpD;
	}
	return 0;
}

