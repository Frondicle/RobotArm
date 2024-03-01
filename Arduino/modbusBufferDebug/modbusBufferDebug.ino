

/*
firmware for WASP extruder to use Ufactory xArm6 to print with clay or pastes
Chris Morrey at the Christopher C Gibbs College of Architecture, University of Oklahoma.10/13/2023 

  This  is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty o
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
*/
#include <SPI.h>
#include <Wire.h>
#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>
#include <CircularBuffer.hpp>
#include <Adafruit_SSD1306.h>

//--I/O-------------------------------------------------------------------------------------- 
#define TI0                6 //extruder digital output to xArm TI0, buffer space available: yes=1
#define TI1                 7 //extruder digital output to xArm TI1, end of file:  end =1
//#define TO0               //to reset pin for arm reset control

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int id = 7;
const int ModbusBaudRate = 9600;
long move_actual = 0.0;
long stepperAccel = 1000;
int lastIndex = 0;
long lastMove = 0.0;
long lastSpeed = 0.0;
int lastDir = 0;
int lastEnd = 0;

struct packet{
    int index;
    long move;
    long speed;
    int dir;
    int end;
    };
//--Initialize everything---------------------------------------------------------
packet data,datas,in,next;

CircularBuffer<packet,144> queue;

void messageHandler(char message[10],long variable,long fontSize,int cursX,int cursY, bool clear){ 
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(cursX,cursY);  
    display.setTextSize(fontSize);
    if  (variable == 0){ 
        for (byte idx = 0; idx < sizeof(message[0]); idx++) {
        display.print(message[idx]);
        display.display();
        }
    }
    else{
        if  (variable == 0){ 
          for (byte idx = 0; idx < sizeof(message[0]); idx++) {
          display.print(message[idx]);
          display.println(variable);
          display.display();
          }
        }
  if (clear == TRUE){
    display.clearDisplay();
    }
 
    
  }
}

void pollAndStoreData(){ 
  int curCap = queue.capacity;
  int cap = (curCap - 5);
  int sz = queue.size();
  if (sz < cap){
    int datas[5] = {ModbusRTUServer.poll()};
          if(!queue.push(in)){
            messageHandler("PUSH",0,1.5,1,9,TRUE);
            messageHandler("FAILED",0,1.5,1,9,TRUE);
          }
          else{
            messageHandler("PUSH",0,1,1,0,FALSE);
            in.index = datas[0];
            in.move = datas[1];
            in.speed = datas[2];
            in.dir = datas[3];
            in.end = datas[4];
            messageHandler("IDX:",in.index,1,1,9,FALSE);
            messageHandler("MV: ",in.move,1,1,18,FALSE);
            messageHandler("SPD: ",in.speed,1,1,27,FALSE);
            messageHandler("DIR: ",in.dir,1,1,36,FALSE); 
            messageHandler("END: ",in.end,1,1,45,TRUE); 
            lastIndex = in.index;
          }
        }
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
0x00,0x00=register 5 index back to arm -----deprecated?
Digitals:
TI0: extruder queue available: 1=ready, 0=not ready
TI1: extruder end flag: 0=end, 1=run
TO0: reset extruder: 0=reset, 1=normal
TO1: */

void setup() {

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    delay(2000);
    for(;;); // Don't proceed, loop forever
    }

  pinMode (TI0,OUTPUT); //
  pinMode (TI1,OUTPUT);

  Serial.begin(ModbusBaudRate);

  if (!ModbusRTUServer.begin(id, ModbusBaudRate)) {
    messageHandler("SHIFT",0,1,1,9,FALSE);
    messageHandler("FAILED",0,1,1,14,TRUE);
    }
  else{
    messageHandler("modBufDebug",0,1,1,9,TRUE);
    messageHandler("2-23-2024",0,1,1,9,TRUE);
    delay(1000);
  }
  ModbusRTUServer.configureHoldingRegisters(0x00,6); 

 
  while (!ModbusRTUServer.poll()) { //modbus initial poll to get first line number and clear registers
    display.clearDisplay();   
    messageHandler("WAITING FOR",0,1.5,1,9,TRUE);
    delay(500);
    messageHandler("MODBUS",0,1.5,1,9,TRUE);
    delay(500);
    }
  
  pollAndStoreData();

  }

void loop() {
  //pollAndStoreData();
  int idx = 0;
  while (lastEnd != 1){ 
    next = queue.shift();
    if (next.index <= idx){
      int desired = (next.index + 1);
      messageHandler("INDEX error:",idx,1,1,0,FALSE);
      messageHandler("WANT: ",desired,1,1,9,TRUE);
    }
    else{
    messageHandler("INDEX OUT:",next.index,1,1,9,FALSE);
      if (next.dir == 1){
        move_actual = next.move;
        }
      else {
        move_actual = (next.move * -1);
        }
      messageHandler("MOVE: ",move_actual,1,1,9,TRUE);
      pollAndStoreData();
      idx = next.index;
      delay(200);
    }
  }
messageHandler("FILE",0,1.5,1,9,FALSE);
messageHandler("COMPLETE",0,1.5,1,20,TRUE);
  delay(2000);
  ModbusRTUServer.end();
  Serial.end();
  digitalWrite(TI1,LOW);
}

