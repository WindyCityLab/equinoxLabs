#include <Wire.h>
#define LOCALPIN 13
void setup()
{
  Wire.begin();
  //TWBR = 12;
  pinMode(LOCALPIN,OUTPUT);
  Serial.begin(9600);
  Serial.println(TWBR);
  Serial.println(TWSR,HEX);
  Serial.println(F_CPU);
}

void loop()
{
  Wire.beginTransmission(5);
  Wire.write('H');
  Wire.write('L');
  Wire.write('H');
  digitalWrite(LOCALPIN,HIGH);
  Wire.endTransmission();
  
  delay(1000);

  Wire.beginTransmission(5);
  Wire.write('L');
  digitalWrite(LOCALPIN,LOW);
  Wire.endTransmission();
  
  delay(1000); 
 
}
