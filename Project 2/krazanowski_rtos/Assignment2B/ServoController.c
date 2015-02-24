#include "ServoController.h"
#include <util\delay.h>

#define F_CPU 8000000UL

void ServoInitialize( ServoControlStruct *SCS )
{
	int sreg = SREG;
	cli();

	/*
	DDRB |= 0b01000000; // set pin B.6 for output (OC1B)

	// TCCR1A:  COM1A1 | COM1A0 | COM1B1 | COM1B0 | FOC1A | FOC1B | WGM11 | WGM10
	// TCCR1B:  ICNC1  | ICES1  | ------ | WGM13  | WGM12 | CS12  | CS11  | CS10

	// WGM11:WGM10 = 10: with WGM13-WGM12 to select timer mode 1110
	// Fast PWM, timer 1 runs from 0 to ICR1
	// COM1A1:COM1A0 = 10: clear OC1A when compare match, set OC1A when 0
	TCCR1A = 0b00100010; // compare match occurs timer = OCR1B
	TCCR1B = 0b00011010; // WGM13:WGM12=11; CS12:CS0=001: internal clock 8MHz, 1/8 prescaler

	ICR1 = SCS->PWMPeriod; // period of output PWM signal
	OCR1B = SCS->PWMHighTime;
	*/

	DDRL |= 0b00010000; // set pin L.4 for output (OC4B)

	// TCCR1A:  COM1A1 | COM1A0 | COM1B1 | COM1B0 | FOC1A | FOC1B | WGM11 | WGM10
	// TCCR1B:  ICNC1  | ICES1  | ------ | WGM13  | WGM12 | CS12  | CS11  | CS10

	// WGM11:WGM10 = 10: with WGM13-WGM12 to select timer mode 1110
	// Fast PWM, timer 1 runs from 0 to ICR1
	// COM1A1:COM1A0 = 10: clear OC1A when compare match, set OC5A when 0
	TCCR5A = 0b00100010; // compare match occurs timer = OCR5B
	TCCR5B = 0b00011010; // WGM13:WGM12=11; CS12:CS0=001: internal clock 8MHz, 1/8 prescaler

	ICR5 = SCS->PWMPeriod; // period of output PWM signal
	OCR5B = SCS->PWMHighTime;
	

	// Since  the range is clamped to -90..90 degrees, 
	// and with a 10ms delay requirement per degree = 1800
	// _delay_ms( 1800 );

	SREG = sreg;
}

void SetServoPeriod( ServoControlStruct *SCS, int MicrosecondPeriod )
{
	SCS->PWMPeriod = MicrosecondPeriod;
}

void SetServoValue( ServoControlStruct *SCS, int TenthPercent )
{
	if( TenthPercent < 0 )
	{
		TenthPercent = 0;
	}
	else if( TenthPercent >= 1000 )
	{
		TenthPercent = 999;
	}

	int OldValue = SCS->Value;

	// Set the percentage value
	SCS->Value = TenthPercent;
	// Interpolate between the low and the high value
	SCS->PWMHighTime = SCS->LowThreshold + (int)(SCS->DeltaThreshold * ((float)SCS->Value / 1000.0f));

	int sreg = SREG;
	cli();

	// OCR1B = SCS->PWMHighTime; // set high time of output PWM signal
	OCR5B = SCS->PWMHighTime; // set high time of output PWM signal
	
	SREG = sreg;

	// int PosDifference = (SCS->Value - OldValue);
	// if( 0 > PosDifference )
	// 	PosDifference = -PosDifference;
	
	// Convert the difference from percent to angle (degrees)
	// then multiply it by the 10ms delay/degree
	// int DelayTime = ((PosDifference * 90) /10) * 100;

	// _delay_ms( DelayTime );
}

void AddServoValueDelta( ServoControlStruct *SCS, int TenthPercent )
{
	int NewValue = SCS->Value + TenthPercent;

	SetServoValue( SCS, NewValue );
}

void SubtractServoValueDelta( ServoControlStruct *SCS, int TenthPercent )
{
	int NewValue = SCS->Value - TenthPercent;

	SetServoValue( SCS, NewValue );
}

void SetServoRange( ServoControlStruct *SCS, int Low, int High )
{
	SCS->LowThreshold = Low;
	SCS->HighThreshold = High;
	SCS->DeltaThreshold = High-Low;

	SCS->LowAngle = -SCS->LowThreshold * 9 / 1000;
	SCS->HighAngle = SCS->HighThreshold * 9 / 1000;
}
