#include "Ass1_globals.h"
#include <avr/interrupt.h>
#include "radio/radio.h"
#include "sonar/sonar.h"
#include "CommunicationStats.h"

#include "uart.h"

volatile int TimeoutCounter;
volatile uint8_t PINGReceivedFlag;
volatile uint8_t SONARReceivedFlag;

volatile radiopacket_t packet;

volatile CommunicationStatStruct WorkingStruct;

volatile uint8_t station_addr[5];
volatile uint8_t remote_addr[5];

static volatile uint16_t sent_time = 0;
static volatile uint16_t received_time = 0;

static volatile uint16_t message_clock = 0;

void ResetMessageClock()
{
	message_clock = 0;

	// Reset the hardware clock
}

uint16_t GetMessageClock()
{
	// Get the hardware clock

	return message_clock;
}

void InitializeCommunication(  uint8_t sa[5], uint8_t ra[5] )
{
	TimeoutCounter = 0;

	WorkingStruct.TriggerSent = 0;
	WorkingStruct.PingResponseReceived = 0;
	WorkingStruct.SonarReceived = 0;

	WorkingStruct.PingReceiveTime = 0;
	WorkingStruct.SonarReceiveTime = 0;

	WorkingStruct.Percent = 0;

	remote_addr[0] = ra[0];
	remote_addr[1] = ra[1];
	remote_addr[2] = ra[2];
	remote_addr[3] = ra[3];
	remote_addr[4] = ra[4];

	station_addr[0] = sa[0];
	station_addr[1] = sa[1];
	station_addr[2] = sa[2];
	station_addr[3] = sa[3];
	station_addr[4] = sa[4];
	
	cli();

	Radio_Init();
	Radio_Configure_Rx( RADIO_PIPE_0, (uint8_t*)station_addr, ENABLE );
	Radio_Configure( RADIO_2MBPS, RADIO_HIGHEST_POWER );

	sonar_init();
	
	// enable interrupts
	sei();

	_delay_ms(500);
}

void CommunicationSendTriggerMessage()
{
	PINGReceivedFlag = 0;
	SONARReceivedFlag = 0;

	// disable interrupts during setup

	packet.type = COMMAND;
	packet.payload.command.command = Trigger;
	packet.payload.command.sender_address[0] = station_addr[0];
	packet.payload.command.sender_address[1] = station_addr[1];
	packet.payload.command.sender_address[2] = station_addr[2];
	packet.payload.command.sender_address[3] = station_addr[3];
	packet.payload.command.sender_address[4] = station_addr[4];

	Radio_Set_Tx_Addr( (uint8_t*)remote_addr );
	// uint8_t result = Radio_Transmit( (radiopacket_t*)(&packet), RADIO_WAIT_FOR_TX);
	Radio_Transmit( (radiopacket_t*)(&packet), RADIO_WAIT_FOR_TX);

	ResetMessageClock();
}

int CommunicationPINGTimeout()
{
	if( 0 == PINGReceivedFlag )
	{
		if( GetMessageClock() > 20000 )
		{
			return 1;
		}
	}
	  
	return 0;  /// Currently wait indefinately, todo: add a timeout
}

void CommunicationPingResetRetryCount()
{
	TimeoutCounter = 0;
}

#define PINGRETRYCOUNT 0

int CommunicationPingUpdateRetryCount()
{
	if( TimeoutCounter<=PINGRETRYCOUNT )
	{
		TimeoutCounter++;
		return 0;
	}

	return 1;
}

int CommuncationReceivedPING()
{
	int Received = PINGReceivedFlag;
	if( 0 != PINGReceivedFlag )
	{
		PINGReceivedFlag = 0;
		SONARReceivedFlag = 0;
		
		radiopacket_t ReceivePacket;
		RADIO_RX_STATUS status = Radio_Receive( &ReceivePacket );

		// Set the current time
		// sonar_reset_trigger_time();
		sonar_trigger();

		_delay_ms(40);
	}

	return Received;
}

int CommunicationReceivedSonarResponse()
{
	if( 0 != sonar_echo_received() )
	{
		SONARReceivedFlag = 1;
	}

	return SONARReceivedFlag;
}

void CommunicationGetResponseStatistics( CommunicationStatStruct *CommStat )
{ 
	
	CommStat->PingReceiveTime = sonar_get_ping_time();
	CommStat->SonarReceiveTime = sonar_get_sonar_time();
	CommStat->Distance = sonar_get_ping_time(); 

	#if 1
	char buf[81];
	snprintf(buf, 81, "R: %u (%u) [%u, %u]\n\r", sonar_get_distance(), (int)CommStat->Distance, sonar_get_ping_time(), sonar_get_sonar_time() );
	uart_putstr(buf);
	#endif
}

void CommunicationReportDistance( CommunicationStatStruct *CommStat )
{
	char output[128];
	float ActualDistance = (340.0 * CommStat->Distance/1000.0)/10.0 - 0.8192;
	snprintf((char*)output, 128, "distance %u (%u)\n\r", (unsigned int)ActualDistance, (unsigned int)CommStat->Distance);
	uart_putstr(output);
}


/**
 * This function is a hook into the radio's ISR.  It is called whenever the radio generates an RX_DR (received data ready) interrupt.
 */
void radio_rxhandler(uint8_t pipenumber)
{
	PINGReceivedFlag = 1;
	SONARReceivedFlag = 0;

	ResetMessageClock();
}
