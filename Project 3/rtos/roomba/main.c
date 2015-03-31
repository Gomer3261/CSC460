#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"
#include "radio.h"

service_t* radio_return_service;
int16_t radio_return_service_value;

int radio_power_pin = PL2;

radiopacket_t packet;

void setup() {
    // RADIO INITIALIZATION
    DDRL |= (_BV(radio_power_pin));
    PORTL &= ~(1 << radio_power_pin);
    _delay_ms(100);  /* max is 262.14 ms / F_CPU in MHz */
    PORTL |= 1 << radio_power_pin;
    _delay_ms(100);

    Radio_Init(112);

    // configure the receive settings for radio pipe 0
    Radio_Configure_Rx(RADIO_PIPE_0, ROOMBA_ADDRESSES[COP1], ENABLE);

    // configure radio transceiver settings.
    Radio_Configure(RADIO_1MBPS, RADIO_HIGHEST_POWER);

    return;
}

void radio_listen() {
    DDRB |= (_BV(PB7));
    PORTB = 0;

    RADIO_RX_STATUS radio_status;

    for(;;){
        Service_Subscribe(radio_return_service, &radio_return_service_value);

        EnablePort1();
        do {
            radio_status = Radio_Receive(&packet);

            if(radio_status == RADIO_RX_MORE_PACKETS || radio_status == RADIO_RX_SUCCESS) {
                // Recieved a packet!
                //PORTB ^= (_BV(PB7));
            }

        } while(radio_status == RADIO_RX_MORE_PACKETS);
        DisablePort1();
    }
}

void empty_round_robin() {
    RADIO_RX_STATUS radio_status;

    for(;;) {
        EnablePort7();
        radio_status = Radio_Receive(&packet);

        if(radio_status == RADIO_RX_MORE_PACKETS || radio_status == RADIO_RX_SUCCESS) {

            // Recieved a packet!

        }

        _delay_ms(2);
        DisablePort7();
    }
}

int r_main(){
    radio_return_service = Service_Init();

    DefaultPorts();

    Task_Create_System(setup, 0);
    Task_Create_RR(radio_listen, 0);
    //Task_Create_RR(empty_round_robin, 0);

    return 0;
}

// Called from a radio interrupt.
void radio_rxhandler(uint8_t pipe_number)
{
    EnablePort0();
    //PORTB ^= (_BV(PB7));
    _delay_ms(1);
    Radio_Flush();
    DisablePort0();
    //EnablePort7();
    //PORTB ^= (_BV(PB7));
    Service_Publish(radio_return_service, pipe_number);
    //DisablePort7();*/
}

