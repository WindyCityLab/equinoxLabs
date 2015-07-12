#include <Wire.h>
#define SLAVEPIN 13
#define I2C_SLAVE_ADDRESS 5

void setup()
{
  Wire.begin(I2C_SLAVE_ADDRESS);
  pinMode(SLAVEPIN, OUTPUT);
  digitalWrite(SLAVEPIN, LOW);
  Wire.onReceive(receiveEvent);
}

void receiveEvent(int byteCountReceived)
{
  
//  while (Wire.available())
//  {
//    char c = Wire.read();
//    
//    if (c == 'H')
//    {
//      digitalWrite(SLAVEPIN,HIGH);
//    }
//    else
//    {
//      digitalWrite(SLAVEPIN,LOW);
//    }
  
      for (int i = 0; i<byteCountReceived;i++) 
      {
       digitalWrite(SLAVEPIN,HIGH);
       delay(1000); 
      digitalWrite(SLAVEPIN,LOW);
       delay(1000); 
    }
//  }
}
void loop()
{

}
