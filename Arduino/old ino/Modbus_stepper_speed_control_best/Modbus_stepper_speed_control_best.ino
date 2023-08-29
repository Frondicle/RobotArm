
/*
   Uno sketch to drive a stepper motor using the AccelStepper library.
   Function runSpeed() is used to run the motor at constant speed. A pot is read to vary the speed.
   Works with a ULN-2003 unipolar stepper driver, or a bipolar, constant voltage motor driver
   such as the L298 or TB6612, or a step/direction constant current driver like the a4988.
   A potentiometer is connected to analog input 0 and to gnd and 5v.

 The motor will rotate one direction. The higher the potentiometer value,
 the faster the motor speed. Because setSpeed() sets the delay between steps,
 you may notice the motor is less responsive to changes in the sensor value at
 low speeds.

 Created 30 Nov. 2009
 Modified 28 Oct 2010
 by Tom Igoe
 12/26/21  Modified to use AccelStepper.  --jkl

 */
// Include the AccelStepper Library
#include <AccelStepper.h>
//#include <elapsedMillis.h> //maybe this could go away?
#include <ArduinoRS485.h>
#include <ArduinoModbus.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Motor Connections (constant current, step/direction bipolar motor driver)
const int dirPin = 4;
const int stepPin = 3;
byte enablePin = 5;

const int numHoldingRegisters = 2;
const int baudrate = 115200;

AccelStepper myStepper(AccelStepper::DRIVER, stepPin, dirPin); // works for a4988 (Bipolar, constant current, step/direction driver)

//elapsedMillis printTime;

const int maxSpeedLimit = 1000.0;  // set this to the maximum speed you want to use.



void setup() {
  Serial.begin(baudrate);
    while (!Serial);

    Serial.println("Modbus_stepper_speed_control");

  // Modbus business***************************************start the Modbus RTU server, with (slave) id 42
  if (!ModbusRTUServer.begin(42, baudrate)) {
    Serial.println("Failed to start Modbus RTU Server!");
    while (1);
}
  // configure holding registers at address 0x00
  ModbusRTUServer.configureHoldingRegisters(0x00, numHoldingRegisters);

   // Screen business*********************************************************Clear the buffer
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000); // Pause for 2 seconds
  
  // Stepper business****************************************************set the maximum speed and initial speed.
  myStepper.setMaxSpeed(maxSpeedLimit); 
  myStepper.setSpeed(0);    // initial speed target
  pinMode(enablePin, OUTPUT);
  
}

void loop() {
  // poll for Modbus RTU requests
  ModbusRTUServer.poll();

  byte mSpeed = ModbusRTUServer.holdingRegisterRead(0);
  Serial.println(mSpeed);
  digitalWrite(enablePin, LOW);
 
    mSpeed = myStepper.speed();
    
    display.clearDisplay();
    display.setTextSize(3);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.println("speed:");
    display.println(mSpeed);

  // set the motor speed:
  if (mSpeed > 0) {
    myStepper.setSpeed(mSpeed);
  }
  display.display();
  myStepper.runSpeed();
}
