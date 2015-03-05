#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

/*
 * This test is designed to prove that pushing values works correctly
 * A peropdic task alternatingly publishes the values 0 and 1.
 * Two round robin tasks each subscribe to a service, and resume when they receive
 * their corresponding value (0 or 1).
 */

/* ---- TRACE ----
 * Defaults all testing output ports
 * Creates 1 periodic task
 * Creates 3 round robin tasks
 * tick 0-0ms rr3: toggle port 3 on;
 * tick 5-0ms service_publisher: start time elapsed;
 * loop:
 *      tick 0-0ms service_publisher: publish value 0, toggle port 0 on;
 *      tick 0-2ms service_publisher: toggle port 0 off;
 *      tick 0-2ms rr_service_subscriber0: toggle port 3 off and port 1 on;
 *      tick 1-0ms rr3: toggle port 3 on, port 1 off;
 *      tick 3-0ms service_publisher: publish value 1, toggle port 0 on;
 *      tick 3-2ms service_publisher: toggle port 0 off;
 *      tick 3-2ms rr_service_subscriber1: toggle port 3 off and port 2 on;
 *      tick 4-0ms rr3: toggle port 3 on, port 1 off;
 *      tick 6-0ms continue;
 * end
 */

service_t* service;

int16_t rr1_value;
int16_t rr2_value;

void service_publisher(){
    int16_t previous_publish = 1;
    for(;;) {
        EnablePort0();
        previous_publish = (previous_publish+1)%2;
        Service_Publish(service, previous_publish);
        _delay_ms(2);
        DisablePort0();
        Task_Next();
    }
}

void rr_service_subscriber0(){
    for(;;) {
        DisablePort1();
        for(;;) {
            Service_Subscribe(service, &rr1_value);
            if(rr1_value == 0) {
                break;
            }
        }
        EnablePort1();
        DisablePort3();
        _delay_ms(2);
    }
}

void rr_service_subscriber1(){
    for(;;) {
        DisablePort2();
        for(;;) {
            Service_Subscribe(service, &rr1_value);
            if(rr1_value == 1) {
                break;
            }
        }
        EnablePort2();
        DisablePort3();
        _delay_ms(2);
    }
}

void rr3(){
    for(;;) {
        EnablePort3();
    }
}

int r_main(){
    DefaultPorts();
    service = Service_Init();
    Task_Create_RR(rr_service_subscriber0, 0);
    Task_Create_RR(rr_service_subscriber1, 1);
    Task_Create_RR(rr3, 0);
    Task_Create_Periodic(service_publisher, 0, 3, 1, 5);
    return 0;
}

