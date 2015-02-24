#define F_CPU 8000000UL

#include <avr\io.h>
#include <util\delay.h>

typedef enum
{
		LOCALIZATIONSTATE_Initialization = 0,
		LOCALIZATIONSTATE_UpdateSweep,
		LOCALIZATIONSTATE_SendRequest,
		LOCALIZATIONSTATE_GetResponse,
		LOCALIZATIONSTATE_UpdateStatistics,
		LOCALIZATIONSTATE_CalcaulateResults,
		LOCALIZATIONSTATE_Report
}
LocalizationState;
