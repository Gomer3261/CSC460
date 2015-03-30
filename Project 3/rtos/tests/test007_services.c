#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

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

service_t* service;

int16_t system_value;
int16_t rr_value;

void service_publisher(){
    for(;;) {
        EnablePort0();
        Service_Publish(service, 255);
        _delay_ms(1);
        DisablePort0();
        Task_Next();
    }
}

void system_service_subscriber(){
    for(;;) {
        EnablePort1();
        Service_Subscribe(service, &system_value);
        DisablePort1();
        _delay_ms(1);
        EnablePort1();
    }
}

void rr_service_subscriber(){
    for(;;) {
        EnablePort2();
        Service_Subscribe(service, &rr_value);
        DisablePort3();
        DisablePort4();
        DisablePort2();
        _delay_ms(1);
    }
}

void rr2(){
    for(;;) {
        EnablePort3();
        DisablePort4();
    }
}

void rr3(){
    for(;;) {
        DisablePort3();
        EnablePort4();
    }
}

int r_main(){
    DefaultPorts();
    service = Service_Init();
    Task_Create_System(system_service_subscriber, 0);
    Task_Create_RR(rr_service_subscriber, 0);
    Task_Create_RR(rr2, 0);
    Task_Create_RR(rr3, 0);
    Task_Create_Periodic(service_publisher, 0, 5, 1, 5);
    return 0;
}

