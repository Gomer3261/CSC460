#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

service_t* service;

/*
 * This test is designed to test the ability to pass arguments to
 * spawned tasks. This is done by adding variable delays to tasks
 * based on the provided arguments.
 */

/* ---- TRACE ----
 * Defaults all testing output ports
 * Creates 1 system task
 * Creates 1 periodic task
 * Creates 1 round robin task
 * tick 0-0ms system toggle port 0 on;
 * tick 1-2ms system toggle port 1 off;
 * loop:
 *      tick+0-0ms rr: toggle port 2 on;
 *      tick+0-3ms rr: toggle port 2 off;
 *      tick+1-1ms rr: continue
 * end
 * loop:
 *      tick+0-0ms periodic: toggle port 1 on.
 *      tick+1-0ms periodic: toggle port 1 off.
 *      loop:
 *          tick+0-0ms rr: toggle port 2 on;
 *          tick+0-3ms rr: toggle port 2 off;
 *          tick+1-1ms rr: continue
 *      end
 *      tick+5-0ms continue
 * end
 */

void system(){
    int16_t delay = Task_GetArg();
    int16_t i;
    EnablePort0();
    for(i=0; i<delay; i++) {
        _delay_ms(1);
    }
    DisablePort0();
}

void periodic(){
    int16_t delay = Task_GetArg();
    uint16_t i;
    for(;;) {
        EnablePort1();
        for(i=0; i<delay; i++) {
            _delay_ms(1);
        }
        DisablePort1();
        Task_Next();
    }
}

void rr(){
    int16_t delay = Task_GetArg();
    int16_t i;
    for(;;) {
        EnablePort2();
        for(i=0; i<delay; i++) {
            _delay_ms(1);
        }
        DisablePort2();
        for(i=0; i<delay; i++) {
            _delay_ms(1);
        }
    }
}

int r_main(){
    DefaultPorts();
    Task_Create_RR(rr, 3);
    Task_Create_Periodic(periodic, 5, 5, 2, 3);
    Task_Create_System(system, 7);
    return 0;
}

