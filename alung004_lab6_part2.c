/* Author: Andrew Lung
 * Partner(s) Name (if applicable):
 * Lab Section: 22
 * Assignment: Lab #6  Exercise #2
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 *
 *  Demo Link: https://www.youtube.com/watch?v=nGrlxsEjJn8&feature=youtu.be
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

unsigned char forward;

enum SM_States { light0, light1, light2, press, release } state;

void Tick_SM() {
	char buttonPress = ~(PINA) & 0x01;
	switch (state) {
		case light0:
			PORTB = 0x01;
			forward = 1;
			if (buttonPress) {
				state = press;
				break;
			}
			state = light1;
			break;
		case light1:
			PORTB = 0x02;
			if (buttonPress) {
				state = press;
				break;
			}
			if (forward) {
				state = light2;
			}
			else {
				state = light0;
			}
			break;
		case light2:
			PORTB = 0x04;
			forward = 0;
			if (buttonPress) {
				state = press;
				break;
			}
			state = light1;
			break;
		case press:
			if (buttonPress) {
				state = press;
			}
			else {
				state = release;
			}
			break;
		case release:
			if (buttonPress) {
				state = light0;
			}
			else {
				state = release;
			}
			break;
		default:
			PORTB = 0x01;
			forward = 1;
			if (buttonPress) {
				state = press;
				break;
			}
			state = light0;
			break;
	}
	
}

void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B = 0x00;
}

void TimerISR() {
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;	
	}
}

void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

int main() {
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	PORTB = 0x00;
	TimerSet(300);
	forward = 1;
	TimerOn();
	state = light0;
	while (1) {
		Tick_SM();
		while (!TimerFlag);
		TimerFlag = 0;	
	}
	return 1;
}
