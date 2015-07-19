#include <Wire.h>

#define I2C_ESP8366_ADDRESS 0x50

void setup() {
  Wire.begin();  // We are master
  
}

void loop() {
  Wire.beginTransmission(I2C_ESP8366_ADDRESS);
  Wire.write(33);
  Wire.endTransmission();
  delay(1000);
}
