#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

void foo(){
    DDRB = 1 << PB7;
    for(;;){
        _delay_ms(500);
        for(;;) {
            if(Now()%5 == 3)
            {
                PORTB ^= 1 << PB7;
                break;
            }
        }
        Task_Next();
    }
}

int r_main(){
    Task_Create_RR(foo, 100);
    //Task_Create_Periodic(foo, 1, 100, 2, 0);
    return 0;
}
