#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

service_t* service;

int16_t rr1_value;
int16_t rr2_value;

/*
 * This test is designed to prove that tasks can be properly freed be returning a value
 * Both system and round robbin tasks are tested. A periodic task creates 6 other tasks each
 * time it is run, each task runs for 1 millisecond before destroying itself.
 */

/* ---- TRACE ----
 * Defaults all testing output ports
 * Creates 1 periodic task
 * Creates 3 round robin tasks
 * Creates 3 system tasks
 * loop:
 *      tick 0-0ms system(0): toggle port 0 on;
 *      tick 0-1ms system(0): toggle port 0 off;
 *      tick 0-1ms system(1): toggle port 1 on;
 *      tick 0-2ms system(1): toggle port 1 off;
 *      tick 0-2ms system(2): toggle port 2 on;
 *      tick 0-3ms system(2): toggle port 2 off;
 *      tick 0-3ms round_robin(3): toggle port 3 on;
 *      tick 0-4ms round_robin(3): toggle port 3 off, destroy itself;
 *      tick 0-4ms round_robin(4): toggle port 4 on;
 *      tick 1-0ms round_robin(5): toggle port 5 on;
 *      tick 1-1ms round_robin(5): toggle port 5 off, destroy itself;
 *      tick 1-2ms round_robin(4): toogle port 4 off, destroy itself;
 *      tick 1-2ms idle:
 *      tick 10-0ms periodic: toogle port 7 on, launch all 6 tasks again, toggle port 6 off, task_next().
 * end
 */

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

