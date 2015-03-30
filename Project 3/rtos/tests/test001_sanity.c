#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

/*
 * This task is designed to prove that the system works and can execute
 * a custom task correctly.
 */

/* ---- TRACE ----
 * Defaults all testing output ports
 * Creates a single round robin task
 *  Round robin toggles debug port 0 with a period of 4 milliseconds.
 */

void rr(){
    for(;;){
        EnablePort0();
        _delay_ms(2);
        DisablePort0();
        _delay_ms(2);
    }
}

int r_main(){
    DefaultPorts();
    Task_Create_RR(rr, 0);
    return 0;
}

