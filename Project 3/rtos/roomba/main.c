#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

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

typedef struct _control_state {
    uint8_t shooting;
    int16_t drive_velocity; // Forwards/backwards speed roomba
    int16_t turn_radius; // Turning speed roomba
} control_state_t;

typedef enum _automation_state {
    STRAIGHT,
    ORBIT,
    IS_DEAD,
    IS_REVIVED
} automation_state_t;

typedef struct _automation_data {
    int16_t distance;
    int16_t rotation;
} automation_data_t;

// OS GLOBALS
service_t* radio_receive_service;
service_t* radio_send_service;

// ROOMBA CONFIG GLOBALS
COPS_AND_ROBBERS roomba_identity = COP2;
IR_TEAM_CODE ir_team = COP_CODE;
IR_TEAM_CODE ir_enemy = ROBBER_CODE;

pf_gamestate_t current_game_state;
uint8_t roomba_state;

roomba_sensor_data_t roomba_sensor_data;
automation_data_t roomba_automation_data;
control_state_t roomba_controls;



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
        //PORTB ^= (-(~roomba_state & DEAD) ^ PORTB) & (1 << PB7);

        if( !(PINB & (_BV(PB6))) ) {
            if(button_pressed == 0 && (roomba_state & FORCED) == 0) {
                button_pressed = 1;
                roomba_state ^= DEAD;
            }
        }
        else {
            button_pressed = 0;
        }
        Task_Next();
    }
}

/**
 * Task that reads stored data and makes operation decisions.
 */
void decision_making() {
    automation_state_t automation_state = STRAIGHT;
    roomba_automation_data.distance = 0;
    roomba_automation_data.rotation = 0;

    int rotating = 0;

    for(;;) {
        switch(current_game_state.game_state) {
            case GAME_STARTING:
                automation_state = STRAIGHT;
                roomba_controls.turn_radius = 0x8000; // Straight
                roomba_controls.drive_velocity = 0; //500 max;
                roomba_controls.shooting = 0;
                break;
            case GAME_RUNNING:
                rotating = 0;
                // Handle death globally from any state.
                if((roomba_state & DEAD) != 0) {
                    automation_state = IS_DEAD;
                }
                switch(automation_state) {
                    // Drive in a straight line for approximately 1 meter.
                    case STRAIGHT:
                        roomba_controls.turn_radius = 0x8000; // Straight
                        roomba_controls.drive_velocity = 300; //500 max;
                        roomba_controls.shooting = 0;
                        if(roomba_automation_data.distance > 1000 || (roomba_sensor_data.bumps_wheeldrops & 0x3) > 0 || roomba_sensor_data.light_bumber > 0) {
                            automation_state = ORBIT;
                            roomba_automation_data.rotation = 0;
                            roomba_automation_data.distance = 0;
                        }
                        break;
                    // Rotate 270 degrees clockwise.
                    case ORBIT:
                        roomba_controls.drive_velocity = 100;
                        roomba_controls.turn_radius = 1; // On spot clockwise.
                        roomba_controls.shooting = 1;
                        if(roomba_automation_data.rotation <= -260) {
                            automation_state = STRAIGHT;
                            roomba_automation_data.rotation = 0;
                            roomba_automation_data.distance = 0;
                        }
                        break;
                    // Cannot move, stay in place.
                    case IS_DEAD:
                        roomba_controls.drive_velocity = 0;
                        roomba_controls.turn_radius = 0;
                        roomba_controls.shooting = 0;
                        if((roomba_state & DEAD) == 0) {
                            automation_state = IS_REVIVED;
                            roomba_automation_data.rotation = 0;
                            roomba_automation_data.distance = 0;
                        }
                        break;
                    // Revent spree! Rotate 720 degrees counter clockwise spamming shoost!
                    case IS_REVIVED:
                        roomba_controls.drive_velocity = 250;
                        roomba_controls.turn_radius = -1; // On spot cclockwise.
                        roomba_controls.shooting = 1;
                        if(roomba_automation_data.rotation >= 710) {
                            automation_state = STRAIGHT;
                            roomba_automation_data.rotation = 0;
                            roomba_automation_data.distance = 0;
                        }
                }
                break;
            case GAME_OVER:


                automation_state = STRAIGHT;
                // Victory dance! (rotate on the spot)
                if((roomba_state & DEAD) == 0) {
                    roomba_controls.turn_radius = -1; // Clockwise
                    roomba_controls.drive_velocity = 200; //500 max;
                // Loosers corner! (shake head back and forth.)
                } else {
                    if(rotating == 0) {
                        rotating = 1;
                        roomba_controls.turn_radius = 1; // Clockwise
                        roomba_controls.drive_velocity = 100;
                        break;
                    }

                    if(roomba_automation_data.rotation >= 10) {
                        roomba_controls.turn_radius = 1; // cClockwise
                        roomba_controls.drive_velocity = 100;
                    } else if(roomba_automation_data.rotation <= -10) {
                        roomba_controls.turn_radius = -1; // Clockwise
                        roomba_controls.drive_velocity = 100;
                    }
                }
                roomba_controls.shooting = 0;
                break;
        }

        Task_Next();
    }
}

/**
 * Roomba interface task
 */
void roomba_interface() {
    int m_sensor_stage = 0;

    for(;;) {
        // Updates the sensors structure. Alternates which sensors to check from based on runtime.
        switch(m_sensor_stage) {
            case 0:
                Roomba_UpdateSensorPacket(CHASSIS, &roomba_sensor_data); // 10.5ms
                roomba_automation_data.distance += 120;// roomba_sensor_data.distance.value; Doesn't work on our firmware
                roomba_automation_data.rotation += roomba_sensor_data.angle.value*3;

                // Fire IR
                if(roomba_controls.shooting != 0) {
                    IR_transmit(ir_team);
                }
                break;
            case 1:
                Roomba_UpdateSensorPacket(EXTERNAL, &roomba_sensor_data); // 17ms
                break;
            case 2:
                Roomba_UpdateSensorPacket(LIGHT_SENSOR, &roomba_sensor_data); // 25ms
                break;
            default:
                OS_Abort();
                break;
        }
        m_sensor_stage = (m_sensor_stage+1) % 3;

        // Sends the drive command to the roomba.
        Roomba_Drive(roomba_controls.drive_velocity, -1*roomba_controls.turn_radius); // 3ms

        Task_Next();
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

    // Configure the receive settings for radio pipe 0
    Radio_Configure_Rx(RADIO_PIPE_0, ROOMBA_ADDRESSES[roomba_identity], ENABLE);

    // Configure radio transceiver settings.
    Radio_Configure(RADIO_1MBPS, RADIO_HIGHEST_POWER);

    // IR INITIALIZATION
    IR_init();

    // ROOMBA INITIALIZATION
    Roomba_Init();

    // Prevent movement on init.
    roomba_controls.drive_velocity = 0;
    roomba_controls.turn_radius = 0;

    // RTOS INITIALIZATION
    radio_receive_service = Service_Init();
    radio_send_service = Service_Init();

    DefaultPorts();

    Task_Create_System(radio_receive, 0);
    Task_Create_System(radio_send, 0);
    Task_Create_Periodic(roomba_interface, 0, 20, 8, 200);
    Task_Create_RR(user_input, 0);
    Task_Create_RR(decision_making, 0);

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

    // IR only effects the roomba when state is changable.
    if((roomba_state & FORCED) == 0) {
        // Revive if shot by a team member.
        if (ir_value == ir_team) {
            PORTB ^= (1 << PB7);
            roomba_state &= ~DEAD;
        // Kill if shot by an enemy.
        } else if (ir_value == ir_enemy) {
            PORTB ^= (1 << PB7);
            roomba_state |= DEAD;
        }
        _delay_ms(1);
    }

}

