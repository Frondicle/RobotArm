

/*
  ModbusRTUSlave Library - Holding Registers
  
  This sketch demonstrates how to setup ModbusRTUSlave with 2 holding registers.
  This sketch requires a Modbus RTU master/client device with an RS-232 or UART/USART port.
  If the port is a RS-232 port, a RS-232 to UART converter will be needed.
  If the port is UART but the logic levels of the port are different than the logic levels of your Arduino, a logic level converter will be needed.
  
  The master/client port will need to be cofigured using the following settings:
  - Baud Rate: 38400
  - Data Bits: 8
  - Parity: None
  - Stop Bit(s): 1
  - Slave/Server ID: 1
  - Holding Registers: 0 and 1
  
  Created: 2022-11-19
  By: C. M. Bulliner
  Modified: 2022-12-17
  By: C. M. Bulliner
*/

#include <ModbusSerial.h>
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

const int txenPin = 5;
const int stepEnablePin = 10;
const int readyPin = 6;
const int TIO1 = 7;
const int id = 9;
uint32_t baud = 9600;

const int MoveBig = 0; // registers 200,201
const int MoveSmall = 1; // registers 202,203
const int SpeedBig = 2;
const int SpeedSmall = 3;
const int Dir = 4; // register 204, start forward
const int Set = 5; //
float move_converted = 0.0;
float ext_speed_converted = 0.0;
float move_actual = 0.0;

int i=0;

ModbusSerial mod(Serial,id,txenPin);

void readyToGo(){
    ext_speed_converted = 0.0;
    move_actual = 0.0;
    digitalWrite(readyPin,HIGH);
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
  pinMode(10,OUTPUT);
  pinMode(6,OUTPUT); 
  pinMode(7,OUTPUT); 
  Serial.begin(baud,SERIAL_8N1);
  while (! Serial)
    ; 
  mod.config(baud);
  mod.addHreg(MoveBig,0);
  mod.addHreg(MoveSmall,0);
  mod.addHreg(SpeedBig,0);
  mod.addHreg(SpeedSmall,0);
  mod.addHreg(Dir,1);
  mod.addHreg(Set,0);
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)){
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();  
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1,9);  
  display.setTextSize(1);     
  display.println("All set, boss");
  display.display();
  delay(1000);
  display.print("Starting up");
  delay(1000);
}

void loop() {
  
  mod.task();
  //Serial.println(mod.hreg(MoveBig)," ");
  //Serial.print(mod.hreg(MoveSmall)," ");
  //Serial.print(mod.hreg(SpeedBig)," ");
  //Serial.print(mod.hreg(SpeedSmall)," ");
  float ext_speed_converted = (hexconvert(mod.hreg(SpeedBig),mod.hreg(SpeedSmall)));
  
  if (mod.hreg(Dir) == 1){
    float move_actual = hexconvert(mod.hreg(MoveBig),mod.hreg(MoveSmall));
    display.clearDisplay();   
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,9);  
    display.setTextSize(1);     
    display.println(mod.hreg(MoveBig));
    display.println(mod.hreg(SpeedBig));
    display.display();
    }
  else {
    float move_actual = (hexconvert(mod.hreg(MoveBig),mod.hreg(MoveSmall))*-1);
    display.clearDisplay();    
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,9); 
    display.setTextSize(1);     
    display.println(mod.hreg(MoveBig));
    display.println(mod.hreg(SpeedBig));
    display.display();
  }
}
