/*
  ModbusRTUSlaveExample

  Created: 2023-07-22
  By: C. M. Bulliner
  Modified: 2023-07-29
  By: C. M. Bulliner
  
*/

#include <SoftwareSerial.h>
#include <ModbusRTUSlave.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define EXTRUDE_GLCD_HEIGHT 7 
#define EXTRUDE_GLCD_WIDTH  24 

ModbusRTUSlave modbus(Serial); // serial port, driver enable pin for rs-485 (optional)

const int stepEnablePin = 10;
const int readyPin = 6;
const int TIO1 = 7;
const byte id = 9;
uint32_t baud = 38400;

uint16_t holdingRegisters[6];
const int MoveBig = 0; 
const int MoveSmall = 0; 
const int SpeedBig = 0;
const int SpeedSmall = 0;
const int Dir = 1; 
const int Set = 0; 
float move_converted = 0.0;
float ext_speed_converted = 0.0;
float move_actual = 0.0;

int i=0;

void readyToGo(){
    ext_speed_converted = 0.0;
    move_actual = 0.0;
    digitalWrite(readyPin,HIGH);
    i=i+1;
}

float hexconvert(byte modinfosbig, byte modinfoslittle){
    int big = modinfosbig;
    big <<= 8;
    big = (big | modinfoslittle);
    float converted = ((float)big/100);
    return converted;
  }
void setup() {
  pinMode(10,OUTPUT);
  pinMode(6,OUTPUT); 
  pinMode(7,OUTPUT); 
  Serial.begin(baud,SERIAL_8N1);
  while (! Serial)
    ; 
  modbus.configureHoldingRegisters(holdingRegisters, 6); // unsigned 16 bit integer array of holding register values, number of holding registers
  modbus.begin(1, 38400);
  
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  //if(!
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);//){
    //Serial.println(F("SSD1306 allocation failed"));
    //for(;;); // Don't proceed, loop forever
  //}
  display.clearDisplay();  
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1,9);  
  display.setTextSize(1);     
  display.println("All set, boss");
  display.display();
  delay(2000);
  display.println("Starting up");
  display.display();
  delay(1000);
}

void loop() {
  
  modbus.poll();
    readyToGo();
    float ext_speed_converted = hexconvert(holdingRegisters[2],holdingRegisters[3]);
    if (holdingRegisters[4] == 1){
      float move_actual = hexconvert(holdingRegisters[0],holdingRegisters[1]);
      display.clearDisplay();   
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(1,9);  
      display.setTextSize(1);  
      display.println(move_actual);
      display.println();   
      display.display();
    }
  else {
    float move_actual = ((hexconvert(holdingRegisters[0],holdingRegisters[1]))*-1);
    display.clearDisplay();    
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,9); 
    display.setTextSize(1);     
    display.println(move_actual);
    display.println(ext_speed_converted);
    display.display();
  }
  


}
