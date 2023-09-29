//firmware for extruder to use xArm6 to print with clay or pastes
//Chris Morrey at the Christopher C Gibbs College of Architecture, University of Oklahoma.9/22/2023 

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Stepper.h>
#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>

#define TI0              6 //extruder digital output to xArm TI0, LOW for busy
#define TI1              7 //spare IO to xArm TI1
#define dirPinStepper    8 //stepper direction, HIGH for forward
#define stepPinStepper   9  //stepper pin: high/low sequence makes one step
#define enablePinStepper 10 //stepper driver enable
#define stepsPerRevolution 200

#define EXTRUDE_GLCD_HEIGHT 7 // height of extrude graphic
#define EXTRUDE_GLCD_WIDTH  24 //width of extrude graphic
static unsigned char PROGMEM const extrude_glcd_bmp[] =
{ B00001100, B00011000, B00000000,
  B00011100, B00111000, B00000000,
  B00111100, B01111111, B11111111,
  B01111100, B11111111, B11111111,
  B00111100, B01111111, B11111111,
  B00011100, B00111000, B00000000,
  B00001100, B00011000, B00000000};

//___________________________________________
#define RETRACT_GLCD_HEIGHT 7 
#define RETRACT_GLCD_WIDTH  24 
static unsigned char PROGMEM const retract_glcd_bmp[] =
{ B00000000, B00011000, B00110000,
  B00000000, B00011100, B00111000,
  B11111111, B11111110, B00111100,
  B11111111, B11111111, B00111110,
  B11111111, B11111110, B00111100,
  B00000000, B00011100, B00111000,
  B00000000, B00011000, B00110000};


//____________________________________________
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int id = 9;
const int baudRate = 9600;
float extruder_speed;
float move = 0.0;
float move_actual = 0.0;
uint8_t move_received[];
uint8_t speed_received[];
uint8_t dir_received = 1;
int direction = 0;
float lastMove = 0.0;
float lastSpeed = 0;
int lastDir = 0;
int i = 0;

void readyToGo(){ //reset all variables and set readyPin to HIGH 
  extruder_speed = 0.0;
  move = 0.0;
  move_actual = 0.0;
  digitalWrite(TI0,HIGH);
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

  pinMode (dirPinStepper,OUTPUT);
  pinMode (enablePinStepper,OUTPUT);
  pinMode (stepPinStepper,OUTPUT);
  pinMode (TI0,OUTPUT);
  pinMode (TI1,OUTPUT);

  Serial.begin(baudRate);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
   }
      // start the Modbus RTU server
  if (!ModbusRTUServer.begin(id, baudRate)) {
    Serial.println("Failed to start Modbus RTU Server!");
    while (1);
    }

  display.clearDisplay();   
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,9);  
    display.setTextSize(1); 
    display.print("WASP_extruder_core_working");
    display.setTextSize(1.5);
    display.print("Baudrate: ");   
    display.print(baudRate);
    display.print("  ID: ");
    display.println(id);
    display.display();

  delay(2500);

  // configure Modbus holding registers at address 0x00 thru 0x0B (0-11)
  //0x00,0x01: movement length
  //0x02,0x03: extruder speed in...???
  //0x04: extruder direction 1 for forward, 0 for backward
  //0x05 thru 0x0B for error messages, and other stuff I don't know about yet

  ModbusRTUServer.configureHoldingRegisters(0x00,6); 


}
void loop() {

  readyToGo();// function to clear all the variables and hold the readyPin HIGH

  // poll for Modbus RTU requests****************************************************************
  ModbusRTUServer.poll();
  int move_0_received= ModbusRTUServer.holdingRegisterRead(0);
  int move_1_received= ModbusRTUServer.holdingRegisterRead(1);
  int speed_0_received = ModbusRTUServer.holdingRegisterRead(2);
  int speed_1_received = ModbusRTUServer.holdingRegisterRead(3);
  int dir_0_received = ModbusRTUServer.holdingRegisterRead(4);
  int dir_1_received = ModbusRTUServer.holdingRegisterRead(5);
  move = hexconvert(ModbusRTUServer.holdingRegisterRead(0),ModbusRTUServer.holdingRegisterRead(1));
  extruder_speed = hexconvert(ModbusRTUServer.holdingRegisterRead(2),ModbusRTUServer.holdingRegisterRead(3));
  direction = hexconvert(ModbusRTUServer.holdingRegisterRead(4),ModbusRTUServer.holdingRegisterRead(5));
  int data[] = {move_0_received,move_1_received,speed_0_received,speed_1_received,dir_0_received,dir_1_received};
  while(extruder_speed != lastSpeed || move != lastMove){
    if (direction == 1){
      move_actual = move;
      display.clearDisplay();
      display.drawBitmap(4, 1,  extrude_glcd_bmp, 24, 7, 1);    
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(1,9);  
      display.setTextSize(1); 
      //display.print("move: ");   
      //display.print(move_actual);
      //display.print(extruder_speed);
      //display.print(" ");
      display.display();
      }
    else {
      move_actual = (move * -1);
      display.clearDisplay();
      display.drawBitmap(4, 1,  retract_glcd_bmp, 24, 7, 1);     
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(1,9); 
      display.setTextSize(1); 
      //display.print("move_actual: ");   
      //display.print(move_actual);
      //display.print(move_received[0]);
      //display.print(" ");
      //display.println(move_received[1]);
      //display.print("extruder_speed: ");
      //display.print(extruder_speed);
      //display.print(" ");
      //display.print(speed_received[0]);
      //display.print(" ");
      //display.println(speed_received[1])
      display.println(data[0]," ",data[1]," ",data[2]," ",data[3]," ",data[4]," ",data[5]);
      //display.print("line: ");
      //display.println(i);
      display.display();
  lastMove = move;
  lastSpeed = extruder_speed;
  lastDir = direction;
  }

  //***********************************STEPPER**************************
/* Example sketch to control a stepper motor with TB6560 stepper motor driver and Arduino without a library. More info: https://www.makerguides.com

digitalWrite (busyPin,low); //let the xArm know that the extruder is busy
digitalWrite (enablePin,LOW);

  // Set the spinning direction clockwise:
  digitalWrite(dirPinStepper, HIGH);

  // Spin the stepper motor 1 revolution slowly:
  for (int i = 0; i < stepsPerRevolution; i++) {
    // These four lines result in 1 step:
    digitalWrite(stepPinStepper, HIGH);
    delayMicroseconds(2000);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(2000);
  }

  delay(1000);

  // Set the spinning direction counterclockwise:
  digitalWrite(dirPin, LOW);

  // Spin the stepper motor 1 revolution quickly:
  for (int i = 0; i < stepsPerRevolution; i++) {
    // These four lines result in 1 step:
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(1000);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(1000);
  }

  delay(1000);

  // Set the spinning direction clockwise:
  digitalWrite(dirPinStepper, HIGH);

  // Spin the stepper motor 5 revolutions fast:
  for (int i = 0; i < 5 * stepsPerRevolution; i++) {
    // These four lines result in 1 step:
    digitalWrite(stepPinStepper, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPinStepper, LOW);
    delayMicroseconds(500);
 

  delay(1000);

  // Set the spinning direction counterclockwise:
  //digitalWrite(dirPinStepper, LOW);

  // Spin the stepper motor 5 revolutions fast:
  for (int i = 0; i < 5 * stepsPerRevolution; i++) {
    // These four lines result in 1 step:
    digitalWrite(stepPinStepper, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPinStepper, LOW);
    delayMicroseconds(500);
  }

  delay(1000);
  //digitalWrite(enablePin,HIGH);*/

  }
}