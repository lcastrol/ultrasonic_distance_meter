
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


/*
 * Description: Ultrasonic distance meter, multidevice code
 */

/**********************************************
 * Arduino 2560 setup
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
 *Arduino UNO
 *
 *pin8 - Max ON/OFF
 *pin7 - Signal
 *pin6 - midpoint
 *pin5 - Max Input
 *
 */


/* setting serial communication params */
#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega328P__)
	#define F_CPU 16000000UL
	#define BAUD 9600
#elif defined(__AVR_ATtiny24__)
	#define F_CPU 8000000UL
#endif

//#define SILENCE_TIME 10
#define SILENCE_TIME 40

#define HITS_TO_VALID 3

#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega328P__)

	#define T0_PRESCALE 8
	#define ULTRASONIC_MATCH 26 // = ((F_CPU/(2 * T0_PRESCALE * 40000UL)) - 1) -> 25 us = 40kHz

	#define T1_PRESCALE 256
	#define TRAIN_LENGTH 11 // = floor(((T1_PRESCALE) * 2)/F_CPU) for 2ms
	#define TIMEOUT_VALUE_L 0xFF // (floor(((T1_PRESCALE) * 11)/F_CPU)&0xFF00)>>8 for 11ms
	#define TIMEOUT_VALUE_H 0xFF // (floor(((T1_PRESCALE) * 11)/F_CPU)&0xFF) for 11ms

#elif defined(__AVR_ATtiny24__)

	#define T1_PRESCALE 256
	#define TRAIN_LENGTH 63 // = floor((F_CPU  * 2)/(T1_PRESCALE *1000)) for 2ms

	#define TIMEOUT_VALUE_L 0x00 // (floor((F_CPU * 11)/(T1_PRESCALE * 1000))&0xFF) for 11ms
	#define TIMEOUT_VALUE_H 0x29 // (floor((F_CPU * 11)/(T1_PRESCALE * 1000))&0xFF00)>>8 for 11ms
	#define T0_PRESCALE 1
	#define ULTRASONIC_MATCH 99 // = F_CPU/(2 * T0_PRESCALE * 40000UL) - 1) -> 25 us = 40kHz

#endif

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
volatile boolean_t reception_int_flag; // Flag to communicate the reception of the message
volatile boolean_t time_out_flag; // Flag to communicate the timeout status
volatile uint8_t hits_count;

char distance_ss;
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
	reception_int_flag = FALSE;
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
	TCCR0B |= _BV(WGM02); //TODO: check this, this is not needed for CTC
	OCR0A = ULTRASONIC_MATCH; /* note [1], This is Arduino Mega pin 13*/
	OCR0B = ULTRASONIC_MATCH; /* note [1] */

    /* Enable OC0A and OC0B as outputs. */
    DDR_OC0B |= _BV(OC0B);
    //DDR_OC0A |= _BV(OC0A);

    //Drive the pin to ground
    COMP_SIGNAL_DDR &= ~(_BV(SIGNAL_PIN));

    //--------------------------------------------------------------------
    //Configure pin to turn on the MAX233
    MAX_DDR |= _BV(MAX_PIN);
    MAX_PORT &= ~(_BV(MAX_PIN)); //MAX ON, pin 25 in arduino mega256

    //--------------------------------------------------------------------
    /* Timer 1 */
    TCCR1A |= _BV(COM1A0) | _BV(WGM11); /* CTC,*/
    //OCR1A = 0x02FF;


    OCR1AH = TIMEOUT_VALUE_H; //TODO: test the writing method
    OCR1AL = TIMEOUT_VALUE_L;

    //--------------------------------------------------------------------
    //Analog comparator configuration
    //ACSR |= _BV(ACIE) | _BV(ACIS1) | _BV(ACIS0); /* raising edge */
	//ACSR = (0<<ACD) | (1<<ACBG) | (1<<ACIE) | (0<<ACIC) | (1<<ACIS1) | (1<<ACIS0); //TODO: remove this if not required

	//No bandgap reference
	ACSR = (0<<ACD) | (0<<ACBG) | (1<<ACIE) | (0<<ACIC) | (1<<ACIS1) | (1<<ACIS0);

    //ATMEGA configuration, not required for attiny24
    //The arduino mega 256 has no AIN0 connected in the board, we will use ADC1/PF1 in the device, A0	 in the mega board
	//ADCSRB |= _BV(ACME);
    //ADCSRA &= ~(_BV(ADEN));
    //ADMUX |= _BV(MUX0);

    //--------------------------------------------------------------------
    /* Enable timer 1 compare A match. */
    TIMSK = _BV (OCIE1A); //TODO: Figure out if required
    sei();

    //Debug pin for hits
    //enable output
    DDRB |= _BV(DDB1);

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
}

/******************************
 * Function: listen_function
 * Description: LISTEN state function, this enables and listens to the RX hardware for the "ECHO"/"RESPONSE"
 ******************************/
void listen_function(void){

	//reception_int_flag = TRUE; //debug
	//uart_putchar('L');
	//time_out_flag = TRUE;

	//Wait
	//while(TCNT1L <= 2*TRAIN_LENGTH){}
	//Listen to the RX
	ACSR |= _BV(ACIE);

	if (time_out_flag == TRUE){
		command_status = TIMEOUT;
		uart_putchar(0xFF); // Timeout code
#if DEBUG_STATES
		uart_putchar('T'); //debug
		uart_putchar((char)TCNT1L);
#endif
	}
	else if (reception_int_flag == TRUE) {
#if DEBUG_STATES
		uart_putchar('R');
#endif
		//distance_ss = distance;
		reception_int_flag = FALSE;
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
	if (time_out_flag == TRUE){
		//Transmit the distance
		uart_putchar(distance);
		comm_status = OVER;
	}
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

    ////////////////////////////////////////////////////////////////

    TIMSK |= _BV(OCIE1A); //Timer Interrupt on
    TIMER1_OFF;
    TCNT1L = 0;
    TCNT1H = 0;
    TIMER1_ON;
    while(time_out_flag == FALSE){}
    time_out_flag = FALSE;
    TIMER1_OFF;

    ///////////////////////////////////////////////////////

	//------------------TURN ON Interrupts ----------------------
	TIMSK |= _BV(OCIE1A); //Timer Interrupt on
    ACSR &= ~(_BV(ACIE)); //Analog comparator on

	//Clear counter 0
	TCNT0 = 0;

    //clean counter 1
    TCNT1L = 0;
    TCNT1H = 0;

	//Configure direction of pin for PWM
	//DDR_OC0A |= _BV(OC0A);
	DDR_OC0B |= _BV(OC0B);

	//MAX ON
	TURN_MAX_ON;

	//-------------------TURN ON TIMERS ---------------------------
	//Timer0 prescaler PWM train generation on
    TIMER0_ON;

    //Timer1 for train on distance measure on
    TIMER1_ON;

    //MAX OFF, This is for noise contention, funny thing it works,
    //MAX will be on for a while after
    TURN_MAX_OFF;

    while(TCNT1L <= TRAIN_LENGTH){} //Do nothing while waiting

    //Timer off
    TIMER0_OFF;
    //DDR_OC0A &= ~(_BV(OC0A));
    DDR_OC0B &= ~(_BV(OC0B));

    //DEBUG clean
    PORTB &= ~(_BV(DDB1));

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
}

/*************************************
 * Interrupt handler ANALOG_COMP_vect
 ************************************/
ISR (ANALOG_COMP_vect)
{
	if(TCNT1L > SILENCE_TIME){
		hits_count += 1;
	}

	if(hits_count == HITS_TO_VALID){
//
//		TIMSK &= ~(_BV(OCIE1A)); //Timer Interrupt off
//		ACSR &= ~( _BV(ACIE)); //Analog comparator off
//		TCCR1B &= ~(_BV(CS12)| _BV(CS11) | _BV(CS10));
		reception_int_flag = TRUE;

		//Measure distance
		distance = (char)TCNT1L;

		//DEBUG capture
		PORTB = (_BV(DDB1) ^ PORTB);

		//hits_count = 0;
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





