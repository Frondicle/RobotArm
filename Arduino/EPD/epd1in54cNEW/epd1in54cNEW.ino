/**
 *  @filename   :   epd1in54b-demo.ino
 *  @brief      :   1.54inch e-paper display (C) demo
 *  @author     :   Yehui from Waveshare
 *
 *  Copyright (C) Waveshare     June 1 2018
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <SPI.h>
#include "epd1in54c.h"
#include "imagedata.h"
#include "epdpaint.h"
#include <stdio.h>

#define COLORED     0
#define UNCOLORED   1
#define ARRAY_SIZE  5
const char *msg[] = {"Howdy all", "this here", "epaper thingy", "is","pretty cool"};

Epd epd;
unsigned char image[1024];
Paint paint(image, 0, 0);


void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
    epd.Sleep();
}

void loop() {
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
}
