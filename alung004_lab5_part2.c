/*	Author: lab
 *  Partner(s) Name: Andrew Lung
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

enum SM_States { release, pressA0, pressA1, reset } State;

void TickSM()
{
	char bitMaskA0 = 0x01;
	char bitMaskA1 = 0x02;
	char tmpA = (~PINA) & 0x03;

	char A0_isActive = tmpA & bitMaskA0;
	char A1_isActive = tmpA & bitMaskA1;

  switch(State) {   // Transitions
     case release:
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
        if (A0_isActive && !A1_isActive) {
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
	if (A0_isActive && !A1_isActive) {
           State = pressA0;
	   if (PORTC < 9) {
	   	PORTC = PORTC + 1;
	   }
        }
        else if (!A0_isActive && A1_isActive) {
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
/*
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
*/
	if (!A0_isActive && !A1_isActive) {
	   State = release;
	}
	else {
	   State = reset;
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

   while(1) {
      TickSM();
   }
   return 0;
}
