
#include "Ass1_Servo.h"
#include <util\delay.h>

#define SWEEPINCREMENTAMOUNT 100

volatile int SweepPercent;

#define OUTPUTDIRECTION 0

void InitializeServoController( ServoControlStruct *ServoController )
{
	SweepPercent = 0;

	SetServoPeriod( ServoController, 20000 );
	SetServoRange( ServoController, 1000, 2000 );
	SetServoValue( ServoController, 0 );

	ServoInitialize( ServoController );
}

void ServoStartSweep()
{
	SweepPercent = 0;
}

int ServoUpdateSweepPosition( CommunicationStatStruct *CommStat )
{
	if( (SweepPercent + SWEEPINCREMENTAMOUNT) < 1000 )
	{
		SweepPercent = SweepPercent + SWEEPINCREMENTAMOUNT;
		CommStat->Percent = SweepPercent;


		return 0;
	}

	return 1;
}

void ServoSetDirection( ServoControlStruct *ServoController, CommunicationStatStruct *CommStat )
{
#if OUTPUTDIRECTION
	char buf[81];
	snprintf( buf, 81, "Set servo direction %u\n\r", (int)CommStat->Percent );
	uart_putstr( buf );
#endif 

	SetServoValue( ServoController, CommStat->Percent );
}
