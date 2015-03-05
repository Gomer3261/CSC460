#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

/*
 * This test is designed to prove that the OS will properly exit when the
 * user attempts to allocate more services than are available.
 */

/* ---- TRACE ----
 * Defaults all testing output ports
 * Creates 9 services
 * ERROR: OS should stop, and begin dispaying error output.
 */

void system(){
    for(;;){
        OS_Abort();
    }
}

int r_main(){
    DefaultPorts();
    Task_Create_System(system, 100);
    return 0;
}

