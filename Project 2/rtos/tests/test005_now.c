#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

/*
 * This test is designed to test the Now() function in the OS.
 * The test should display the number of milliseconds that have
 * passed since the last tick occured via debug ports.
 */

/* ---- TRACE ----
 * Defaults all testing output ports
 * Creates 1 round robin task
 * loop:
 *      tick+0-0ms now_test: toggle port 0 on and others off.
 *      tick+0-1ms now_test: toggle port 1 on and others off.
 *      tick+0-2ms now_test: toggle port 2 on and others off.
 *      tick+0-3ms now_test: toggle port 3 on and others off.
 *      tick+0-4ms now_test: toggle port 4 on and others off.
 *      tick+1-0ms continue
 * end
 */

void now_test(){
    for(;;){
        if(Now()%5 == 0) {
            EnablePort0();
        } else {
            DisablePort0();
        }
        if(Now()%5 == 1) {
            EnablePort1();
        } else {
            DisablePort1();
        }
        if(Now()%5 == 2) {
            EnablePort2();
        } else {
            DisablePort2();
        }
        if(Now()%5 == 3) {
            EnablePort3();
        } else {
            DisablePort3();
        }
        if(Now()%5 == 4) {
            EnablePort4();
        } else {
            DisablePort4();
        }
    }
}

int r_main(){
    DefaultPorts();
    Task_Create_RR(now_test, 100);
    return 0;
}

