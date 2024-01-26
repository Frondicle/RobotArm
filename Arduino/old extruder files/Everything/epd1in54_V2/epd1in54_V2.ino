#include <SPI.h>
#include "epd1in54_V2.h"
#include "imagedata.h"
#include "epdpaint.h"
#include <stdio.h>

Epd epd;
unsigned char image[2896];
Paint paint(image, 0, 0);
const char *msg[] = {"Howdy all", "this here", "epaper thingy", "is","pretty cool"};

unsigned long time_start_ms;
unsigned long time_now_s;
#define COLORED     0
#define UNCOLORED   1
#define ARRAY_SIZE  5

void setup()
{


  epd.LDirInit();
  epd.Clear();
  paint.SetRotate(0);
  paint.SetWidth(176);
  paint.SetHeight(126);

  paint.Clear(UNCOLORED);

  Serial.println("e-Paper clear and goto sleep");
  epd.HDirInit();
  epd.Clear();
  epd.Sleep();
}

void loop()
{
    paint.Clear(UNCOLORED);
    paint.DrawStringAt(0, 10, "this ", &Font20, COLORED);
    paint.DrawStringAt(68, 8, "epaper", &Font24, COLORED);
    paint.DrawStringAt(0, 34, "is a", &Font20, COLORED);
    paint.DrawStringAt(64, 32, "total", &Font24, COLORED);
    paint.DrawStringAt(0, 55, "beeeeech", &Font24, COLORED);
    epd.SetFrameMemoryPartial(paint.GetImage(), 6, 124, paint.GetWidth(), paint.GetHeight());
    epd.DisplayFrame();
    delay(100);
}
