#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

void periodic1(){
    for(;;) {
        EnablePort1();
        _delay_ms(5);
        DisablePort1();
        Task_Next();
    }
}

void periodic2(){
    for(;;) {
        EnablePort2();
        _delay_ms(5);
        DisablePort2();
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
    Task_Create_Periodic(periodic1, 5, 5, 2, 0);
    Task_Create_Periodic(periodic2, 5, 5, 2, 3);
    Task_Create_RR(rr1, 5);
    return 0;
}

