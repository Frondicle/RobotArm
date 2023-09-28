//    FILE: RS485_simple_slave.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: simple listening slave
//     URL: https://github.com/RobTillaart/RS485

//  this is the code of a simple slave  (needs simple master)
//  it receives one of 3 (single char) commands to the slave
//  '0' == set LED LOW.
//  '1' == set LED HIGH.
//  '2' == return status to master.
//
//  print debug messages SEND and RECV with data.
//  Note that one needs a 2nd Serial port for nice debugging.
//  (or an LCD screen whatever).


#include "Arduino.h"
#include "RS485.h"
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

const uint8_t sendPin  = -1;
const uint8_t deviceID = 9;


RS485 rs485(&Serial, sendPin, deviceID);


const uint8_t LED = 13;
uint8_t status = LOW;


void setup()
{
  Serial.begin(115200);
  while (!Serial);
  
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
 
  //display.println(__FILE__);

  rs485.setMicrosPerByte(115200);

  //pinMode(LED, OUTPUT);
  //digitalWrite(LED, LOW);
}


void loop()
{
  if (rs485.available() > 0)
  {
    int c = rs485.read();
    display.clearDisplay();  
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,9);  
    display.setTextSize(1);     
    display.println("RECV: ");
    display.print(c);
    display.display();
    delay(500);


    if (c == '0') status = LOW;
    if (c == '1') status = HIGH;
    if (c == '2')
    {
      display.clearDisplay();  
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(1,9);  
      display.setTextSize(1); 
      display.println("SEND: ");
      display.print(status);
      rs485.print(status);
    }
    //digitalWrite(LED, status);
  }
}


//  -- END OF FILE --
