
#include "CommunicationStats.h"

static volatile CommunicationStatStruct ShortestPath;

void StatisticsInitialize()
{
	// Set the distance to FAR AWAY
	ShortestPath.Distance = 100000;

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
	CommStat->Distance = 1000000;

	CommStat->Percent = 500;
}

void StatisticsAddNewSample( CommunicationStatStruct *CommStat )
{
	char buf[81];

	#if 0
	snprintf( buf, 81, "Here: %u %u\n\r", (unsigned int)CommStat->Distance, (unsigned int)ShortestPath.Distance );
	uart_putstr( buf );
	#endif

	// Check if this result is closer
	if( (unsigned int)CommStat->Distance < (unsigned int)ShortestPath.Distance )
	{	
		snprintf( buf, 81, "SU: %u, %u, %u, %u\n\r", (int)CommStat->Distance, (int)CommStat->Percent, (unsigned int)ShortestPath.Distance, (int)ShortestPath.Percent );
		uart_putstr( buf );

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
