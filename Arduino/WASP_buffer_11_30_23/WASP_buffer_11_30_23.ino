

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
#include <CircularBuffer.h>
#include <SPI.h>
#include <Wire.h>
#include <FlexyStepper.h>
#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>
#include <C:\Users\morr0289\Documents\Arduino\libraries\CircularBuffer\CircularBuffer.h>
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

const int id = 7;
const int ModbusBaudRate = 9600;
long index = 0;
long move = 0;
long move_actual = 0;
long speed = 0;
long dir = 0;
long end = 0;
long stepperAccel = 1000;
long lastIndex = 0;
long lastMove = 0;
long lastSpeed = 0;
long lastDir = 0;
long lastEnd = 0;
unsigned int bufferCap = 500;


 struct packet{
    long index;
    long move;
    long speed;
    long dir;
    long end;
    };

FlexyStepper stepper;
packet in,next;
CircularBuffer<packet,100> queue;

void pollAndStoreData(){ 
  int cap = queue.capacity;
  while (queue.size() < cap){
    ModbusRTUServer.holdingRegisterWrite(4,1);
    ModbusRTUServer.poll();
    in.index = ModbusRTUServer.holdingRegisterRead(0);
    in.move = ModbusRTUServer.holdingRegisterRead(1);
    in.speed = ModbusRTUServer.holdingRegisterRead(2);
    in.dir = ModbusRTUServer.holdingRegisterRead(3);
    in.end = ModbusRTUServer.holdingRegisterRead(4);
    queue.push(in);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(9,100); 
    display.setTextSize(1);    
    display.println("push");
    display.display();
    delay(200);
    }
  }

void retrieveData(){
  next = queue.shift();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(18,100); 
  display.setTextSize(1);    
  display.println("shft");
  display.display();
  delay(200);
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
    delay(2000);
    }

  display.clearDisplay();   
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1,9);  
  display.setTextSize(1); 
  display.println("WASP_buf_12/4/23");
  display.display();
  delay(5000);
   

/*configure Modbus holding registers at address 0x00 thru 0x06

0x00: extruder movement in steps
0x01: extruder speed in steps/minute
0x02 extruder direction
0x03 file end: 1 for end
0x04 : extruder set to 1 if there is space in the CircularBuffer 'queue'

Format from Python: [0x09 slave id,0x10 function 16 write registers,0x00,0x00 beginning address to write,0x00,0x05 number of registers,0x0a number of bytes(2x registers)
,0x00,0x15 register 0 movement,0x00,0x06 register 1 speed,0x00,0x01 register 2 direction,0x00,0x64  register 3 end of file, register 4 buffer available ]*/

ModbusRTUServer.configureHoldingRegisters(0x00,6); 

stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);

digitalWrite(TI0,HIGH);
delay(500);

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
  }

void loop() {

  while (lastEnd != 1){
    pollAndStoreData();
    retrieveData();

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

    while(!stepper.motionComplete()){
      digitalWrite(TI0,LOW);
      stepper.processMovement();
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(35,0); 
      display.setTextSize(1);    
      display.println("extrude");
      display.setCursor(35,9);
      display.println(queue.size());
      display.display();
    }
    if(stepper.motionComplete()){
      digitalWrite(TI0,HIGH);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(35,0); 
      display.setTextSize(1);    
      display.println("done");
      display.setCursor(100,9);
      display.println(queue.size());
      display.display();
    }
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,9); 
    display.setTextSize(1); 
    display.print("move: ");   
    display.println(move_actual);
    display.print("speed: ");
    display.println(next.speed);
    display.print("line: ");
    display.println(next.index);
    display.display();
    //lastLine = next.line;
    lastMove = next.move;
    lastSpeed = next.speed;
    lastDir = next.dir;
    lastEnd = next.end;
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