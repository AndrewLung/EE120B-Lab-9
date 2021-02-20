/* Author: Andrew Lung
 * Partner(s) Name (if applicable):
 * Lab Section: 22
 * Assignment: Lab #11  Exercise #5
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 *
 *  Demo Link: https://www.youtube.com/watch?v=Q73Npv065Co&feature=youtu.be
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

task tasks[3];

const unsigned char tasksNum = 3;
const unsigned long tasksPeriodGCD = 50;
const unsigned long periodSequence1 = 250;
const unsigned long periodGameHandler = 250;
const unsigned long periodPlayerController = 100;
unsigned char cnt = 0;
unsigned char go = 0;

unsigned short playerPos;
unsigned short enemyPos;
unsigned char enemyDir; // 0 for left, 1 for right

int TickSequence1(int state);
enum Sequence1 { firstS1, secondS1 };

int TickGameHandler(int state);
enum GameHandler { gameReset, gameOn, gameBlink };

int TickPlayerController(int state);
enum PlayerController { release, press, jump, wait };

unsigned short dataOut;
unsigned short dataS1;

int TickPlayerController(int state) {
	unsigned char tmpA = ~PINA;
	switch (state) {
		case release:
			if (tmpA & 0x01) { //go left pressed
				if (playerPos != 0x8000) {
					playerPos = playerPos << 1; // if at very left, cannot shift left
					state = press;
				}
			}
			else if (tmpA & 0x02) {
				if (playerPos != 0x0001) {
					playerPos = playerPos >> 1;
					state = press;
				}
			}
			else if (tmpA & 0x04) { //jmp key
				state = press;
			}
			else {
				state = release;
			}
			break;
		case press:
			if ((tmpA & 0x01) && (tmpA & 0x04)) {
				state = jump;
			}
			else if ((tmpA & 0x02) && (tmpA & 0x04)) {
				state = jump;
			}
			else if (!(tmpA & 0x01) && !(tmpA & 0x02) && !(tmpA & 0x04)) {
				state = release;
			}
			else {
				state = press;
			}
			break;
		case jump:
			if ((tmpA & 0x01) && (tmpA & 0x04)) {
				unsigned char i = 0;
				while ((playerPos < 0x8000) && (i < 4)) {
					playerPos = playerPos << 1;
					++i;
				}
				state = wait;
			}
			else if ((tmpA & 0x02) && (tmpA & 0x04)) {
				unsigned char i = 0;
				while ((playerPos > 0x0001) && (i < 4)) {
					playerPos = playerPos >> 1;
					++i;
				}
				state = wait;			
			}
			else {
				state = wait;
			}
			break;
		case wait:
			if (!(tmpA & 0x01) && !(tmpA & 0x02) && !(tmpA & 0x04)) {
				state = release;
			}
			else {
				state = wait;
			}
			break;
	}
	return state;
}

int TickGameHandler(int state) {
	switch (state) {
		case gameReset:
			playerPos = 0x8000;
			enemyPos = 0x0001;
			state = gameOn;
			break;
		case gameOn:
			if (playerPos == enemyPos) {
				state = gameBlink;
			}
			else {
				if (enemyPos >= 0x8000) {
					//set dir to 1, which is right
					enemyDir = 1;
				}
				else if (enemyPos <= 0x0001) {
					enemyDir = 0;
				}
				if (enemyDir == 0) {
					enemyPos = enemyPos << 1;
				}
				else if (enemyDir == 1) {
					enemyPos = enemyPos >> 1;
				}
				state = gameOn;
			}
			break;
		case gameBlink:
			go = 1; // 1 is Sequence1
			if (cnt < 3) {
				state = gameBlink;
			}
			else {
				cnt = 0;
				go = 0;
				state = gameReset;
			}
			break;
	}
	return state;
}

int TickSequence1(int state) {
	switch (state) {
		case firstS1:
			if (go == 1) {
				++cnt;
			}
			dataS1 = 0xFFFF;
			state = secondS1;
			break;
		case secondS1:
			dataS1 = 0x0000;
			state = firstS1;
			break;
	}
	return state;
}

void transmit_data(unsigned short data) {
	int i;
	for (i = 0; i < 16; ++i) {
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
	if (go == 1) {
		dataOut = dataS1;
	}
	else if (go == 0) {
		dataOut = 0 | enemyPos | playerPos; 
	}

	transmit_data(dataOut);
	//transmit_data(0xFFFF);
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
	
	tasks[i].state = gameBlink;
	tasks[i].period = periodGameHandler;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickGameHandler;
	++i;
	tasks[i].state = firstS1;
	tasks[i].period = periodSequence1;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickSequence1;
	++i;
	tasks[i].state = release;
	tasks[i].period = periodPlayerController;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickPlayerController;

	TimerOn();
	TimerSet(tasksPeriodGCD);
	
	while(1) {
		WriteOutput();
		while(!TimerFlag);
		TimerFlag = 0;
	}

   return 0;
}
