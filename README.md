# arduino-digital-carb-sync
Motorcycle carburetor synchronization based on Arduino Uno, Nextion Display and analog pressure sensors.

## the theory
The part of the signal that ramps downward is the intake stroke of the engine. The part of the signal that ramps upward is the compression, power and exhaust stroke combined.

![Pressure in the intake duct.](https://github.com/yz88/arduino-digital-carb-sync/blob/master/pressure-intake-duct.png)

The entire vacuum signal from the intake stroke alone is analyzed for synchronization by finding the min peak of the vacuum. If the min peaks of both cylinders are on the same level, the carburetors are syncronized.

## bill of material
* Arduino Uno
* Nextion Display (NX3224T024_011R (R: Resistive touchscreen) )
* Analog Pressure Sensors (MPX4115AP)

## step by step development
![Part 1 - Arduino freerunning analog read (first steps)](https://github.com/yz88/arduino-digital-carb-sync/blob/master/part1/)

![Part 2 - Arduino freerunning analog read and smoothing](https://github.com/yz88/arduino-digital-carb-sync/blob/master/part2/)

![Part 3 - Arduino and Nextion Display (first steps)](https://github.com/yz88/arduino-digital-carb-sync/blob/master/part3/)

![Part 4 - Arduino and Nextion Display serial plot](https://github.com/yz88/arduino-digital-carb-sync/blob/master/part4/)
