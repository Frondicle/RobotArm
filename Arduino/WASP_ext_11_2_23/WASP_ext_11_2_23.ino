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

  You should have received a copy of the GNU Lesser General Public
  License along with this software; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <SPI.h>
#include <Wire.h>
#include <FlexyStepper.h>
#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define TI0                6 //extruder digital output to xArm TI0, LOW for busy
#define TI1                 7 //spare digital output to xArm TI0
//#define TO0               to reset pin for arm reset control
//#define TO1                 19 //digital input for robot arm

#define dirPinStepper       8 //stepper direction, HIGH for forward
#define stepPinStepper      9  //stepper pin: high/low sequence makes one step
#define stepsPerRevolution  200

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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

int steps_per_revolution = 200;
float barrel_diameter_mm = 10;
float barrel_area = 75.39;//area minus auger shaft in mm2 
float auger_pitch = 7.0;//mm
float volume_per_rotation_mm = barrel_area * auger_pitch;
int nozzle_diameter_mm = 2;
float nozzle_area_mm = 3.14; 
float extrusion_per_rotation_mm = volume_per_rotation_mm / nozzle_area_mm;
float extrusion_per_step_mm = extrusion_per_rotation_mm / steps_per_revolution;

const int MOTOR_STEP_PIN = 8;
const int MOTOR_DIRECTION_PIN = 9;

const int id = 9;
const int ModbusBaudRate = 9600;
float speed = 0.0;
float speed_mapped = 0.0;
float move = 0.0;
float move_actual = 0.0;
int dir = 0;
int end = 0;
int line = 0;
float lastMove = 0.0;
float lastSpeed = 0.0;
int lastDir = 0;
int lastEnd = 0;
int i = 0;

FlexyStepper stepper;

void resetAndPoll(){ //reset all stepper variables and set readyPin to HIGH 
  speed = 0.0;
  move = 0.0;
  move_actual = 0.0;
  digitalWrite(TI0,HIGH);
  delay(20);
  ModbusRTUServer.poll();
  move = ModbusRTUServer.holdingRegisterRead(0);
  speed = ModbusRTUServer.holdingRegisterRead(1);
  dir = ModbusRTUServer.holdingRegisterRead(2);
  line = ModbusRTUServer.holdingRegisterRead(3);
  end = ModbusRTUServer.holdingRegisterRead(4);
}

void setup() {

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
    }

  stepper.connectToPins(MOTOR_STEP_PIN,MOTOR_DIRECTION_PIN);

  pinMode (TI0,OUTPUT);
  pinMode (TI1,OUTPUT);
  //pinMode (TO0,INPUT);
  //pinMode (TO1,INPUT);

  Serial.begin(ModbusBaudRate);

  if (!ModbusRTUServer.begin(id, ModbusBaudRate)) {
    display.clearDisplay();   
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,9);  
    display.setTextSize(2); 
    display.println("NO");
    display.println("MODBUS");
    display.display();
    while (1);
    }

  display.clearDisplay();   
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1,9);  
  display.setTextSize(1); 
  display.println("WASP_ext_11/2/23");
  display.setTextSize(1.5);
  display.print("Baudrate: ");   
  display.println(ModbusBaudRate);
  display.display();
  delay(5000);

/*configure Modbus holding registers at address 0x00 thru 0x0B (0-11)
0x00: movement length
0x01: extruder speed in...???
0x02: extruder direction 1 for forward, 0 for backward
0x03 file line number
0x04 file end: 1 for end
0x05 thru 0x06 for error messages, and other stuff I don't know about yet
Format frpom Python: [0x09 slave id,0x10 function 16 write registers,0x00,0x00 beginning address to write,0x00,0x05 number of registers,0x0a number of bytes(2x registers)
,0x00,0x15 register 0 movement,0x00,0x06 register 1 speed,0x00,0x01 register 2 direction,0x00,0x64 register 3 line number,0x00,0x00 register 4 end of file]*/

ModbusRTUServer.configureHoldingRegisters(0x00,5); 

stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);

digitalWrite(TI0,HIGH);

if (!ModbusRTUServer.poll()) {
    display.clearDisplay();   
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,0);  
    display.setTextSize(1.5); 
    display.println("Waiting for:");
    display.setTextSize(2);
    display.println("xArm MODBUS");
    display.display();
    while (1);
    }
  }

void loop() {

  while (lastEnd!=1){
    resetAndPoll();
    if (dir == 1){
      move_actual = move;
      display.clearDisplay();
      display.drawBitmap(4, 1,  extrude_glcd_bmp, 24, 7, 1);
      }
    else {
      move_actual = (move * -1);
      display.clearDisplay();
      display.drawBitmap(4, 1,  retract_glcd_bmp, 24, 7, 1);
    }
    stepper.setSpeedInStepsPerSecond(speed);
    stepper.setAccelerationInStepsPerSecondPerSecond(1000);
    stepper.moveRelativeInSteps(move_actual);

    digitalWrite(TI0,LOW);

    stepper.processMovement();
     
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,9); 
    display.setTextSize(1); 
    display.print("move: ");   
    display.println(move_actual);
    display.print("speed: ");
    display.println(speed);
    display.print("line: ");
    display.println(line);
    display.display();
    lastMove = move;
    lastSpeed = speed;
    lastDir = dir;
    lastEnd = end;
    }
  display.clearDisplay(); 
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1,9); 
  display.setTextSize(1.5); 
  display.println("FILE");   
  display.println("COMPLETE");
  display.display();
  ModbusRTUServer.end();
  Serial.end();
}