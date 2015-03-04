#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

service_t *service;
int16_t service_return;

void periodic_task(){
    for(;;){
        EnablePort0();
        Service_Subscribe(service, &service_return);
        DisablePort0();
        _delay_ms(2);
        Task_Next();
    }
}

int r_main(){
    DefaultPorts();
    service = Service_Init();
    Task_Create_Periodic(task, 0, 5, 2, 0);
    return 0;
}

