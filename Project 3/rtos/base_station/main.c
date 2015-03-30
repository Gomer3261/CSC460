#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"
#include "radio.h"

service_t* radio_fire_service;
int16_t radio_fire_service_value;

service_t* radio_return_service;
int16_t radio_return_service_value;

int radio_power_pin = PL2;

void setup() {
    // RADIO INITIALIZATION
    DDRL = (_BV(radio_power_pin));
    PORTL &= ~(1 << radio_power_pin);
    _delay_ms(100);  /* max is 262.14 ms / F_CPU in MHz */
    PORTL |= 1 << radio_power_pin;
    _delay_ms(100);

    Radio_Init(BASE_FREQUENCY);

    // configure the receive settings for radio pipe 0
    Radio_Configure_Rx(RADIO_PIPE_0, BASE_ADDRESS, ENABLE);

    // configure radio transceiver settings.
    Radio_Configure(RADIO_2MBPS, RADIO_HIGHEST_POWER);

    return;
}

void sendState() {
    for(;;) {
        int i;
        for(i=COP1; i<=ROBBER2; i++) {

            // The address to which the next transmission is to be sent
            Radio_Set_Tx_Addr(ROOMBA_ADDRESSES[i]);
            //TODO: Load data into packet
            Service_Publish(radio_fire_service, i);
        }

        Task_Next();
    }
}

void sendPacket(){
    for(;;){
        Service_Subscribe(radio_fire_service, &radio_fire_service_value);
        //TODO: Send Packet
    }
}

void updateState() {
    for(;;){
        Service_Subscribe(radio_return_service, &radio_return_service_value);
        // Pipe number is in radio_return_service_value
        //TODO: Consume incomming packets.
    }
}

void user_input(){
    for(;;){
        //TODO: Listen to joystick controls and modify gamestate.
    }
}

int r_main(){
    radio_fire_service = Service_Init();
    radio_return_service = Service_Init();

    Task_Create_System(setup, 0);
    Task_Create_System(sendPacket, 0);
    Task_Create_System(updateState, 0);
    Task_Create_Periodic(sendState, 0, 250, 5, 1000);
    Task_Create_RR(user_input, 0);
    DefaultPorts();
    return 0;
}

// Called from a radio interrupt.
void radio_rxhandler(uint8_t pipe_number)
{
    Service_Publish(radio_return_service, pipe_number);
}

