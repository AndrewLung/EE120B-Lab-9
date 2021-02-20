/* Author: Andrew Lung
 * Partner(s) Name (if applicable):
 * Lab Section: 22
 * Assignment: Lab #11  Exercise #2
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 *
 *  Demo Link: https://www.youtube.com/watch?v=ufu5YD_-D2o&feature=youtu.be
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

task tasks[4];

const unsigned char tasksNum = 4;
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

int TickSelectSeq(int state);
enum SelectSeq { release, pressA0, pressA1, reset };

unsigned char dataOut;
unsigned char dataS1;
unsigned char dataS2;
unsigned char dataS3;

unsigned char go = 0;
unsigned char on = 1;

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

int TickSelectSeq(int state)
{
	char bitMaskA0 = 0x01;
	char bitMaskA1 = 0x02;
	char tmpA = ~PINA;

	char A0_isActive = tmpA & bitMaskA0;
	char A1_isActive = tmpA & bitMaskA1;

  switch(state) {   // Transitions
     case release:
        if (A0_isActive && !A1_isActive) {
           state = pressA0;
	   if (go < 2) {
	   	go = go + 1;
	   }
		else {
			go = 0;
		}
        }
        else if (!A0_isActive && A1_isActive) {
           state = pressA1;
	   if (go > 0) {
		go = go - 1;
	   }
		else {
			go = 2;
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
	   if (go > 0) {
		go = go - 1;
	   }
		else {
			go = 2;
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
	   if (go < 2) {
	   	go = go + 1;
	   }
		else {
			go = 0;
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
		on = !on;
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

void transmit_data(unsigned char data) {
	int i;
	for (i = 0; i < 8; ++i) {
		PORTC = 0x08;
		PORTC |= ((data >> i) & 0x01);
		PORTC |= 0x02;
	}
	PORTC |= 0x04;
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
	if (go == 0) {
		dataOut = dataS1;
	}
	else if (go == 1) {
		dataOut = dataS2;
	}
	else if (go == 2) {
		dataOut = dataS3;
	}
	if (!on) {
		dataOut = 0;
	}
	transmit_data(dataOut);
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
	tasks[i].TickFct = &TickSelectSeq;
	
	TimerOn();
	TimerSet(tasksPeriodGCD);
	
	while(1) {
		WriteOutput();
		while(!TimerFlag);
		TimerFlag = 0;
	}

   return 0;
}
