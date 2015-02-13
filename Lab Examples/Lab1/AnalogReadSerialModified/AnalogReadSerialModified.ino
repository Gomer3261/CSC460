/*
  AnalogReadSerial
  Reads an analog input on pin 0, prints the result to the serial monitor.
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.
 
 This example code is in the public domain.
 */

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:
  int x = analogRead(A3);
  int y = analogRead(A4);
  // print out the value you read:
  Serial.print("X:");
  if(x > 528) {
    Serial.print("Left");
  } else if(x < 496) {
    Serial.print("Right");
  } else {
    Serial.print("Idle");
  }
  Serial.print("  Y:");
  if(y > 528) {
    Serial.println("Up");
  } else if(y < 496) {
    Serial.println("Down");
  } else {
    Serial.println("Idle");
  }
  delay(1);        // delay in between reads for stability
}
