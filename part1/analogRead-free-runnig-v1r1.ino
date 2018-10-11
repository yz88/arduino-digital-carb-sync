// Testing interrupt-based analog reading
// ATMega328p

// Note, many macro values are defined in <avr/io.h> and
// <avr/interrupts.h>, which are included automatically by
// the Arduino interface


volatile int readFlag;      // High when a value is ready to be read
volatile int analogVal_ADC4;     // Value to store analog result from ADC4
volatile int analogVal_ADC5;     // Value to store analog result from ADC5
unsigned long time;

#define sensor1 4           // left pressure sensor
#define sensor2 5           // right pressure sensor

int PressureLeft = 1023;
int previousPressureLeft = 1023;
int PressureLeftMax = 1023;

int PressureRight = 1023;
int previousPressureRight = 1023;
int PressureRightMax = 1023;

int DifferenceLeft = 0;
int DifferenceRight = 0;

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif 

  
void setup() {
  Serial.begin(115200);
  Serial.println("Setup");
  Serial.println(ADMUX);
  
  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);

  // clear ADLAR in ADMUX (0x7C) to right-adjust the result
  // ADCL will contain lower 8 bits, ADCH upper 2 (in last two bits)
  cbi(ADMUX, ADLAR);
  Serial.println("Set ADLAR in ADMUX");
  Serial.println(ADMUX);

  // Set REFS1..0 in ADMUX (0x7C) to change reference voltage to the
  // proper source (01)
  sbi(ADMUX, REFS0);
  cbi(ADMUX, REFS1);
  Serial.println("Set REFS1..0 in ADMUX");
  Serial.println(ADMUX);
  
  // Set MUX3..0 in ADMUX (0x7C) to read from ADC4 (0100) 
  cbi(ADMUX, MUX3);
  sbi(ADMUX, MUX2);
  cbi(ADMUX, MUX1);
  cbi(ADMUX, MUX0);
  Serial.println("Set MUX3..0 in ADMUX");
  Serial.println(ADMUX);

  // Set ADEN in ADCSRA (0x7A) to enable the ADC.
  sbi(ADCSRA, ADEN);  // enable ADC
  Serial.println("Set ADEN in ADCSRA");
  Serial.println(ADCSRA);

  // Set ADATE in ADCSRA (0x7A) to enable auto-triggering.
  sbi(ADCSRA, ADATE); // enable auto trigger
  Serial.println("Set ADATE in ADCSRA");
  Serial.println(ADCSRA);
  
  // Clear ADTS2..0 in ADCSRB (0x7B) to set trigger mode to free running (000).
  // This means that as soon as an ADC has finished, the next will be
  // immediately started.
  cbi(ADCSRB, ADTS2);
  cbi(ADCSRB, ADTS1);
  cbi(ADCSRB, ADTS0);
  Serial.println("Set ADTS2..0 in ADCSRB");
  Serial.println(ADCSRA);

  // Set ADPS2:0: ADC Prescaler Select Bits to 128 (111) (16000KHz/128 = 125KHz)
  // Above 200KHz 10-bit results are not reliable.
  sbi(ADCSRA, ADPS2);
  sbi(ADCSRA, ADPS1);
  sbi(ADCSRA, ADPS0);
  Serial.println("Set ADPS2..0 in ADCSRA");
  Serial.println(ADCSRA);

  // Set ADIE in ADCSRA (0x7A) to enable the ADC interrupt.
  // Without this, the internal interrupt will not trigger.
  sbi(ADCSRA, ADIE);  // enable ADC interrupt when measurement complete
  Serial.println("Set ADIE in ADCSRA");
  Serial.println(ADCSRA);

  // Enable global interrupts
  // AVR macro included in <avr/interrupts.h>, which the Arduino IDE
  // supplies by default.
  sei();

  // Kick off the first ADC
  readFlag = 0;

  // Set ADSC: ADC Start Conversion in ADCSRA (0x7A) to start the ADC conversion
  sbi(ADCSRA, ADSC);  // start ADC measurements
  Serial.println("Set ADSC in ADCSRA");
  Serial.println(ADCSRA);

  // calibrate sensors
  PressureLeft = analogVal_ADC4;
  PressureRight = analogVal_ADC5;
  if (PressureLeft > PressureRight){
    DifferenceRight = PressureLeft - PressureRight;
  } else {
    DifferenceLeft = PressureRight - PressureLeft;
  }
    
}


void loop(){ 
  // Check to see if the value has been updated
  if (readFlag == 1){
    PressureLeft = analogVal_ADC4 + DifferenceLeft;
    PressureRight = analogVal_ADC5 + DifferenceRight;
    Serial.print(PressureLeft);
    Serial.print("  ");
    Serial.println (PressureRight);
    readFlag = 0;
  }
  // Whatever else you would normally have running in loop().
}


// Interrupt service routine for the ADC completion
ISR(ADC_vect){
  // Done reading
  readFlag = 1;

  // toggling analog input pin between 4 (A4) and 5 (A5)
  if (ADMUX == 0x44) {
    // Must read low first
    analogVal_ADC4 = ADCL | (ADCH << 8);
    // Set MUX3..0 in ADMUX (0x7C) to read from ADC4 (0100) to read from ACD5 (0101)
    sbi(ADMUX, MUX0);
  }
  else if (ADMUX == 0x45){
    // Must read low first
    analogVal_ADC5 = ADCL | (ADCH << 8);
    // Set MUX3..0 in ADMUX (0x7C) to read from ADC5 (0101) to read from ACD5 (0100)
    cbi(ADMUX, MUX0);
  }
  
  // Not needed because free-running mode is enabled.
  // Set ADSC in ADCSRA (0x7A) to start another ADC conversion
  // sbi(ADCSRA, ADSC);  // start ADC measurements
}
