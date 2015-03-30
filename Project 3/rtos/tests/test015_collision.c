#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

/*
 * This test is designed to prove that the OS will properly exit when two
 * periodic tasks should be running at the same time.
 */

/* ---- TRACE ----
 * Defaults all testing output ports
 * Creates 2 periodic tasks
 * tick 0-0ms task1: toggle port 0 on;
 * tick 3-0ms task2: attempts to begin running, task1 however is incomplete.
 * ERROR: OS should stop, and begin dispaying error output.
 */

void task1(){
    for(;;){
        EnablePort0();
        _delay_ms(30);
        DisablePort0();
        Task_Next();
    }
}

void task2(){
    for(;;){
        EnablePort1();
        _delay_ms(4);
        DisablePort1();
        Task_Next();
    }
}

int r_main(){
    DefaultPorts();
    Task_Create_Periodic(task1, 0, 10, 7, 0);
    Task_Create_Periodic(task2, 0, 10, 2, 3);
    return 0;
}

