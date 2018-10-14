/*
The following sketch is an example of how to send data to the Nextion display without any library.
The data can be text, numbers, or any other atribute of an object.
You can also send System Variables to change settings on the display like: brightness, serial baud rate, etc.

Connection with Arduino Uno/Nano:
* +5V = 5V
* TX  = nonea
* RX  = pin 1 (TX)
* GND = GND
*/

int variable1 = 200;  // counter


void setup() {
  Serial.begin(9600);  // Start serial comunication at baud=9600 (default for Nextion)

  delay(500);  // give the isplay some time to start
  Serial.print("baud=57600"); // Set new baud rate of nextion to 57600, but is temporal. Next time nextion is
                               // power on, it will retore to default baud of 9600.
                               // possible baudrates: 9600, 19200, 38400, 57600 und 115200
                               // To take effect, make sure to reboot the arduino (reseting arduino is not enough).
                               // If you want to change the default baud, send the command as "bauds=57600", instead of "baud=57600".
                               // If you change default, everytime the nextion is power on is going to have that baud rate.
                               // see: https://nextion.itead.cc/resources/documents/instruction-set/
  Serial.write(0xff);          // We always have to send this three lines after each command sent to nextion.
  Serial.write(0xff);
  Serial.write(0xff);

  Serial.end();  // End the serial comunication of baud=9600.

  Serial.begin(57600);  // Start serial comunication at baud=57600.
}


void loop() {  // Put your main code here, to run repeatedly:
  variable1--;  // decrease the value of the variable by 1.
  if(variable1 == 50){  // If the variable reaches 50...
    variable1 = 200;  // Set the variable to 0 so it starts over again.
  }


  // We are going to send the variable value to the object called s0 (waveform)
  // waveform: objectname: s0, id: 1; ch: 4 (four channels 0..3);
  // Waveform component only support 8-bit values, 0 minimum, 255 maximum.

  // write to waveform channel 0
  Serial.print("add 1,0,");  // This is sent to the nextion display to set what object name (before the dot) and what atribute (after the dot) are you going to change.
  Serial.print(variable1);  // This is the value you want to send to that object and atribute mention before.
  Serial.write(0xff);  // We always have to send this three lines after each command sent to the nextion display.
  Serial.write(0xff);
  Serial.write(0xff);

  // write to waveform channel 1
  Serial.print("add 1,1,");  // This is sent to the nextion display to set what object name (before the dot) and what atribute (after the dot) are you going to change.
  Serial.print(variable1 + 5);  // This is the value you want to send to that object and atribute mention before.
  Serial.write(0xff);  // We always have to send this three lines after each command sent to the nextion display.
  Serial.write(0xff);
  Serial.write(0xff);

  // write to waveform channel 2
  Serial.print("add 1,2,");  // This is sent to the nextion display to set what object name (before the dot) and what atribute (after the dot) are you going to change.
  Serial.print(variable1 + 10);  // This is the value you want to send to that object and atribute mention before.
  Serial.write(0xff);  // We always have to send this three lines after each command sent to the nextion display.
  Serial.write(0xff);
  Serial.write(0xff);

  // write to waveform channel 3
  Serial.print("add 1,3,");  // This is sent to the nextion display to set what object name (before the dot) and what atribute (after the dot) are you going to change.
  Serial.print(variable1 + 15);  // This is the value you want to send to that object and atribute mention before.
  Serial.write(0xff);  // We always have to send this three lines after each command sent to the nextion display.
  Serial.write(0xff);
  Serial.write(0xff);
}   // End of loop
