#include <avr\io.h>
#include <avr\interrupt.h>

typedef struct ServoControlStruct
{
	int PWMPeriod;
	int PWMDutyCycle;
	int PWMHighTime;

	int Value;

	int LowThreshold;
	int HighThreshold;
	int DeltaThreshold;

	int LowAngle;
	int HighAngle;
}
ServoControlStruct;

void ServoInitialize( ServoControlStruct *SCS );
void SetServoPeriod( ServoControlStruct *SCS, int MicrosecondPeriod );
void SetServoRange( ServoControlStruct *SCS, int Low, int High );
void SetServoValue( ServoControlStruct *SCS, int TenthPercent );
void AddServoValueDelta( ServoControlStruct *SCS, int TenthPercent );
void SubtractServoValueDelta( ServoControlStruct *SCS, int TenthPercent );
