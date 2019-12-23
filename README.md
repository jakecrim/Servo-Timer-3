# Servo-Timer-3
A WIP version of the Arduino Servo library that works on the Atmega32u4 microcontroller.
Allows for usage of the 16 bit Timer1 while doing servo control. The original servo library made use of this timer and would often interfere with any timer1 related tasks. This library attempts to make use of 8 bit timers for controlling the servos.

Example usage included in 'src' folder where the library was used to control a prosthetic hand.
