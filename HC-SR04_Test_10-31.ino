
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


#define TRIG PD2  // trigger pin
#define ECHO PD3  // echo output

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
  DDRD |= (1<<TRIG);
  DDRD &= ~(1<<ECHO);

  // Setting up pin change interrupt for echo (PD3), PCINT19
  PCICR = (1<<PCIE2); // only enabling PCIE2, because PCIE2 constains PCINT[23:16]
  PCMSK2 = (1<<PCINT19); // only enabling PCINT19  

  //Buzzer Pin 5
  
  TCCR0A = 0x23;
  TCCR0B = 0x0B;
  OCR0A = 250; // clk/64/250 = 1000
  OCR0B = 125;
}

void loop() 
{
  float range = getDistance();
  if (range < 100){
    beep();
  }
  Serial.println(range);
  delay(100);
}

// function to calculate the distance
float getDistance()
{
  // clear trig pin
  PORTD &= ~(1<<TRIG);
  delayMicroseconds(2);
  
  // send out 10 us pulse
  PORTD = (1<<TRIG);
  delayMicroseconds(10);
  PORTD &= ~(1<<TRIG);

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
// We need PCINT2_vect because we are using PD3 as input (echo)
ISR(PCINT2_vect)
{  
  if ((PIND & (1<<ECHO)) == (1<<ECHO)) // check if pin PD3 is high
    { // Measure time when echo pin goes high
      riseTime = micros(); // micros() calculates the run time in us since the program's start
    }    
  else 
    {
      // measure the time when echo pin goes low
      fallTime = micros();
      pulseTime = fallTime - riseTime; // this is our echo pulse, its length is proportional to the measured distance
    }

}
void beep(){
  DDRD |= (1<<5);
  delay(100);
  DDRD &= ~0x20; // set 5th bit to 0
}

