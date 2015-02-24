/**
 * 	This code is implements a periodic-only design for the 
 * 	indoor localization application required for assignment2 
 *  of CSC560 - RTOS
 * 
 * 
 * 
 */

#include "common.h"
#include "OS/os.h"
#include "trace/trace.h"

#include "Assignment2.h"
#include "Ass2_Globals.h"
#include "Ass2_Servo.h"
#include "Ass2_Statistics.h"
#include "Ass2_Communicate.h"

#include "OS/MikesProfiler.h"

#define SIMPLESTATES 1

#define PERIODSCALE 2

// Initialization, UpdateSweep and Calculate_Report 
// all share the first timeslice

const unsigned int PT = 5;
const unsigned char PPP[] = {LOCALIZATIONSTATE_Initialization, 140*PERIODSCALE, 
								LOCALIZATIONSTATE_SendRequest, 2*PERIODSCALE, 
								LOCALIZATIONSTATE_GetPINGResponse, 6*PERIODSCALE, 
								LOCALIZATIONSTATE_GetSonarResponse, 12*PERIODSCALE,
								LOCALIZATIONSTATE_UpdateStatistics, 40*PERIODSCALE };

// Global state-mode variables to switch between 
// the initialization, sweep and report modes
volatile int InitializedSystem = 0;
volatile int ReportingOutcome = 0;

#define UART_OUTPUT_STATE 0
#define     clock8MHz()    CLKPR = _BV(CLKPCE); CLKPR = 0x00;

// A shared buffer for use of the UART
unsigned char ASS_UART_BUFF[ASS_UART_BUFF_MAXLINE];

// Global state strutures to share information between states
ServoControlStruct SonarDirectionServo;
CommunicationStatStruct ComStats;
volatile int SeekState = LOCALIZATIONSTATE_Initialization;

volatile int bPINGRecieved;

// Wireless state information
uint8_t station_addr[5] = {0x01, 0x17, 0x76, 0x97, 0x00 };
uint8_t remote_addr[5] = {0x98, 0x76, 0x54, 0x32, 0x10};
int SweepComplete = 0;

// Process callback function prototypes
void Ass2_Task_InitSweepReport();
void Ass2_Task_Initialization();
void Ass2_Task_UpdateSweep();
void Ass2_Task_SendRequest();
void Ass2_Task_GetPINGResponse();
void Ass2_Task_GetSonarResponse();
void Ass2_Task_UpdateStatistics();

void Ass2_Task_CalculateAndReport();

void Ass2_Task_InitSweepReport()
{
	for(;;)
    { 
		if( 0 == InitializedSystem )
		{
			Ass2_Task_Initialization();
			Task_Next();
			InitializedSystem = 1;
		}
		else if( (0 != InitializedSystem) && (0 == ReportingOutcome) )
		{
			Ass2_Task_UpdateSweep();
			Task_Next();
		}
		else // if( (0 != InitializedSystem) && (0 != ReportingOutcome) )
		{
			Ass2_Task_CalculateAndReport();
			Task_Next();
		}
	}
}

void Ass2_Task_Initialization()
{
	#if PROFILECODE && PROFILE_STATES
	EnableProfileSample1();
	#endif

	int sreg = SREG;
	cli();

	#if !SIMPLESTATES
	// Initialize all necessary states
	InitializeServoController( &SonarDirectionServo );
	InitializeCommunication( station_addr, remote_addr );
	StatisticsInitialize();
	#endif

	ServoStartSweep();
	SweepComplete = 0;
	
	SREG = sreg;

	#if PROFILECODE && PROFILE_STATES
	DisableProfileSample1();
	#endif
}

void Ass2_Task_UpdateSweep()
{
	#if PROFILECODE && PROFILE_STATES
	EnableProfileSample2();
	#endif

	#if PROFILECODE && PROFILE_STATES
	_delay_ms( 1 );
	DisableProfileSample2();
	#endif
	
	#if SIMPLESTATES
	return;
	#endif

	StatisticsResetSample( &ComStats );

	if( 0 == SweepComplete )
	{
		// Redirect the servo and attempt to communicate
		SweepComplete = ServoUpdateSweepPosition( &ComStats );
		ServoSetDirection( &SonarDirectionServo, &ComStats );
		CommunicationPingResetRetryCount();
	}	
	else
	{
		// Set process to REPORTING
		ReportingOutcome = 1;
	}
	
	#if PROFILECODE && PROFILE_STATES
	DisableProfileSample2();
	#endif
}

void Ass2_Task_SendRequest()
{
	for(;;)
    {
		// If we have not yet initialized, or if we are
		// reporting, skip executing this process/state
		if( (0 == InitializedSystem) || (0 != ReportingOutcome) )
		{
			Task_Next();
		}

		#if PROFILECODE && PROFILE_STATES
		EnableProfileSample3();
		#endif

		#if PROFILECODE && PROFILE_STATES
		_delay_ms( 1 );
		DisableProfileSample3();
		#endif
	
		#if SIMPLESTATES
		Task_Next();
		continue;
		#endif

		// Send the request for a ping
		CommunicationSendTriggerMessage();
		bPINGRecieved = 0;
		
		#if PROFILECODE && PROFILE_STATES
		DisableProfileSample3();
		#endif

		Task_Next();
	}
}

void Ass2_Task_GetPINGResponse()
{
	for(;;)
    {
		// If we have not yet initialized, or if we are
		// reporting, skip executing this process/state
		if( (0 == InitializedSystem) || (0 != ReportingOutcome) )
		{
			Task_Next();
		}

		#if PROFILECODE && PROFILE_STATES
		EnableProfileSample4();
		#endif

		#if PROFILECODE && PROFILE_STATES
		_delay_ms( 1 );
		DisableProfileSample4();
		#endif
	
		#if SIMPLESTATES
		Task_Next();
		continue;
		#endif

		// Check for sonar response
		if( CommuncationReceivedPING() )
		{
			bPINGRecieved = 1;		
		}
		else
		{
			// Check wireless response for a timeout
			if( CommunicationPINGTimeout() )
			{
				if( 0 == CommunicationPingUpdateRetryCount() )
				{
					SeekState = LOCALIZATIONSTATE_SendRequest;
				}
				else
				{
					SeekState = LOCALIZATIONSTATE_UpdateSweep;
				}
			}
		}

		#if PROFILECODE && PROFILE_STATES
		DisableProfileSample4();
		#endif
	
		Task_Next();
	}
}

void Ass2_Task_GetSonarResponse()
{
	for(;;)
    {
		// If we have not yet initialized, or if we are
		// reporting, skip executing this process/state
		if( (0 == InitializedSystem) || (0 != ReportingOutcome) )
		{
			Task_Next();
		}

		#if PROFILECODE && PROFILE_STATES
		EnableProfileSample5();
		#endif

		#if PROFILECODE && PROFILE_STATES
		_delay_ms( 1 );
		DisableProfileSample5();
		#endif
	
		#if SIMPLESTATES
		Task_Next();
		continue;
		#endif

		// Check for sonar response
		if( 0 != bPINGRecieved )
		{
		
			// According to the docs, the sonar device will 
			// internally timeout after 36ms anyways

			if( CommunicationReceivedSonarResponse() )
			{
				// Get the communication statistics and go to the
				// update statistics state
				CommunicationGetResponseStatistics( &ComStats );
			}
			else
			{
			// Sonar timed out, ignore sample and move on
			}
		}
	
		#if PROFILECODE && PROFILE_STATES
		DisableProfileSample5();
		#endif

		Task_Next();
	}
}

void Ass2_Task_UpdateStatistics()
{
	for(;;)
    {
		// If we have not yet initialized, or if we are
		// reporting, skip executing this process/state
		if( (0 == InitializedSystem) || (0 != ReportingOutcome) )
		{
			Task_Next();
		}

		#if PROFILECODE && PROFILE_STATES
		EnableProfileSample6();
		#endif

		#if PROFILECODE && PROFILE_STATES
		_delay_ms( 1 );
		DisableProfileSample6();
		#endif
	
		#if SIMPLESTATES
		Task_Next();
		continue;
		#endif

		// Add this new sample point to the statistics pool
		StatisticsAddNewSample( &ComStats );

		#if PROFILECODE && PROFILE_STATES
		DisableProfileSample6();
		#endif

		Task_Next();
	}
}

void Ass2_Task_CalculateAndReport()
{
	#if PROFILECODE && PROFILE_STATES
	EnableProfileSample7();
	#endif

	#if PROFILECODE && PROFILE_STATES
	_delay_ms( 1 );
	DisableProfileSample7();
	#endif


	#if PROFILECODE && PROFILE_CALCAULATERESULTS 
	EnableProfileSample1();
	EnableProfileSample2();
	#endif

	#if SIMPLESTATES
	return;
	#endif

	// Hack
	ComStats.Percent = 0;
	ServoSetDirection( &SonarDirectionServo, &ComStats );

	#if PROFILECODE && PROFILE_CALCAULATERESULTS 
	DisableProfileSample2();
	EnableProfileSample3();
	#endif

	// Get the shortest distance and the associated direction
	StatisticsCalculateDirectionAndDistance( &ComStats );

	#if PROFILECODE && PROFILE_CALCAULATERESULTS 
	DisableProfileSample3();
	DisableProfileSample1();
	#endif
			
	#if PROFILECODE && PROFILE_REPORT 
	EnableProfileSample1();
	EnableProfileSample2();
	#endif

	// Report the calculated beacon direction and distance
	ServoSetDirection( &SonarDirectionServo, &ComStats );

	#if PROFILECODE && PROFILE_REPORT 
	DisableProfileSample2();
	EnableProfileSample3();
	#endif

	CommunicationReportDistance( &ComStats );

	#if PROFILECODE && PROFILE_REPORT 
	DisableProfileSample3();
	#endif

	_delay_ms(100000);

	// Check for reset event
	// if( 1 == ResetEventOccured() )
	// {
	// 	SeekState = LOCALIZATIONSTATE_Initialization;
	// }

	#if PROFILECODE && PROFILE_REPORT 
	DisableProfileSample1();
	#endif	
}

int main(void)
{
	clock8MHz();

	#if PROFILECODE
	InitializeLogicAnalyzerProfiler();
	#endif

    // Set up the UART
	uart_init(UART_38400);
	snprintf((char*)ASS_UART_BUFF, ASS_UART_BUFF_MAXLINE, "HELLO WORLD: starts echoing\n\r");
	uart_putstr(ASS_UART_BUFF);
      
	Task_Create( Ass2_Task_InitSweepReport, LOCALIZATIONSTATE_Initialization, PERIODIC, LOCALIZATIONSTATE_Initialization); 
	Task_Create( Ass2_Task_SendRequest, LOCALIZATIONSTATE_SendRequest, PERIODIC, LOCALIZATIONSTATE_SendRequest);
	Task_Create( Ass2_Task_GetPINGResponse, LOCALIZATIONSTATE_GetPINGResponse, PERIODIC, LOCALIZATIONSTATE_GetPINGResponse);
	Task_Create( Ass2_Task_GetSonarResponse, LOCALIZATIONSTATE_GetSonarResponse, PERIODIC, LOCALIZATIONSTATE_GetSonarResponse);
	Task_Create( Ass2_Task_UpdateStatistics, LOCALIZATIONSTATE_UpdateStatistics, PERIODIC, LOCALIZATIONSTATE_UpdateStatistics);
}
