

/*
firmware for WASP extruder to use Ufactory xArm6 to print with clay or pastes
Chris Morrey at the Christopher C Gibbs College of Architecture, University of Oklahoma.10/13/2023 

  This  is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
*/
#include <CircularBuffer.hpp>
#include <SPI.h>
#include <Wire.h>
#include <FlexyStepper.h>
#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>
#include <C:\Users\morr0289\Documents\Arduino\libraries\CircularBuffer\CircularBuffer.hpp>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "pins.h"

//--GLCD screen------------------------------------------------------------------------------
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//--Graphics-----------------------------------------------------------------------------------
#define EXTRUDE_GLCD_HEIGHT 7 
#define EXTRUDE_GLCD_WIDTH  24 
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

//bool debug = TRUE;

//--Extruder physical characteristics------------------------------------------
int steps_per_revolution = 200;
float barrel_diameter_mm = 10;
float barrel_area = 75.39;//area minus auger shaft in mm2 
float auger_pitch = 7.0;//mm
float volume_per_rotation_mm = barrel_area * auger_pitch;
int nozzle_diameter_mm = 2;
float nozzle_area_mm = 3.14; 
float extrusion_per_rotation_mm = volume_per_rotation_mm / nozzle_area_mm;
float extrusion_per_step_mm = extrusion_per_rotation_mm / steps_per_revolution;
//--Stepper---------------------------------------------------------------------------
const int MOTOR_STEP_PIN = stepPinStepper;
const int MOTOR_DIRECTION_PIN = dirPinStepper;
//--Data-------------------------------------------------------------------------------
const int id = 7;
const int ModbusBaudRate = 9600;
long index = 0;
long move = 0;
long move_actual = 0;
long speed = 0;
long dir = 0;
long end = 0;
long stepperAccel = 1000;
long lastIndex = 1;
long lastMove = 0;
long lastSpeed = 0;
long lastDir = 0;
long lastEnd = 0;
unsigned int bufferCap = 500;

char line1[100];
char line2[100];

bool queuePush = FALSE;

 struct packet{
    long index;
    long move;
    long speed;
    long dir;
    long end;
    long lnOut;
    };
//--Initialize everything---------------------------------------------------------
FlexyStepper stepper;
packet data,datas,in,next;
CircularBuffer<packet,144> queue;


void pollAndStoreData(){ 
  int cap = (queue.capacity - 6);
  while (queue.size() < cap){
    int datas[6] = {ModbusRTUServer.poll()};
    in.index = datas[0];
    in.move = datas[1];
    in.speed = datas[2];
    in.dir = datas[3];
    in.end = datas[4]; 
    }
    if(!queue.push(in)){
      display.clearDisplay();   
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(1,9);  
      display.setTextSize(1.5); 
      display.println("PUSH");
      display.println("FAILED");
      display.display();
    }
      else{
      display.clearDisplay();   
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(1,0);  
      display.setTextSize(1);
      display.println("pollAndStoreData: PUSH:"); 
      display.print(in.index);
      display.print(in.move);
      display.print(in.speed);
      display.println(in.dir);
      display.display();
  }
}
packet retrieveData(){
  data = queue.shift();
  display.clearDisplay();   
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1,100);  
  display.setTextSize(1); 
  display.println(data.index);
  display.display();
  delay(500);
  return data;
}

int readDigitalFromArm(){
  int val = digitalRead(TO1);
      display.clearDisplay();   
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(1,100);  
      display.setTextSize(1); 
      display.println("ARM OK");
      display.display();
  return val;
  }

/*'''Modbus format:
0x07=slave id
0x10=function 16 write registers, function 3 read registers
0x00,0x00=beginning address to write
0x00,0x05 = number of registers
0x0a=number of bytes(2x registers)
0x00,0x15=register 0 index
0x00,0x06=register 1 steps
0x00,0x00=register 2 espeed
0x00,0x01=register 3 direction 1=forward 0=reverse
0x00,0x00=register 4 end of file 1=stop 0=continue
0x00,0x00=register 5 index back to arm
Digitals:
TI0: extruder ready flag: 1=ready, 0=not ready
TI1: extruder end flag: 0=end, 1=run
TO0: reset extruder: 0=reset, 1=normal
TO1: */

void setup() {

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
    }

  stepper.connectToPins(MOTOR_STEP_PIN,MOTOR_DIRECTION_PIN);

  pinMode (TI0,OUTPUT); //
  pinMode (TI1,OUTPUT);
  pinMode (TO0,INPUT);
  pinMode (TO1,INPUT);

  Serial.begin(ModbusBaudRate);

  if (!ModbusRTUServer.begin(id, ModbusBaudRate)) {
    display.clearDisplay();   
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,9);  
    display.setTextSize(2); 
    display.println("NO");
    display.println("MODBUS");
    display.display();
    }
  display.clearDisplay();   
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1,9);  
  display.setTextSize(1); 
  display.println("3/6/24WASPnewHardware");
  display.display();
  delay(3000);
   
  ModbusRTUServer.configureHoldingRegisters(0x00,6); 

  stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);
  delay(500);
  digitalWrite(TI0,HIGH);//
 
  while (!ModbusRTUServer.poll()) { //modbus initial poll to get first line number and clear registers
    display.clearDisplay();   
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,0);  
    display.setTextSize(1.5); 
    display.println("Waiting for:");
    display.display();
    delay(500);
    display.setTextSize(1.5);
    display.println("xArm MODBUS");
    display.display();
    delay(500);
    }
  if (queue.available() > 6){
    pollAndStoreData();
  }
  
  }

void loop() {
  //pollAndStoreData();
  while (lastEnd != 1){
    next = retrieveData();
    display.clearDisplay();   
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,0);  
    display.setTextSize(1); 
    display.println("received: ");   
    display.setTextSize(1);
    /*long index;
    long move;
    long speed;
    long dir;
    long end;
    long lnOut;*/
    display.print(next.index);
    display.print(next.move);
    display.print(next.speed);
    display.println(next.dir);
    display.println(next.end);
    display.print(next.lnOut);
    display.display();
    delay(200);
    if ((stepper.motionComplete() == TRUE)&&(readDigitalFromArm() == HIGH)){
    if (next.index ==(lastIndex + 1)){
      digitalWrite(TI0,HIGH); //set ready flag to 'ready'
      if (next.dir == 1){
        move_actual = next.move;
        display.clearDisplay();
        display.drawBitmap(4, 1,  extrude_glcd_bmp, 24, 7, 1);
        }
      else {
        move_actual = (next.move * -1);
        display.clearDisplay();
        display.drawBitmap(4, 1,  retract_glcd_bmp, 24, 7, 1);
      }
      stepper.setSpeedInStepsPerSecond(next.speed);
      stepper.setAccelerationInStepsPerSecondPerSecond(stepperAccel);
      stepper.setTargetPositionRelativeInSteps(move_actual);
      stepper.processMovement();
      do {  
        digitalWrite(TI0,LOW);  // set ready flag to 'not ready'
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(1,9);
        display.setTextSize(1); 
        display.print("steps: ");   
        display.println(move_actual);
        display.print("speed: ");
        display.println(next.speed);
        display.print("index: ");
        display.println(next.index);
        display.display();
        pollAndStoreData();}
        while(stepper.motionComplete() == FALSE);
        digitalWrite(TI0,HIGH);
      lastIndex = next.index;
      lastMove = next.move;
      lastSpeed = next.speed;
      lastDir = next.dir;
      lastEnd = next.end;
    }
    else {
    int show = (lastIndex +1);
    display.clearDisplay(); 
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,9); 
    display.setTextSize(1); 
    display.println("index error:");   
    display.print("expected: ");
    display.println(show);
    display.print("received: ");
    display.print(next.index);
    display.display();
    ModbusRTUServer.end();
    Serial.end();
    digitalWrite(TI1,LOW); 
    break; 
  }
  display.clearDisplay(); 
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1,9); 
  display.setTextSize(1.5); 
  display.println("FILE");   
  display.println("COMPLETE");
  display.display();
  delay(2000);
  ModbusRTUServer.end();
  Serial.end();
  digitalWrite(TI1,LOW);
  }
}
}