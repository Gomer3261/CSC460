#include "radio.h"
#include "tta.h"
#include "cops_and_robbers.h"

#define IR_MESSAGE_DATA 'A'
#define CONTROLLED_ROOMBA COP1

#define HIGHEST_VELOCITY 500 // MAX 500
#define LOWEST_VELOCITY 100 // MIN 0
#define HIGHEST_TURN_VELOCITY 350
#define LOWEST_TURN_VELOCITY 100
#define LARGEST_TURN_RADIUS 1500 // MAX 2000

// Debug
int led_pin = 13;

// Joystick
int joystick_button_pin = 12;
int joystick_pin_x = A0;
int joystick_pin_y = A1;

boolean clicked = false;

// Radio

// Interrupt fired flag
volatile uint8_t rxflag = 0;

int radio_power_pin = 47; //PL2 ( T5 )

// packets are transmitted to this address
uint8_t roomba_addr[5] = { 0xAB, 0xAB, 0xAB, 0xAB, 0xAB };
 
// this is this radio's address
uint8_t my_addr[5] = { 0xF3, 0xF3, 0xF3, 0xF3, 0xF3 };

pf_command_t roomba_command;
pf_ir_command_t ir_command;

radiopacket_t packet;

char output[128];
 
void setup()
{
  Serial.begin(9600);
  
  pinMode(led_pin, OUTPUT);
  pinMode(joystick_button_pin, INPUT);
  pinMode(radio_power_pin, OUTPUT);
 
  // RADIO INITIALIZATION
  digitalWrite(radio_power_pin, LOW);
  delay(100);
  digitalWrite(radio_power_pin, HIGH);
  delay(100);
 
  Radio_Init(ROOMBA_FREQUENCIES[CONTROLLED_ROOMBA]);

  // configure the receive settings for radio pipe 0
  Radio_Configure_Rx(RADIO_PIPE_0, my_addr, ENABLE);
  
  // configure radio transceiver settings.
  Radio_Configure(RADIO_2MBPS, RADIO_HIGHEST_POWER);
  
  // SCHEDULER INITIALIZATION
  Scheduler_Init();
  
  Scheduler_StartTask(0, 200, fire_ir_task);
  Scheduler_StartTask(20, 200, joystick_control_task);
  Scheduler_StartTask(40, 200, radio_receive_task);
 
  // The address to which the next transmission is to be sent
  Radio_Set_Tx_Addr(ROOMBA_ADDRESSES[CONTROLLED_ROOMBA]);
  
  //memcpy(packet.payload.message.address, my_addr, RADIO_ADDRESS_LENGTH);
  //snprintf((char*)packet.payload.message.messagecontent, sizeof(packet.payload.message.messagecontent), "Carrots");
}

void loop() {
  uint32_t idle_period = Scheduler_Dispatch();
  if (idle_period)
  {
    idle(idle_period);
  }
}

// task function for ir task
void fire_ir_task()
{
  int button_pressed = digitalRead(joystick_button_pin);
  if(button_pressed == LOW) {
    // send the data
    if(!clicked) {
      // INIITIALIZING PACKET DATA
      // put some data into the packet
      memcpy(ir_command.sender_address, my_addr, RADIO_ADDRESS_LENGTH);
      ir_command.ir_command = SEND_BYTE;
      ir_command.ir_data = IR_MESSAGE_DATA;
      ir_command.servo_angle = 0;
      
      packet.type = IR_COMMAND;
      memcpy(&packet.payload.message, &ir_command, sizeof(pf_ir_command_t));
      
      Radio_Transmit(&packet, RADIO_WAIT_FOR_TX);
      clicked = true;
    }
  }
}

// task function for movement task
void joystick_control_task()
{
  int joystick_val_x = analogRead(joystick_pin_x);
  int joystick_val_y = analogRead(joystick_pin_y);
  
  
  
  //move roomba
  int16_t velocity = 0;
  if(joystick_val_x <= 490) {
    velocity = map(joystick_val_x, 0, 512, HIGHEST_VELOCITY, LOWEST_VELOCITY);
  } else if(joystick_val_x >= 520) {
    velocity = map(joystick_val_x, 512, 1020, -LOWEST_VELOCITY, -HIGHEST_VELOCITY);
  }
  
  int16_t radius = 32768;
  if( (joystick_val_y <= 10 && !(joystick_val_x <= 460 || joystick_val_x >= 560)) || (velocity == 0 && joystick_val_y <= 490) ) {
    radius = -1;
    velocity = map(joystick_val_y, 0, 512, HIGHEST_TURN_VELOCITY, LOWEST_TURN_VELOCITY);
  } else if( (joystick_val_y >= 1005 && !(joystick_val_x <= 460 || joystick_val_x >= 560)) || (velocity == 0 && joystick_val_y >= 520) ) {
    radius = 1;
    velocity = map(joystick_val_y, 512, 1023, LOWEST_TURN_VELOCITY, HIGHEST_TURN_VELOCITY);
  } else if(joystick_val_y >= 520) {
    radius = map(joystick_val_y, 512, 1023, LARGEST_TURN_RADIUS, 1);
  } else if(joystick_val_y <= 490) {
    radius = map(joystick_val_y, 0, 512, -1, -LARGEST_TURN_RADIUS);
  }
  
  Serial.print(velocity);
  Serial.print(" : ");
  Serial.println(radius);
  
  memcpy(roomba_command.sender_address, my_addr, RADIO_ADDRESS_LENGTH);
  roomba_command.command = 137;
  roomba_command.num_arg_bytes = 4;
  roomba_command.arguments[0] = velocity >> 8;
  roomba_command.arguments[1] = velocity & 255;
  roomba_command.arguments[2] = radius >> 8;
  roomba_command.arguments[3] = radius & 255;
  //memcpy(roomba_command.arguments, &velocity, 2);
  //memcpy((roomba_command.arguments) + 2, &radius, 2);
  
  packet.type = COMMAND;
  memcpy(&packet.payload.message, &roomba_command, sizeof(roomba_command));
      
  Radio_Transmit(&packet, RADIO_WAIT_FOR_TX);
}

// task function for radio receive task
void radio_receive_task()
{
  if (rxflag)
  {
    // remember always to read the packet out of the radio, even if you don't use the data.
    // if there are no more packets on the radio, clear the receive flag;
    // otherwise, we want to handle the next packet on the next loop iteration.
    
    RADIO_RX_STATUS radio_status;
    do {
      radio_status = Radio_Receive(&packet);
      
      if(radio_status == RADIO_RX_FIFO_EMPTY || radio_status == RADIO_RX_SUCCESS) {
        snprintf(output, 128, "Message from 0x%.2X%.2X%.2X%.2X%.2X: '%s'\n\r",
					packet.payload.message.address[0],
					packet.payload.message.address[1],
					packet.payload.message.address[2],
					packet.payload.message.address[3],
					packet.payload.message.address[4],
					packet.payload.message.messagecontent);
        Serial.print(output);
      }
      
    } while(radio_status == RADIO_RX_MORE_PACKETS);
    
    rxflag = 0;
    digitalWrite(led_pin, LOW);
  }
}

// idle task
void idle(uint32_t idle_period)
{
  // this function can perform some low-priority task while the scheduler has nothing to run.
  // It should return before the idle period (measured in ms) has expired.  For example, it
  // could sleep or respond to I/O.
 
  // example idle function that just pulses a pin.
  digitalWrite(led_pin, HIGH);
  
  int button_pressed = digitalRead(joystick_button_pin);
  if(button_pressed == HIGH && clicked) {
    clicked = false;
  }
  
  delay(idle_period);
  digitalWrite(led_pin, LOW);
}

// Called from a radio interrupt.
// Sets radio recieved flag high
void radio_rxhandler(uint8_t pipe_number)
{
  rxflag = 1;
}
