/*
 * udm_utils.h
 *
 *  Created on: Dec 5, 2016
 *      Author: root
 */

#ifndef UDM_UTILS_H_
#define UDM_UTILS_H_

#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega328P__)

	//PRESCALER T0
	#if T0_PRESCALE == 1
		#define T0_SET_PRESCALER _BV(CS00)
	#endif
	#if T0_PRESCALE == 8
		#define T0_SET_PRESCALER _BV(CS01)
	#endif
	#if T0_PRESCALE == 64
		#define T0_SET_PRESCALER (_BV(CS01) | _BV(CS00))
	#endif
	#define TIMER0_ON TCCR0B |= T0_SET_PRESCALER
	#define TIMER0_OFF TCCR0B &= ~(_BV(CS02) | _BV(CS01) | _BV(CS00))

	//PRESCALER T1
	#if T1_PRESCALE == 1
		#define T1_SET_PRESCALER _BV(CS10)
	#endif
	#if T1_PRESCALE == 8
		#define T1_SET_PRESCALER _BV(CS11)
	#endif
	#if T1_PRESCALE == 64
		#define T1_SET_PRESCALER (_BV(11) | _BV(CS10))
	#endif
	#if T1_PRESCALE == 256
		#define T1_SET_PRESCALER _BV(CS12)
	#endif
	#if T1_PRESCALE == 1024
		#define T1_SET_PRESCALER (_BV(CS12) | _BV(CS10))
	#endif
	#define TIMER1_ON TCCR1B |= T1_SET_PRESCALER
	#define TIMER1_OFF TCCR1B &= ~(_BV(CS12) | _BV(CS11) | _BV(CS10))

#elif defined(__AVR_ATtiny24__)

	//PRESCALER T0
	#if T0_PRESCALE == 1
		#define T0_SET_PRESCALER _BV(CS00)
	#endif
	#if T0_PRESCALE == 8
		#define T0_SET_PRESCALER _BV(CS01)
	#endif
	#define TIMER0_ON TCCR0B |= T0_SET_PRESCALER
	#define TIMER0_OFF TCCR0B &= ~(_BV(CS02) | _BV(CS01) | _BV(CS00))

	//PRESCALER T1
	#if T1_PRESCALE == 1
		#define T1_SET_PRESCALER _BV(CS10)
	#endif
	#if T1_PRESCALE == 8
		#define T1_SET_PRESCALER _BV(CS11)
	#endif
	#if T1_PRESCALE == 64
		#define T1_SET_PRESCALER (_BV(CS11) | _BV(CS10))
	#endif
	#if T1_PRESCALE == 256
		#define T1_SET_PRESCALER _BV(CS12)
	#endif
	#if T1_PRESCALE == 1024
		#define T1_SET_PRESCALER (_BV(CS12) | _BV(CS10))
	#endif

	#define TIMER1_ON TCCR1B |= T1_SET_PRESCALER
	#define TIMER1_OFF TCCR1B &= ~(_BV(CS12) | _BV(CS11) | _BV(CS10))

#endif

void calculate_function(void);
void idle_function(void);
void listen_function(void);
void send_function(void);

void uart_putchar(char c);



#endif /* UDM_UTILS_H_ */
