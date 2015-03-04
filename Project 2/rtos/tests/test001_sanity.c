#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

void rr(){
    for(;;){
        EnablePort0();
        _delay_ms(2);
        DisablePort0();
        _delay_ms(2);
    }
}

int r_main(){
    DefaultPorts();
    Task_Create_RR(rr, 0);
    return 0;
}

