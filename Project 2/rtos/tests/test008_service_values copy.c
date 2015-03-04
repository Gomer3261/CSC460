#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

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

