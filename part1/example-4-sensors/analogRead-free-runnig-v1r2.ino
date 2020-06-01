// Testing interrupt-based analog reading
// ATMega328p

// Note, many macro values are defined in <avr/io.h> and
// <avr/interrupts.h>, which are included automatically by
// the Arduino interface

volatile int readFlag;       // High when a value is ready to be read
volatile int analogVal_ADC0; // Value to store analog result from ADC0
volatile int analogVal_ADC1; // Value to store analog result from ADC1
volatile int analogVal_ADC2; // Value to store analog result from ADC2
volatile int analogVal_ADC3; // Value to store analog result from ADC3
unsigned long time;

#define sensor0 0 // pressure sensor ADC0
#define sensor1 1 // pressure sensor ADC1
#define sensor2 2 // pressure sensor ADC2
#define sensor3 3 // pressure sensor ADC3

int PressureSensor0 = 1023;
int LowestValueSensor0 = 1023;
int DifferenceSensor0 = 0;

int PressureSensor1 = 1023;
int LowestValueSensor1 = 1023;
int DifferenceSensor1 = 0;

int PressureSensor2 = 1023;
int LowestValueSensor2 = 1023;
int DifferenceSensor2 = 0;

int PressureSensor3 = 1023;
int LowestValueSensor3 = 1023;
int DifferenceSensor3 = 0;

int LowestValueAllSensors = 1023;

/** 
 ** Clear BIT function
 * this function is used to clear a BIT in a register
 * -> set BIT to logic '0'
 */
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

/** 
 ** Set BIT function 
 * this function is used t set  a BIT in a register
 * -> set BIT to logic '1' 
 */
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

void setup()
{
    Serial.begin(115200);

    pinMode(sensor0, INPUT);
    pinMode(sensor1, INPUT);
    pinMode(sensor2, INPUT);
    pinMode(sensor3, INPUT);

    // clear ADLAR in ADMUX (0x7C) to right-adjust the result
    // ADCL will contain lower 8 bits, ADCH upper 2 (in last two bits)
    cbi(ADMUX, ADLAR);

    // Set REFS1..0 in ADMUX (0x7C) to change reference voltage to the
    // proper source (01)
    sbi(ADMUX, REFS0);
    cbi(ADMUX, REFS1);

    /**
   ** ADMUX - ADC Mulitplexer Selection Register
   * Bit            |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
   *                +-------+-------+-------+-------+-------+-------+-------+-------+
   * (0x7C)         | REFS1 | REFS0 | ADLAR |  MUX4 |  MUX3 |  MUX2 |  MUX1 |  MUX0 |
   *                +-------+-------+-------+-------+-------+-------+-------+-------+
   * Read/Write     |  R/W  |  R/W  |  R/W  |  R/W  |  R/W  |  R/W  |  R/W  |  R/W  |
   * Inital Value   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |
   * 
   ** Changing Channel or Reference Selection
   * The Analog Channel Selection bits (MUX) in the ADC Multiplexer Selection Register (ADMUX.MUX[3:0])
   * determine the analog input to be read from.
   * 
   * +------+------+------+------+------------+
   * | MUX3 | MUX2 | MUX1 | MUX0 | Analog PIN |
   * +------+------+------+------+------------+
   * |  0   |  0   |  0   |  0   |   ADC0     |
   * |  0   |  0   |  0   |  1   |   ADC1     |
   * |  0   |  0   |  1   |  0   |   ADC2     |
   * |  0   |  0   |  1   |  1   |   ADC3     |
   * |  0   |  1   |  0   |  0   |   ADC4     |
   * |  0   |  1   |  0   |  1   |   ADC5     |
   * |  0   |  1   |  1   |  0   |   ADC6     |
   * +------+------+------+------+------------+
   */

    // Set MUX3..0 in ADMUX (0x7C) to read from ADC0 (0000)
    cbi(ADMUX, MUX3);
    cbi(ADMUX, MUX2);
    cbi(ADMUX, MUX1);
    cbi(ADMUX, MUX0);

    // Set ADEN in ADCSRA (0x7A) to enable the ADC.
    sbi(ADCSRA, ADEN); // enable ADC

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
    sbi(ADCSRA, ADIE); // enable ADC interrupt when measurement complete

    // Enable global interrupts
    // AVR macro included in <avr/interrupts.h>, which the Arduino IDE
    // supplies by default.
    sei();

    // Kick off the first ADC
    readFlag = 0;

    // Set ADSC: ADC Start Conversion in ADCSRA (0x7A) to start the ADC conversion
    sbi(ADCSRA, ADSC); // start ADC measurements

    delay(100);

    // calibrate sensors to the same level
    // get some values and find the lowest value
    for (int i = 0; i <= 255; i++)
    {
        PressureSensor0 = analogVal_ADC0;
        PressureSensor1 = analogVal_ADC1;
        PressureSensor2 = analogVal_ADC2;
        PressureSensor3 = analogVal_ADC3;
        if (PressureSensor0 < LowestValueSensor0)
        {
            LowestValueSensor0 = PressureSensor0; // store the lowest value
        }
        if (PressureSensor1 < LowestValueSensor1)
        {
            LowestValueSensor1 = PressureSensor1; // store the lowest value
        }
        if (PressureSensor2 < LowestValueSensor2)
        {
            LowestValueSensor2 = PressureSensor2; // store the lowest value
        }
        if (PressureSensor3 < LowestValueSensor3)
        {
            LowestValueSensor3 = PressureSensor3; // store the lowest value
        }
    }

    // compare the lowest values of ADC0, ADC1, ADC2 and ADC3, then calculating the difference

    if (LowestValueSensor0 < LowestValueAllSensors)
    {
        LowestValueAllSensors = LowestValueSensor0
    }
    if (LowestValueSensor1 < LowestValueAllSensors)
    {
        LowestValueAllSensors = LowestValueSensor1
    }
    if (LowestValueSensor2 < LowestValueAllSensors)
    {
        LowestValueAllSensors = LowestValueSensor2
    }
    if (LowestValueSensor3 < LowestValueAllSensors)
    {
        LowestValueAllSensors = LowestValueSensor3
    }

    // Difference of each sensor is positive or in one case zero
    DifferenceSensor0 = LowestValueSensor0 - LowestValueAllSensors;
    DifferenceSensor1 = LowestValueSensor1 - LowestValueAllSensors;
    DifferenceSensor2 = LowestValueSensor2 - LowestValueAllSensors;
    DifferenceSensor3 = LowestValueSensor3 - LowestValueAllSensors;
}

void loop()
{
    // Check to see if the value has been updated
    if (readFlag == 1)
    {
        PressureSensor0 = analogVal_ADC0 - DifferenceSensor0;
        PressureSensor1 = analogVal_ADC1 - DifferenceSensor1;
        PressureSensor1 = analogVal_ADC2 - DifferenceSensor2;
        PressureSensor1 = analogVal_ADC3 - DifferenceSensor3;
        Serial.print(PressureSensor0);
        Serial.print("  ");
        Serial.print(PressureSensor1);
        Serial.print("  ");
        Serial.print(PressureSensor2);
        Serial.print("  ");
        Serial.println(PressureSensor3);
        readFlag = 0;
    }
    // Whatever else you would normally have running in loop().
}

// Interrupt service routine for the ADC completion
ISR(ADC_vect)
{
    // Done reading
    readFlag = 1;

    // toggling analog input pins
    if (ADMUX == 0x40)
    {
        /**
        ** ADMUX - ADC Mulitplexer Selection Register
        * actually ADMUX is set to read from ADC0 (0000)
        * Bit     |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
        *         +-------+-------+-------+-------+-------+-------+-------+-------+
        * (0x7C)  | REFS1 | REFS0 | ADLAR |  MUX4 |  MUX3 |  MUX2 |  MUX1 |  MUX0 |
        *         +-------+-------+-------+-------+-------+-------+-------+-------+
        * Value   |   1   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |
        */
        // Must read low first
        analogVal_ADC0 = ADCL | (ADCH << 8);
        /**
         ** Set MUX3..0 in ADMUX (0x7C) to read from ACD1 (0001)
         * do this by setting MUX0 to '1'
         */
        sbi(ADMUX, MUX0);
    }
    else if (ADMUX == 0x41)
    {
        /**
        ** ADMUX - ADC Mulitplexer Selection Register
        * actually ADMUX is set to read from ADC1 (0001)
        * Bit     |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
        *         +-------+-------+-------+-------+-------+-------+-------+-------+
        * (0x7C)  | REFS1 | REFS0 | ADLAR |  MUX4 |  MUX3 |  MUX2 |  MUX1 |  MUX0 |
        *         +-------+-------+-------+-------+-------+-------+-------+-------+
        * Value   |   1   |   0   |   0   |   0   |   0   |   0   |   0   |   1   |
        */
        // Must read low first
        analogVal_ADC1 = ADCL | (ADCH << 8);
        /**
         ** Set MUX3..0 in ADMUX (0x7C) to read from ACD2 (0010)
         * do this by setting MUX0 to '0' and  MUX1 to '1'
         */
        cbi(ADMUX, MUX0);
        sbi(ADMUX, MUX1);
    }
    else if (ADMUX == 0x42)
    {
        /**
        ** ADMUX - ADC Mulitplexer Selection Register
        * actually ADMUX is set to read from ADC2 (0010)
        * Bit     |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
        *         +-------+-------+-------+-------+-------+-------+-------+-------+
        * (0x7C)  | REFS1 | REFS0 | ADLAR |  MUX4 |  MUX3 |  MUX2 |  MUX1 |  MUX0 |
        *         +-------+-------+-------+-------+-------+-------+-------+-------+
        * Value   |   1   |   0   |   0   |   0   |   0   |   0   |   1   |   0   |
        */
        // Must read low first
        analogVal_ADC2 = ADCL | (ADCH << 8);
        /**
         ** Set MUX3..0 in ADMUX (0x7C) to read from ACD3 (0011)
         * do this by setting MUX0 to '1'
         */
        sbi(ADMUX, MUX0);
    }
    else if (ADMUX == 0x43)
    {
        /**
        ** ADMUX - ADC Mulitplexer Selection Register
        * actually ADMUX is set to read from ADC3 (0011)
        * Bit     |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
        *         +-------+-------+-------+-------+-------+-------+-------+-------+
        * (0x7C)  | REFS1 | REFS0 | ADLAR |  MUX4 |  MUX3 |  MUX2 |  MUX1 |  MUX0 |
        *         +-------+-------+-------+-------+-------+-------+-------+-------+
        * Value   |   1   |   0   |   0   |   0   |   0   |   0   |   1   |   1   |
        */
        // Must read low first
        analogVal_ADC2 = ADCL | (ADCH << 8);
        /**
         ** Set MUX3..0 in ADMUX (0x7C) to read from ACD0 (0000)
         * do this by setting MUX0 and MUX1 to '0'
         */
        cbi(ADMUX, MUX0);
        cbi(ADMUX, MUX1);
    }

    // Not needed because free-running mode is enabled.
    // Set ADSC in ADCSRA (0x7A) to start another ADC conversion
    // sbi(ADCSRA, ADSC);  // start ADC measurements
}
