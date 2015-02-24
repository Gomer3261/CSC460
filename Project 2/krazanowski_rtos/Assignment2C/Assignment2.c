/**
 * @file   test010.c
 * @author Scott Craig and Justin Tanner
 * @date   Mon Oct 29 16:19:32 2007
 * 
 * @brief  Test 010 - Many things at once, event's, RR's and Periodic's
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

#define SIMPLESTATES 0

#define TASKPERIOD 1

#define PERIODSCALE 2

const unsigned int PT = 5;
const unsigned char PPP[] = {LOCALIZATIONSTATE_UpdateSweep, 140*PERIODSCALE, 
								LOCALIZATIONSTATE_SendRequest, 2*PERIODSCALE, 
								LOCALIZATIONSTATE_GetPINGResponse, 6*PERIODSCALE, 
								LOCALIZATIONSTATE_GetSonarResponse, 12*PERIODSCALE,
								LOCALIZATIONSTATE_UpdateStatistics, 40*PERIODSCALE };

volatile int ReportingOutcome = 0;

// uint8_t big_int = 200;
// EVENT* test_event;
EVENT* print_event;

#define UART_OUTPUT_STATE 0
#define     clock8MHz()    CLKPR = _BV(CLKPCE); CLKPR = 0x00;

unsigned char ASS_UART_BUFF[ASS_UART_BUFF_MAXLINE];

ServoControlStruct SonarDirectionServo;
CommunicationStatStruct ComStats;
volatile int SeekState = LOCALIZATIONSTATE_Initialization;

volatile int bPINGRecieved;

uint8_t station_addr[5] = {0x01, 0x17, 0x76, 0x97, 0x00 };
uint8_t remote_addr[5] = {0x98, 0x76, 0x54, 0x32, 0x10};
int SweepComplete = 0;

void Ass2_Task_Initialization();
void Ass2_Task_UpdateSweep();
void Ass2_Task_SendRequest();
void Ass2_Task_GetPINGResponse();
void Ass2_Task_GetSonarResponse();
void Ass2_Task_UpdateStatistics();

void Ass2_Task_CalculateAndReport();

void Ass2_Task_Initialization()
{
	#if PROFILECODE && PROFILE_STATES
	EnableProfileSample1();
	#endif

	/*
	#if PROFILECODE && PROFILE_INITIALIZATION 
	EnableProfileSample1();
	#endif
	*/
	
	/*
	#if PROFILECODE && PROFILE_INITIALIZATION 
	EnableProfileSample2();
	// Initialize all necessary states
	InitializeServoController( &SonarDirectionServo );
	DisableProfileSample2();
	EnableProfileSample3();
	InitializeCommunication( station_addr, remote_addr );
	DisableProfileSample3();
	EnableProfileSample4();
	StatisticsInitialize();
	DisableProfileSample4();

	EnableProfileSample5();
	ServoStartSweep();
	DisableProfileSample5();
	SweepComplete = 0;

	#else
	*/

	int sreg = SREG;
	cli();

	
	// Initialize all necessary states
	InitializeServoController( &SonarDirectionServo );
	InitializeCommunication( station_addr, remote_addr );
	StatisticsInitialize();

	ServoStartSweep();
	SweepComplete = 0;
	
	/*
	#endif
	*/
	Task_Create( Ass2_Task_UpdateSweep, LOCALIZATIONSTATE_UpdateSweep, PERIODIC, LOCALIZATIONSTATE_UpdateSweep); 
	Task_Create( Ass2_Task_SendRequest, LOCALIZATIONSTATE_SendRequest, PERIODIC, LOCALIZATIONSTATE_SendRequest);
	Task_Create( Ass2_Task_GetPINGResponse, LOCALIZATIONSTATE_GetPINGResponse, PERIODIC, LOCALIZATIONSTATE_GetPINGResponse);
	Task_Create( Ass2_Task_GetSonarResponse, LOCALIZATIONSTATE_GetSonarResponse, PERIODIC, LOCALIZATIONSTATE_GetSonarResponse);
	Task_Create( Ass2_Task_UpdateStatistics, LOCALIZATIONSTATE_UpdateStatistics, PERIODIC, LOCALIZATIONSTATE_UpdateStatistics);

	SREG = sreg;

	/*
	#if PROFILECODE && PROFILE_INITIALIZATION 
	DisableProfileSample1();
	#endif
	*/

	#if PROFILECODE && PROFILE_STATES
	DisableProfileSample1();
	#endif
}

void Ass2_Task_UpdateSweep()
{
	for(;;)
    {
		#if PROFILECODE && PROFILE_STATES
		EnableProfileSample2();
		#endif

		#if PROFILECODE && PROFILE_STATES
		_delay_ms( 1 );
		DisableProfileSample2();
		#endif
	
		#if SIMPLESTATES
		Task_Next();
		continue;
		#endif

		/*
		#if PROFILECODE && PROFILE_UPDATESWEEP 
		EnableProfileSample1();
		#endif
		*/

		StatisticsResetSample( &ComStats );

		if( 0 == SweepComplete )
		{
			/*
			#if PROFILECODE && PROFILE_UPDATESWEEP
			// Redirect the servo and attempt to communicate
			EnableProfileSample2();
			SweepComplete = ServoUpdateSweepPosition( &ComStats );
			DisableProfileSample2();
			EnableProfileSample3();
			ServoSetDirection( &SonarDirectionServo, &ComStats );
			DisableProfileSample3();
			EnableProfileSample4();
			CommunicationPingResetRetryCount();
			DisableProfileSample4();
			#else
			*/
			// Redirect the servo and attempt to communicate
			SweepComplete = ServoUpdateSweepPosition( &ComStats );
			ServoSetDirection( &SonarDirectionServo, &ComStats );
			CommunicationPingResetRetryCount();
			/*
			#endif
			*/
		}	
		else
		{
			// Set process to REPORTING
			ReportingOutcome = 1;
		}

		// If we are done with the periodic states
		// return so this process can be killed
		if( ReportingOutcome )
		{
			Task_Create( Ass2_Task_CalculateAndReport, LOCALIZATIONSTATE_Report, RR, LOCALIZATIONSTATE_Report); 
	
			return;
		}
	
		/*
		#if PROFILECODE && PROFILE_UPDATESWEEP 
		DisableProfileSample1();
		#endif
		*/

		#if PROFILECODE && PROFILE_STATES
		DisableProfileSample2();
		#endif

		Task_Next();
	}
}

void Ass2_Task_SendRequest()
{
	for(;;)
    {
		// If we are done with the periodic states
		// return so this process can be killed
		if( ReportingOutcome )
		{
			return;
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

		/*
		#if PROFILECODE && PROFILE_SENDREQUEST 
		EnableProfileSample1();
		#endif
		*/

		/*
		#if PROFILECODE && PROFILE_SENDREQUEST
		EnableProfileSample2();
		#endif

		#if PROFILECODE && PROFILE_AVR 
		EnableProfileSample1();  // Needs to be disabled in the call
		#endif
		*/
		// Send the request for a ping
		CommunicationSendTriggerMessage();
		bPINGRecieved = 0;
		/*
		#if PROFILECODE && PROFILE_SENDREQUEST
		DisableProfileSample2();
		DisableProfileSample1();
		#endif
		*/

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
		// If we are done with the periodic states
		// return so this process can be killed
		if( ReportingOutcome )
		{
			return;
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

		/*
		#if PROFILECODE && PROFILE_GETRESPONSE 
		EnableProfileSample1();
		#endif
		*/

		
		/*
		#if PROFILECODE && PROFILE_GETRESPONSE 
		EnableProfileSample8();
		#endif
		*/

		// Check for sonar response
		if( CommuncationReceivedPING() )
		{
			bPINGRecieved = 1;		
		}
		else
		{
			/*
			#if PROFILECODE && PROFILE_AVR 
			EnableProfileSample1();  // Needs to be disabled in the call
			#endif
			*/

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

		/*
		#if PROFILECODE && PROFILE_GETRESPONSE 
		DisableProfileSample8();
		#endif

		#if PROFILECODE && PROFILE_GETRESPONSE 
		DisableProfileSample1();
		#endif
		*/

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
		// If we are done with the periodic states
		// return so this process can be killed
		if( ReportingOutcome )
		{
			return;
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

		/*
		#if PROFILECODE && PROFILE_GETRESPONSE 
		EnableProfileSample1();
		#endif
		*/

		// Check for sonar response
		if( 0 != bPINGRecieved )
		{
		
			// According to the docs, the sonar device will 
			// internally timeout after 36ms anyways

			if( CommunicationReceivedSonarResponse() )
			{
				/*
				#if PROFILECODE && PROFILE_GETRESPONSE 
				EnableProfileSample2();
				#endif
				*/
				// Get the communication statistics and go to the
				// update statistics state
				CommunicationGetResponseStatistics( &ComStats );
				/*
				#if PROFILECODE && PROFILE_GETRESPONSE 
				DisableProfileSample2();
				#endif
				*/
			}
			else
			{
			// Sonar timed out, ignore sample and move on
			}
		}
	
		/*
		#if PROFILECODE && PROFILE_GETRESPONSE 
		DisableProfileSample1();
		#endif
		*/

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
		// If we are done with the periodic states
		// return so this process can be killed
		if( ReportingOutcome )
		{
			return;
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

		/*
		#if PROFILECODE && PROFILE_UPDATESTATISTICS 
		EnableProfileSample1();
		#endif
		*/

		/*
		#if PROFILECODE && PROFILE_UPDATESTATISTICS 
		EnableProfileSample2();
		#endif
		*/

		// Add this new sample point to the statistics pool
		StatisticsAddNewSample( &ComStats );

		/*
		#if PROFILECODE && PROFILE_UPDATESTATISTICS 
		DisableProfileSample2();
		DisableProfileSample1();
		#endif
		*/

		#if PROFILECODE && PROFILE_STATES
		DisableProfileSample6();
		#endif

		Task_Next();
	}
}

void Ass2_Task_CalculateAndReport()
{
	for(;;)
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
      
	// Ass2_Task_Initialization();
	Task_Create( Ass2_Task_Initialization, LOCALIZATIONSTATE_Initialization, SYSTEM, LOCALIZATIONSTATE_Initialization); 
	
}
