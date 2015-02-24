#ifndef __ASSIGNMENT2_H__
#define __ASSIGNMENT2_H__

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr\io.h>
#include <util\delay.h>

typedef enum
{
		LOCALIZATIONSTATE_Initialization = 1,
		LOCALIZATIONSTATE_UpdateSweep,
		LOCALIZATIONSTATE_SendRequest,
		LOCALIZATIONSTATE_GetPINGResponse,
		LOCALIZATIONSTATE_GetSonarResponse,
		LOCALIZATIONSTATE_UpdateStatistics,
		LOCALIZATIONSTATE_CalcaulateResults,
		LOCALIZATIONSTATE_Report
}
LocalizationState;

#endif // __ASSIGNMENT2_H__
