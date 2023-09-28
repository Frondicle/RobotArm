//
//    FILE: float2byteorder.ino
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.00
// PURPOSE: demo
//    DATE: 2014-02-04
//     URL:
//
// Released to the public domain
//
union 
{
  uint8_t b[4];
  float f;
} X;

void setup()
{
  // iniitalize modbus here
  Serial.begin(115200);

  X.f = 123.45;

  // order 0
  dump(0,1,2,3);
  dump(0,1,3,2);
  dump(0,2,1,3);
  dump(0,2,3,1);
  dump(0,3,1,2);
  dump(0,3,2,1);
  
  // order 1
  dump(1,0,2,3);
  dump(1,0,3,2);
  dump(1,2,0,3);
  dump(1,2,3,0);
  dump(1,3,0,2);
  dump(1,3,2,0);

  // order 2
  dump(2,1,0,3);
  dump(2,1,3,0);
  dump(2,0,1,3);
  dump(2,0,3,1);
  dump(2,3,1,0);
  dump(2,3,0,1);

  // order 3
  dump(3,1,2,0);
  dump(3,1,0,2);
  dump(3,2,1,0);
  dump(3,2,0,1);
  dump(3,0,1,2);
  dump(3,0,2,1);
  
}

void loop(){}

void dump(int a, int b, int c, int d)
{
  Serial.print(X.b[a]);  
  Serial.print(' ');
  Serial.print(X.b[b]);  
  Serial.print(' ');
  Serial.print(X.b[c]);  
  Serial.print(' ');
  Serial.println(X.b[d]);  
}
