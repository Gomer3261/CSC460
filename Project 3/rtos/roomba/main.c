#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"
#include "radio.h"

service_t* radio_return_service;

radiopacket_t packet;

void setup() {


    return;
}

void radio_listen() {
    DDRB |= (_BV(PB7));
    PORTB = 0;

    RADIO_RX_STATUS radio_status;
    int16_t radio_return_service_value;

    for(;;){
        Service_Subscribe(radio_return_service, &radio_return_service_value);

        EnablePort1();
        do {
            radio_status = Radio_Receive(&packet);

            if(radio_status == RADIO_RX_MORE_PACKETS || radio_status == RADIO_RX_SUCCESS) {
                // Recieved a packet!
                //PORTB ^= (_BV(PB7));
                PORTB ^= (1 << PB7);
            }

        } while(radio_status == RADIO_RX_MORE_PACKETS);

        DisablePort1();
    }
}

int r_main(){
    // RADIO INITIALIZATION
    DDRL |= (1 << PL2);
    PORTL &= ~(1 << PL2);
    _delay_ms(500);  /* max is 262.14 ms / F_CPU in MHz */
    PORTL |= 1 << PL2;
    _delay_ms(500);

    Radio_Init(BASE_FREQUENCY);

    // configure the receive settings for radio pipe 0
    Radio_Configure_Rx(RADIO_PIPE_0, ROOMBA_ADDRESSES[COP1], ENABLE);

    // configure radio transceiver settings.
    Radio_Configure(RADIO_1MBPS, RADIO_HIGHEST_POWER);

    radio_return_service = Service_Init();

    DefaultPorts();

    //Task_Create_System(setup, 0);
    Task_Create_RR(radio_listen, 0);
    //Task_Create_RR(empty_round_robin, 0);

    return 0;
}

// Called from a radio interrupt.
void radio_rxhandler(uint8_t pipe_number)
{
    Service_Publish(radio_return_service, pipe_number);
}

