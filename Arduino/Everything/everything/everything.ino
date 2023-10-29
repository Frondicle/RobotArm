

//firmware for WASP extruder to use Ufactory xArm6 to print with clay or pastes
//Chris Morrey at the Christopher C Gibbs College of Architecture, University of Oklahoma.10/13/2023 

#include <SPI.h>
#include <Wire.h>
#include "FlexyStepper.h"
#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>
#include "epd1in54_V2.h"  /////
#include "epdpaint.h"/////////
#include <stdio.h>

Epd epd;
unsigned char image[2048];
Paint paint(image, 0, 0);

#define TI0                 20 //extruder digital output to xArm TI0, LOW for busy
#define TI1                 21 //spare digital output to xArm TI0
//#define TO0                 13//to reset pin for arm reset control
//#define TO1                 4 //digital input for robot arm
#define stepsPerRevolution  200

#define COLORED     0
#define UNCOLORED   1




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
const int id = 9;
const int ModbusBaudRate = 9600;
char cModbusBaudRate[8] = {char(ModbusBaudRate)};
char cfilename[11] = {"Everything"};
char input[10]; 
float speed = 0.0;
float move = 0.0;
float move_actual = 0.0;
int dir = 0;
float lastMove = 0.0;
float lastSpeed = 0.0;
int lastDir = 0;
int i = 0;
const int paintWidth = 160;
const int paintHeight = 100;
int frameX = 15;
int frameY = 120;
int epdRotation = 2;

FlexyStepper stepper;
String sFilename = String(cfilename);
String sSpeed = String(speed, DEC);
String sMove = String(move_actual,DEC);
String sBaud = String(ModbusBaudRate);

void screenWrite(int tabx,int taby,String(sInput),sFONT* font,int black){
    const char *cInput = sInput.c_str();
    paint.DrawStringAt(tabx, taby, cInput, font, black);
  }

void readyToGo(){ //reset all variables and set readyPin to HIGH 
  speed = 0.0;
  move = 0.0;
  move_actual = 0.0;
  stepper.setCurrentPositionInSteps(0);
  digitalWrite(TI0,HIGH);
  i=i+1;
}

float hexconvert(int modinfosbig, int modinfoslittle){
    //int big = modinfosbig;
    int big = modinfoslittle;
    big <<= 8;
    //big = (big | modinfoslittle);
    big = (big | modinfosbig);
    float converted = ((float)big/100);
    return converted;
  }

void setup() {

  stepper.connectToPins(MOTOR_STEP_PIN,MOTOR_DIRECTION_PIN);

  pinMode (TI0,OUTPUT);
  pinMode (TI1,OUTPUT);
  //pinMode (TO1,INPUT);

  epd.LDirInit();
  paint.SetRotate(epdRotation);
  paint.SetWidth(paintWidth);  //2896
  paint.SetHeight(paintHeight);

  paint.Clear(UNCOLORED);

  epd.HDirInit();
  epd.Clear();
  epd.Sleep();

  Serial.begin(ModbusBaudRate);

      // start the Modbus RTU server
  if (!ModbusRTUServer.begin(id, ModbusBaudRate)) {
    screenWrite(0,40,"NO",&Font24,0);
    screenWrite(0,34,"MODBUS",&Font24,0);
    
    epd.SetFrameMemoryPartial(paint.GetImage(),frameX, frameY, paint.GetWidth(), paint.GetHeight());
    epd.DisplayFrame();
    while (1);
    }

    paint.Clear(UNCOLORED);
    //paint.DrawStringAt(0, 0, "test",&Font24, 0);
    screenWrite(0,20,"file: ",&Font20,0);
    screenWrite(0,38,sFilename,&Font20,0);
    screenWrite(0,62,"baudrate: ",&Font20,0);
    screenWrite(0,80,sBaud,&Font24,0);
    
    epd.SetFrameMemoryPartial(paint.GetImage(),frameX,frameY, paint.GetWidth(), paint.GetHeight());
    epd.DisplayFrame();



  // configure Modbus holding registers at address 0x00 thru 0x0B (0-11)
  //0x00,0x01: movement length
  //0x02,0x03: extruder speed in...???
  //0x04: extruder direction 1 for forward, 0 for backward
  //0x05 thru 0x0B for error messages, and other stuff I don't know about yet

  ModbusRTUServer.configureHoldingRegisters(0x00,6); 

  stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);

  delay(2500);
}

void loop() {
  readyToGo();// function to clear all the variables and hold the readyPin HIGH
  
  bool stopFlag = false;
  stepper.setStepsPerMillimeter(extrusion_per_step_mm);

  // poll for Modbus RTU requests****************************************************************
  ModbusRTUServer.poll();
  float move = (ModbusRTUServer.holdingRegisterRead(0));//*.01);
  float speed = (ModbusRTUServer.holdingRegisterRead(1));//*.01);
  int dir = ModbusRTUServer.holdingRegisterRead(2);
  //move = hexconvert(ModbusRTUServer.holdingRegisterRead(0),ModbusRTUServer.holdingRegisterRead(1));
  //extruder_speed = hexconvert(ModbusRTUServer.holdingRegisterRead(2),ModbusRTUServer.holdingRegisterRead(3));
  //direction = hexconvert(ModbusRTUServer.holdingRegisterRead(4),ModbusRTUServer.holdingRegisterRead(5));
  //int data[] = {move_0_received,move_1_received,speed_0_received,speed_1_received,dir_0_received,dir_1_received};
  while(speed != lastSpeed || move != lastMove){
    if (dir == 1){
      move_actual = move;
      stepper.setSpeedInMillimetersPerSecond(speed);
      stepper.moveRelativeInSteps(move_actual);

      paint.Clear(UNCOLORED);
      screenWrite(0,20,"move: ",&Font20,0);
      String cMove = String(move_actual,DEC);
      screenWrite(0,40,cMove,&Font24,0);
      screenWrite(0,62,"speed: ",&Font20,0);
      String cSpeed = String(speed, DEC);
      screenWrite(0,80,cSpeed,&Font24,0);
      
      epd.SetFrameMemoryPartial(paint.GetImage(),frameX,frameY, paint.GetWidth(), paint.GetHeight());
      epd.DisplayFrame();
      }

    else {
      move_actual = (move * -1);
      stepper.setSpeedInStepsPerSecond(speed);
      stepper.moveRelativeInSteps(move_actual);

      paint.Clear(UNCOLORED);
      screenWrite(0,20,"move: ",&Font20,0);
      String cMove = String(move_actual,DEC);
      screenWrite(0,40,cMove,&Font24,0);
      screenWrite(0,62,"speed: ",&Font20,0);
      String cSpeed = String(speed, DEC);
      screenWrite(0,80,cSpeed,&Font24,0);
      
      epd.SetFrameMemoryPartial(paint.GetImage(),frameX,frameY, paint.GetWidth(), paint.GetHeight());
      epd.DisplayFrame();
      }
    while(!stepper.motionComplete())   
      {
      digitalWrite(TI0,LOW);
      stepper.processMovement();
      }
    lastMove = move;
    lastSpeed = speed;
    lastDir = dir;
  }

  paint.Clear(UNCOLORED);
  screenWrite(0,10,"DONE",&Font24,0);

}