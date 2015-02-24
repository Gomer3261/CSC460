#include "Ass2_globals.h"
#include "ServoController.h"
#include "CommunicationStats.h"

void InitializeServoController( ServoControlStruct *ServoController );
void ServoStartSweep();
int ServoUpdateSweepPosition( CommunicationStatStruct *CommStat );
void ServoSetDirection(  ServoControlStruct *ServoController, CommunicationStatStruct *CommStat );
