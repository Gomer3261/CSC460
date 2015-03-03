#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"

void now_test(){
    for(;;){
        if(Now()%5 == 0) {
            EnablePort0();
        } else {
            DisablePort0();
        }
        if(Now()%5 == 1) {
            EnablePort1();
        } else {
            DisablePort1();
        }
        if(Now()%5 == 2) {
            EnablePort2();
        } else {
            DisablePort2();
        }
        if(Now()%5 == 3) {
            EnablePort3();
        } else {
            DisablePort3();
        }
        if(Now()%5 == 4) {
            EnablePort4();
        } else {
            DisablePort4();
        }
    }
}

int r_main(){
    DefaultPorts();
    Task_Create_RR(now_test, 100);
    return 0;
}

