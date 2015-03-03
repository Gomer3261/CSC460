#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

void periodic_test(){
    EnablePort0();
    _delay_ms(5);
    DisablePort0();
    Task_Next();
}

int r_main(){
    DefaultPorts();
    Task_Create_Periodic(periodic_test, 5, 10, 20, 0);
    return 0;
}

