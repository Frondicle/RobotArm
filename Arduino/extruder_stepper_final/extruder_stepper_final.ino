//Extruder firmware 7/17/2023 Chris Morrey Gibbs College of Architecture 

#include <SoftwareSerial.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "FlexyStepper.h" // Documentation at: https://github.com/Stan-Reifel/FlexyStepper
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


const int steps_per_revolution = 200;
const int baudRate = 9600;
byte move_received[2]; // registers 200,201
byte speed_received[2]; // registers 204,205
byte dir_received = [2]; // registers 202,203
float move_converted = 0;
int dir_converted = 0;
float speed_converted = 0;
float move_actual = 0;
const float nozzle_diameter = 2; //mm

//Wasp extruder 2mm nozzle sizes---------------------------------------------------
int barrel_radius = 5; //millimeters 
float barrel_area = (barrel_radius * 3.14159); //mm2
int auger_pitch = 7; //mm
float rotation_volume = (barrel_area * auger_pitch);
float nozzle_area = ((nozzle_diameter / 2)*3.14159); //mm2: 
float ext_length_per_revolution = rotation_volume / nozzle_area; //mm
float ext_length_per_step = ext_length_per_revolution / steps_per_revolution;

// initialize the stepper library
FlexyStepper stepper;
 
void setup() {
  Serial.begin(baudRate);
  stepper.connectToPins(8,9);
  stepper.setStepsPerMillimeter(ext_length_per_step);
  stepper.setAccelerationInMillimetersPerSecondPerSecond(100);
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
  // configure extruder move, direction and speed holding registers at address 200 thru 205
  ModbusRTUServer.configureHoldingRegisters(0xC8,6); 
}
void loop() {
  bool stopFlag = false;
  ModbusRTUServer.poll();
  display.clearDisplay();

  move_received = ModbusRTUServer.holdingRegisterRead(0xC8,0xC9); //registers 200, 201
  dir_received = ModbusRTUServer.holdingRegisterRead(0xCA,0xCB);//registers 202,203
  speed_received = ModbusRTUServer.holdingRegisterRead(0xCC,0xCD); ///registers 204,205

  //Convert bytes to floats and integers

  
  if (dir > 0){
    move_actual = move_in_millimeters;
    display.clearDisplay();
    display.drawBitmap(4, 1,  extrude_glcd_bmp, 24, 7, 1);
    display.setTextSize(1);      
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(5,9);  
    display.setTextSize(2);     
    display.println(move_actual);
    display.display();
    }
  else {
    move_actual = (move_in_millimeters * -1);
    display.clearDisplay();
    display.drawBitmap(4, 1,  retract_glcd_bmp, 24, 7, 1);
    display.setTextSize(1);      
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(5,9);  
    display.setTextSize(2);     
    display.println(move_actual);
    display.display();
  //***********************************STEPPER**************************
    stepper.setSpeedInMillimetersPerSecond(extruder_speed);
    stepper.setTargetPositionRelativeInMillimeters(move_actual);
    while(!stepper.motionComplete())
    {
    stepper.processMovement();
    }
  }
} 
