#include <avr/io.h>
#include <util/delay.h>
#include "port_map.h"
#include "os.h"
#include "kernel.h"
#include "radio.h"

#define JOYSTICK_X_CHANNEL 0
#define JOYSTICK_Y_CHANNEL 1

service_t* radio_fire_service;
int16_t radio_fire_service_value;

service_t* radio_return_service;
int16_t radio_return_service_value;

int radio_power_pin = PL2;

void setup() {
    // RADIO INITIALIZATION
    DDRL |= (_BV(radio_power_pin));
    PORTL &= ~(1 << radio_power_pin);
    _delay_ms(100);  /* max is 262.14 ms / F_CPU in MHz */
    PORTL |= 1 << radio_power_pin;
    _delay_ms(100);

    Radio_Init(BASE_FREQUENCY);

    // configure the receive settings for radio pipe 0
    Radio_Configure_Rx(RADIO_PIPE_0, BASE_ADDRESS, ENABLE);

    // configure radio transceiver settings.
    Radio_Configure(RADIO_1MBPS, RADIO_HIGHEST_POWER);

    return;
}

void sendState() {
    DDRB |= (_BV(PB7));
    PORTB = 0;
    for(;;) {
        EnablePort6();
        int i;
        for(i=COP1; i<=COP1/*ROBBER2*/; i++) {

            // The address to which the next transmission is to be sent
            Radio_Set_Tx_Addr(ROOMBA_ADDRESSES[i]);
            //TODO: Load data into packet
            Service_Publish(radio_fire_service, i);
        }
        DisablePort6();

        PORTB ^= (_BV(PB7));

        Task_Next();
    }
}

void sendPacket(){
    radiopacket_t packet;
    pf_gamestate_t gamestate_command;
    for(;;){
        Service_Subscribe(radio_fire_service, &radio_fire_service_value);
        EnablePort7();

        Radio_Set_Tx_Addr(ROOMBA_ADDRESSES[COP1]);

        // INIITIALIZING PACKET DATA
        // put some data into the packet
        gamestate_command.game_state = GAME_STARTING;

        packet.type = GAMESTATE_PACKET;
        memcpy(&packet.payload.message, &gamestate_command, sizeof(pf_gamestate_t));

        uint8_t status = Radio_Transmit(&packet, RADIO_RETURN_ON_TX);
        DisablePort7();
    }
}

void updateState() {
    for(;;){
        Service_Subscribe(radio_return_service, &radio_return_service_value);
        // Pipe number is in radio_return_service_value
        //TODO: Consume incomming packets.
    }
}


/**
 * Read an analog value from a given channel.
 * On the AT mega2560, there are 16 available channels, thus
 * channel can be any value 0 to 15, which will correspond
 * to the analog input on the arduino board.
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

        uint8_t analog_value = read_analog(JOYSTICK_X_CHANNEL);
        if(analog_value < 20) { // Down
            EnablePort1();
            _delay_ms(2);
            DisablePort1();
        }
        if(analog_value > 235) { // Up
            EnablePort2();
            _delay_ms(2);
            DisablePort2();
        }

        analog_value = read_analog(JOYSTICK_Y_CHANNEL);
        if(analog_value < 20) { // Left
            EnablePort3();
            _delay_ms(2);
            DisablePort3();
        }
        if(analog_value > 235) { // Right
            EnablePort4();
            _delay_ms(2);
            DisablePort4();
        }

        if( !(PINB & (_BV(PB6))) ) {
            EnablePort5();
            _delay_ms(2);
            DisablePort5();
        }

        DisablePort0();
    }
}

int r_main(){
    radio_fire_service = Service_Init();
    radio_return_service = Service_Init();

    DefaultPorts();

    Task_Create_System(setup, 0);
    Task_Create_System(sendPacket, 0);
    Task_Create_System(updateState, 0);
    Task_Create_Periodic(sendState, 0, 20, 5, 1000);
    Task_Create_RR(user_input, 0);

    return 0;
}

// Called from a radio interrupt.
void radio_rxhandler(uint8_t pipe_number)
{
    Service_Publish(radio_return_service, pipe_number);
}

