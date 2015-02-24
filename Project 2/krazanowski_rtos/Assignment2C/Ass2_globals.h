#include "assignment2.h"

#define ASS_UART_BUFF_MAXLINE 81
extern unsigned char ASS_UART_BUFF[];

// This list must be mutually exclusive 
// to ensure the profile results are correct
#define PROFILECODE 1
#define PROFILE_STATES 1
#define PROFILE_INITIALIZATION 0
#define PROFILE_UPDATESWEEP  0
#define PROFILE_SENDREQUEST  0
#define PROFILE_GETRESPONSE  0
#define PROFILE_UPDATESTATISTICS  0
#define PROFILE_CALCAULATERESULTS  0
#define PROFILE_REPORT  0
#define PROFILE_SERVOS  0
#define PROFILE_WIRELESS  0
#define PROFILE_ULTRASOUND  0
#define PROFILE_UART  1
#define PROFILE_AVR  0
// AVR S1: = function call
// AVR S2: = disable interrupts
// AVR S3: = reenable interrupts
// AVR S4: = _delay
// End mux list

