#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

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
    DefaultPorts();
    Task_Create_RR(rr1, 0);
    Task_Create_RR(rr2, 0);
    Task_Create_RR(rr3, 0);
    return 0;
}

