/*
 * sonar.c
 *
 *  Created on: 2010-05-12
 *      Author: lienh
 */
#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "sonar.h"

static volatile uint16_t trigger_time = 0;
static volatile uint16_t echo_time = 0;
static volatile uint8_t received = 0;

uint16_t _get_TCNT3();

void sonar_init()
{
	// set PE6 for output
	DDRE |= _BV(PORTE6);

	// set timer3 to run 1/8 of the microprocessor's clock frequency
	TCCR3B &= ~_BV(CS30);
	TCCR3B |= _BV(CS31);
	TCCR3B &= ~_BV(CS32);

	// enable noise canceler for input capture
	TCCR3B |= _BV(ICNC3);

	// set input capture on rising edge
	TCCR3B |= _BV(ICES3);

	// clear input capture flag
	TIFR3 |= _BV(ICF3);

	// enable input capture interrupt
	TIMSK3 |= _BV(ICIE3);

	sei();
}

void sonar_trigger()
{
	// trigger a 10 us pulse
	PORTE |= _BV(PE6);
	_delay_us(10);
	PORTE &= ~_BV(PE6);
}

uint8_t sonar_echo_received()
{
	char sreg = SREG;
	cli();
	uint8_t temp = received;
	received = 0;
	SREG = sreg;
	return temp;
}

uint16_t sonar_get_distance()
{
	return trigger_time - echo_time;
}

uint16_t sonar_get_ping_time()
{
	return trigger_time;
}

uint16_t sonar_get_sonar_time()
{
	return echo_time;
}

ISR(TIMER3_CAPT_vect)
{
	if (TCCR3B & _BV(ICES3)) { // a rising edge is captured

		echo_time = ICR3;

		TCNT3 = 0;

		// set input capture on falling edge
		TCCR3B &= ~_BV(ICES3);

		// clear input capture flag
		TIFR3 |= _BV(ICF3);

	} else { // a falling edge is captured
		received = 1;
		trigger_time = ICR3;
		// echo_time = trigger_time;

		// set input capture on rising edge
		TCCR3B |= _BV(ICES3);

		// clear input capture flag
		TIFR3 |= _BV(ICF3);
	}
}
