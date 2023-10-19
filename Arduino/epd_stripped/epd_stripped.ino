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

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("e-Paper init and clear");
  epd.LDirInit();
  epd.Clear();
  }


void loop()
{
  paint.SetWidth(50);
  paint.SetHeight(60);
  paint.SetRotate(ROTATE_180);
  paint.Clear(UNCOLORED);
  paint.DrawStringAt(10, 10, str[i], &Font24, COLORED);
  epd.SetFrameMemoryPartial(paint.GetImage(), 80, 70, paint.GetWidth(), paint.GetHeight());
  epd.DisplayPartFrame();
  delay(100);




  epd.HDirInit();
  epd.Clear();
  epd.Sleep();
}
