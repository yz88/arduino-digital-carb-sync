// interrupt-based analog reading with Arduino's ATMega328p
// to measure pressure sensor values for synchronisation of
// two motorcycle carburators
// https://github.com/yz88/arduino-digital-carb-sync/


volatile int readFlag;      // High when a value is ready to be read
volatile int analogVal_ADC4;     // Value to store analog result from ADC4
volatile int analogVal_ADC5;     // Value to store analog result from ADC5
unsigned long time;

#define sensor1 4           // left pressure sensor
#define sensor2 5           // right pressure sensor

int PressureLeft = 1023;
int previousPressureLeft = 1023;
int PressureLeftMin = 1023;
float PressureLeftFiltered = 1023;

int PressureRight = 1023;
int previousPressureRight = 1023;
int PressureRightMin = 1023;
float PressureRightFiltered = 1023;

int DifferenceLeft = 0;
int DifferenceRight = 0;

const int RANGE = 3;

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


void setup() {
  // Start serial comunication at baud=9600 (default for Nextion display)
  Serial.begin(9600);

  // give the Nextion display some time to start
  delay(500);

  // Set new baud rate for Nextion display to 115200
  Serial.print("baud=115200");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  // End the serial comunication of baud=9600.
  Serial.end();
  // start serial comunication at baud=115200.
  Serial.begin(115200);

  // initialise Arduino's input pins
  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);

  // settign Arduino's registers for freerunning anlaog read
  // clear ADLAR in ADMUX (0x7C) to right-adjust the result
  // ADCL will contain lower 8 bits, ADCH upper 2 (in last two bits)
  cbi(ADMUX, ADLAR);

  // Set REFS1..0 in ADMUX (0x7C) to change reference voltage to the
  // proper source (01)
  sbi(ADMUX, REFS0);
  cbi(ADMUX, REFS1);

  // Set MUX3..0 in ADMUX (0x7C) to read from ADC4 (0100)
  cbi(ADMUX, MUX3);
  sbi(ADMUX, MUX2);
  cbi(ADMUX, MUX1);
  cbi(ADMUX, MUX0);

  // Set ADEN in ADCSRA (0x7A) to enable the ADC.
  sbi(ADCSRA, ADEN);  // enable ADC

  // Set ADATE in ADCSRA (0x7A) to enable auto-triggering.
  sbi(ADCSRA, ADATE); // enable auto trigger

  // Clear ADTS2..0 in ADCSRB (0x7B) to set trigger mode to free running (000).
  // This means that as soon as an ADC has finished, the next will be
  // immediately started.
  cbi(ADCSRB, ADTS2);
  cbi(ADCSRB, ADTS1);
  cbi(ADCSRB, ADTS0);

  // Set ADPS2:0: ADC Prescaler Select Bits to 128 (111) (16000KHz/128 = 125KHz)
  // Above 200KHz 10-bit results are not reliable.
  sbi(ADCSRA, ADPS2);
  sbi(ADCSRA, ADPS1);
  sbi(ADCSRA, ADPS0);

  // Set ADIE in ADCSRA (0x7A) to enable the ADC interrupt.
  // Without this, the internal interrupt will not trigger.
  sbi(ADCSRA, ADIE);  // enable ADC interrupt when measurement complete

  // Enable global interrupts
  // AVR macro included in <avr/interrupts.h>, which the Arduino IDE
  // supplies by default.
  sei();

  // Kick off the first ADC
  readFlag = 0;

  // Set ADSC: ADC Start Conversion in ADCSRA (0x7A) to start the ADC conversion
  sbi(ADCSRA, ADSC);  // start ADC measurements

  delay(100);

  // etermine difference between senor values
  // get some values and find the lowest value
  for (int i=0; i <= 1000; i++){
    PressureLeft = analogVal_ADC4;
    PressureRight = analogVal_ADC5;
    if (PressureLeft < previousPressureLeft) {
      previousPressureLeft = PressureLeft;  // store the lowest value
    }
    if (PressureRight < previousPressureRight) {
      previousPressureRight = PressureRight;  // store the lowest value
    }
  }

  // compare the lowest values of ADC4 and ADC5, then calculating the difference
  if (previousPressureLeft > previousPressureRight){
    DifferenceRight = previousPressureLeft - previousPressureRight;
  } else {
    DifferenceLeft = previousPressureRight - previousPressureLeft;
  }
}


// Interrupt service routine for the ADC completion
ISR(ADC_vect){
  // toggling analog input pin between 4 (A4) and 5 (A5)
  if (ADMUX == 0x44) {
    // Must read low first
    analogVal_ADC4 = ADCL | (ADCH << 8);
    // Set MUX3..0 in ADMUX (0x7C) to read from ADC4 (0100) to read from ACD5 (0101)
    sbi(ADMUX, MUX0);
  } else if (ADMUX == 0x45){
    // Must read low first
    analogVal_ADC5 = ADCL | (ADCH << 8);
    // Set MUX3..0 in ADMUX (0x7C) to read from ADC5 (0101) to read from ACD5 (0100)
    cbi(ADMUX, MUX0);
  }
}


void loop(){
  // calibrate pressure sensors to the same level
  PressureLeft = analogVal_ADC4 + DifferenceLeft;
  PressureRight = analogVal_ADC5 + DifferenceRight;

  // filter out noise, the measured value The input signals vary in value by one up and down.
  if (PressureLeft > (PressureLeftFiltered + RANGE) || PressureLeft < (PressureLeftFiltered - RANGE)) {
    PressureLeftFiltered = PressureLeft;
  }
  if (PressureRight > (PressureRightFiltered + RANGE) || PressureRight < (PressureRightFiltered - RANGE)) {
    PressureRightFiltered = PressureRight;
  }

  // check for min values
  if (PressureLeftFiltered < previousPressureLeft && PressureLeftFiltered < 780) {
    PressureLeftMin = PressureLeftFiltered;
  }
  if (PressureRightFiltered < previousPressureRight && PressureRightFiltered < 780) {
    PressureRightMin = PressureRightFiltered;
  }

  Serial.print("add 1,0,");
  // Nextion isplay waveform accepts values between 0 - 255
  // Measured values are in the range from 850 down to 400, thus divide the
  // values by 5 to fit the waveform's value range
  Serial.print((int)PressureLeftFiltered/5);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  Serial.print("add 1,1,");
  Serial.print((int)PressureLeftMin/5);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  Serial.print("add 1,2,");
  Serial.print((int)PressureRightFiltered/5);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  Serial.print("add 1,3,");
  Serial.print((int)PressureRightMin/5);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);

  previousPressureLeft = PressureLeftFiltered;
  previousPressureRight = PressureRightFiltered;
}
