

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
#include <CircularBuffer.h>
#include <SPI.h>
#include <Wire.h>
#include <FlexyStepper.h>
#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>
#include <C:\Users\morr0289\Documents\Arduino\libraries\CircularBuffer\CircularBuffer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel pixels(1, 10, NEO_RGB + NEO_KHZ800);//pin 10
#define DELAYVAL 250 // Time (in milliseconds) to pause between pixels: changed from 500

//--I/O-------------------------------------------------------------------------------------- 
#define TI0                6 //extruder digital output to xArm TI0, LOW for busy, red wire on standard harness
#define TI1                 7 //spare digital output to xArm TI1, purple wire on standard harness
//#define TO0               //to reset pin for arm reset control
#define TO1                 21 //digital output from robot arm: high when arm is not moving
//--Stepper----------------------------------------------------------------------------------
#define dirPinStepper       9 //stepper direction, HIGH for forward: green wire
#define stepPinStepper      8  //stepper pin: high/low sequence makes one step: yellow wire
#define stepsPerRevolution  200
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
const int MOTOR_STEP_PIN = 8;
const int MOTOR_DIRECTION_PIN = 9;
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

int green = 1;
int red = 2;
int blue = 3;
int none = 0;

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


void statusLED(int color){
    pixels.setBrightness(50);
    pixels.clear(); // Set all pixel colors to 'off'
    switch(color){
      case 0:
      pixels.clear();
      case 1://green
        pixels.setPixelColor(0, pixels.Color(250, 0, 0));
      case 2://red
        pixels.setPixelColor(0, pixels.Color(0, 250, 0));
      case 3://blue
        pixels.setPixelColor(0, pixels.Color(0, 0, 250));
    }
    pixels.show();   // Send the updated pixel colors to the hardware.
}

void statusLEDflash(int color,long interval,long numFlashes){
    pixels.setBrightness(50);
    pixels.clear(); // Set all pixel colors to 'off'
      switch(color){
      case 1://green
        pixels.setPixelColor(0, pixels.Color(250, 0, 0));
      case 2://red
        pixels.setPixelColor(0, pixels.Color(0,250, 0));
      case 3://blue
        pixels.setPixelColor(0, pixels.Color(0, 0, 250));
      }
     // Send the updated pixel colors to the hardware.
    int i = 0;  
    while (i < numFlashes){
      pixels.show();
      delay(interval);
      pixels.clear();
      delay(interval);
      i++;
      }
}
void pollAndStoreData(){ 
  int cap = (queue.capacity - 6);
  while (queue.size() < cap){
    statusLED(blue);
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
    statusLED(none);
  }
}
packet retrieveData(){
  statusLED(green);
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
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  statusLED(green);

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    statusLED(none);
    delay(2000);
    statusLED(red);
    for(;;); // Don't proceed, loop forever
    }

  stepper.connectToPins(MOTOR_STEP_PIN,MOTOR_DIRECTION_PIN);

  pinMode (TI0,OUTPUT); //
  pinMode (TI1,OUTPUT);
  //pinMode (TO0,INPUT);
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
    statusLEDflash(red,700,300);
    }
  statusLED(green);
  display.clearDisplay();   
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1,9);  
  display.setTextSize(1); 
  display.println("1/30/24_WASP_latest");
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
    statusLED(red);
    }
  statusLED(green);
  if (queue.available() > 6){
    pollAndStoreData();
  }
  
  }

void loop() {
  //pollAndStoreData();
  statusLED(none);
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
        statusLED(red);
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
      statusLED(green);
      digitalWrite(TI0,HIGH);
      lastIndex = next.index;
      lastMove = next.move;
      lastSpeed = next.speed;
      lastDir = next.dir;
      lastEnd = next.end;
    }
    else {
    int show = (lastIndex +1);
    statusLEDflash(2,2000,10);
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
  statusLEDflash(green,2000,6);
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