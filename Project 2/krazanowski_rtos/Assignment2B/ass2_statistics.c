
#include "Ass2_globals.h"
#include "CommunicationStats.h"
#include "uart.h"

static volatile CommunicationStatStruct ShortestPath;

void StatisticsInitialize()
{
	// Set the distance to FAR AWAY
	ShortestPath.Distance = 0x8FFF;

	// Set the default angle to the mid-range
	ShortestPath.Percent = 500;
}

void StatisticsResetSample( CommunicationStatStruct *CommStat )
{
	CommStat->TriggerSent = 0;
	CommStat->PingResponseReceived = 0;
	CommStat->SonarReceived = 0;

	CommStat->PingReceiveTime = 0;
	CommStat->SonarReceiveTime = 0;
	CommStat->Distance = 0x8FFF;

	CommStat->Percent = 500;
}

void StatisticsAddNewSample( CommunicationStatStruct *CommStat )
{
	#if 1
	snprintf((char*)ASS_UART_BUFF, ASS_UART_BUFF_MAXLINE, "Here: %u %u\n\r", (unsigned int)CommStat->Distance, (unsigned int)ShortestPath.Distance );
	uart_putstr( ASS_UART_BUFF );
	#endif

	// Check if this result is closer
	if( (unsigned int)CommStat->Distance < (unsigned int)ShortestPath.Distance )
	{	
		snprintf((char*)ASS_UART_BUFF, ASS_UART_BUFF_MAXLINE, "SU: %u, %u, %u, %u\n\r", (int)CommStat->Distance, (int)CommStat->Percent, (unsigned int)ShortestPath.Distance, (int)ShortestPath.Percent );
		uart_putstr( ASS_UART_BUFF );

		// If it is closer, set this 
		// as the current distance
		ShortestPath.Distance = CommStat->Distance;
		ShortestPath.Percent = CommStat->Percent;
	}
}

void StatisticsCalculateDirectionAndDistance( CommunicationStatStruct *CommStat )
{
	CommStat->Distance = ShortestPath.Distance;
	CommStat->Percent = ShortestPath.Percent;
}
