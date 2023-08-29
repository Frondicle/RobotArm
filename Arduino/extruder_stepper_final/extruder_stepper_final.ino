//Extruder firmware 7/17/2023 Chris Morrey Gibbs College of Architecture 

//#include <SoftwareSerial.h>
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

const int enablePin = 10;
const int steps_per_revolution = 200;
const int baudRate = 9600;
uint8_t move_received[2]; // registers 200,201
uint8_t speed_received[2]; // registers 202,203
int dir_received = 0; // coil 101
int extruder_ready = 0; //coil 100
uint16_t move_temp = 0;
uint16_t speed_temp = 0;
float move_converted = 0.0;
float speed_converted = 0.0;
float move_actual = 0.0;
int stepper_movement = 0;
const float nozzle_diameter = 2.0; //mm


//Wasp extruder 2mm nozzle sizes---------------------------------------------------
int barrel_radius = 5; //millimeters 
float barrel_area = (barrel_radius * 3.14159); //mm2
float auger_pitch = 7.15; //mm
float rotation_volume = (barrel_area * auger_pitch);
float nozzle_area = ((nozzle_diameter / 2)*3.14159); //mm2: 
float ext_length_per_revolution = rotation_volume / nozzle_area; //mm
float ext_length_per_step = ext_length_per_revolution / steps_per_revolution;
int i=0;
// initialize the stepper library
FlexyStepper stepper;

void readyToGo(){
    move_received[0,1] = 0,0;  
    speed_received[0,1] = 0,0;
    move_converted = 0;
    speed_converted = 0;
    move_actual = 0;
    ModbusRTUServer.coilWrite(100,1);
    i=i+1;
}
void setup() {
  Serial.begin(baudRate);
  stepper.connectToPins(8,9); //step, direction
  stepper.setStepsPerMillimeter(3);//(ext_length_per_step);
  stepper.setAccelerationInMillimetersPerSecondPerSecond(100);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)){
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
   }
// start the Modbus RTU server, with (slave) id 9 
if (!ModbusRTUServer.begin(9,baudRate,SERIAL_8N1)) {
    Serial.println("Failed to start Modbus RTU Server!");
    while (1);
    }
  // configure extruder move and speed holding registers at address 200 thru 203
  ModbusRTUServer.configureHoldingRegisters(200,4); 
  ModbusRTUServer.configureCoils(100,2);
}
void loop() {

  bool stopFlag = false;
  readyToGo();
  ModbusRTUServer.poll();

  extruder_ready = ModbusRTUServer.coilRead (100);//coil 100
  dir_received = ModbusRTUServer.coilRead (101);//coil 101
  move_received[0] = ModbusRTUServer.holdingRegisterRead(200);//least significant bit: little-endian
  move_received[1] = ModbusRTUServer.holdingRegisterRead(201);
  speed_received[0] = ModbusRTUServer.holdingRegisterRead (202);//least significant bit: little-endian
  speed_received[1] = ModbusRTUServer.holdingRegisterRead (203);

  move_temp = move_received[0];
  move_temp <<= 8;
  move_temp = (move_temp | move_received[1]);
  move_converted = ((float)move_temp/100);

  speed_temp = speed_received[0];
  speed_temp <<= 8;
  speed_temp = (speed_temp | speed_received[1]);
  speed_converted = ((float)speed_temp/100);
  
  if (dir_received == 1){
    move_actual = move_converted;
    display.clearDisplay();
    display.drawBitmap(4, 1,  extrude_glcd_bmp, 24, 7, 1);    
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,9);  
    display.setTextSize(1);     
    display.println(move_actual);
    display.println(speed_converted);
    display.println(i);
    display.display();
    }
  else {
    move_actual = (move_converted * -1);
    display.clearDisplay();
    display.drawBitmap(4, 1,  retract_glcd_bmp, 24, 7, 1);     
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,9); 
    display.setTextSize(1);     
    display.println(move_actual);
    display.println(speed_converted);
    display.println(stepper.getCurrentPositionInSteps());
    display.println(i);
    display.display();
  //***********************************STEPPER**************************
    digitalWrite(enablePin, HIGH);
    stepper.setSpeedInRevolutionsPerSecond(speed_converted);
    stepper.setTargetPositionRelativeInRevolutions(move_actual);
    ModbusRTUServer.coilWrite(100,0);
    //stepper.moveRelativeInRevolutions(move_actual);
    while(!stepper.motionComplete())
    {
    stepper.processMovement(); 
    }

  }
} 
