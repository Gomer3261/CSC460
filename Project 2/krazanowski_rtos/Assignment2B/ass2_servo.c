
#include "Ass2_Servo.h"
#include <util\delay.h>

#define SWEEPINCREMENTAMOUNT 100

volatile int SweepPercent;

#define OUTPUTDIRECTION 0

void InitializeServoController( ServoControlStruct *ServoController )
{
	#if PROFILECODE && PROFILE_SERVOS 
	EnableProfileSample1();
	#endif
	SweepPercent = 0;

	SetServoPeriod( ServoController, 20000 );
	SetServoRange( ServoController, 1000, 2000 );
	SetServoValue( ServoController, 0 );

	ServoInitialize( ServoController );

	#if PROFILECODE && PROFILE_SERVOS 
	DisableProfileSample1();
	#endif
}

void ServoStartSweep()
{
	SweepPercent = 0;
}

int ServoUpdateSweepPosition( CommunicationStatStruct *CommStat )
{
	#if PROFILECODE && PROFILE_SERVOS 
	EnableProfileSample2();
	#endif

	if( (SweepPercent + SWEEPINCREMENTAMOUNT) < 1000 )
	{
		SweepPercent = SweepPercent + SWEEPINCREMENTAMOUNT;
		CommStat->Percent = SweepPercent;

		#if PROFILECODE && PROFILE_SERVOS 
		DisableProfileSample2();
		#endif

		return 0;
	}

	#if PROFILECODE && PROFILE_SERVOS 
	DisableProfileSample2();
	#endif

	return 1;
}

void ServoSetDirection( ServoControlStruct *ServoController, CommunicationStatStruct *CommStat )
{
#if OUTPUTDIRECTION
	snprintf( (char*)ASS_UART_BUFF, ASS_UART_BUFF_MAXLINE, "Set servo direction %u\n\r", (int)CommStat->Percent );
	uart_putstr( ASS_UART_BUFF );
#endif 

	#if PROFILECODE && PROFILE_SERVOS 
	EnableProfileSample3();
	#endif

	SetServoValue( ServoController, CommStat->Percent );

	#if PROFILECODE && PROFILE_SERVOS 
	DisableProfileSample3();
	#endif
}
