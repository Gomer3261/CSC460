#include <avr/io.h>

#include "Assignment1.h"

#include "Ass1_Servo.h"
#include "Ass1_Communicate.h"
#include "Ass1_Statistics.h"

#include "uart.h"				// routines for UART comms.

int ResetEventOccured();

#define MAXLINE 81

volatile LocalizationState SeekState;

#define UART_OUTPUT_STATE 0
#define     clock8MHz()    CLKPR = _BV(CLKPCE); CLKPR = 0x00;

int main(void)
{
	char buf[MAXLINE];

	clock8MHz();

	#if PROFILECODE 
	InitializeLogicAnalyzerProfiler();
	#endif

	ServoControlStruct SonarDirectionServo;
	CommunicationStatStruct ComStats;
	SeekState = LOCALIZATIONSTATE_Initialization;

	uint8_t station_addr[5] = {0x01, 0x17, 0x76, 0x97, 0x00 };
	uint8_t remote_addr[5] = {0x98, 0x76, 0x54, 0x32, 0x10};

	int SweepComplete = 0;

	uart_init(UART_38400);

	snprintf((char*)buf, MAXLINE, "HELLO WORLD: starts echoing\n\r");
	uart_putstr(buf);

	while (1)
	{
		#if UART_OUTPUT_STATE
		snprintf((char*)buf, MAXLINE, "State: %u\n\r", SeekState);
		uart_putstr(buf);
		#endif

		// Switch on the current state
		switch( SeekState )
		{
			// State for initialize all necessary application
			// and hardware states
			case LOCALIZATIONSTATE_Initialization:
				{
					#if PROFILECODE && PROFILE_INITIALIZATION 
					EnableProfileSample0();
					#endif

					#if UART_OUTPUT_STATE
					snprintf((char*)buf, MAXLINE, "I\n\r");
					uart_putstr(buf);
					#endif

					#if PROFILECODE && PROFILE_INITIALIZATION 
					EnableProfileSample1();
					// Initialize all necessary states
					InitializeServoController( &SonarDirectionServo );
					DisableProfileSample1();
					EnableProfileSample2();
					InitializeCommunication( station_addr, remote_addr );
					DisableProfileSample2();
					EnableProfileSample3();
					StatisticsInitialize();
					DisableProfileSample3();

					EnableProfileSample4();
					ServoStartSweep();
					DisableProfileSample4();
					SweepComplete = 0;

					#else
					// Initialize all necessary states
					InitializeServoController( &SonarDirectionServo );
					InitializeCommunication( station_addr, remote_addr );
					StatisticsInitialize();

					ServoStartSweep();
					SweepComplete = 0;
					#endif

					// Then set the state machine into the sweep state mode
					SeekState = LOCALIZATIONSTATE_UpdateSweep;

					#if PROFILECODE && PROFILE_INITIALIZATION 
					DisableProfileSample0();
					#endif
				}
				break;
			// Set the servo position and check to see if the sweep
			// is complete or not
			case LOCALIZATIONSTATE_UpdateSweep:
				{
					#if PROFILECODE && PROFILE_UPDATESWEEP 
					EnableProfileSample0();
					#endif

					StatisticsResetSample( &ComStats );

					if( 0 == SweepComplete )
					{
						// Redirect the servo and attempt to communicate
						SweepComplete = ServoUpdateSweepPosition( &ComStats );
						ServoSetDirection( &SonarDirectionServo, &ComStats );
						CommunicationPingResetRetryCount();

						SeekState = LOCALIZATIONSTATE_SendRequest;
					}
					else
					{
						// If done sweeping for a signal
						// go to the calculate results state
						SeekState = LOCALIZATIONSTATE_CalcaulateResults;
					}	
					
					#if UART_OUTPUT_STATE
					snprintf(buf, MAXLINE, "S: %d\n\r", ComStats.Percent);
					uart_putstr(buf);				
					#endif

					#if PROFILECODE && PROFILE_UPDATESWEEP 
					DisableProfileSample0();
					#endif
				}
				break;
			case LOCALIZATIONSTATE_SendRequest:
				{
					#if PROFILECODE && PROFILE_SENDREQUEST 
					EnableProfileSample0();
					#endif

					#if UART_OUTPUT_STATE
					snprintf(buf, MAXLINE, "T\n\r");
					uart_putstr(buf);
					#endif

					// Send the request for a ping
					CommunicationSendTriggerMessage();
					SeekState = LOCALIZATIONSTATE_GetResponse;

					#if UART_OUTPUT_STATE
					snprintf(buf, MAXLINE, "R\n\r");
					uart_putstr(buf);
					#endif

					#if PROFILECODE && PROFILE_SENDREQUEST 
					DisableProfileSample0();
					#endif
				}
				break;
			case LOCALIZATIONSTATE_GetResponse:
				{
					#if PROFILECODE && PROFILE_GETRESPONSE 
					EnableProfileSample0();
					#endif

					#if UART_OUTPUT_STATE
					snprintf(buf, MAXLINE, "R\n\r");
					uart_putstr(buf);
					#endif

					// Check for sonar response
					if( CommuncationReceivedPING() )
					{
						// According to the docs, the sonar device will 
						// internally timeout after 36ms anyways

						if( CommunicationReceivedSonarResponse() )
						{
							#if UART_OUTPUT_STATE
							snprintf(buf, MAXLINE, "S F\n\r");
							uart_putstr(buf);
							#endif
						
							// Get the communication statistics and go to the
							// update statistics state
							CommunicationGetResponseStatistics( &ComStats );
							SeekState = LOCALIZATIONSTATE_UpdateStatistics;
						}
						else
						{
							#if UART_OUTPUT_STATE
							snprintf(buf, MAXLINE, "S TO\n\r");
							uart_putstr(buf);
							#endif

								
							// Sonar timed out, ignore sample and move on
							SeekState = LOCALIZATIONSTATE_UpdateSweep;
						}

						_delay_ms(300);
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

					#if PROFILECODE && PROFILE_GETRESPONSE 
					DisableProfileSample0();
					#endif
				}
				break;
			case LOCALIZATIONSTATE_UpdateStatistics:
				{
					#if PROFILECODE && PROFILE_UPDATESTATISTICS 
					EnableProfileSample0();
					#endif

					#if UART_OUTPUT_STATE
					snprintf(buf, MAXLINE, "U %u\n\r", ComStats.Distance);
					uart_putstr(buf);
					#endif

					// Add this new sample point to the statistics pool
					StatisticsAddNewSample( &ComStats );
					SeekState = LOCALIZATIONSTATE_UpdateSweep;

					#if PROFILECODE && PROFILE_UPDATESTATISTICS 
					DisableProfileSample0();
					#endif
				}
				break;
			case LOCALIZATIONSTATE_CalcaulateResults:
				{
					#if PROFILECODE && PROFILE_CALCAULATERESULTS 
					EnableProfileSample0();
					#endif

					#if UART_OUTPUT_STATE
					snprintf(buf, MAXLINE, "C\n\r");
					uart_putstr(buf);
					#endif

					// Hack
					ComStats.Percent = 0;
					ServoSetDirection( &SonarDirectionServo, &ComStats );

					// Get the shortest distance and the associated direction
					StatisticsCalculateDirectionAndDistance( &ComStats );
					SeekState = LOCALIZATIONSTATE_Report;

					#if PROFILECODE && PROFILE_CALCAULATERESULTS 
					DisableProfileSample0();
					#endif
				}
				break;
			case LOCALIZATIONSTATE_Report:
				{
					#if PROFILECODE && PROFILE_REPORT 
					EnableProfileSample0();
					#endif

					#if UART_OUTPUT_STATE
					snprintf(buf, MAXLINE, "O\n\r");
					uart_putstr(buf);
					#endif

					// Report the calculated beacon direction and distance
					ServoSetDirection( &SonarDirectionServo, &ComStats );
					CommunicationReportDistance( &ComStats );

					_delay_ms(100000);

					// Check for reset event
					if( 1 == ResetEventOccured() )
					{
					// 	SeekState = LOCALIZATIONSTATE_Initialization;
					}

					#if PROFILECODE && PROFILE_REPORT 
					DisableProfileSample0();
					#endif
				}
				break;
			default:
				SeekState = LOCALIZATIONSTATE_Initialization;
				break;
		}
	}
}

int ResetEventOccured()
{
	return 0;
}


