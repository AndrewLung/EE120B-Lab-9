/* Author: Andrew Lung
 * Partner(s) Name (if applicable):
 * Lab Section: 22
 * Assignment: Lab #8  Exercise #2
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 *
 *  Demo Link: https://www.youtube.com/watch?v=jLhM2I43hjE&feature=youtu.be
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

double frequency_vals[] = { 261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25 };
unsigned char i = 0;
unsigned char on = 0;

enum SM_States { release, hold } State;

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;
unsigned char cycles;

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

void set_PWM(double frequency) {
	static double current_frequency;
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; }
		else { TCCR3B |= 0x03; }
		if (frequency < 0.954) { OCR3A = 0xFFFF; }
		else if (frequency > 31250) { OCR3A = 0x0000; }
		else { OCR3A = (short)(8000000 / (128*frequency)) - 1; }
		TCNT3 = 0;
		current_frequency = frequency;	
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

void Tick_SM() {
	char tmpA = ~PINA;
	char A0pressed = tmpA & 0x01;
	char A1pressed = tmpA & 0x02;
	char A2pressed = tmpA & 0x04;

	switch (State) {
		case release:
			if (A0pressed && !(A1pressed || A2pressed)) {
				on = !on;
				if (on) {
					set_PWM(frequency_vals[i]);
				}
				else {
					set_PWM(0);
				}
				State = hold;
			}
			else if (A1pressed && on && !(A0pressed || A2pressed)) {
				if (i < 7 ) {
					++i;
				}
				set_PWM(frequency_vals[i]);
				State = hold;
			}
			else if (A2pressed && on && !(A0pressed || A1pressed)) {
				if (i > 0) {
					--i;
				}
				set_PWM(frequency_vals[i]);
				State = hold;
			}
			else {
				State = release;
			}
			break;
		case hold:
			if (A0pressed || A1pressed || A2pressed) {
				State = hold;
			}
			else {
				State = release;
			}
			break;
		default:
			i = 0;
			set_PWM(frequency_vals[i]);
			State = release;
			break;
	}
}

int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;              // Initialize outputs
	PWM_on();
	TimerSet(100);
	TimerOn();
	i = 0;
	State = release;
	while (1) {
		Tick_SM();
		while(!TimerFlag);
		TimerFlag = 0;
	}
   return 0;
}