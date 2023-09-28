

//Extruder firmware 7/17/2023 Chris Morrey Gibbs College of Architecture 

#include <ModbusRtu.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "FlexyStepper.h" // Documentation at: https://github.com/Stan-Reifel/FlexyStepper

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define EXTRUDE_GLCD_HEIGHT 7 
#define EXTRUDE_GLCD_WIDTH  24 
static unsigned char PROGMEM const extrude_glcd_bmp[] =
{ B00001100, B00011000, B00000000,
  B00011100, B00111000, B00000000,
  B00111100, B01111111, B11111111,
  B01111100, B11111111, B11111111,
  B00111100, B01111111, B11111111,
  B00011100, B00111000, B00000000,
  B00001100, B00011000, B00000000};

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

const int enablePin = 10;
const int readyPin = 4;
const int comPin = 6;
const int steps_per_revolution = 200;
const int baudRate = 9600;
uint16_t modTimeOut = 1000;
unsigned int move_received[2] = {0,0}; // registers 200,201
unsigned int ext_speed_received[2] = {0,0}; // registers 202,203
unsigned int dir_received = 0x01; // register 204, start forward
unsigned int extruder_ready = 0; 
unsigned int arm_speed_received[2] = {0,0}; //registers 206,207
//uint16_t move_temp = 0x0000;
//uint16_t ext_speed_temp = 0x0000;
//uint16_t arm_speed_temp = 0x0000;
float move_converted = 0.0;
float ext_speed_converted = 0.0;
float arm_speed_converted = 0.0;
float move_actual = 0.0;
int stepper_movement = 0;
const float nozzle_diameter = 2.0; //mm

//Wasp extruder 2mm nozzle sizes---------------------------------------------------
int barrel_radius = 5; //millimeters 
float barrel_area = (barrel_radius * 3.14159); //mm2
float auger_pitch = 7.15; //mm
float rotation_volume = (barrel_area * auger_pitch);
float nozzle_area = ((nozzle_diameter / 2)*3.14159); //mm2: 
float ext_length_per_revolution = rotation_volume / nozzle_area; //mm
float ext_length_per_step = ext_length_per_revolution / steps_per_revolution;
int i=0;
#define TxEnPin 5

// initialize the stepper library
FlexyStepper stepper;


// data array for modbus network sharing
uint16_t au16data[] = {
  200,201,202,203,204,205,206,207 };
/**
 *  Modbus object declaration
 *  u8id : node id = 0 for master, = 1..247 for slave
 *  port : serial port
 *  u8txenpin : 0 for RS-232 and USB-FTDI 
 *               or any pin number > 1 for RS-485
 */
Modbus slave(9,Serial,TxEnPin); // this is slave @9 and RS-485 on pin 5

void readyToGo(){
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
  pinMode(4,OUTPUT);
  Serial.begin(baudRate,SERIAL_8N1);
  stepper.connectToPins(8,9); //step, direction
  stepper.setStepsPerMillimeter(3);//(ext_length_per_step)
  stepper.setAccelerationInMillimetersPerSecondPerSecond(100);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)){
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
   }
 
slave.start();
slave.setTimeOut(modTimeOut);

display.clearDisplay();  
display.setTextColor(SSD1306_WHITE);
display.setCursor(1,9);  
display.setTextSize(1);     
display.println("All set, boss");
display.display();
delay(2000);
}

void loop() {

  bool stopFlag = false;
  readyToGo();
  int8_t registers[] = {slave.poll( au16data, 8 )};
  dir_received = registers[4];//204
  move_received[0] = registers[0];//200
  move_received[1] = registers[1];//201
  ext_speed_received[0] = registers[2];//202 least significant bit: little-endian
  ext_speed_received[1] = registers[3]; //(203)
  arm_speed_received[0] = registers[6]; //(206)least significant bit: little-endian
  arm_speed_received[1] = registers[7];//(207);
  
  if (dir_received == 1){
    move_actual = hexconvert(move_received[0],move_received[1]);
    display.clearDisplay();
    display.drawBitmap(4, 1,  extrude_glcd_bmp, 24, 7, 1);    
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,9);  
    display.setTextSize(1);     
    display.println(move_actual);
    display.println(ext_speed_converted);
    display.println(stepper.getCurrentPositionInMillimeters());
    display.display();
    }
  else {
    move_actual = (hexconvert(move_received[0],move_received[1])*-1);
    display.clearDisplay();
    display.drawBitmap(4, 1,  retract_glcd_bmp, 24, 7, 1);     
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(1,9); 
    display.setTextSize(1);     
    display.println(move_actual);
    display.println(ext_speed_converted);
    display.println(stepper.getCurrentPositionInMillimeters());
    display.display();
    
  //***********************************STEPPER**************************
    
    digitalWrite(enablePin, HIGH);
    stepper.setSpeedInMillimetersPerSecond(ext_speed_converted);
    stepper.moveRelativeInMillimeters(move_actual);
    while(!stepper.motionComplete()){
      digitalWrite(readyPin,LOW);
      stepper.processMovement(); 
      }
  }
} 
