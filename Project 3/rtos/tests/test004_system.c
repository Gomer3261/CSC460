#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

/*
 * This test is designed to verify that system tasks have priority over all other tasks.
 * Additionally, this task should prove that if a periodic task is interrupted,
 * control returns to the task once the interruption is complete.
 * Finally, the runtime of the system task should be extended by the interruption,
 * as well as the worst case execution time.
 */

/* ---- TRACE ----
 * Defaults all testing output ports
 * Creates 1 periodic task
 * Creates 1 round robin task.
 * loop:
 *      tick+0-0ms periodic: toggles port 1 on
 *      tick+1-0ms periodic: spawns system task
 *      tick+1-0ms system: enables port 2
 *      tick+2-0ms system: disables port 2 and exits
 *      tick+2-0ms periodic: control returns to periodic
 *      tick+3-0ms periodic: toggles port 1 off, ends task.
 *      loop:
 *          tick+0-0ms rr1: enables port 0
 *          tick+0-1ms rr1: disables port 0
 *          tick+0-2ms continue
 *      end
 * end
 */

void system() {
    EnablePort2();
    _delay_ms(5);
    DisablePort2();
    Task_Next();
}

void periodic(){
    for(;;) {
        EnablePort1();
        _delay_ms(5);
        Task_Create_System(system, 0);
        _delay_ms(5);
        DisablePort1();
        Task_Next();
    }
}

void rr1(){
    for(;;){
        EnablePort0();
        _delay_ms(1);
        DisablePort0();
    }
}

int r_main(){
    DefaultPorts();
    Task_Create_Periodic(periodic, 5, 6, 4, 0);
    Task_Create_RR(rr1, 5);
    return 0;
}

