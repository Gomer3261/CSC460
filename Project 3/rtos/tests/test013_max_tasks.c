#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

/*
 * This test is designed to prove that the OS will properly exit when the
 * user attempts to allocate more tasks than are available.
 */

/* ---- TRACE ----
 * Defaults all testing output ports
 * Creates 9 tasks
 * ERROR: OS should stop, and begin dispaying error output.
 */

void task(){
    for(;;){
        EnablePort0();
    }
}

int r_main(){
    DefaultPorts();
    int i=0;
    for(i=0; i<MAXPROCESS+1; i++) {
        Task_Create_RR(task, 0);
    }
    return 0;
}

