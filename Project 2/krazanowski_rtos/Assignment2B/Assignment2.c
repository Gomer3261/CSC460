/**
 * 	This code is implements a system/RR design for the 
 * 	indoor localization application required for assignment2 
 *  of CSC560 - RTOS.  State transitions are handled through 
 * 	the use of events and an event handling process.
 * 	All state transitions set a state request value, and the 
 * 	state machine process may handle the request.
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

// Initialization, UpdateSweep and Calculate_Report 
// all share the first timeslice

const unsigned int PT = 0;
const unsigned char PPP[] = {};

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

volatile int bPINGRecieved;

// Wireless state information
uint8_t station_addr[5] = {0x01, 0x17, 0x76, 0x97, 0x00 };
uint8_t remote_addr[5] = {0x98, 0x76, 0x54, 0x32, 0x10};
int SweepComplete = 0;

volatile int CurrentActiveState;

// Process callback function prototypes
void Ass2_Task_Initialization();
void Ass2_Task_UpdateSweep();
void Ass2_Task_SendRequest();
void Ass2_Task_GetPINGResponse();
void Ass2_Task_GetSonarResponse();
void Ass2_Task_UpdateStatistics();

void Ass2_Task_CalculateAndReport();


EVENT* state_complete_event;
EVENT* task_init_event;
EVENT* task_sweep_event;
EVENT* task_request_event;
EVENT* task_ping_event;
EVENT* task_sonar_event;
EVENT* task_statistics_event;
EVENT* task_report_event;


void Ass2_Task_Initialization()
{
	for(;;)
    {
		// Hand control back to the state machine
		// Event_Signal(state_complete_event);

		// Wait till this state is signal'd
		Event_Wait(task_init_event);

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

		// Request to move to the sweep state
		CurrentActiveState = LOCALIZATIONSTATE_UpdateSweep;
	}
}

void Ass2_Task_UpdateSweep()
{
	for(;;)
    {
		// Hand control back to the state machine
		// Event_Signal(state_complete_event);

		// Wait till this state is signal'd
		Event_Wait(task_sweep_event);

		#if PROFILECODE && PROFILE_STATES
		Profile2();
		#endif
	
		#if SIMPLESTATES
		CurrentActiveState = LOCALIZATIONSTATE_SendRequest;
		continue;
		#endif

		StatisticsResetSample( &ComStats );

		if( 0 == SweepComplete )
		{
			// Redirect the servo and attempt to communicate
			SweepComplete = ServoUpdateSweepPosition( &ComStats );
			ServoSetDirection( &SonarDirectionServo, &ComStats );
			CommunicationPingResetRetryCount();

			// Request to move to the send request state
			CurrentActiveState = LOCALIZATIONSTATE_SendRequest;
		}	
		else
		{
			// Request to move to the calculate/report state chain
			CurrentActiveState = LOCALIZATIONSTATE_CalcaulateResults;
		}
	}
}

void Ass2_Task_SendRequest()
{
	for(;;)
    {
		// Hand control back to the state machine
		// Event_Signal(state_complete_event);

		// Wait till this state is signal'd
		Event_Wait(task_request_event);

		#if PROFILECODE && PROFILE_STATES
		Profile3();
		#endif
	
		#if SIMPLESTATES
		CurrentActiveState = LOCALIZATIONSTATE_GetPINGResponse;
		continue;
		#endif

		// Send the request for a ping
		CommunicationSendTriggerMessage();
		bPINGRecieved = 0;

		// Request to move to the PING response state
		CurrentActiveState = LOCALIZATIONSTATE_GetPINGResponse;
	}
}

void Ass2_Task_GetPINGResponse()
{
	for(;;)
    {
		// Hand control back to the state machine
		// Event_Signal(state_complete_event);

		// Wait till this state is signal'd
		Event_Wait(task_ping_event);

		#if PROFILECODE && PROFILE_STATES
		Profile4();
		#endif	

		#if SIMPLESTATES
		CurrentActiveState = LOCALIZATIONSTATE_GetSonarResponse;
		continue;
		#endif

		// Check for sonar response
		if( CommuncationReceivedPING() )
		{
			bPINGRecieved = 1;
				
			// Request to move to the Sonar response state
			CurrentActiveState = LOCALIZATIONSTATE_GetSonarResponse;	
		}
		else
		{
			// Check wireless response for a timeout
			if( CommunicationPINGTimeout() )
			{
				if( 0 == CommunicationPingUpdateRetryCount() )
				{
					// Request to move to the [re]send [the] request state
					CurrentActiveState = LOCALIZATIONSTATE_SendRequest;
				}
				else
				{
					// Request to move to the update sweep state
					CurrentActiveState = LOCALIZATIONSTATE_UpdateSweep;
				}
			}
		}
	}
}

void Ass2_Task_GetSonarResponse()
{
	for(;;)
    {
		// Hand control back to the state machine
		// Event_Signal(state_complete_event);

		// Wait till this state is signal'd
		Event_Wait(task_sonar_event);

		#if PROFILECODE && PROFILE_STATES
		Profile5();
		#endif

		#if SIMPLESTATES
		CurrentActiveState = LOCALIZATIONSTATE_UpdateStatistics;
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

				// Request to move to the update statistics state
				CurrentActiveState = LOCALIZATIONSTATE_UpdateStatistics;
			}
			else
			{
				// Sonar timed out, ignore sample and move on
				CurrentActiveState = LOCALIZATIONSTATE_UpdateSweep;
			}
		}
		else
		{
			// This should never get hit
			CurrentActiveState = LOCALIZATIONSTATE_UpdateSweep;
		}
	}
}

void Ass2_Task_UpdateStatistics()
{
	for(;;)
    {
		// Hand control back to the state machine
		// Event_Signal(state_complete_event);

		// Wait till this state is signal'd
		Event_Wait(task_statistics_event);

		#if PROFILECODE && PROFILE_STATES
		Profile6();
		#endif
	
		#if SIMPLESTATES
		CurrentActiveState = LOCALIZATIONSTATE_UpdateSweep;
		continue;
		#endif

		// Add this new sample point to the statistics pool
		StatisticsAddNewSample( &ComStats );

		#if PROFILECODE && PROFILE_STATES
		DisableProfileSample6();
		#endif

		// Request to move to the update statistics state
		CurrentActiveState = LOCALIZATIONSTATE_UpdateSweep;
	}
}

void Ass2_Task_CalculateAndReport()
{
	for(;;)
    {
		// Hand control back to the state machine
		// Event_Signal(state_complete_event);

		// Wait till this state is signal'd
		Event_Wait(task_report_event);

		#if PROFILECODE && PROFILE_STATES
		Profile7();
		#endif

		#if SIMPLESTATES
		// Event_Signal(state_complete_event);
		continue;
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

		// Check for reset event
		// if( 1 == ResetEventOccured() )
		// {
		// 		// Request to move to the initialization state
		//		CurrentActiveState = LOCALIZATIONSTATE_Initialization;
		// }

		#if PROFILECODE && PROFILE_REPORT 
		DisableProfileSample1();
		#endif	
	}
}

void Ass2_StateMachine()
{
	// The delays defined below are set in part to ensure timing requirements
	// for devices that have a completion latency.  They are also defined in 
	// part to match the values defined to match the periodic implementation 
	// of this assignment (2).
	const int TICKSCALAR = 5;

	for(;;)
	{
		switch( CurrentActiveState )
		{
			case LOCALIZATIONSTATE_Initialization:
				Event_Signal( task_init_event );
				break;
			case LOCALIZATIONSTATE_UpdateSweep:
				// Delay required to allow initialization to 
				// settle before everything continues
				_delay_ms( 200 * TICKSCALAR );	
				Event_Signal( task_sweep_event );
				break;
			case LOCALIZATIONSTATE_SendRequest:
				// Delay required to allow servo to finish 
				// moving.  I gave 140ms delay in other implementations.
				_delay_ms( 140*TICKSCALAR );
				Event_Signal( task_request_event );
				break;
			case LOCALIZATIONSTATE_GetPINGResponse:
				// Radio response delay
				_delay_ms( 10*TICKSCALAR );
				Event_Signal( task_ping_event );
				break;
			case LOCALIZATIONSTATE_GetSonarResponse:
				// The sonar transceiver claims a 36ms max timeout
				// for a sonic ping response.  
				_delay_ms( 40*TICKSCALAR );
				Event_Signal( task_sonar_event );
				break;
			case LOCALIZATIONSTATE_UpdateStatistics:
				// Just to keep it consistent with other implementations.  
				_delay_ms( 10*TICKSCALAR );
				Event_Signal( task_statistics_event );
				break;

		};

		// Yield till the active state is done
		Task_Next();
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

	state_complete_event = Event_Init();
	task_init_event = Event_Init();
    task_sweep_event = Event_Init();
	task_request_event = Event_Init();
	task_ping_event = Event_Init();
	task_sonar_event = Event_Init();
	task_statistics_event = Event_Init();
	task_report_event = Event_Init();

	// 0 == undefined
	CurrentActiveState = LOCALIZATIONSTATE_Initialization;
	  
	Task_Create( Ass2_StateMachine, 100, RR, 0); 
	Task_Create( Ass2_Task_Initialization, LOCALIZATIONSTATE_Initialization, SYSTEM, LOCALIZATIONSTATE_Initialization); 
	Task_Create( Ass2_Task_UpdateSweep, LOCALIZATIONSTATE_UpdateSweep, RR, LOCALIZATIONSTATE_UpdateSweep); 
	Task_Create( Ass2_Task_SendRequest, LOCALIZATIONSTATE_SendRequest, RR, LOCALIZATIONSTATE_SendRequest);
	Task_Create( Ass2_Task_GetPINGResponse, LOCALIZATIONSTATE_GetPINGResponse, RR, LOCALIZATIONSTATE_GetPINGResponse);
	Task_Create( Ass2_Task_GetSonarResponse, LOCALIZATIONSTATE_GetSonarResponse, RR, LOCALIZATIONSTATE_GetSonarResponse);
	Task_Create( Ass2_Task_UpdateStatistics, LOCALIZATIONSTATE_UpdateStatistics, RR, LOCALIZATIONSTATE_UpdateStatistics);
	Task_Create( Ass2_Task_CalculateAndReport, LOCALIZATIONSTATE_CalcaulateResults, SYSTEM, LOCALIZATIONSTATE_CalcaulateResults); 
}

