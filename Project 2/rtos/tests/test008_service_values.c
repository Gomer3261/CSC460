#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

service_t* service;

uint16_t system_value;
uint16_t rr_value;

void service_publisher(){
    for(;;) {
        EnablePort0();
        Service_Publish(service, 255);
        _delay_ms(2);
        Task_Next();
        DisablePort0();
    }
}

void system_service_subscriber(){
    for(;;) {
        EnablePort1();
        Service_Subscribe(service, &system_value);
        _delay_ms(2);
        DisablePort1();
    }
}

void rr_service_subscriber(){
    for(;;) {
        EnablePort2();
        Service_Subscribe(service, &rr_value);
        DisablePort3();
        DisablePort4();
        _delay_ms(2);
        DisablePort2();
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
    Taks_Create_System(system_service_subscriber, 0);
    Task_Create_RR(rr_service_subscriber, 0);
    Task_Create_RR(rr2, 0);
    Task_Create_RR(rr3, 0);
    Task_Create_Periodic(service_publisher, 0, 5, 1, 5);
    return 0;
}

