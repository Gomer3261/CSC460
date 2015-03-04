#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

service_t* services;

void task(){
    for(;;){
        EnablePort0();
        _delay_ms(5);
        DisablePort0();
    }
}

int r_main(){
    DefaultPorts();
    int i = 0;
    for(i=0; i<MAXSERVICES+1; i++) {
        services = Service_Init(); // Initializes too many tasks!
    }
    Task_Create_RR(task, 0);
    return 0;
}

