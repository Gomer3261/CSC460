#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

service_t* service;

uint16_t system_value;
uint16_t rr_value;

void system(){
    int16_t delay = Task_GetArg();
    int16_t i;
    EnablePort0();
    for(i=0; i<delay; i++) {
        _delay_ms(1);
    }
    DisablePort0();
}

void periodic(){
    int16_t delay = Task_GetArg();
    uint16_t i;
    for(;;) {
        EnablePort1();
        for(i=0; i<delay; i++) {
            _delay_ms(1);
        }
        DisablePort1();
        Task_Next();
    }
}

void rr(){
    int16_t delay = Task_GetArg();
    int16_t i;
    for(;;) {
        EnablePort2();
        for(i=0; i<delay; i++) {
            _delay_ms(1);
        }
        DisablePort2();
        for(i=0; i<delay; i++) {
            _delay_ms(1);
        }
    }
}

int r_main(){
    DefaultPorts();
    service = Service_Init();
    Task_Create_RR(rr, 3);
    Task_Create_Periodic(periodic, 5, 5, 2, 3);
    Task_Create_System(system, 7);
    return 0;
}

