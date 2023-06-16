#include <SoftwareSerial.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "FlexyStepper.h"
#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define EXTRUDE_GLCD_HEIGHT 7 
#define EXTRUDE_GLCD_WIDTH  24 
static unsigned char PROGMEM const extrude_glcd_bmp[] =
{ B00001100, B00011000, B00000000,
  B00011100, B00111000, B00000000,
  B00111100, B01111111, B11111111,
  B01111100, B11111111, B11111111,
  B00111100, B01111111, B11111111,
  B00011100, B00111000, B00000000,
  B00001100, B00011000, B00000000};

#define RETRACT_GLCD_HEIGHT 7 
#define RETRACT_GLCD_WIDTH  24 
static unsigned char PROGMEM const retract_glcd_bmp[] =
{ B00000000, B00011000, B00110000,
  B00000000, B00011100, B00111000,
  B11111111, B11111110, B00111100,
  B11111111, B11111111, B00111110,
  B11111111, B11111110, B00111100,
  B00000000, B00011100, B00111000,
  B00000000, B00011000, B00110000};

//Wasp extruder: 
//200 steps = 1 rotation
//Barrel diameter: 10mm 
//area: 78.54mm2
//Auger pitch: 7mm
//549.78 mm3 : 1 rotation
//Nozzle diameter: 2mm
// Area: 3.14 mm2: 
//1 rotation = 175.09mm long extrusion
//1 step = .875mm long extrusion

const int steps_per_revolution = 200;
const int baudRate = 115200;
unsigned int extruder_speed = 0;
unsigned int extruder_dir = 0;
int true_speed = 0;

// initialize the stepper library on pins 8 through 11:
FlexyStepper stepper;

 
void setup() {
  Serial.begin(baudRate);
  stepper.connectToPins(8,9);
  stepper.setStepsPerMillimeter(.875);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)){
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
   }
// start the Modbus RTU server, with (slave) id 8 
if (!ModbusRTUServer.begin(8,baudRate,SERIAL_8N1)) {
    Serial.println("Failed to start Modbus RTU Server!");
    while (1);
    }
  // configure speed holding registers at address 
  ModbusRTUServer.configureHoldingRegisters(256,4); 
}
void loop() {
  display.clearDisplay();
  
  int spd = ModbusRTUServer.holdingRegisterRead(258);
  int dir = ModbusRTUServer.holdingRegisterRead(256);
  extruder_speed = spd;
  extruder_dir = dir;
  
  if (extruder_dir > 10){
    display.clearDisplay();
    display.drawBitmap(4, 1,  extrude_glcd_bmp, 24, 7, 1);
    display.display();
    }
  else {
    int true_speed = (extruder_speed * -1)
    display.clearDisplay();
    display.drawBitmap(4, 1,  retract_glcd_bmp, 24, 7, 1);
    display.display();
  }
 
  display.clearDisplay();
  display.clearDisplay();
  display.setTextSize(1);      
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5,9);  
  display.setTextSize(2);     
  display.println(true_speed);
  display.display(); 
  
  //***********************************STEPPER**************************
  //
  // setup the motor so that it will rotate 2000 steps, note: this 
  // command does not start moving yet
  //
  stepper.setTargetPositionInSteps(2000);
  
  //
  // now execute the move, looping until the motor has finished
  //
  while(!stepper.motionComplete())
  {
    stepper.processMovement();       // this call moves themotor
    
    //
    // check if motor has moved past position 400, if so turn On the LED
    //
    if (stepper.getCurrentPositionInSteps() == 400)
      digitalWrite(LED_PIN, HIGH);
  }

}
    // poll for Modbus RTU requests****************************************************************
  ModbusRTUServer.poll();
}
