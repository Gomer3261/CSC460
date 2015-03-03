#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

void system() {
    EnablePort2();
    _delay_ms(5);
    DisablePort2();
    Task_Next();
}

void periodic(){
    for(;;) {
        EnablePort1();
        _delay_ms(5);
        Task_Create_System(system, 0);
        _delay_ms(5);
        DisablePort1();
        Task_Next();
    }
}

void rr1(){
    for(;;){
        EnablePort0();
        _delay_ms(1);
        DisablePort0();
    }
}

int r_main(){
    DefaultPorts();
    Task_Create_Periodic(periodic, 5, 6, 4, 0);
    Task_Create_RR(rr1, 5);
    return 0;
}

