#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

void system(){
    for(;;){
        OS_Abort();
    }
}

int r_main(){
    DefaultPorts();
    Task_Create_System(system, 100);
    return 0;
}

