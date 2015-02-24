
#ifndef COMMUNICATIONSTATSTRUCT_H__
#define COMMUNICATIONSTATSTRUCT_H__

#include <avr/interrupt.h>

typedef struct CommunicationStatStruct
{
	uint8_t TriggerSent;
	uint8_t PingResponseReceived;
	uint8_t SonarReceived;

	int PingReceiveTime;
	int SonarReceiveTime;
	int Distance;

	int Percent;
}
CommunicationStatStruct;

#endif // COMMUNICATIONSTATSTRUCT_H__
