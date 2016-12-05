/*
Copyright (c) 2016, Luis Castro Leiva
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or other
materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may
be used to endorse or promote products derived from this software without specific
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "iocompat.h"
#include "udm_utils.h"

typedef enum { IDLE, SEND, LISTEN, CALCULATE } state_t;

state_t current_state;
typedef void (*func_ptr)(void);
func_ptr state_function_ptr[4]; /* We have 4 states*/

typedef enum { ON, OFF } tx_condition_t;
typedef enum { TIMEOUT, RECEIVED } command_st_t;
typedef enum { OVER, ONGOING } comm_status_t;

tx_condition_t tx_status;
command_st_t command_status;
comm_status_t comm_status;

void
ioinit (void)
{

	/* Init state */
	current_state = IDLE;

	/* Init communcation status */
	comm_status = OVER;

	/* Init function pointers */
	state_function_ptr[IDLE] = &idle_function;
	state_function_ptr[SEND] = &send_function;
	state_function_ptr[LISTEN] = &listen_function;
	state_function_ptr[CALCULATE] = &calculate_function;

    /* Timer 0 is 8-bit PWM */
	/* Timer 0 is used to generate the 40 KHz pulses in OC0A and OC0B */
	TCCR0A |= _BV(COM0A0) |_BV(COM0B0) | _BV(WGM01) ; /* CTC, and toggle OC0A on Compare Match */

	//TIMSK0 = 1;
	TCCR0B |= _BV(WGM02);
	OCR0A = 24; /* 25 decnote [1], This is Arduino Mega pin 13*/
	OCR0B = 24; /* 25 dec note [1] */

	TCNT1
    /* Enable OC0A and OC0B as outputs. */
    DDR_OC0B |= _BV(OC0B);
    DDR_OC0A |= _BV(OC0A);

    /* Timer 1 Fast PWM */
    //TCCR1A = TIMER1_PWM_INIT;

    //TCCR1B |= TIMER1_CLOCKSOURCE;
    /*
     * Run any device-dependent timer 1 setup hook if present.
    */
    //#if defined(TIMER1_SETUP_HOOK)
    //    TIMER1_SETUP_HOOK();
    //#endif
    /* Set PWM value to 0. */
    //OCR = 0;
    /* Enable OC1 as output. */
    //DDROC = _BV (OC1);

    /* Enable timer 1 overflow interrupt. */
    //TIMSK = _BV (TOIE1);
    //sei ();
}

uint8_t
get_next_state(void){
	uint8_t nstate;

	switch(current_state){
		case IDLE:
			nstate = SEND;
			break;
		case SEND:
			if (tx_status == ON){
				nstate = LISTEN;
			} else {
				nstate = current_state;
			}
			break;
		case LISTEN:
			if (command_status == RECEIVED){
				nstate = CALCULATE;
			} else if (command_status == TIMEOUT){
				nstate = IDLE;
			} else {
				nstate = current_state;
			}
			break;
		case CALCULATE:
			if (comm_status == OVER){
				nstate = IDLE;
			} else {
				nstate = current_state;
			}
			break;
		default:
			nstate = current_state;
			break;
	}


	return nstate;
}

void
execute_state(void){

	(* state_function_ptr[current_state]) ();
}

int
main (void)
{
    ioinit ();
    /* loop forever, the interrupts are doing the rest */
    while(1)
    {
    	current_state = get_next_state();
    	execute_state();
    }
       //sleep_mode();
    return (0);
}

void idle_function(void){

}


void listen_function(void){

}

void calculate_function(void){

}

void send_function(void){

    //Timer on
    TCCR0B |=  _BV(CS01); /* Set the prescale to 8, note [1] */
    //TODO fix the interrupt bug
    tx_status = ON;
}


ISR (TIMER1_OVF_vect)       /* Note [2] */
{
	//Timer off
	TCCR0B &=  ~(_BV(CS02) | _BV(CS01) | _BV(CS00));
	tx_status = OFF;
}
/* note [1]: to calculate the value of the 4kHz pulse we follow this equation:  10MHz / ( 64 (prescale) * 39 (compare value)) = 4006,41 Hz */