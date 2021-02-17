/* Author: Andrew Lung
 * Partner(s) Name (if applicable):
 * Lab Section: 22
 * Assignment: Lab #10  Exercise #4
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 *
 *  Demo Link: https://www.youtube.com/watch?v=CmY1-822x3M&feature=youtu.be
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
unsigned char ignoreFlag = 0;

double frequency_vals[] = { 523.25, 523.25, 493.88, 523.25, 523.25, 523.25, 493.88, 523.25, 523.25, 523.25, 493.88, 523.25, 392.00, 392.00, 392.00, 392.00, 415.30, 392.00, 440, 493.88, 523.25};
double mult[] = { .125, .125, 1.0, 1.125, .125, .125, 1.0, 1.125, .125, .125, 1, 1.125, .125, .125, 1, .125, 1, .125, 1, 1, 1, 1.125};

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
if (!ignoreFlag) {
	if (GetBit(PINC, 3)==0) { return('*'); }
}

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

// SPEAKER

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

enum speaker { release, hold };

int speakerSM(int state) {
	unsigned char tmpA = ~PINA;
	unsigned char A0pressed = tmpA & 0x80;
	switch (state) {
		case release:
			if (A0pressed) {
				for (unsigned char i = 0; i < 21; ++i) {
					set_PWM(frequency_vals[i]);
					TimerSet(500.0 * mult[i]);
					while(!TimerFlag);
					TimerFlag = 0;

					set_PWM(0);
					TimerSet(100);
					while(!TimerFlag);
					TimerFlag = 0;
				}
				TimerSet(10);
				TimerFlag = 0;
				state = hold;
			}
			if (!A0pressed) {
				state = release;
			}
			break;
		case hold:
			if (A0pressed) {
				state = hold;
			}
			else {
				state = release;
			}
			break;
	}
	return state;
}

enum changeCombo { noChange, change };

int changeComboSM(int state) {
	unsigned char tmpB = ~PINB;
	unsigned char x = GetKeypadKey();
	switch (state) {
		case noChange:
			if (x == '*' && (tmpB & 0x80)) {
				state = change;
			}
			else {
				state = noChange;
			}
			break;
		case change:
			;
			unsigned char replacement[] = {'\0', '\0', '\0', '\0', '\0'};
			unsigned char replaceIndex = 0;
			PORTC = 0xEF;
			asm("nop");
			while (((GetBit(PINC, 3)==0) && (tmpB & 0x80)) && (replaceIndex < 5)) {
				ignoreFlag = 1;
				unsigned char y = GetKeypadKey();
				if (y != '\0') {
					replacement[replaceIndex] = y;
				}
				++replaceIndex;
				PORTC = 0xEF;
				ignoreFlag = 0;
			}
			if (replaceIndex != 5) {
				ignoreFlag = 0;
				state = noChange;
				break;
			}
			TimerSet(2000);
			replaceIndex = 0;
			while (!TimerFlag) {
				unsigned char y = GetKeypadKey();
				if (y != '\0') {
					if (y == replacement[replaceIndex]) {
						++replaceIndex;
					}
				}
				if (replaceIndex == 5) {
					for (unsigned i = 0; i < 5; ++i) {
						combination[1 + i] = replacement[i];
					}
				}
			}
			TimerSet(10);
			TimerFlag = 0;
			ignoreFlag = 0;
			state = noChange;
			break;
		
	}
	return state;
}



// END SPEAKER

int main(void) {
	DDRA = 0x00; PORTA = 0xFF; //output
	DDRB = 0x7F; PORTB = 0x80; //input
	DDRC = 0xF0; PORTC = 0x0F;
	
	PWM_on();
	TimerOn();

	static task task1, task2, task3, task4, task5;
	task *tasks[] = { &task1, &task2, &task3, &task4, &task5 };	
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	//const char start = -1;

	task1.state = display_display;
	task1.period = 10;
	task1.elapsedTime = task1.period;
	task1.TickFct = &displaySMTick;

	task2.state = KPunpressed;
	task2.period = 100;
	task2.elapsedTime = task2.period;
	task2.TickFct = &combinationLockSM;

	task3.state = locked;
	task3.period = 150;
	task3.elapsedTime = task3.period;
	task3.TickFct = &lockMechanismSM;

	task4.state = release;
	task4.period = 200;
	task4.elapsedTime = task4.period;
	task4.TickFct = &speakerSM;

	task5.state = noChange;
	task5.period = 200;
	task5.elapsedTime = task5.period;
	task5.TickFct = &changeComboSM;

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
