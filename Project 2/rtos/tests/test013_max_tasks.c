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
    int i=0;
    for(i=0; i<MAXPROCESS+1; i++) {
        Task_Create_RR(task, 0);
    }
    return 0;
}

