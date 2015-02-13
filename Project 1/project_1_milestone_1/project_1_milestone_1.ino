
//#include <Servo.h>

// Digital
//Servo servo;
int servo_pin = 9;
int button_pin = 5;

// Analog
int joystick_pin = A3;

// Control
boolean clicked = false;
int signature = 'D';
int sequence = 0;
int sequence_length = 10;
int sequence_location = 0;

void setup() {
  //servo.attach(servo_pin);
  
  pinMode(button_pin, INPUT);
  pinMode(servo_pin, OUTPUT);
  pinMode(13, OUTPUT);
   
  //Clear timer
  TCCR3A = 0;
  TCCR3B = 0; // Timer Counter Control Register 3
  
  // Set timer to ctc mode (resets when we hit assigned max value) Fires interrupt.
  TCCR3B |= (1<<WGM32);
  TCCR3B |= (1<<CS10); // Prescale to 1
  OCR3A = 8000; //0.0005s
  
  // Output Compare Interupt Enable 3A
  TIMSK3 |= (1<<OCIE3A);
  TCNT3 = 0;
  
  //PWM (Runs a pin, creates a square wave.)
  // Clear timer
  TCCR1A = 0;
  TCCR1B = 0;
  
  // Clear interrupt;
  TIMSK1 &= ~(1<<OCIE1C);
  //Fast PWM mode
  TCCR1A |= (1<<WGM10) | (1<<WGM11);
  TCCR1B |= (1<<WGM12) | (1<<WGM13);
  
  //Enable output on pin 13
  TCCR1A |= (1<<COM1C1);
  //No Prescaler
  TCCR1B |= (1<<CS10);
  
  OCR1A = 421; //1/38000 s period
  OCR1C = 0;
  //OCR1C = 210; // Target
  
  sequence = (signature<<2) + 1;
  sequence_location = sequence_length;
}

ISR(TIMER3_COMPA_vect) {
  if(sequence_location < sequence_length) {
    int data = (sequence & (1 << sequence_location));
    
    if(data != 0) {
      OCR1C = 210;
    } else {
      OCR1C = 0;
    }
    
    sequence_location++;
  } else {
    OCR1C = 0;
  }
}

void loop() {
  int joystick_val = analogRead(joystick_pin);
  //int servo_val = map(joystick_val, 0, 1023, 0, 180);
  int servo_val = map(joystick_val, 0, 1023, 500, 2500);
  
  // Found at: http://www.robotshop.com/blog/en/arduino-5-minute-tutorials-lesson-5-servo-motors-3636
  digitalWrite(servo_pin, HIGH);
  delayMicroseconds(servo_val);
  digitalWrite(servo_pin, LOW);
  
  //servo.write(servo_val);
  int button_pressed = digitalRead(button_pin);    
  
  if(button_pressed == LOW) {
    if(sequence_location >= sequence_length && !clicked) {
      clicked = true;
      sequence_location = 0;
    }
  } else if(clicked) {
    clicked = false;
  }
  
  delay(15);
}
