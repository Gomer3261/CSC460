void setup() {
   pinMode(13, OUTPUT);
   
  //Clear timer
  // C also exists, but we can ignore this. We are just clearing flags.
  TCCR3A = 0;
  TCCR3B = 0; // Timer Counter Control Register 3
  
  // Set timer to ctc mode (resets when we hit assigned max value) Fires interrupt.
  TCCR3B |= (1<<WGM32);
  
  // Prescale to 256
  TCCR3B |= (1<<CS32);
  
  // Point at which the timer hits max and resets
  OCR3A = 3125; //0.05s
  
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
  
  OCR1A = 800; //50 us period
  OCR1C = 0; // Target
}

ISR(TIMER3_COMPA_vect) {
  // Slowly increase the duy cycle of the LED
  // Could change OCR1A to increase/decrease the frequency also.
  OCR1C += 10;
  if(OCR1C >= 800) {
    OCR1C = 0;
  }
}

void loop() {
}
