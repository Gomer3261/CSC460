
#include "CommunicationStats.h"

void StatisticsInitialize();
void StatisticsResetSample( CommunicationStatStruct *CommStat );
void StatisticsAddNewSample( CommunicationStatStruct *CommStat );
void StatisticsCalculateDirectionAndDistance( CommunicationStatStruct *CommStat );
