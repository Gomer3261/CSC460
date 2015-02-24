#define F_CPU 8000000UL

#define PROFILECODE 1
#define PROFILE_INITIALIZATION 1
#define PROFILE_UPDATESWEEP  0
#define PROFILE_SENDREQUEST  0
#define PROFILE_GETRESPONSE  0
#define PROFILE_UPDATESTATISTICS  0
#define PROFILE_CALCAULATERESULTS  0
#define PROFILE_REPORT  0

#define PROFILE_DATADIRECTIONREGISTER 	DDRC
#define PROFILE_OUTPORT 				PORTC
#define PROFILE_OUT_PIN0 				PINC0
#define PROFILE_OUT_PIN1 				PINC1
#define PROFILE_OUT_PIN2 				PINC2
#define PROFILE_OUT_PIN3 				PINC3
#define PROFILE_OUT_PIN4 				PINC4
#define PROFILE_OUT_PIN5 				PINC5
#define PROFILE_OUT_PIN6 				PINC6
#define PROFILE_OUT_PIN7 				PINC7

#if PROFILECODE

#define InitializeLogicAnalyzerProfiler() { PROFILE_DATADIRECTIONREGISTER |= 0xff; }
#define EnableProfileSample0() { PROFILE_OUTPORT &= ~(_BV(PROFILE_OUT_PIN0)); }
#define EnableProfileSample1() { PROFILE_OUTPORT &= ~(_BV(PROFILE_OUT_PIN1)); }
#define EnableProfileSample2() { PROFILE_OUTPORT &= ~(_BV(PROFILE_OUT_PIN2)); }
#define EnableProfileSample3() { PROFILE_OUTPORT &= ~(_BV(PROFILE_OUT_PIN3)); }
#define EnableProfileSample4() { PROFILE_OUTPORT &= ~(_BV(PROFILE_OUT_PIN4)); }
#define EnableProfileSample5() { PROFILE_OUTPORT &= ~(_BV(PROFILE_OUT_PIN5)); }
#define EnableProfileSample6() { PROFILE_OUTPORT &= ~(_BV(PROFILE_OUT_PIN6)); }
#define EnableProfileSample7() { PROFILE_OUTPORT &= ~(_BV(PROFILE_OUT_PIN7)); }


#define DisableProfileSample0() { PROFILE_OUTPORT |= (_BV(PROFILE_OUT_PIN0)); }
#define DisableProfileSample1() { PROFILE_OUTPORT |= (_BV(PROFILE_OUT_PIN1)); }
#define DisableProfileSample2() { PROFILE_OUTPORT |= (_BV(PROFILE_OUT_PIN2)); }
#define DisableProfileSample3() { PROFILE_OUTPORT |= (_BV(PROFILE_OUT_PIN3)); }
#define DisableProfileSample4() { PROFILE_OUTPORT |= (_BV(PROFILE_OUT_PIN4)); }
#define DosableProfileSample5() { PROFILE_OUTPORT |= (_BV(PROFILE_OUT_PIN5)); }
#define DisableProfileSample6() { PROFILE_OUTPORT |= (_BV(PROFILE_OUT_PIN6)); }
#define DosableProfileSample7() { PROFILE_OUTPORT |= (_BV(PROFILE_OUT_PIN7)); }

#endif
