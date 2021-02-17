/* Author: Andrew Lung
 * Partner(s) Name (if applicable):
 * Lab Section: 22
 * Assignment: Lab #10  Exercise #2
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 *
 *  Demo Link: https://www.youtube.com/watch?v=7KtbXXI5UFQ&feature=youtu.be
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

typedef struct _task {
	signed char state;
	unsigned long int period;
	unsigned long int elapsedTime;
	int (*TickFct)(int);
} task;

unsigned char led0_output = 0x00;
unsigned char combination[] = { '#', '1', '2', '3', '4', '5' };
unsigned char chIndex = 0;

enum display_States { display_display };

int displaySMTick(int state) {
	unsigned char output;
	switch (state) {
		case display_display: state = display_display; break;
		default: state = display_display; break;
	}
	switch (state) {
		case display_display:
			output = led0_output;
			break;
	}
	PORTB = output;
	return state;
}

unsigned char GetKeypadKey();

enum combinationLock { KPunpressed, KPpressed };

int combinationLockSM(int state) {
	unsigned char x;
	x = GetKeypadKey();
	switch (state) {
		case KPunpressed:
			if (x == '\0') {
				state = KPunpressed;
			}
			else if (chIndex < 6 && x == combination[chIndex]) {
				state = KPpressed;
				++chIndex;
			}
			else if (chIndex < 6 && x != combination[chIndex]) {
				state = KPpressed;
				chIndex = 0;
			}
			else if (chIndex >= 6) {
				state = KPunpressed;
			}
			break;
		case KPpressed:
			if (x != '\0') {
				state = KPpressed;
			}
			else {
				state = KPunpressed;
			}
			break;
	}
	return state;
}

enum lockMechanism { locked, unlocked };

int lockMechanismSM(int state) {
	unsigned char tmpB = ~PINB;
	switch (state) {
		case locked:
			if (chIndex >= 6) {
				led0_output = 0x01;
				state = unlocked;
			}
			break;
		case unlocked:
			if (tmpB & 0x80) {
				led0_output = 0;
				chIndex = 0;
				state = locked;
			}
			break;
		default:
			state = locked;
			break;
	}
	return state;
}

unsigned char GetBit(unsigned char port, unsigned char number) 
{
	return ( port & (0x01 << number) );
}

unsigned char GetKeypadKey() {
	PORTC = 0xEF;
	asm("nop");
	if (GetBit(PINC, 0)==0) { return('1'); }
	if (GetBit(PINC, 1)==0) { return('4'); }
	if (GetBit(PINC, 2)==0) { return('7'); }
	if (GetBit(PINC, 3)==0) { return('*'); }

	PORTC = 0xDF;
	asm("nop");
	if (GetBit(PINC, 0)==0) { return('2'); }
	if (GetBit(PINC, 1)==0) { return('5'); }
	if (GetBit(PINC, 2)==0) { return('8'); }
	if (GetBit(PINC, 3)==0) { return('0'); }

	PORTC = 0xBF;
	asm("nop");
	if (GetBit(PINC, 0)==0) { return('3'); }
	if (GetBit(PINC, 1)==0) { return('6'); }
	if (GetBit(PINC, 2)==0) { return('9'); }
	if (GetBit(PINC, 3)==0) { return('#'); }

	PORTC = 0x7F;
	asm("nop");
	if (GetBit(PINC, 0)==0) { return('A'); }
	if (GetBit(PINC, 1)==0) { return('B'); }
	if (GetBit(PINC, 2)==0) { return('C'); }
	if (GetBit(PINC, 3)==0) { return('D'); }

	return('\0');
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
	DDRA = 0x00; PORTA = 0xFF; //output
	DDRB = 0x7F; PORTB = 0x80; //input
	DDRC = 0xF0; PORTC = 0x0F;
	
	static task task1, task2, task3;
	task *tasks[] = { &task1, &task2, &task3 };	
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	const char start = -1;

	task1.state = display_display;
	task1.period = 10;
	task1.elapsedTime = task1.period;
	task1.TickFct = &displaySMTick;

	task2.state = KPunpressed;
	task2.period = 100;
	task2.elapsedTime = task2.period;
	task2.TickFct = &combinationLockSM;

	task3.state = locked;
	task3.period = 200;
	task3.elapsedTime = task3.period;
	task3.TickFct = &lockMechanismSM;

	TimerSet(10);
	TimerOn();

	unsigned short i;
	while (1) {
		for (i = 0; i < numTasks;i++) {
			if (tasks[i]->elapsedTime == tasks[i]->period) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 10;
		}
		while (!TimerFlag);
		TimerFlag = 0;
	}

   return 0;
}
