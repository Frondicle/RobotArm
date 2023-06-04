
/*
 Stepper Motor Control - speed control

 This program drives a unipolar or bipolar stepper motor.
 The motor is attached to digital pins 8 - 11 of the Arduino.
 A potentiometer is connected to analog input 0.


 Created 30 Nov. 2009
 Modified 28 Oct 2010
 by Tom Igoe

 */
#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>
#include <Stepper.h>

const int baudRate = 115200;

const int stepsPerRevolution = 200;  // change this to fit the number of steps per revolution
// for your motor
const int numHoldingRegisters = 1;
const int numInputRegisters = 1;

// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 8, 9, 10, 11);

int stepCount = 0;  // number of steps the motor has taken

void setup() {
  Serial.begin(baudRate);
  while (!Serial);

  Serial.println("Modbus RTU Server Kitchen Sink");

  // start the Modbus RTU server, with (slave) id 42
  if (!ModbusRTUServer.begin(42, baudRate)) {
    Serial.println("Failed to start Modbus RTU Server!");
    while (1);
}
  // configure holding registers at address 0x00
  ModbusRTUServer.configureHoldingRegisters(0x00, numHoldingRegisters);
}
  
void loop() {
  // poll for Modbus RTU requests
  ModbusRTUServer.poll();

  for (int i = 0; i < numHoldingRegisters; i++) {
    int motorSpeed = ModbusRTUServer.holdingRegisterRead(i);

  // set the motor speed:
  if (motorSpeed > 0) {
    myStepper.setSpeed(motorSpeed);
    // step 1/100 of a revolution:
    myStepper.step(stepsPerRevolution / 100);
  }
}
}
