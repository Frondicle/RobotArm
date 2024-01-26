

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

//#include <SoftwareSerial.h>
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
//#define rxPin 17
//#define txPin 16

const int enablePin = 10;
const int readyPin = 6;
const int TIO1 = 7;
//const int rxPin = 17;
//const int txPin = 16;
uint8_t id = 9;
uint32_t baud = 115200;
uint8_t buf = 8;
uint16_t bufSize = 256;
uint32_t responseDelay = 0;
uint16_t numHoldingRegisters = 8;
uint16_t holdingRegisters[8];// = {200,201,202,203,204,205,206,207};

unsigned int move_received[2] = {0,0}; // registers 200,201
unsigned int ext_speed_received[2] = {0,0}; // registers 202,203
unsigned int dir_received = 0x01; // register 204, start forward
unsigned int extruder_ready = 0; //
unsigned int arm_speed_received[2] = {0,0}; //registers 206,207
float move_converted = 0.0;
float move_actual = 0.0;

int i=0;
//SoftwareSerial SerialMod(rxPin,txPin);
//SoftwareSerial SerialUSB();
ModbusRTUSlave modbus(Serial);

void readyToGo(){
    //holdingRegisters[] = {0,0,0,0,1,0,0,0};
    move_received[0] = {0}; 
    move_received[1] = {0}; 
    ext_speed_received[0] = {0};
    ext_speed_received[1] = {0};
    move_converted = 0.0;
    arm_speed_received [0]= {0};
    arm_speed_received [1]= {0};
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
  Serial.begin(baud); 
  //SerialMod.begin(baud,SERIAL_8E1);
  modbus.begin(id, baud,SERIAL_8E1);
  modbus.configureHoldingRegisters(holdingRegisters, numHoldingRegisters);
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
  delay(2000);
  Serial.print("Starting up");
}

void loop() {
  
  modbus.poll();
  Serial.print(holdingRegisters[0]," ");
  Serial.print(holdingRegisters[1]," ");
  Serial.print(holdingRegisters[2]," ");
  Serial.print(holdingRegisters[3]," ");
  dir_received = holdingRegisters[4];//204
  //float move_actual = (hexconvert(holdingRegisters[0],holdingRegisters[1]));
  //move_received[0] = holdingRegisters{0};//200
  //move_received[1] = holdingRegisters{1};//201
  //ext_speed_received[0] = holdingRegisters{2};//202 least significant bit: little-endian
  //ext_speed_received[1] = holdingRegisters{3}; //(203)
  float ext_speed_converted = (hexconvert(holdingRegisters[2],holdingRegisters[3]));
  //arm_speed_received[0] = holdingRegisters{6}; //(206)least significant bit: little-endian
  //arm_speed_received[1] = holdingRegisters{7};//(207);
  float arm_speed_converted = (hexconvert(holdingRegisters[6],holdingRegisters[7]));
  
  if (dir_received == 1){
    move_actual = hexconvert(holdingRegisters[0],holdingRegisters[1]);
    display.clearDisplay();   
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,9);  
    display.setTextSize(1);     
    display.println(move_actual);
    display.println(ext_speed_converted);
    display.display();
    }
  else {
    move_actual = (hexconvert(move_received[0],move_received[1])*-1);
    display.clearDisplay();    
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,9); 
    display.setTextSize(1);     
    display.println(move_actual);
    display.println(ext_speed_converted);
    display.display();
  }
}
