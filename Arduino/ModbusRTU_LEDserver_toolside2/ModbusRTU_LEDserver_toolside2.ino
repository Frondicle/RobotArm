

/*
  Modbus RTU Server Kitchen Sink

  This sketch creates a Modbus RTU Server and demonstrates
  how to use various Modbus Server APIs.

  created 18 July 2018
  by Sandeep Mistry
*/

#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>
#include <Adafruit_NeoPixel.h>
#define PIN 6
#define NUMPIXELS 1 // Popular NeoPixel ring size
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);
#define DELAYVAL 250 // Time (in milliseconds) to pause between pixels: changed from 500

const int numHoldingRegisters = 2;
const int baudRate = 200000;
void setup() {
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  Serial.begin(baudRate);
  while (!Serial) {
  ; // wait for serial port to connect. Needed for native USB
  Serial.println(F("connecting serial"));
  }

  // start the Modbus RTU server, with (slave) id 8
  if (!ModbusRTUServer.begin(8, baudRate)) {
    Serial.println("Failed to start Modbus RTU Server!");
    while (1);
  }
 // configure speed holding registers at address 0x0303
  ModbusRTUServer.configureHoldingRegisters(0x0303, numHoldingRegisters); 
 // configure mode holding registers at address 0x0303
  ModbusRTUServer.configureHoldingRegisters(0x0101, numHoldingRegisters);   

void loop() {
    pixels.clear(); // Set all pixel colors to 'off'
    // poll for Modbus RTU requests
    ModbusRTUServer.poll();
    int R = ModbusRTUServer.holdingRegisterRead(0);
    int G = ModbusRTUServer.holdingRegisterRead(1);
    int B = ModbusRTUServer.holdingRegisterRead(2);
    
    // The first NeoPixel in a strand is #0, second is 1, all the way up
    // to the count of pixels minus one.
    for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(R, G, B));

    pixels.show();   // Send the updated pixel colors to the hardware.

    delay(DELAYVAL); // Pause before next pass through loop
  }  
  }
}
