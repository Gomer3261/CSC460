#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

void task1(){
    for(;;){
        EnablePort0();
        _delay_ms(30);
        DisablePort0();
        Task_Next();
    }
}

void task2(){
    for(;;){
        EnablePort1();
        _delay_ms(4);
        DisablePort1();
        Task_Next();
    }
}

int r_main(){
    DefaultPorts();
    Task_Create_Periodic(task1, 0, 10, 7, 0);
    Task_Create_Periodic(task2, 0, 10, 2, 3);
    return 0;
}

