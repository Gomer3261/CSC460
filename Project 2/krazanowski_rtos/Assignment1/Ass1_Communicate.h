#include "Ass1_globals.h"
#include "CommunicationStats.h"

void InitializeCommunication( uint8_t station_addr[5], uint8_t remote_addr[5] );
void CommunicationSendTriggerMessage();
int CommunicationPINGTimeout();
int CommunicationPingRetryCount();
void CommunicationPingResetRetryCount();
int CommunicationPingUpdateRetryCount();
int CommuncationReceivedPING();
int CommunicationReceivedSonarResponse();
void CommunicationGetResponseStatistics( CommunicationStatStruct *CommStat );
void CommunicationReportDistance( CommunicationStatStruct *CommStat );
