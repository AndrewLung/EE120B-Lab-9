/* Author: Andrew Lung
 * Partner(s) Name (if applicable):
 * Lab Section: 22
 * Assignment: Lab #5  Exercise #3
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 *
 *  Demo Link: https://www.youtube.com/watch?v=2kM6yd7D5T8
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

enum SM_States { split1, hold1, split2, hold2 } state;

void Tick_SM() {
	char buttonPress = (PINA) & 0x01;
	
	switch ( state ) { //Transitions
		case split1:
			PORTB = 0xAA;
			if (buttonPress) {
				state = hold1;
			}
			else {
				state = split1;
			}
			break;
		case hold1:
			PORTB = 0x55;
			if (buttonPress) {
				state = hold1;
			}
			else {
				state = split2;
			}
			break;
		case split2:
			if (buttonPress) {
				state = hold2;
			}
			else {
				state = split2;
			}
			break;
		case hold2:
			PORTB = 0xAA;
			if (buttonPress) {
				state = hold2;
			}
			else {
				state = split1;
			}
			break;
		default:
			state = split1;
			PORTB = 0xAA;
		break;
	}
}

int main() {
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	PORTB = 0xAA; 
	state = split1; 
	while (1) {
		Tick_SM();
	}
	return 1;
}
