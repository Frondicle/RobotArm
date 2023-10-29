

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
unsigned char image[2896];
Paint paint(image, 0, 0);

#define TI0                 6 //extruder digital output to xArm TI0, LOW for busy
#define TI1                 7 //spare digital output to xArm TI0
#define TO0                 13//to reset pin for arm reset control
#define TO1                 4 //digital input for robot arm
#define stepsPerRevolution  200
#define enablePinStepper    10

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

const int MOTOR_STEP_PIN = 9;
const int MOTOR_DIRECTION_PIN = 8;
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

FlexyStepper stepper;
String sFilename = String(cfilename);
String sSpeed = String(speed, DEC);
String sMove = String(move_actual,DEC);
String sBaud = String(ModbusBaudRate);
String label_file = ("cfilename: ");
String label_baud =  ("baudrate: ");
String label_speed = ("speed: ");
String label_move = ("move: ");



void screenWrite(int tabx,int taby,int locx,int locy,String(sInput),sFONT* font,int black, int rotate){
      const char *cInput = sInput.c_str();
      paint.SetRotate(rotate);
      paint.DrawStringAt(tabx, taby, cInput, font, black);
      epd.SetFrameMemoryPartial(paint.GetImage(), locx, locy, paint.GetWidth(), paint.GetHeight());
      epd.DisplayFrame();
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
    int big = modinfosbig;
    big <<= 8;
    big = (big | modinfoslittle);
    float converted = ((float)big/100);
    return converted;
  }

void setup() {

  stepper.connectToPins(MOTOR_STEP_PIN,MOTOR_DIRECTION_PIN);

  pinMode (enablePinStepper,OUTPUT);
  pinMode (TI0,OUTPUT);
  pinMode (TI1,OUTPUT);
  pinMode (TO1,INPUT);

  epd.LDirInit();
  paint.SetRotate(0);
  paint.SetWidth(160);  //2896
  paint.SetHeight(140);

  paint.Clear(UNCOLORED);

  epd.HDirInit();
  epd.Clear();
  epd.Sleep();

  Serial.begin(ModbusBaudRate);

      // start the Modbus RTU server
  if (!ModbusRTUServer.begin(id, ModbusBaudRate)) {
    paint.Clear(UNCOLORED);
    screenWrite(0,10,0,30,"NO",&Font24,0,0);
    screenWrite(0,34,0,30,"MODBUS",&Font24,0,0);
    while (1);
    }

    paint.Clear(UNCOLORED);
    screenWrite(0,10,0,30,"file: ",&Font16,0,0);
    screenWrite(0,27,0,30,sFilename,&Font16,0,0);
    screenWrite(0,44,0,30,"baudrate: ",&Font16,0,0);
    screenWrite(0,61,0,30,sBaud,&Font16,0,0);



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
  float move = (ModbusRTUServer.holdingRegisterRead(0)*.01);
  float speed = (ModbusRTUServer.holdingRegisterRead(1)*.01);
  int dir = ModbusRTUServer.holdingRegisterRead(2);
  //move = hexconvert(ModbusRTUServer.holdingRegisterRead(0),ModbusRTUServer.holdingRegisterRead(1));
  //extruder_speed = hexconvert(ModbusRTUServer.holdingRegisterRead(2),ModbusRTUServer.holdingRegisterRead(3));
  //direction = hexconvert(ModbusRTUServer.holdingRegisterRead(4),ModbusRTUServer.holdingRegisterRead(5));
  //int data[] = {move_0_received,move_1_received,speed_0_received,speed_1_received,dir_0_received,dir_1_received};
  while(speed != lastSpeed || move != lastMove){
    if (dir == 1){
      move_actual = move;
      digitalWrite(enablePinStepper,HIGH);
      stepper.setSpeedInMillimetersPerSecond(speed);
      stepper.moveToPositionInMillimeters(move_actual);

      paint.Clear(UNCOLORED);
      screenWrite(0,10,0,30,"move: ",&Font20,0,0);
      String cMove = String(move_actual,DEC);
      screenWrite(0,25,0,30,cMove,&Font16,0,0);
      screenWrite(0,45,0,30,"speed: ",&Font20,0,0);
      String cSpeed = String(speed, DEC);
      screenWrite(0,65,0,30,cSpeed,&Font16,0,0);
      }

    else {
      move_actual = (move * -1);
      digitalWrite(enablePinStepper,HIGH);
      stepper.setSpeedInMillimetersPerSecond(speed);
      stepper.moveToPositionInMillimeters(move_actual);

      paint.Clear(UNCOLORED);
      paint.Clear(UNCOLORED);
      screenWrite(0,10,0,30,"move: ",&Font20,0,0);
      String cMove = String(move_actual,DEC);
      screenWrite(0,25,0,30,cMove,&Font16,0,0);
      screenWrite(0,45,0,30,"speed: ",&Font20,0,0);
      String cSpeed = String(speed, DEC);
      screenWrite(0,65,0,30,cSpeed,&Font16,0,0);
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
  paint.Clear(UNCOLORED);
  screenWrite(0,10,0,30,"DONE",&Font24,0,0);

}