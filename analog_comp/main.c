/*
 * main.c
 *
 *  Created on: Dec 12, 2016
 *      Author: root
 */

///* setting serial communication params */
#define F_CPU 16000000UL
#define BAUD 115200

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>

volatile uint8_t alarm = 0;

void init_comp()
{
    // Disable the digital input buffers.
    DIDR1 = (1<<AIN1D) | (1<<AIN0D);

    // Setup the comparator...
    // Enabled, no bandgap, interrupt enabled,
    // no input capture, interrupt on falling edge.
    ACSR = (1<<ACD) | (0<<ACBG) | (1<<ACIE) | (0<<ACIC) | (1<<ACIS1) | (0<<ACIS0);

    sei();
}

int main()
{
    init_comp();
    uart_init();

    while (1)
    {
        if (alarm)
        {
        	uart_putchar('a');
        	// Do something important.
        	alarm = 0;
        }
        else{
        	uart_putchar('b');
        }

    }
}

ISR(ANALOG_COMP_vect)
{
    // Did it just go high?
    if (ACSR & ACO)
    {
        // Indicate a problem.
        alarm = 1;

        // Change to falling edge.
        ACSR &= ~(1<<ACIS0);
    }
    else
    {
        // Indicate a all is well.
        alarm = 1;

        // Set up the rising edge.
        ACSR |= (1<<ACIS0);
    }
}

void uart_putchar(char c) {

	//UDR0 = c;
    //loop_until_bit_is_set(UCSR0A, TXC0); /* Wait until transmission ready. */
	loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
	UDR0 = c;
}

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

