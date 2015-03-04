#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

void task(){
    for(;;){
        EnablePort0();
    }
}

int r_main(){
    DefaultPorts();
    Task_Create_Periodic(task, 0, 10, 5, 0);
    return 0;
}

