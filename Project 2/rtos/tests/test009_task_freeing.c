#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

service_t* service;

int16_t rr1_value;
int16_t rr2_value;

void system() {
    int16_t arg = Task_GetArg();
    EnablePort(arg);
    _delay_ms(1);
    DisablePort(arg);
    return;
}

void round_robin() {
    int16_t arg = Task_GetArg();
    EnablePort(arg);
    _delay_ms(1);
    DisablePort(arg);
    return;
}

void periodic() {
    for (;;){
        EnablePort7();
        Task_Create_System(system, PORT_PIN0);
        Task_Create_System(system, PORT_PIN1);
        Task_Create_System(system, PORT_PIN2);
        Task_Create_RR(round_robin, PORT_PIN3);
        Task_Create_RR(round_robin, PORT_PIN4);
        Task_Create_RR(round_robin, PORT_PIN5);
        DisablePort7();
        Task_Next();
    }
}

int r_main(){
    DefaultPorts();
    service = Service_Init();
    Task_Create_System(system, PORT_PIN0);
    Task_Create_System(system, PORT_PIN1);
    Task_Create_System(system, PORT_PIN2);
    Task_Create_RR(round_robin, PORT_PIN3);
    Task_Create_RR(round_robin, PORT_PIN4);
    Task_Create_RR(round_robin, PORT_PIN5);
    Task_Create_Periodic(periodic, 0, 10, 1, 10);
    return 0;
}

