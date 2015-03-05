#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

/*
 * This test is designed to prove that the OS will properly exit when a
 * periodic task runs past is worst case execution time.
 */

/* ---- TRACE ----
 * Defaults all testing output ports
 * Creates 1 periodic task
 * tick 0-0ms task: toggle port 0 on;
 * tick 5-0ms task: runs past worst case execution;
 * ERROR: OS should stop, and begin dispaying error output.
 */

void task(){
    for(;;){
        EnablePort0();
    }
}

int r_main(){
    DefaultPorts();
    Task_Create_Periodic(task, 0, 10, 5, 0);
    return 0;
}

