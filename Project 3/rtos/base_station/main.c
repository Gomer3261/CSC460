#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"
#include "radio.h"

#define JOYSTICK_X_CHANNEL 0
#define JOYSTICK_Y_CHANNEL 1

#define RADIO_POWER_PIN PL2

#define COP1_STATUS_LIGHT PD7 //38
#define COP2_STATUS_LIGHT PG1 //40
#define ROBBER1_STATUS_LIGHT PL7 //42
#define ROBBER2_STATUS_LIGHT PL5 //44
#define GAMESTATE_RUNNING PL6 //43
#define GAMESTATE_IDLE_LIGHT PG0 //41

service_t* radio_send_service;
service_t* radio_receive_service;

pf_gamestate_t current_game_state;

/**
 * Periodic task that pushes the base station's gamestate to all 4 roombas.
 */
void sendState() {
    //DEBUG
    DDRB |= (_BV(PB7));
    PORTB = 0;
    //PORTB ^= (-(current_game_state.roomba_states[COP1] & DEAD) ^ PORTB) & (1 << PB7);

    for(;;) {

        // Publish gamestate to each roomba in turn.
        int i;
        for(i=COP1; i<=COP1; i++) {
            Service_Publish(radio_send_service, i);
        }

        Task_Next();
    }
}

/**
 * Simple task designed to send a single packet.
 */
void sendPacket() {
    int16_t radio_send_service_value;

    RADIO_TX_STATUS radio_status;
    radiopacket_t packet;

    for(;;){
        Service_Subscribe(radio_send_service, &radio_send_service_value);

        // Set target roomba
        Radio_Set_Tx_Addr(ROOMBA_ADDRESSES[radio_send_service_value]);

        // Set packet data
        packet.type = GAMESTATE_PACKET;
        memcpy(&packet.payload.gamestate, &current_game_state, sizeof(pf_gamestate_t));

        // Fire packet
        radio_status = Radio_Transmit(&packet, RADIO_RETURN_ON_TX);
        (void)radio_status; //remove unused warning.
    }
}

/**
 * Task designed to consume all packets waiting in the radio.
 */
void receivePacket() {
    int16_t radio_receive_service_value;

    RADIO_RX_STATUS radio_status;
    radiopacket_t in_packet;

    pf_roombastate_t roomba_state;

    for(;;){
        // Pipe number is in radio_return_service_value
        Service_Subscribe(radio_receive_service, &radio_receive_service_value);

        do {
            radio_status = Radio_Receive(&in_packet);

            // We recieved a packet.
            if(radio_status == RADIO_RX_MORE_PACKETS || radio_status == RADIO_RX_SUCCESS) {

                switch(in_packet.type) {
                    // Handle roomba state change requests.
                    case ROOMBASTATE_PACKET:
                        roomba_state = in_packet.payload.roombastate;

                        // A roomba can only change its state if the state isn't forced.
                        if( (current_game_state.roomba_states[roomba_state.roomba_id] & FORCED) == 0) {
                            current_game_state.roomba_states[roomba_state.roomba_id] = (roomba_state.roomba_state & (DEAD));
                        }
                        break;
                    // Ignore everything else.
                    default:
                        break;
                }
            }

        } while(radio_status == RADIO_RX_MORE_PACKETS);
    }
}


/**
 * Read an analog value from a given channel.
 * On the AT mega2560, there are 16 available channels, thus
 * channel can be any value 0 to 15, which will correspond
 * to the analog input on the arduino board.
 *
 * Thanks to Paul Hunter and Justin Guze!
 */
uint8_t read_analog(int channel) {
    /* Set the three LSB of the Mux value. */
    /* Caution modifying this line, we want MUX4 to be set to zero, always */
    ADMUX = (ADMUX & 0xF0 ) | (0x07 & channel);

    /* We set the MUX5 value based on the fourth bit of the channel, see page 292 of the
     * ATmega2560 data sheet for detailed information */
    ADCSRB = (ADCSRB & 0xF7) | (channel & (1 << MUX5));

    /* We now set the Start Conversion bit to trigger a fresh sample. */
    ADCSRA |= (1 << ADSC);
    /* We wait on the ADC to complete the operation, when it completes, the hardware
       will set the ADSC bit to 0. */
    while ((ADCSRA & (1 << ADSC)));

    /* We setup the ADC to shift input to left, so we simply return the High register. */
    return ADCH;
}

/**
 * A Task that periodically polls a joystick for operator control of our game.
 */
void user_input() {
    /* Configure PORTB to received digital inputs for pin 12 */
    DDRB &= ~(_BV(PB6));

    /* Configure Analog Inputs using ADC */
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Set ADC prescalar to 128 - 125KHz sample rate @ 16MHz

    ADMUX |= (1 << REFS0); // Set ADC reference to AVCC
    ADMUX |= (1 << ADLAR); // Left adjust ADC result to allow easy 8 bit reading

    ADCSRA |= (1 << ADEN);  // Enable ADC
    ADCSRA |= (1 << ADSC); //Start a conversion to warmup the ADC.

    for(;;){
        EnablePort0();

        // Read joystick down.
        uint8_t analog_value = read_analog(JOYSTICK_X_CHANNEL);
        if(analog_value < 20) {
            current_game_state.roomba_states[COP1] = DEAD | FORCED;
        }
        // Read joystick up.
        if(analog_value > 235) {
            current_game_state.roomba_states[COP2] = DEAD | FORCED;
        }

        // Read joystick left.
        analog_value = read_analog(JOYSTICK_Y_CHANNEL);
        if(analog_value < 20) {
            current_game_state.roomba_states[ROBBER1] = DEAD | FORCED;
        }
        // Read joystick right.
        if(analog_value > 235) {
            current_game_state.roomba_states[ROBBER2] = DEAD | FORCED;
        }

        // If the button is pressed
        if( !(PINB & (_BV(PB6))) ) {
            // Starts the game if the game isn't currently running.
            if(current_game_state.game_state != GAME_RUNNING) {
                current_game_state.game_state = GAME_RUNNING;
                int i;
                for(i=COP1; i<ROBBER2; i++) {
                    current_game_state.roomba_states[i] = 0;
                }
            }
        }

        DisablePort0();
    }
}

/**
 * Checks if one team is dead within the game. If a team is dead, ends the game.
 */
void update_gamestate() {
    for(;;) {
        switch(current_game_state.game_state) {
        case GAME_RUNNING:
            // Check if both members of one team are dead.
            if( ( (current_game_state.roomba_states[COP1] & DEAD) != 0 && (current_game_state.roomba_states[COP2] & DEAD) != 0) ||
                ( (current_game_state.roomba_states[ROBBER1] & DEAD) != 0 && (current_game_state.roomba_states[ROBBER2] & DEAD) != 0)) {

                // End the game, force all roombas into their current state.
                current_game_state.game_state = GAME_OVER;
                int i;
                for(i=COP1; i<ROBBER2; i++) {
                    current_game_state.roomba_states[i] = current_game_state.roomba_states[i] | FORCED;
                }

                // TODO: discuss automatic revive of winning team.
            }
        default:
            break;
        }
        Task_Next();
    }
}

/**
 * A task designed to simply update the onboard leds for the base station.
 */
void display_gamestate() {
    // Set up output registers.
    DDRG |= (_BV(PG0)) | (_BV(PG1)) | (_BV(PG2));
    DDRL |= (_BV(PL5)) | (_BV(PL7));
    DDRD |= (_BV(PD7));

    PORTG &= ~( (_BV(PG0)) | (_BV(PG1)) | (_BV(PG2)) );
    PORTL &= ~( (_BV(PL5)) | (_BV(PL7)) );
    PORTD &= ~(_BV(PD7));

    for(;;) {
        // Update player status lights
        PORTD ^= (-(~current_game_state.roomba_states[COP1] & DEAD) ^ PORTD) & (1 << COP1_STATUS_LIGHT);
        PORTG ^= (-(~current_game_state.roomba_states[COP2] & DEAD) ^ PORTG) & (1 << COP2_STATUS_LIGHT);
        PORTL ^= (-(~current_game_state.roomba_states[ROBBER1] & DEAD) ^ PORTL) & (1 << ROBBER1_STATUS_LIGHT);
        PORTL ^= (-(~current_game_state.roomba_states[ROBBER2] & DEAD) ^ PORTL) & (1 << ROBBER2_STATUS_LIGHT);

        // Update gamestate status lights
        PORTL ^= (-((current_game_state.game_state == GAME_RUNNING) ? 1 : 0) ^ PORTL) & (1 << GAMESTATE_RUNNING);
        PORTG ^= (-((current_game_state.game_state != GAME_RUNNING) ? 1 : 0) ^ PORTG) & (1 << GAMESTATE_IDLE_LIGHT);

        // No need to actually loop, pass control back to another round robin.
        Task_Next();
    }
}

/**
 * RTOS initialization function.
 */
int r_main(){
    // RADIO INITIALIZATION
    DDRL |= (1 << RADIO_POWER_PIN);
    PORTL &= ~(1 << RADIO_POWER_PIN);
    _delay_ms(500);  /* max is 262.14 ms / F_CPU in MHz */
    PORTL |= 1 << RADIO_POWER_PIN;
    _delay_ms(500);

    Radio_Init(BASE_FREQUENCY);

    // configure the receive settings for radio pipe 0
    Radio_Configure_Rx(RADIO_PIPE_0, BASE_ADDRESS, ENABLE);

    // configure radio transceiver settings.
    Radio_Configure(RADIO_1MBPS, RADIO_HIGHEST_POWER);

    // GAME INITIALIZATION
    current_game_state.game_state = GAME_STARTING;
    int i;
    for(i=COP1; i<ROBBER2; i++) {
        current_game_state.roomba_states[i] = 0;
    }

    // OS INITIALIZATION
    radio_send_service = Service_Init();
    radio_receive_service = Service_Init();

    DefaultPorts();

    Task_Create_System(sendPacket, 0);
    Task_Create_System(receivePacket, 0);
    Task_Create_Periodic(sendState, 0, 20, 5, 1000);
    Task_Create_RR(user_input, 0);
    Task_Create_RR(update_gamestate, 0);
    Task_Create_RR(display_gamestate, 0);

    return 0;
}

/**
 * Interrupt called when the radio recieves a message.
 */
void radio_rxhandler(uint8_t pipe_number)
{
    // Resume the radio recieve task.
    PORTB ^= (1 << PB7);
    Service_Publish(radio_receive_service, pipe_number);
}

