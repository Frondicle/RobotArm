

//firmware for WASP extruder to use Ufactory xArm6 to print with clay or pastes
//Chris Morrey at the Christopher C Gibbs College of Architecture, University of Oklahoma.10/13/2023 

#include <SPI.h>
#include <Wire.h>
#include "FlexyStepper.h"
#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define TI0                 20 //extruder digital output to xArm TI0, LOW for busy
#define TI1                 21 //spare digital output to xArm TI0
//#define TO0               to reset pin for arm reset control
#define TO1                 19 //digital input for robot arm
#define dirPinStepper       14 //stepper direction, HIGH for forward
#define stepPinStepper      16  //stepper pin: high/low sequence makes one step
#define enablePinStepper    15 //stepper driver enable
#define stepsPerRevolution  200

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

int steps_per_revolution = 200;
float barrel_diameter_mm = 10;
float barrel_area = 75.39;//area minus auger shaft in mm2 
float auger_pitch = 7.0;//mm
float volume_per_rotation_mm = barrel_area * auger_pitch;
int nozzle_diameter_mm = 2;
float nozzle_area_mm = 3.14; 
float extrusion_per_rotation_mm = volume_per_rotation_mm / nozzle_area_mm;
float extrusion_per_step_mm = extrusion_per_rotation_mm / steps_per_revolution;

const int MOTOR_STEP_PIN = 16;
const int MOTOR_DIRECTION_PIN = 14;
const int MOTOR_ENABLE_PIN = 15;
const int id = 9;
const int ModbusBaudRate = 9600;
float speed = 0.0;
float speed_mapped = 0.0;
float move = 0.0;
float move_actual = 0.0;
int dir = 0;
float lastMove = 0.0;
float lastSpeed = 0.0;
int lastDir = 0;
int i = 0;

FlexyStepper stepper;

void readyToGo(){ //reset all variables and set readyPin to HIGH 
  speed = 0.0;
  move = 0.0;
  move_actual = 0.0;
  digitalWrite(TI0,HIGH);
  i=i+1;
}

float hexconvert(int modinfosbig, int modinfoslittle){
    int big = modinfosbig;
    big <<= 8;
    big = (big | modinfoslittle);
    float converted = ((float)big/100);
    return converted;
  }

void setup() {

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
   }

  stepper.connectToPins(MOTOR_STEP_PIN,MOTOR_DIRECTION_PIN);

  pinMode (enablePinStepper,OUTPUT);
  pinMode (TI0,OUTPUT);
  pinMode (TI1,OUTPUT);
  pinMode (TO1,INPUT);

  Serial.begin(ModbusBaudRate);


      // start the Modbus RTU server
  if (!ModbusRTUServer.begin(id, ModbusBaudRate)) {
    Serial.println("Failed to start Modbus RTU Server!");
    while (1);
    }

  display.clearDisplay();   
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,9);  
    display.setTextSize(1); 
    display.print("WASP_extruder_core_epaper");
    display.setTextSize(1.5);
    display.print("Baudrate: ");   
    display.println(ModbusBaudRate);
    display.print("  ID: ");

  delay(2500);

  // configure Modbus holding registers at address 0x00 thru 0x0B (0-11)
  //0x00,0x01: movement length
  //0x02,0x03: extruder speed in...???
  //0x04: extruder direction 1 for forward, 0 for backward
  //0x05 thru 0x0B for error messages, and other stuff I don't know about yet

  ModbusRTUServer.configureHoldingRegisters(0x00,6); 

  stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);

}
void loop() {

  readyToGo();// function to clear all the variables and hold the readyPin HIGH

  stepper.setStepsPerMillimeter(extrusion_per_step_mm);

  // poll for Modbus RTU requests****************************************************************
  ModbusRTUServer.poll();
  float move = ModbusRTUServer.holdingRegisterRead(0);
  float speed = ModbusRTUServer.holdingRegisterRead(1);
  int dir = ModbusRTUServer.holdingRegisterRead(2);
  //move = hexconvert(ModbusRTUServer.holdingRegisterRead(0),ModbusRTUServer.holdingRegisterRead(1));
  //extruder_speed = hexconvert(ModbusRTUServer.holdingRegisterRead(2),ModbusRTUServer.holdingRegisterRead(3));
  //direction = hexconvert(ModbusRTUServer.holdingRegisterRead(4),ModbusRTUServer.holdingRegisterRead(5));
  //int data[] = {move_0_received,move_1_received,speed_0_received,speed_1_received,dir_0_received,dir_1_received};
  while(speed != lastSpeed || move != lastMove){
    if (dir == 1){
      move_actual = move;
      }
    else {
      move_actual = (move * -1);
    }
    digitalWrite(MOTOR_ENABLE_PIN,HIGH);
    stepper.setSpeedInMillimetersPerSecond(speed_mapped);
    stepper.moveToPositionInMillimeters(move_actual);
   display.clearDisplay();
      display.drawBitmap(4, 1,  retract_glcd_bmp, 24, 7, 1);     
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(1,9); 
      display.setTextSize(1); 
      display.print("move_actual: ");   
      display.print(" ");
      display.println(move_actual);
      display.print("speed: ");
      display.print(" ");
      display.print(speed);
      display.display();
  lastMove = move;
  lastSpeed = speed;
  lastDir = dir;
  }

}