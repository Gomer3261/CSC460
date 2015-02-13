/* 
 Controlling a servo position using a potentiometer (variable resistor) 
 by Michal Rinott <http://people.interaction-ivrea.it/m.rinott> 

 modified on 8 Nov 2013
 by Scott Fitzgerald
 http://arduino.cc/en/Tutorial/Knob
*/

#include <Servo.h> 
 
Servo xservo;  // create servo object to control a servo 
Servo yservo;
 
int xpin = 0;  // Pins used for inputs
int ypin = 1;

int xval; // Storing values read from analog.
int yval;

int testpin = 4;
 
void setup() 
{ 
  xservo.attach(9);
  yservo.attach(8);
  
  pinMode(testpin, OUTPUT);
} 
 
void loop() 
{ 
  xval = analogRead(xpin);            
  xval = map(xval, 0, 1023, 0, 180);
  xservo.write(xval);
  
  if(xval > 95) {
    digitalWrite(testpin, HIGH);
  } else {
    digitalWrite(testpin, LOW);
  }
  
  yval = analogRead(ypin); 
  yval = map(yval, 0, 1023, 0, 180);
  yservo.write(yval);
  delay(15);
} 
