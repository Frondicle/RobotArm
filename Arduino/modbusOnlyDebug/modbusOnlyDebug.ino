

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
#include <Adafruit_SSD1306.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int id = 7;
const int baudRate = 19200;//:param baud: 4800/9600/19200/38400/57600/115200/230400/460800/921600/1000000/1500000/2000000/2500000


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

  Serial.begin(baudRate);
  display.clearDisplay();   
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1,0);  
  display.setTextSize(1); 
  display.println(baudRate);
  display.display();
  delay(1000);


    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    delay(1000);
    for(;;); // Don't proceed, loop forever
    }
  
  delay(1000);
  ModbusRTUServer.configureHoldingRegisters(0x00,6); 

 
  while (!ModbusRTUServer.begin(id,baudRate)) { //modbus initial poll to get first line number and clear registers
    display.clearDisplay();   
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,10);  
    display.setTextSize(1); 
    display.println("WAITING FOR");
    display.setTextSize(1.5); 
    display.println("MODBUS");
    display.display();
    delay(500);
    }
}
void loop() {
  ModbusRTUServer.poll();
    long index = ModbusRTUServer.holdingRegisterRead(0);
    long move = ModbusRTUServer.holdingRegisterRead(1);
    long dir = ModbusRTUServer.holdingRegisterRead(2);
    long speed = ModbusRTUServer.holdingRegisterRead(3);
    long end = ModbusRTUServer.holdingRegisterRead(4);
    display.clearDisplay();   
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,0);  
    display.setTextSize(1); 
    display.print(index);
    display.print(", ");
    display.println(move);
    display.print(dir);
    display.print(", ");
    display.println(speed);
    display.print(end);
    display.display();
    delay(500);

  ModbusRTUServer.end();

}

