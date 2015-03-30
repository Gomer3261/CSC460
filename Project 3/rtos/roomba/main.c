#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

void foo(){
    DDRB = (_BV(PB7));
    for(;;){
        PORTB ^= 1 << PB7;
        Task_Next();
    }
}

void rr1(){
    for(;;){
        EnablePort0();
        DisablePort1();
        DisablePort2();
    }
}

void rr2(){
    for(;;){
        DisablePort0();
        EnablePort1();
        DisablePort2();
    }
}

void rr3(){
    for(;;){
        DisablePort0();
        DisablePort1();
        EnablePort2();
    }
}

int r_main(){
    //Task_Create_RR(foo, 100);
    Task_Create_Periodic(foo, 1, 100, 2, 0);
    DefaultPorts();
    //Task_Create_RR(rr1, 0);
    //Task_Create_RR(rr2, 0);
    //Task_Create_RR(rr3, 0);
    return 0;
}

