#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

/*
 * This test is designed to prove that periodic tasks fire with correct timings
 * Additionally, if a periodic task is not run control should return to RR tasks.
 */

/* ---- TRACE ----
 * Defaults all testing output ports
 * Creates 2 periodic tasks
 * Creates 1 round robin task.
 * loop:
 *      tick+0-0ms periodic1: toggle port 1 on.
 *      tick+1-0ms periodic1: toggle port 1 off.
 *      loop
 *          tick+0-0ms rr1: toggle port 0 on.
 *          tick+0-1ms rr1: toggle port 0 off.
 *          tick+0-2ms continue
 *      end
 *      tick+3-0ms periodic2: toggle port 2 on.
 *      tick+4-0ms periodic2: toggle port 2 off.
 *      loop
 *          tick+0-0ms rr1: toggle port 0 on.
 *          tick+0-1ms rr1: toggle port 0 off.
 *          tick+0-2ms continue
 *      end
 *      tick+5-0ms continue
 * end
 */

void periodic1(){
    for(;;) {
        EnablePort1();
        _delay_ms(5);
        DisablePort1();
        Task_Next();
    }
}

void periodic2(){
    for(;;) {
        EnablePort2();
        _delay_ms(5);
        DisablePort2();
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
    Task_Create_Periodic(periodic1, 5, 5, 2, 0);
    Task_Create_Periodic(periodic2, 5, 5, 2, 3);
    Task_Create_RR(rr1, 5);
    return 0;
}

