#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"
#include "radio.h"

service_t* radio_receive_service;
service_t* radio_send_service;

uint8_t roomba_identity = COP1;

pf_gamestate_t current_game_state;
uint8_t roomba_state;

void setup() {


    return;
}

void radio_receive() {
    DDRB |= (_BV(PB7));
    PORTB = 0;

    int16_t radio_receive_service_value;

    RADIO_RX_STATUS radio_status;
    radiopacket_t in_packet;

    for(;;) {
        Service_Subscribe(radio_receive_service, &radio_receive_service_value);

        do {
            radio_status = Radio_Receive(&in_packet);

            if(radio_status == RADIO_RX_MORE_PACKETS || radio_status == RADIO_RX_SUCCESS) {
                // Recieved a packet!
                //PORTB ^= (_BV(PB7));

                switch(in_packet.type) {
                    case GAMESTATE_PACKET:
                        current_game_state = in_packet.payload.gamestate;

                        if(current_game_state.roomba_states[roomba_identity] != roomba_state) {
                            current_game_state.roomba_states[roomba_identity] = roomba_state;
                            Service_Publish(radio_send_service, roomba_identity);
                        }
                        break;
                    default:
                        break;
                }
            }

        } while(radio_status == RADIO_RX_MORE_PACKETS);
    }
}

void radio_send() {
    int16_t radio_send_service_value;

    RADIO_TX_STATUS radio_status;
    radiopacket_t out_packet;

    pf_roombastate_t roombastate_command;
    roombastate_command.roomba_id = roomba_identity;

    Radio_Set_Tx_Addr(BASE_ADDRESS);

    for(;;) {
        Service_Subscribe(radio_send_service, &radio_send_service_value);

        roombastate_command.roomba_state = roomba_state;

        out_packet.type = ROOMBASTATE_PACKET;
        memcpy(&out_packet.payload.roombastate, &roombastate_command, sizeof(pf_roombastate_t));

        radio_status = Radio_Transmit(&out_packet, RADIO_RETURN_ON_TX);

        (void)radio_status; //Removed unused warning. :P
    }
}

void user_input() {
    /* Configure PORTB to received digital inputs for pin 12 */
    DDRB &= ~(_BV(PB6));

    int button_pressed = 0;

    for(;;){
        PORTB ^= (-(roomba_state & DEAD) ^ PORTB) & (1 << PB7);

        if( !(PINB & (_BV(PB6))) ) {
            if(button_pressed == 0) {
                button_pressed = 1;
                roomba_state ^= DEAD;
            }
        }
        else {
            button_pressed = 0;
        }
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

    radio_receive_service = Service_Init();
    radio_send_service = Service_Init();

    DefaultPorts();

    //Task_Create_System(setup, 0);
    Task_Create_System(radio_receive, 0);
    Task_Create_System(radio_send, 0);
    Task_Create_RR(user_input, 0);

    return 0;
}

// Called from a radio interrupt.
void radio_rxhandler(uint8_t pipe_number)
{
    //PORTB ^= (1 << PB7);
    Service_Publish(radio_receive_service, pipe_number);
}

