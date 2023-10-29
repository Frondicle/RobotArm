#include <SPI.h>
#include "epd1in54_V2.h"
#include "epdpaint.h"
#include <stdio.h>

Epd epd;
unsigned char image[1024];
Paint paint(image, 0, 0);

unsigned long time_start_ms;
unsigned long time_now_s;
#define COLORED     0
#define UNCOLORED   1
#define ARRAY_SIZE  5
const char *msg[] = {"Howdy all", "this here", "epaper thingy", "is","pretty cool"};

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("e-Paper init and clear");
  epd.LDirInit();
  epd.Clear();
  }

void loop()
{
  paint.SetWidth(196);
  paint.SetHeight(196);
  paint.SetRotate(ROTATE_180);

  int height = 96;
  //paint.Clear(UNCOLORED);
  for (int i = 0; i < ARRAY_SIZE; i++) {
    paint.DrawStringAt(0, 24, msg[i], &Font24, UNCOLORED);
    epd.SetFrameMemory(paint.GetImage(), 2, 2, paint.GetWidth(), paint.GetHeight());
    epd.DisplayFrame();
    height = height - 24;
    delay(200);
  }
  //epd.HDirInit();
  //epd.Clear();
  epd.Sleep();
}
