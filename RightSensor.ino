//New format
//GND -> GND
//Echo -> Pin 13 -> PB5
//Trig -> Pin 12 -> PB4
//VCC -> Pin 11 -> PB3


// HC-SR04 ultrasonic rangefinder demo
// Popular code uses pulseIn to measure echo pulse, pulseIn is a blocking function
// Using interrupts for reading the incoming pulse is better approach because it won't stall your main program
// You are welcome to use this code for the project
// You might need to do additional calibration and data filtering
// Note sources of innacuracies of some components like micros(). It has a small error, check online why this happens

// Pinout:
// VCC (5VDC)
// GND (return)
// TRIG (trigger input)
// ECHO (echo output)

// For sensor operation refer to datasheet:
// https://www.mouser.com/ds/2/813/HCSR04-1022824.pdf


#define TRIG PB4  // trigger pin Arduino Pin 12
#define ECHO PB5  // echo output Arduino Pin 13
#define VCC PB3 //Vcc Arduino Pin 11
#define vmotor PB0 //vibration motor output Arduino Pin8

// riseTime and fallTime will be modified by interrupt so we need to declare them as volatile
volatile unsigned long riseTime;  // timestamp when echo signal goes high
volatile unsigned long fallTime;  // timestamp when echo signal goes low
unsigned long pulseTime;          // difference between riseTime and fallTime
unsigned long distance;           // our range

float getDistance();

void setup() 
{
  Serial.begin(9600);

  // Setting direction for trig and echo pins
  DDRB |= (1<<TRIG);
  DDRB &= ~(1<<ECHO);

  // Setting up pin change interrupt for echo (PB5), PCINT5
  PCICR = (1<<PCIE0); // only enabling PCIE0
  PCMSK0 = (1<<PCINT5); // only enabling PCINT5  

  DDRB |= (1<<vmotor); //Set as output
  DDRB |= (1<<VCC); //Set as output
  PORTB |= (1<<VCC); //Turn on pin 11
}

void loop() 
{
  
  float range = getDistance();
  if (range < 85){ //Beeps if close to right wall/obstacle
    beep();
  }
  if(range > 400 && range < 600){ //vibrates when doorway is reached
    vibrate();
  }
  Serial.println(range);
  delay(100);
}

// function to calculate the distance
float getDistance()
{
  // clear trig pin
  PORTB &= ~(1<<TRIG);
  delayMicroseconds(2);
  
  // send out 10 us pulse
  PORTB = (1<<TRIG);
  delayMicroseconds(10);
  PORTB &= ~(1<<TRIG);

  // sound travels at 343 m/s which is 0.0343 cm/us
  // distance = time*velocity
  // we need to divide result by 2 because sound travels 
  // to object and back so the incoming pulse is twice longer
  distance = pulseTime*0.0343/2; // result in cm 
  return distance;
}

// Interrupt service vector for pin change:
// ISR (PCINT0_vect) pin change interrupt for D8 to D13
// ISR (PCINT1_vect) pin change interrupt for A0 to A5
// ISR (PCINT2_vect) pin change interrupt for D0 to D7
// We need PCINT0_vect because we are using PB5 as input (echo)
ISR(PCINT0_vect)
{  
  if ((PINB & (1<<ECHO)) == (1<<ECHO)) // check if pin PB5 is high
    { // Measure time when echo pin goes high
      riseTime = micros(); // micros() calculates the run time in us since the program's start
    }    
  else 
    {
      // measure the time when echo pin goes low
      fallTime = micros();
      pulseTime = fallTime - riseTime; // this is our echo pulse, its length is proportional to the measured distance
    }
  Serial.println("ISR");
}

void beep(){
  tone(A0, 80, 200);
  delay(100);
  // stop the tone playing:
  noTone(A0);
}

void vibrate(){
   PORTB |= (1<<vmotor); //vibrate motor
   delay(100);
   PORTB &= ~(1<<vmotor); //turn off motor
}

