/* Author: Andrew Lung
 * Partner(s) Name (if applicable):
 * Lab Section: 22
 * Assignment: Lab #6  Exercise #2
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 *
 *  Demo Link: https://www.youtube.com/watch?v=FVLaRwmRE50&feature=youtu.be
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

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


enum SM_States { release, pressA0, pressA1, reset } State;

void TickSM()
{
	char bitMaskA0 = 0x01;
	char bitMaskA1 = 0x02;
	char tmpA = PINA;

	char A0_isActive = (~tmpA) & bitMaskA0;
	char A1_isActive = (~tmpA) & bitMaskA1;

  switch(State) {   // Transitions
     case release:
	cycles = 0;
        if (A0_isActive && !A1_isActive) {
           State = pressA0;
	   if (PORTC < 9) {
	   	PORTC = PORTC + 1;
	   }
        }
        else if (!A0_isActive && A1_isActive) {
           State = pressA1;
	   if (PORTC > 0) {
		PORTC = PORTC - 1;
	   }
        }
	else if (A0_isActive && A1_isActive) {
	   State = reset;
	   PORTC = 0;
	}
	else if (!A0_isActive && !A1_isActive) {
	   State = release;
	}
        break;

     case pressA0:
	++cycles;
        if (A0_isActive && !A1_isActive) {
	   if (PORTC < 9 && cycles == 10) {
		cycles = 0;
	   	PORTC = PORTC + 1;
	   }
           State = pressA0;
        }
        else if (!A0_isActive && A1_isActive) {
           State = pressA1;
	   if (PORTC > 0) {
		PORTC = PORTC - 1;
	   }
        }
	else if (A0_isActive && A1_isActive) {
	   State = reset;
	   PORTC = 0;
	}
	else if (!A0_isActive && !A1_isActive) {
	   State = release;
	}
        break;

     case pressA1:
	++cycles;
	if (A0_isActive && !A1_isActive) {
           State = pressA0;
	   if (PORTC < 9) {
	   	PORTC = PORTC + 1;
	   }
        }
        else if (!A0_isActive && A1_isActive) {
	   if (PORTC > 0 && cycles == 10) {
           	cycles = 0;
		PORTC = PORTC - 1;
	   }
	   State = pressA1;
        }
	else if (A0_isActive && A1_isActive) {
	   State = reset;
	   PORTC = 0;
	}
	else if (!A0_isActive && !A1_isActive) {
	   State = release;
	}
        break;

     case reset:
	if (A0_isActive && !A1_isActive) {
           State = pressA0;
	   if (PORTC < 9) {
	   	PORTC = PORTC + 1;
	   }
        }
        else if (!A0_isActive && A1_isActive) {
           State = pressA1;
	   if (PORTC > 0) {
		PORTC = PORTC - 1;
	   }
        }
	else if (A0_isActive && A1_isActive) {
	   State = reset;
	}
	else if (!A0_isActive && !A1_isActive) {
	   State = release;
	}
        break;

     default:
        State = release;
        break;
  } // Transitions and Actions
}

int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;              // Initialize outputs
   State = release; // Indicates initial call
   PORTC = 7;
	TimerSet(100);
	TimerOn();
	cycles = 0;
   while(1) {
      TickSM();
	while(!TimerFlag);
	TimerFlag = 0;
   }
   return 0;
}
