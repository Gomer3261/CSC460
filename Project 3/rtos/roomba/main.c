#include <avr/io.h>
#include <util/delay.h>

// OPERATING SYSTEM
#include "port_map.h"
#include "os.h"

// RADIO COMMUNICATION
#include "radio.h"

// ROOMBA COMMUNICATION
#include "blocking_uart.h"
#include "roomba.h"
#include "roomba_sci.h"
#include "sensor_struct.h"

// IR CONTROL
#include "ir.h"

// OS GLOBALS
service_t* radio_receive_service;
service_t* radio_send_service;

// ROOMBA CONFIG GLOBALS
COPS_AND_ROBBERS roomba_identity = COP2;
IR_TEAM_CODE ir_team = COP_CODE;
IR_TEAM_CODE ir_enemy = ROBBER_CODE;

pf_gamestate_t current_game_state;
uint8_t roomba_state;

/**
 * Taskt to consume radio messages from the base station.
 */
void radio_receive() {
    DDRB |= (_BV(PB7));
    PORTB = 0;

    int16_t radio_receive_service_value;

    RADIO_RX_STATUS radio_status;
    radiopacket_t in_packet;

    for(;;) {
        Service_Subscribe(radio_receive_service, &radio_receive_service_value);

        // Consume all recieved messages from the queue.
        do {
            radio_status = Radio_Receive(&in_packet);

            if(radio_status == RADIO_RX_MORE_PACKETS || radio_status == RADIO_RX_SUCCESS) {
                // Recieved a packet!
                //PORTB ^= (_BV(PB7));

                switch(in_packet.type) {
                    // Handling gamestate packets;
                    case GAMESTATE_PACKET:
                        current_game_state = in_packet.payload.gamestate;

                        // Roomba is in control of their own state.
                        if((current_game_state.roomba_states[roomba_identity] & FORCED) == 0) {
                            // If previously not in control, copy from gamestate.
                            if((roomba_state & FORCED) != 0) {
                                roomba_state = current_game_state.roomba_states[roomba_identity];
                            }
                            // If gamestate does not match your expectations, update the base station.
                            else if(current_game_state.roomba_states[roomba_identity] != roomba_state) {
                                // Update your own gamestate and send a packet.
                                current_game_state.roomba_states[roomba_identity] = roomba_state;
                                Service_Publish(radio_send_service, roomba_identity);
                            }
                        }
                        // Just copy your state from the base station.
                        else {
                            roomba_state = current_game_state.roomba_states[roomba_identity];
                        }
                        break;
                    default:
                        break;
                }
            }

        } while(radio_status == RADIO_RX_MORE_PACKETS);
    }
}

/**
 * Sends a state message to the base station via radio.
 */
void radio_send() {
    // Setup
    int16_t radio_send_service_value;

    RADIO_TX_STATUS radio_status;
    radiopacket_t out_packet;

    pf_roombastate_t roombastate_command;
    roombastate_command.roomba_id = roomba_identity;

    Radio_Set_Tx_Addr(BASE_ADDRESS);

    for(;;) {
        Service_Subscribe(radio_send_service, &radio_send_service_value);

        // Assemble packet.
        roombastate_command.roomba_state = roomba_state;

        out_packet.type = ROOMBASTATE_PACKET;
        memcpy(&out_packet.payload.roombastate, &roombastate_command, sizeof(pf_roombastate_t));

        // Send packet.
        radio_status = Radio_Transmit(&out_packet, RADIO_RETURN_ON_TX);

        (void)radio_status; //Removed unused warning. :P
    }
}

// TODO: Delete me, the roomba has no user input! FOOL!
void user_input() {
    /* Configure PORTB to received digital inputs for pin 12 */
    DDRB &= ~(_BV(PB6));

    int button_pressed = 0;

    for(;;){
        PORTB ^= (-(~roomba_state & DEAD) ^ PORTB) & (1 << PB7);

        if( !(PINB & (_BV(PB6))) ) {
            if(button_pressed == 0 && (roomba_state & FORCED) == 0) {
                button_pressed = 1;
                roomba_state ^= DEAD;
            }
        }
        else {
            button_pressed = 0;
        }
    }
}

/**
 * Setup function called by the RTOS on initialization.
 */
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

    // RTOS INITIALIZATION
    radio_receive_service = Service_Init();
    radio_send_service = Service_Init();

    DefaultPorts();

    Task_Create_System(radio_receive, 0);
    Task_Create_System(radio_send, 0);
    Task_Create_RR(user_input, 0);

    return 0;
}

/**
 * Called whenever a radio message is ready.
 */
void radio_rxhandler(uint8_t pipe_number)
{
    //PORTB ^= (1 << PB7);
    Service_Publish(radio_receive_service, pipe_number);
}

/**
 * Called whenever a proper IR message is recieved.
 */
void ir_rxhandler() {
    uint8_t ir_value = IR_getLast();
    if((roomba_state & FORCED) == 0) {
        if (ir_value == ir_team) {
            roomba_state &= ~DEAD;
        } else if (ir_value == ir_enemy){
            roomba_state |= DEAD;
        }
    }

}

