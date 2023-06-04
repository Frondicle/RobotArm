#include <SoftwareSerial.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Stepper.h>
#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>

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

//Stepper-------------------------------------------------------------------------------------

//Wasp extruder: 
//200 steps = 1 rotation
//Barrel diameter: 10mm 
//area: 78.54mm2
//Auger pitch: 7mm
//549.78 mm3 : 1 rotation
//Nozzle diameter: 2mm
//  Area: 3.14 mm2: 
//  1 rotation = 175.09mm long extrusion
//1 step = .875mm long extrusion

#define dirPinStepper    8  // yellow wire
#define stepPinStepper   9  // green wire
#define stepsPerRevolution 200

const int baudRate = 921600;
unsigned int extruder_speed = 0;
unsigned int extruder_dir = 0;

void setup() {
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  Serial.begin(baudRate);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)){
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
   }
// start the Modbus RTU server, with (slave) id 8
if (!ModbusRTUServer.begin(8,921600,SERIAL_8N1)) {
    Serial.println("Failed to start Modbus RTU Server!");
    while (1);
    }
  // configure speed holding registers at address 
  ModbusRTUServer.configureHoldingRegisters(771,1); 
  // configure mode holding registers at address 256 / 0x0100
  ModbusRTUServer.configureHoldingRegisters(256,1); 
}
void loop() {
  display.clearDisplay();
  delay (10);
  int bigByteSpd = ModbusRTUServer.holdingRegisterRead(771);
  //int lilByteSpd = ModbusRTUServer.holdingRegisterRead(772);
  long x0 = (long)bigByteSpd<<8;
  //long x1 = (long)lilByteSpd;

  

  int bigByteDir = ModbusRTUServer.holdingRegisterRead(256);
  //int lilByteDir = ModbusRTUServer.holdingRegisterRead(257);
  long y0 = (long)bigByteDir<<8;
  //long y1 = (long)lilByteDir;

  extruder_speed = x0; //|x1;
  Serial.println (extruder_speed);
  extruder_dir = bigByteDir;
  Serial.println (extruder_dir);
  if (extruder_dir > 0){
    digitalWrite(dirPinStepper, HIGH);
    display.clearDisplay();
    display.drawBitmap(4, 1,  extrude_glcd_bmp, 24, 7, 1);
    display.display();
    }
  else {
    digitalWrite(dirPinStepper, LOW);
    display.clearDisplay();
    display.drawBitmap(4, 1,  retract_glcd_bmp, 24, 7, 1);
    display.display();
  }
  //else{
  //display.clearDisplay();
  //display.clearDisplay();
  //display.setTextSize(1);      
 // display.setTextColor(SSD1306_WHITE);  
  //display.setCursor(0,0);     
  //display.println("NO DIR");
  //display.display();   
  //}

  display.print("Direction: ");  
 
  display.clearDisplay();
  display.clearDisplay();
  display.setTextSize(1);      
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5,9);  
  display.setTextSize(2);     
  display.println(extruder_speed);
  display.display(); 
  
  //***********************************STEPPER**************************
    // poll for Modbus RTU requests****************************************************************
  ModbusRTUServer.poll();
}
