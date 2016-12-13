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


/**********************************************
 * Arduino setup
 * Arduino - Sensor
 * Vcc - Vcc
 * GND - GND
 * pin13 - Max IN
 * pinA0 - Signal
 *
 * pin25 - Max ON/OFF
 *
 * TBD - thre
 *
 * X - MID point --> not connected.
 *
 *
 */


/* setting serial communication params */
#define F_CPU 16000000UL
#define BAUD 115200
#define TRAIN_LENGTH 0x2 // ms
//#define TRAIN_LENGTH 0x9
#define TIMEOUT_VALUE 0xA4 // 10.7 ms
#define HITS_TO_VALID 4

#define DEBUG_STATES 0

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/setbaud.h>
#include <util/delay.h>
#include "iocompat.h"
#include "udm_utils.h"

typedef enum { IDLE, SEND, LISTEN, CALCULATE } state_t;

state_t current_state;
typedef void (*func_ptr)(void);
func_ptr state_function_ptr[4]; /* We have 4 states*/

typedef enum { ON, OFF } tx_condition_t;
typedef enum { ALIVE, TIMEOUT, RECEIVED } command_st_t;
typedef enum { OVER, ONGOING } comm_status_t;
typedef enum { FALSE, TRUE } boolean_t;

tx_condition_t tx_status;
command_st_t command_status;
comm_status_t comm_status;
boolean_t tx_triggered; // This is a flag to change state SEND->LISTEN
volatile uint8_t reception_int_flag; // Flag to communicate the reception of the message
volatile boolean_t time_out_flag; // Flag to communicate the timeout status
volatile uint8_t hits_count;


volatile char distance = 'a';

/**************************
 * Function: uart_init
 * Description: initialize the UART for serial transmissions
 *************************/
void
uart_init(void) {
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

#if USE_2X
    UCSR0A |= _BV(U2X0);
#else
    UCSR0A &= ~(_BV(U2X0));
#endif

    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
    //UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
    UCSR0B = _BV(TXEN0);   /* Enable RX and TX */
}

/**************************
 * Function: ioinit
 * Description: initialize io and counters
 *************************/
void
ioinit (void)
{

	//------------------flags and states ------------------
	/* Init state */
	current_state = IDLE;
	/* Init communication status */
	comm_status = ONGOING;
	//Init triggered flag
	tx_triggered = FALSE;
	//Init reception flag
	//reception_int_flag = FALSE;
	reception_int_flag = 0;
	//Init hits counter
	hits_count = 0;

	//-------------------------------------------------------------------
	/* Init function pointers */
	state_function_ptr[IDLE] = &idle_function;
	state_function_ptr[SEND] = &send_function;
	state_function_ptr[LISTEN] = &listen_function;
	state_function_ptr[CALCULATE] = &calculate_function;


	//-------------------------------------------------------------------
    /* Timer 0 is 8-bit PWM */
	/* Timer 0 is used to generate the 40 KHz pulses in OC0A and OC0B */
	TCCR0A |= _BV(COM0A0) |_BV(COM0B0) | _BV(WGM01) ; /* CTC, and toggle OC0A on Compare Match */

	//TIMSK0 = 1;
	TCCR0B |= _BV(WGM02);
	OCR0A = 24; /* note [1], This is Arduino Mega pin 13*/
	//OCR0A = 80;
	OCR0B = 24; /* note [1] */

    /* Enable OC0A and OC0B as outputs. */
    DDR_OC0B |= _BV(OC0B);
    DDR_OC0A |= _BV(OC0A);

    PORTB &= ~(_BV(DDB7));

    //--------------------------------------------------------------------
    //Configure pin to turn on the MAX233
    DDRA |= _BV(DDA3);
    PORTA &= ~(_BV(DDA3)); //MAX ON, pin 25 in arduino mega256

    //--------------------------------------------------------------------
    /* Timer 1 */
    TCCR1A |= _BV(COM1A0) | _BV(WGM11); /* CTC,*/
    OCR1A = TIMEOUT_VALUE;

    //--------------------------------------------------------------------
    //Analog comparator configuration
    //ACSR |= _BV(ACIE) | _BV(ACIS1) | _BV(ACIS0); /* raising edge */
	  ACSR = (0<<ACD) | (1<<ACBG) | (1<<ACIE) | (0<<ACIC) | (1<<ACIS1) | (1<<ACIS0);
    //ACSR |= _BV(ACIE);
    //ATMEGA configuration, not required for attiny24
    //The arduino mega 256 has no AIN0 connected in the board, we will use ADC1/PF1 in the device, A0	 in the mega board
    ADCSRB |= _BV(ACME);
    ADCSRA &= ~(_BV(ADEN));
    ADMUX |= _BV(MUX0);

    //PORTF |= _BV(PF1);
    //PORTF |= _BV(PF0);
    //PORTE |= _BV(PE3);


    //--------------------------------------------------------------------
    /* Enable timer 1 compare A match. */
    //TIMSK = _BV (OCIE1A);
    sei();
}

/**************************
 * Function: get_next_state
 * Description: calculate the next state
 *************************/
uint8_t
get_next_state(void){
	uint8_t nstate;

	switch(current_state){
		case IDLE:
			nstate = SEND;
			break;
		case SEND:
			if (tx_triggered == TRUE){
				nstate = LISTEN;
			} else {
				nstate = current_state;
			}
			break;
		case LISTEN:
			//command_status = RECEIVED; //debug purposes
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

/************************************************************
 * Function: execute_state
 * Description: call the respective function of each state
 ************************************************************/
void
execute_state(void){
	(* state_function_ptr[current_state]) ();
}

/************************************************************
 * Function: main
 * Description: well main, loop forever
 ************************************************************/
int
main (void)
{
    ioinit ();
    uart_init();
    /* loop forever, the interrupts are doing the rest */
    while(1)
    {
    	current_state = get_next_state();
    	execute_state();
    }
    uart_putchar('D');
    //sleep_mode();
    return (0);
}

/************************************************************
 * Function: idle_function
 * Description: execute idle state
 ************************************************************/
void idle_function(void){
	//Well do nothing, this state can be removed
	TIMSK &= ~(_BV(OCIE1A));
	//ACSR &= ~(_BV(ACIE));
#if DEBUG_STATES
	uart_putchar('I');
#endif
	comm_status = ONGOING;
	tx_triggered = FALSE;
	time_out_flag = FALSE;
	//reception_int_flag = FALSE;
	//reception_int_flag = 0;
}

/******************************
 * Function: listen_function
 * Description: LISTEN state function, this enables and listens to the RX hardware for the "ECHO"/"RESPONSE"
 ******************************/
void listen_function(void){

	//reception_int_flag = TRUE; //debug
	//uart_putchar('L');
	//time_out_flag = TRUE;

	if (time_out_flag == TRUE){
		command_status = TIMEOUT;
#if DEBUG_STATES
		uart_putchar('T'); //debug
		uart_putchar((char)TCNT1L);
#endif
	}
	else if (reception_int_flag == 1) {
#if DEBUG_STATES
		uart_putchar('R');
#endif
		reception_int_flag = 0;
		command_status = RECEIVED;
	}


}

/******************************
 * Function: calculate_function
 * Description: transmit in serial the output of the sensor
 ******************************/
void calculate_function(void){

#if DEBUG_STATES
	uart_putchar('C'); //debug
#endif
	//Transmit the distance
	uart_putchar(distance);
	comm_status = OVER;
}
/******************************
 * Function: send_function
 * Description: SEND state function, this triggers the
 ******************************/
void send_function(void){

#if DEBUG_STATES
	uart_putchar('S'); //debug
#endif
    //---Update flags ----------------
    tx_status = ON;
    command_status = ALIVE;

    tx_triggered = TRUE;

	//------------------TURN ON Interrupts ----------------------
	TIMSK |= _BV (OCIE1A); //Timer Interrupt on
    //ACSR |= _BV(ACIE); //Analog comparator on
    //sei();

	//Clear counter 0
	TCNT0 = 0;

    //clean counter 1
    TCNT1L = 0;
    TCNT1H = 0;

	//Configure direction of pin for PWM
	DDR_OC0A |= _BV(OC0A);

	//MAX ON
	PORTA &= ~(_BV(DDA3));

	//-------------------TURN ON TIMERS ---------------------------
	//Timer0 prescaler PWM train generation on
    TCCR0B |=  _BV(CS01); /* Set the prescale to 8, note [1] */

    //Timer1 for train on distance measure on
    TCCR1B |= _BV(CS12) | _BV(CS10); // Set the prescaler to 1024

    while(TCNT1L <= TRAIN_LENGTH){} //Do nothing while waiting

    //Timer off
    TCCR0B &=  ~(_BV(CS02) | _BV(CS01) | _BV(CS00));
    DDR_OC0A &= ~(_BV(OC0A));

    //MAX OFF
    PORTA |= _BV(DDA3);

}

/*********************************
 * Interrupt handler TIMER1_OVF vector
 *********************************/
ISR (TIMER1_COMPA_vect)       /* Note [2] */
{
	TIMSK &= ~(_BV(OCIE1A)); //Timer Interrupt off
	//ACSR &= ~(_BV(ACIE)); //Analog comparator off

	TCCR1B &= ~(_BV(CS12) | _BV(CS11) | _BV(CS10));
	//TCNT1L = 0;
	hits_count = 0;
	time_out_flag = TRUE;
	//uart_putchar('V');
}

/*************************************
 * Interrupt handler ANALOG_COMP_vect
 ************************************/
ISR (ANALOG_COMP_vect)
{
	hits_count += 1;

	//uart_putchar((char)hits_count);
	//_delay_ms(1.0);
	//uart_putchar('W');
	//reception_int_flag = 1;
	if(hits_count >= HITS_TO_VALID){
//
//		TIMSK &= ~(_BV(OCIE1A)); //Timer Interrupt off
//		ACSR &= ~( _BV(ACIE)); //Analog comparator off
//
//
//		TCCR1B &= ~(_BV(CS12)| _BV(CS11) | _BV(CS10));
		reception_int_flag = 1;
//		//Measure distance
		distance = (char)TCNT1L;
//
		hits_count = 0;
//		uart_putchar('V');
//
	}
}

/******************************
 * Function: uart_putchar
 * Description: write in the serial communication
 ******************************/
void uart_putchar(char c) {

	//UDR0 = c;
    //loop_until_bit_is_set(UCSR0A, TXC0); /* Wait until transmission ready. */
	loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
	UDR0 = c;
}

/* note [1]: to calculate the value of the 4kHz pulse we follow this equation:  10MHz / ( 64 (prescale) * 39 (compare value)) = 4006,41 Hz */

//****************************************************************************
//****************************************************************************
//****************************************************************************
//****************************************************************************


///* setting serial communication params */
//#define F_CPU 16000000UL
//#define BAUD 115200
//
//#include <avr/io.h>
//#include <avr/interrupt.h>
//#include <util/setbaud.h>
//#include "iocompat.h"
//
//volatile uint8_t alarm = 0;
//
//void init_comp()
//{
//    // Disable the digital input buffers.
//    //DIDR0 = (1<<AIN1D) | (1<<AIN0D);
//
//    // Setup the comparator...
//    // Enabled, no bandgap, interrupt enabled,
//    // no input capture, interrupt on falling edge.
//    ACSR = (0<<ACD) | (0<<ACBG) | (1<<ACIE) | (0<<ACIC) | (1<<ACIS1) | (1<<ACIS0);
//
//    ADCSRB |= _BV(ACME);
//    ADCSRA &= ~(_BV(ADEN));
//    ADMUX |= _BV(MUX0);
//    //PORTF |= _BV(PF1);
//    //PORTF |= _BV(PF0);
//    //PORTE |= _BV(PE3);
//
//    sei();
//}
//
//int main()
//{
//    init_comp();
//    uart_init();
//
//    while (1)
//    {
//    	//uart_putchar('Z');
//    	if (alarm)
//        {
//        	uart_putchar('a');
//        	// Do something important.
//        	alarm = 0;
//        } else {
//        	uart_putchar('b');
//        }
//
//    }
//}
//
//ISR(ANALOG_COMP_vect)
//{
//    // Did it just go high?
//    //if (ACSR & ACO)
//    //{
//    //    // Indicate a problem.
//    //    alarm = 1;
//
//        // Change to falling edge.
//    //    ACSR &= ~(1<<ACIS0);
//    //}
//    //else
//    //{
//    //    // Indicate a all is well.
//    //    alarm = 1;
//
//    //    // Set up the rising edge.
//    //    ACSR |= (1<<ACIS0);
//    //}
//	alarm = 1;
//}
//
//void uart_putchar(char c) {
//
//	//UDR0 = c;
//    //loop_until_bit_is_set(UCSR0A, TXC0); /* Wait until transmission ready. */
//	loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
//	UDR0 = c;
//}
//
//
//void
//uart_init(void) {
//    UBRR0H = UBRRH_VALUE;
//    UBRR0L = UBRRL_VALUE;
//
//#if USE_2X
//    UCSR0A |= _BV(U2X0);
//#else
//    UCSR0A &= ~(_BV(U2X0));
//#endif
//
//    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
//    //UCSR0B = _BV(RXEN0) | _BV(TXEN0);   /* Enable RX and TX */
//    UCSR0B = _BV(TXEN0);   /* Enable RX and TX */
//}
//


