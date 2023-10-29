

//firmware for WASP extruder to use Ufactory xArm6 to print with clay or pastes
//Chris Morrey at the Christopher C Gibbs College of Architecture, University of Oklahoma.10/13/2023 

#include <SPI.h>
#include <Wire.h>
#include "FlexyStepper.h"
#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>
#include <Adafruit_GFX.h>//LCD
#include <Adafruit_SSD1306.h>//LCD

#define TI0                 6 //extruder digital output to xArm TI0, LOW for busy: red on standard cable
#define TI1                 7 //spare digital output to xArm TI0
#define stepsPerRevolution  200

#define EXTRUDE_GLCD_HEIGHT 7 // height of extrude graphic
#define EXTRUDE_GLCD_WIDTH  24 //width of extrude graphic
static unsigned char PROGMEM const extrude_glcd_bmp[] =
{ 0b00001100, 0b00011000, 0b00000000,
  0b00011100, 0b00011100, 0b00000000,
  0b00111100, 0b01111111, 0b11111111,
  0b01111100, 0b11111111, 0b11111111,
  0b00111100, 0b01111111, 0b11111111,
  0b00011100, 0b00011100, 0b00000000,
  0b00001100, 0b00011000, 0b00000000};

//___________________________________________
#define RETRACT_GLCD_HEIGHT 7 
#define RETRACT_GLCD_WIDTH  24 
static unsigned char PROGMEM const retract_glcd_bmp[] =
  { 0b00000000, 0b00011000, 0b00110000,
  0b00000000, 0b00011100, 0b00011100,
  0b11111111, 0b11111110, 0b00111100,
  0b11111111, 0b11111111, 0b00111110,
  0b11111111, 0b11111110, 0b00111100,
  0b00000000, 0b00011100, 0b00011100,
  0b00000000, 0b00011000, 0b00110000};

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int steps_per_revolution = 200;
float barrel_diameter_mm = 10;
float barrel_area = 75.39;//area minus auger shaft in mm2 
float auger_pitch = 7.0;//mm
float volume_per_rotation_mm = barrel_area * auger_pitch;
int nozzle_diameter_mm = 2;
float nozzle_area_mm = 3.14; 
float extrusion_per_rotation_mm = volume_per_rotation_mm / nozzle_area_mm;
float extrusion_per_step_mm = extrusion_per_rotation_mm / steps_per_revolution;
const string fileName = ""

const int MOTOR_STEP_PIN = 8;
const int MOTOR_DIRECTION_PIN = 9;
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
  stepper.setCurrentPositionInSteps(0);
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

  stepper.connectToPins(MOTOR_STEP_PIN,MOTOR_DIRECTION_PIN);

  pinMode (TI0,OUTPUT);
  pinMode (TI1,OUTPUT);


  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
   }

  Serial.begin(ModbusBaudRate);
  //ModbusRTUServer.begin(id, ModbusBaudRate);
      // start the Modbus RTU server
  if (!ModbusRTUServer.begin(id, ModbusBaudRate)) {
    display.clearDisplay();   
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,9);  
    display.setTextSize(1); 
    display.print("Modbus offline");
    display.display();
  while (1);
  }

    display.clearDisplay();   
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,9);  
    display.setTextSize(1); 
    display.print(fileName);
    display.setTextSize(1.5);
    display.print("Baudrate: ");   
    display.println(ModbusBaudRate);
    display.print("  ID: ");
    display.display();

  delay(3500);

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
  
  bool stopFlag = false;
  stepper.setStepsPerMillimeter(extrusion_per_step_mm);

  // poll for Modbus RTU requests****************************************************************
  ModbusRTUServer.poll();
  float move = (ModbusRTUServer.holdingRegisterRead(0)*.01);
  float speed = (ModbusRTUServer.holdingRegisterRead(1)*.01);
  int dir = ModbusRTUServer.holdingRegisterRead(2);
  while(ModbusRTUServer.holdingRegisterRead(0) == 0.0){   
    display.clearDisplay();   
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,9);  
    display.setTextSize(1); 
    display.println("waiting for");
    display.println("Modbus data");
    display.display();
    }
  //Serial.print("reg0 = ");
  //Serial.println(move);
  //Serial.print("reg1 = ");
  //Serial.println(speed);
  //Serial.print("reg2 = ");
  //Serial.println(dir);
  //move = hexconvert(ModbusRTUServer.holdingRegisterRead(0),ModbusRTUServer.holdingRegisterRead(1));
  //extruder_speed = hexconvert(ModbusRTUServer.holdingRegisterRead(2),ModbusRTUServer.holdingRegisterRead(3));
  //direction = hexconvert(ModbusRTUServer.holdingRegisterRead(4),ModbusRTUServer.holdingRegisterRead(5));
  //int data[] = {move_0_received,move_1_received,speed_0_received,speed_1_received,dir_0_received,dir_1_received};
  while(speed != lastSpeed || move != lastMove){
    if (dir == 1){
      move_actual = move;
      stepper.setSpeedInMillimetersPerSecond(speed);
      stepper.moveToPositionInMillimeters(move_actual);
      display.clearDisplay();
      display.drawBitmap(4, 1, extrude_glcd_bmp, 24, 7, 1);     
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(1,9); 
      display.setTextSize(1); 
      display.print("move: ");   
      display.println(move_actual);
      display.print("speed: ");
      display.println(speed);
      display.display();
    }

    else {
      move_actual = (move * -1);
      stepper.setSpeedInMillimetersPerSecond(speed);
      stepper.moveToPositionInMillimeters(move_actual);
      display.clearDisplay();
      display.drawBitmap(4, 1, retract_glcd_bmp, 24, 7, 1);     
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(1,9); 
      display.setTextSize(1); 
      display.print("move: ");   
      display.println(move_actual);
      display.print("speed: ");
      display.println(speed);
      display.display();
    }

    stepper.processMovement();
    while(!stepper.motionComplete()){
      digitalWrite(TI0,LOW);
    }
  lastMove = move;
  lastSpeed = speed;
  lastDir = dir;
  }

  display.clearDisplay();
  display.setCursor(1,9); 
  display.setTextSize(1); 
  display.print("done");   
  display.display();
  delay(2500);
  display.clearDisplay();
}