


#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal.h>
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>
#include <ZX_Sensor.h>
//#include <SFE_BMP180.h>


const int ZX_ADDR = 0x10;  // ZX Sensor I2C address
ZX_Sensor zx_sensor = ZX_Sensor(ZX_ADDR);
uint8_t x_pos;
uint8_t z_pos;

//  #include <SoftwareSerial.h>

//#include "Adafruit_BLE.h"
//#include "Adafruit_BluefruitLE_SPI.h"
//#include "Adafruit_BluefruitLE_UART.h"
////#include "BluefruitConfig.h"

//Adafruit_BluefruitLE_SPI ble(1, 2, 3);

#define CARDCS 4     // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin

#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7      // VS1053 chip select pin (output)
#define SHIELD_DCS    6      // VS1053 Data/command select pin (output)
#define VOLUME       20


Adafruit_VS1053_FilePlayer musicPlayer = Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);
//SFE_BMP180 pressure;


bool DKDEBUG = true;

RTC_DS1307 rtc;

bool alarmOn = false;
const int snoozeAmount = 300;
bool snoozing;
DateTime alarm1;
DateTime alarm1SnoozeTime;
bool alarm1Enabled;
bool alarm1Active;

DateTime alarm2;
DateTime alarm2SnoozeTime;
bool alarm2Enabled;
bool alarm2Active;


enum clockState {
  clockStateNormalOperation,
  clockStateSetTime,
  clockStateSetAlarm1,
  clockStateSetAlarm2,
  clockStateCount
  
};
clockState setMode;


DateTime currentSetTime;


const int buzzerPin = 6;
const int hourPin = 1;
const int minutePin = 2;
const int setPin = 3;
const int snoozePin = 4;



LiquidCrystal lcd(8,9,17,16,15,14);

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(9600);
#ifdef AVR
  Wire.begin();
#else
  Wire1.begin(); // Shield I2C pins connect to alt I2C bus on Arduino Due
#endif
  rtc.begin();
 
  DateTime now = rtc.now();
//  alarm1 = DateTime(now.year(),now.month(),now.day(),6,0,0);
  alarm2 = DateTime(now.year(),now.month(),now.day(),6,0,0);  

  alarm1Enabled = true;
  alarm1 = DateTime(now.unixtime() + 10);
  

  synchronizeSnoozeTimesToAlarmTimes();
  
  lcd.begin(16, 2);
  lcd.clear();
  
  setupMusicPlayer();
  
  // Initialize ZX Sensor (configure I2C and read model ID)
  if ( zx_sensor.init() ) {
    Serial.println("ZX Sensor initialization complete");
  } else {
    Serial.println("Something went wrong during ZX Sensor init!");
  }


}

void setupMusicPlayer() 
{
   if (! musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
     
  }
  musicPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working
  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }
  Serial.println("SD OK!");

    printDirectory(SD.open("/"), 0);
  musicPlayer.setVolume(VOLUME,VOLUME);
  if (! musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT))
    lcd.print("hi, World");
    
  musicPlayer.GPIO_pinMode(hourPin, INPUT);
  musicPlayer.GPIO_pinMode(minutePin, INPUT);
  musicPlayer.GPIO_pinMode(setPin, INPUT);
  musicPlayer.GPIO_pinMode(snoozePin, INPUT);
  musicPlayer.GPIO_pinMode(buzzerPin, OUTPUT);


}

void loop() 
{

    if (setMode == clockStateNormalOperation) //If we're not in set mode
    {      
        if (debounceRead(setPin)) // Lets see if our button is pressed
        {
          setMode = clockStateSetTime; // If it is, we're in set mode now
          currentSetTime = rtc.now();
          lcd.clear();
          Serial.println("now in set mode");
        }
        else // if its not, carry on
        {
           updateDisplayNormalOperation();
           monitorAlarmButtons();
           checkAlarms();
           
           if(alarm1Active || alarm2Active)
           {
             monitorSnoozeSensor();
           }
        }
    }else //We're in set mode.
    {
      blinkDisplay();
      updateDisplaySetMode();
      monitorHourMinuteButtons();
      
      if (debounceRead(setPin)) //If the button is pressed again, increment.
      {
         lcd.clear();
         setMode=clockState(int(setMode)+1);
         Serial.println(setMode);
         if (setMode == clockStateCount) 
         {
           setMode = clockStateNormalOperation;
           lcd.display();
           setRTCTime();
         } 
       }
    }
}

void setRTCTime()
{
  rtc.adjust(currentSetTime);
}
int buttonStates[4] = {LOW, LOW, LOW, LOW};
int lastButtonStates[4];

int buttonState = LOW;      // the current reading from the input pin
int lastButtonState = LOW;  // the previous reading from the input pin
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers
long lastChange = 0;


bool debounceRead(int pin)
{
  
  if ((millis() - lastChange) < 200)
  {
    return false;
  }


  switch (pin)
  {
    case hourPin:
    {
      buttonState = buttonStates[0];
      lastButtonState = lastButtonStates[0];
    }
    break;
    case minutePin:
    {
      buttonState = buttonStates[1];
      lastButtonState = lastButtonStates[1];
    }
    break;
    case setPin:
    {
      buttonState = buttonStates[2];
      lastButtonState = lastButtonStates[2];
    }
    break;
    case snoozePin:
    {
      buttonState = buttonStates[3];
      lastButtonState = lastButtonStates[3];
    }
    break;
  }
    
  int reading = musicPlayer.GPIO_digitalRead(pin);

  
  if (reading != lastButtonState) 
  {
//    Serial.println("reading is different than last button state");
    lastDebounceTime = millis();
  }



  if ((millis() - lastDebounceTime) > debounceDelay) 
  {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:
    // if the button state has changed:
    if (reading != buttonState) 
    {
      
      Serial.println(F("Button press successful"));
      buttonState = reading;
      lastChange = millis();
      return buttonState;
    }
  }
  
    lastButtonState = reading;
    
      switch (pin)
  {
    case hourPin:
    {
      buttonStates[0] = buttonState;
      lastButtonStates[0] = lastButtonState;
    }
    break;
    case minutePin:
    {
      buttonStates[1] = buttonState;
      lastButtonStates[1] = lastButtonState;
    }
    break;
    case setPin:
    {
      buttonStates[2] = buttonState;
      lastButtonStates[2] = lastButtonState;
    }
    break;
    case snoozePin:
    {
      buttonStates[3] = buttonState;
      lastButtonStates[3] = lastButtonState;
    }
    break;
  }

}

void monitorAlarmButtons()
{
  if (debounceRead(hourPin))
  {
    if (alarm1Active)
    {
      alarm1Active = false;
      synchronizeSnoozeTimesToAlarmTimes();
      stopAlarm();
    }
    else
    {
      alarm1Enabled = !alarm1Enabled;
    }  
  } 
  if (debounceRead(minutePin))
  {
    if (alarm2Active)
    {
      alarm2Active = false;
      synchronizeSnoozeTimesToAlarmTimes();
      stopAlarm();
    }
    else
    {
      alarm2Enabled = !alarm2Enabled;
    } 
  }
}

void monitorHourMinuteButtons()
{
  if (debounceRead(hourPin))
  {
    switch (setMode)
    {
      case clockStateSetTime:
      {
        currentSetTime = (currentSetTime.unixtime() + 3600); // One hour later.
      }break;
      case clockStateSetAlarm1:
      {
        alarm1 = (alarm1.unixtime() + 3600); // One hour later.
      }break;
      case clockStateSetAlarm2:
      {
        alarm2 = (alarm2.unixtime() + 3600); // One hour later.
      }break;
    }
    synchronizeSnoozeTimesToAlarmTimes();
  } 
  
  if (debounceRead(minutePin))
  {
    switch (setMode)
    {
      case clockStateSetTime:
      {
        currentSetTime = (currentSetTime.unixtime() + 60); // One min later.
      }break;
      case clockStateSetAlarm1:
      {
        alarm1 = (alarm1.unixtime() + 60); // One min later.
      }break;
      case clockStateSetAlarm2:
      {
        alarm2 = (alarm2.unixtime() + 60); // One min later.
      }break;
    }
    synchronizeSnoozeTimesToAlarmTimes();
  }
}

void synchronizeSnoozeTimesToAlarmTimes()
{
  alarm1SnoozeTime = alarm1.unixtime(); //Synchronize SnoozeTime to new alarm
  alarm2SnoozeTime = alarm2.unixtime();
}
void monitorSnoozeSensor()
{
  if ( zx_sensor.positionAvailable() ) {
    z_pos = zx_sensor.readZ();
    if ( z_pos != ZX_ERROR ) {
      Serial.print(" Z: ");
      Serial.println(z_pos);
      
      if (z_pos < 10)
      {
        snoozeHit();
      }
      
    }
  }

}

void checkAlarms() 
{
  DateTime now = rtc.now();
  
  static long timeUpdate = 0;
  if (millis() - timeUpdate > 1000)
  {
    timeUpdate = millis();
      //alarm 1
      if ((shouldTriggerAlarmForTime(alarm1) || shouldTriggerAlarmForTime(alarm1SnoozeTime)) &&
          alarm1Enabled)
      {
        startAlarm();
        alarm1Active = true;
      }
      
      //alarm 2
      if ((shouldTriggerAlarmForTime(alarm2) || shouldTriggerAlarmForTime(alarm2SnoozeTime)) &&
          alarm2Enabled )
      {

        startAlarm();
        alarm2Active = true;
      }
  }
}

bool shouldTriggerAlarmForTime(DateTime alarmTime)
{
  if (alarmOn)  return false;

   DateTime now = rtc.now();
   return (now.hour() == alarmTime.hour()     &&
           now.minute() == alarmTime.minute() &&
           now.second() == alarmTime.second());
}


#pragma mark - Alarm Ringing functionality


void startAlarm() 
{
  
 playSong(); 
 alarmActiveLEDOn(); 
   alarmOn = true;

}

void stopAlarm()
{
  stopSong();
  alarmActiveLEDOff();
    alarmOn = false;

}
void playSong() 
{
  musicPlayer.startPlayingFile("track001.mp3");
}

void stopSong()
{
  musicPlayer.stopPlaying();
}


void alarmActiveLEDOn() 
{
  musicPlayer.GPIO_digitalWrite(buzzerPin, HIGH);
}

void alarmActiveLEDOff() 
{
  musicPlayer.GPIO_digitalWrite(buzzerPin, LOW);
}

void snoozeHit() 
{
  DateTime now = rtc.now();
  Serial.println("SNOOZING");
  if (alarm1Active)
  {
    alarm1Active = false;
    alarm1SnoozeTime = now.unixtime() + snoozeAmount;
  }
  if (alarm2Active)
  {
    alarm2Active = false;
    alarm2SnoozeTime = now.unixtime() + snoozeAmount;
  }

  stopAlarm();
  
  
}

void updateDisplayNormalOperation() 
{
  static long timeUpdate = 0;
  if (millis() - timeUpdate > 500)
  {
    timeUpdate = millis();
    writeCurrentTime();  
    writeAlarmsStatus();
  }
  
}


void updateDisplaySetMode()
{
  switch (setMode){
    case clockStateSetTime:
    {
      
      writeTimeOnDisplay(currentSetTime,false,0,0);
    }break;
    case clockStateSetAlarm1:
    {
      writeTimeOnDisplay(alarm1,false,0,1);

    }break;
    case clockStateSetAlarm2:
    {
      writeTimeOnDisplay(alarm2,false,10,1);

    }break;
  }
  

}


bool displayOn = true;
void blinkDisplay()
{
  static long lastUpdate;
  if (millis() - lastUpdate > 500)
  {
    lastUpdate = millis();
    displayOn = !displayOn;
    if (displayOn)
    {
       lcd.noDisplay();     
    }
    else
    {
      lcd.display();
    }
  }
}


void writeAlarmsStatus() 
{ 
  lcd.setCursor(0,1);
  if (alarm1Enabled) 
  {  
    if(alarm1Active)
    {
      lcd.print("*");
    }
    writeTimeOnDisplay(alarm1,false,0,1);  
  }
  else 
  {
    lcd.print("--:-- ");
  }
  lcd.setCursor(10,1);
  if (alarm2Enabled) 
  {
    if(alarm2Active)
    {
      lcd.print("*");
    }
    writeTimeOnDisplay(alarm2,false,10,1);  
  }  
  else 
  {
    lcd.print("--:-- ");
  }
}

void writeCurrentTime()
{
 
  writeTimeOnDisplay(rtc.now(),true,0,0);  
  
  if (DKDEBUG) 
  {
    DateTime now = rtc.now();
    int armyHour = now.hour();
    if (armyHour > 12) 
    {
      armyHour -= 12;
    }
    Serial.print(armyHour, DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
  }
}

void writeTimeOnDisplay(DateTime time, bool withSeconds, int atCharacterIndex, int onLine) 
{
  bool isAM = true;
    int armyHour = time.hour();
    if (armyHour > 12) {
      isAM = false;
      armyHour -= 12;
    }

    lcd.setCursor(atCharacterIndex,onLine);
    if (armyHour < 10) {
      lcd.print('0');
    } 
    lcd.print(armyHour, DEC);

    lcd.print(':');
    if (time.minute() < 10) {
      lcd.print('0');
    } 
    lcd.print(time.minute(), DEC);
    
    if (withSeconds) 
    {
      lcd.print(':');
      if (time.second() < 10) {
        lcd.print('0');
      } 
  
      lcd.print(time.second(), DEC);
    }
    
    if (isAM)
    {
      lcd.print("A");
    }
    else
  {
      lcd.print("P");
  }
}  


void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}

