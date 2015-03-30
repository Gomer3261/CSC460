
// Use primarily for debugging. Maps ports to a digital analyzer
// Port0 maps to Pin 30 on the ATMega2560
// Port7 maps to Pin37 on the ATMega2560

#define USE_MAP 1

#define PORT_DATADIRECTIONREGISTER  DDRC
#define PORT_OUT 				    PORTC
#define PORT_PIN0                   PC0  // 37
#define PORT_PIN1                   PC1  // 36
#define PORT_PIN2                   PC2  // 35
#define PORT_PIN3                   PC3  // 34
#define PORT_PIN4                   PC4  // 33
#define PORT_PIN5                   PC5  // 32
#define PORT_PIN6                   PC6  // 31
#define PORT_PIN7                   PC7  // 30

#define DefaultPorts() { PORT_DATADIRECTIONREGISTER |= 0xff; }

#if USE_MAP

#define EnablePort(a) { PORT_OUT |= (_BV(a)); }

#define EnablePort0() EnablePort(PORT_PIN0);
#define EnablePort1() EnablePort(PORT_PIN1);
#define EnablePort2() EnablePort(PORT_PIN2);
#define EnablePort3() EnablePort(PORT_PIN3);
#define EnablePort4() EnablePort(PORT_PIN4);
#define EnablePort5() EnablePort(PORT_PIN5);
#define EnablePort6() EnablePort(PORT_PIN6);
#define EnablePort7() EnablePort(PORT_PIN7);

#define DisablePort(a) { PORT_OUT &= ~(_BV(a)); }

#define DisablePort0() DisablePort(PORT_PIN0);
#define DisablePort1() DisablePort(PORT_PIN1);
#define DisablePort2() DisablePort(PORT_PIN2);
#define DisablePort3() DisablePort(PORT_PIN3);
#define DisablePort4() DisablePort(PORT_PIN4);
#define DisablePort5() DisablePort(PORT_PIN5);
#define DisablePort6() DisablePort(PORT_PIN6);
#define DisablePort7() DisablePort(PORT_PIN7);

#define PortBlip(a) {              \
    EnablePort(a);                 \
	volatile count = 0;            \
	for( int i = 0; i < 2; i++ )   \
	{                              \
		count++;                   \
	}                              \
    DisablePort(a);                \
}

#define PortBlip1() PortBlip(PORT_PIN0)
#define PortBlip2() PortBlip(PORT_PIN1)
#define PortBlip3() PortBlip(PORT_PIN2)
#define PortBlip4() PortBlip(PORT_PIN3)
#define PortBlip5() PortBlip(PORT_PIN4)
#define PortBlip6() PortBlip(PORT_PIN5)
#define PortBlip7() PortBlip(PORT_PIN6)
#define PortBlip8() PortBlip(PORT_PIN7)

#endif


