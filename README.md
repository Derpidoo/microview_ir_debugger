# microview_ir_debugger
Simple IR Debugger for the SparkFun MicroView

## INTRODUCTION

Here's a simple Arduino project, utilizing (i.e. mostly ripped from) Ken Shirriff's
wonderful Arduino IRremote library, to turn the SparkFun Microview into a nifty 
little portable IR debugger. 

The included wiring schematic describes:

* TSOP 38238 IR receiver connected to MicroView pin 6/Arduino pin A1
* 940nm IR LED connected via transistor to MicroView pin 12/Arduino pin 3 (PWM)
* Momentary push-button connected to MicroView pin 11/Arduino pin 2 

With this configuration, Tx and Rx distances of at least 8-10 meters in normal
conditions is possible. Additional Tx range could be obtained by driving an array
of PNP transistor->IR LED chains from the 2222A signal, or replacing the transistor
entirely with a power MOSFET setup i.e. IRLB8721.

## OPERATION

Once you've connected up the hardware and pushed the sketch to the MicroView, 
simply direct an IR remote you wish to debug at the IR receiver and press a button.
The signal should be received, parsed by the IRremote library, repeated via the IR
blaster, and its type and code displayed on the MicroView's OLED. To repeat the 
last command issued, just press the push-button!

