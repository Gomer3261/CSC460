#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

/*
 * This test is designed to prove that the system can detect when a periodic
 * tasks worst case exection time is longer than it's period, and produce an error.
 */

/* ---- TRACE ----
 * Defaults all testing output ports
 * Creates 1 periodic task with incorrect parameters.
 * ERROR: OS should stop, and begin dispaying error output.
 */

void periodic_test(){
    EnablePort0();
    _delay_ms(5);
    DisablePort0();
    Task_Next();
}

int r_main(){
    DefaultPorts();
    Task_Create_Periodic(periodic_test, 5, 10, 20, 0);
    return 0;
}

