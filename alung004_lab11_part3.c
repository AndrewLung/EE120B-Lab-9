/* Author: Andrew Lung
 * Partner(s) Name (if applicable):
 * Lab Section: 22
 * Assignment: Lab #11  Exercise #3
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 *
 *  Demo Link: https://www.youtube.com/watch?v=j85ztyIeOsc&feature=youtu.be
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
	int (*TickButton)(int, unsigned char*, unsigned char*, unsigned char, unsigned char);
	unsigned char go;
	unsigned char on;
} task;

task tasks[5];

const unsigned char tasksNum = 3;
const unsigned long tasksPeriodGCD = 50;
const unsigned long periodSequence1 = 100;
const unsigned long periodSequence2 = 150;
const unsigned long periodSequence3 = 200;
const unsigned long periodSelectSeq = 200;

int TickSequence1(int state);
enum Sequence1 { firstS1, secondS1, thirdS1, fourthS1 };

int TickSequence2(int state);
enum Sequence2 { firstS2, secondS2 };

int TickSequence3(int state);
enum Sequence3 { firstS3, secondS3 };

int TickSelectSeq(int state, unsigned char*, unsigned char*, unsigned char, unsigned char);
enum SelectSeq { release, pressA0, pressA1, reset };

unsigned char dataOut;
unsigned char dataOut2;
unsigned char dataS1;
unsigned char dataS2;
unsigned char dataS3;

int TickSequence1(int state) {
	switch (state) {
		case firstS1:
			dataS1 = 0x81;
			state = secondS1;
			break;
		case secondS1:
			dataS1 = 0x42;
			state = thirdS1;
			break;
		case thirdS1:
			dataS1 = 0x24;
			state = fourthS1;
			break;
		case fourthS1:
			dataS1 = 0x18;
			state = firstS1;
			break;
	}
	return state;
}

int TickSequence2(int state) {
	switch (state) {
		case firstS2:
			dataS2 = 0x55;
			state = secondS2;
			break;
		case secondS2:
			dataS2 = 0xAA;
			state = firstS2;
			break;
	}
	return state;
}

int TickSequence3(int state) {
	switch (state) {
		case firstS3:
			dataS3 = 0xF0;
			state = secondS3;
			break;
		case secondS3:
			dataS3 = 0x0F;
			state = firstS3;
			break;
	}
	return state;
}

int TickSelectSeq(int state, unsigned char* _go, unsigned char* _on, unsigned char bitMask1, unsigned char bitMask2)
{
	char bitMaskA0 = bitMask1;
	char bitMaskA1 = bitMask2;
	char tmpA = ~PINA;

	char A0_isActive = tmpA & bitMaskA0;
	char A1_isActive = tmpA & bitMaskA1;

  switch(state) {   // Transitions
     case release:
        if (A0_isActive && !A1_isActive) {
           state = pressA0;
	   if (*_go < 2) {
	   	*_go = *_go + 1;
	   }
		else {
			*_go = 0;
		}
        }
        else if (!A0_isActive && A1_isActive) {
           state = pressA1;
	   if (*_go > 0) {
		*_go = *_go - 1;
	   }
		else {
			*_go = 2;
		}
        }
	else if (A0_isActive && A1_isActive) {
	   state = reset;
	}
	else if (!A0_isActive && !A1_isActive) {
	   state = release;
	}
        break;

     case pressA0:
        if (A0_isActive && !A1_isActive) {
           state = pressA0;
        }
        else if (!A0_isActive && A1_isActive) {
           state = pressA1;
	   if (*_go > 0) {
		*_go = *_go - 1;
	   }
		else {
			*_go = 2;
		}
        }
	else if (A0_isActive && A1_isActive) {
	   state = reset;
	}
	else if (!A0_isActive && !A1_isActive) {
	   state = release;
	}
        break;

     case pressA1:
	if (A0_isActive && !A1_isActive) {
           state = pressA0;
	   if (*_go < 2) {
	   	*_go = *_go + 1;
	   }
		else {
			*_go = 0;
		}
        }
        else if (!A0_isActive && A1_isActive) {
           state = pressA1;
        }
	else if (A0_isActive && A1_isActive) {
	   state = reset;
	}
	else if (!A0_isActive && !A1_isActive) {
	   state = release;
	}
        break;

     case reset:
	if (!A0_isActive && !A1_isActive) {
	   state = release;
		*_on = !*_on;
	}
	else {
	   state = reset;
	}
        break;

     default:
        state = release;
        break;
  } // Transitions and Actions
	return state;
}

void transmit_data(unsigned char data, unsigned char regNum) {
	int i;
	for (i = 0; i < 8; ++i) {
		if (regNum == 1) {
			PORTC = 0x08;
		}
		else {
			PORTC = 0x20;
		}
		PORTC |= ((data >> i) & 0x01);
		PORTC |= 0x02;
	}
	if (regNum == 1) {
		PORTC |= 0x04;
	}
	else {
		PORTC |= 0x10;
	}
	PORTC = 0x00;
}

void WriteOutput() {
	unsigned char i;
	for (i = 0; i < tasksNum; ++i) {
		if (tasks[i].elapsedTime >= tasks[i].period) {
			tasks[i].state = tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime = 0;
		}
		tasks[i].elapsedTime += tasksPeriodGCD;
	}
	if (tasks[3].elapsedTime >= tasks[3].period) {
		tasks[3].state = tasks[3].TickButton(tasks[3].state, &tasks[3].go, &tasks[3].on, 0x01, 0x02);
		tasks[3].elapsedTime = 0;
	}
	tasks[3].elapsedTime += tasksPeriodGCD;

	if (tasks[4].elapsedTime >= tasks[4].period) {
		tasks[4].state = tasks[4].TickButton(tasks[4].state, &tasks[4].go, &tasks[4].on, 0x04, 0x08);
		tasks[4].elapsedTime = 0;
	}
	tasks[4].elapsedTime += tasksPeriodGCD;
//
	if (tasks[3].go == 0) {
		dataOut = dataS1;
	}
	else if (tasks[3].go == 1) {
		dataOut = dataS2;
	}
	else if (tasks[3].go == 2) {
		dataOut = dataS3;
	}
	if (!tasks[3].on) {
		dataOut = 0;
	}
	transmit_data(dataOut, 1);
//
	if (tasks[4].go == 0) {
		dataOut2 = dataS1;
	}
	else if (tasks[4].go == 1) {
		dataOut2 = dataS2;
	}
	else if (tasks[4].go == 2) {
		dataOut2 = dataS3;
	}
	if (!tasks[4].on) {
		dataOut2 = 0;
	}
	transmit_data(dataOut2, 2);
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
	DDRC = 0xFF; PORTC = 0x00;           

	unsigned char i = 0;

	tasks[i].state = firstS1;
	tasks[i].period = periodSequence1;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickSequence1;
	++i;
	tasks[i].state = firstS2;
	tasks[i].period = periodSequence2;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickSequence2;
	++i;
	tasks[i].state = firstS3;
	tasks[i].period = periodSequence3;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickSequence3;
	++i;
	tasks[i].state = release;
	tasks[i].period = periodSelectSeq;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickButton = &TickSelectSeq;
	tasks[i].go = 0;
	tasks[i].on = 0;
	++i;
	tasks[i].state = release;
	tasks[i].period = periodSelectSeq;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickButton = &TickSelectSeq;
	tasks[i].go = 0;
	tasks[i].on = 0;

	TimerOn();
	TimerSet(tasksPeriodGCD);
	
	while(1) {
		WriteOutput();
		while(!TimerFlag);
		TimerFlag = 0;
	}

   return 0;
}
