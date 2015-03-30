#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

/*
 * This test is designed to test round robin tasks.
 * The primary goal is to ensure that all tasks run in the correct order.
 */

/* ---- TRACE ----
 * Defaults all testing output ports
 * Creates 3 round robin task
 * loop:
 *      tick+0-0ms rr1: toggles debug port 0 on and other ports off
 *      tick+1-0ms rr2: toggles debug port 1 on and other ports off
 *      tick+2-0ms rr3: toggles debug port 2 on and other ports off
 * end
 */

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

