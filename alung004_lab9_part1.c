/* Author: Andrew Lung
 * Partner(s) Name (if applicable):
 * Lab Section: 22
 * Assignment: Lab #9  Exercise #1
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 *
 *  Demo Link: https://www.youtube.com/watch?v=_OQLet_KS0U
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

typedef struct task {
	int state;
	unsigned long period;
	unsigned long elapsedTime;
	int (*TickFct)(int);
} task;

task tasks[2];

const unsigned char tasksNum = 2;
const unsigned long tasksPeriodGCD = 1000;
const unsigned long periodBlinkLED = 1000;
const unsigned long periodThreeLEDs = 1000;

int TickBlinkLED(int state);
enum BlinkLED { LEDon, LEDoff };

int TickThreeLEDs(int state);
enum ThreeLEDs { light0, light1, light2 };

int TickAll();

unsigned char threeLEDs;
unsigned char blinkingLED;

int TickBlinkLED(int state) {
	switch (state) {
		case LEDon:
			state = LEDoff;
			blinkingLED = 0x08;
			break;
		case LEDoff:
			state = LEDon;
			blinkingLED = 0x00;
			break;
	}
	return state;
}

int TickThreeLEDs(int state) {
	switch (state) {
		case light0:
			state = light1;
			threeLEDs = 0x01;
			break;
		case light1:
			state = light2;
			threeLEDs = 0x02;
			break;
		case light2:
			state = light0;
			threeLEDs = 0x04;
			break;
	}
	return state;
}

void CombineLEDs() {
	unsigned char i;
	for (i = 0; i < tasksNum; ++i) {
		if (tasks[i].elapsedTime >= tasks[i].period) {
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += 1;
	}
	PORTB = 0 | threeLEDs | blinkingLED;
}

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



int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;              // Initialize outputs
	threeLEDs = 0;
	blinkingLED = 0;

	unsigned char i = 0;
	tasks[i].state = light0;
	tasks[i].period = periodThreeLEDs;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickThreeLEDs;
	++i;
	tasks[i].state = LEDon;
	tasks[i].period = periodBlinkLED;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickBlinkLED;
	
	TimerOn();
	TimerSet(1);
	
	while(1) {
		CombineLEDs();
		while(!TimerFlag);
		TimerFlag = 0;
	}

   return 0;
}
