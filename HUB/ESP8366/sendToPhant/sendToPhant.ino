// Include the ESP8266 WiFi library. (Works a lot like the
// Arduino WiFi library.)
#include <ESP8266WiFi.h>
#include <Wire.h>
// Include the SparkFun Phant library.
#include <Phant.h>

#define I2C_ADDRESS 0x50

//////////////////////
// WiFi Definitions //
//////////////////////
const char WiFiSSID[] = "Catalyze Office";
const char WiFiPSK[] = "perpetualimprovement";

/////////////////////
// Pin Definitions //
/////////////////////
const int LED_PIN = 5; // Thing's onboard, green LED
const int ANALOG_PIN = A0; // The only analog pin on the Thing
const int DIGITAL_PIN = 12; // Digital pin to be read

////////////////
// Phant Keys //
////////////////
const char PhantHost[] = "data.sparkfun.com";
const char PublicKey[] = "MGw00mdVaLf53Vnj7an6";
const char PrivateKey[] = "nz4GGPnBmXTpVjEn0rEe";

/////////////////
// Post Timing //
/////////////////
const unsigned long postRate = 30000;
unsigned long lastPost = 0;

void setup() 
{
  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(gotData);
  initHardware();
  connectWiFi();
  digitalWrite(LED_PIN, HIGH);
}

volatile bool receivedData = false;
volatile int value = 0;
void gotData(int count)
{
    value = Wire.read();
    receivedData = true;
}
void loop() 
{
  if (receivedData)
  {
 //   postToPhant();
    Serial.println(value);
    digitalWrite(LED_PIN,LOW);
    delay(500);
    digitalWrite(LED_PIN,HIGH);
    receivedData = false;
  }
  yield();
}

void connectWiFi()
{
  byte ledStatus = LOW;

  // Set WiFi mode to station (as opposed to AP or AP_STA)
  WiFi.mode(WIFI_STA);

  // WiFI.begin([ssid], [passkey]) initiates a WiFI connection
  // to the stated [ssid], using the [passkey] as a WPA, WPA2,
  // or WEP passphrase.
  WiFi.begin(WiFiSSID, WiFiPSK);

  // Use the WiFi.status() function to check if the ESP8266
  // is connected to a WiFi network.
  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink the LED
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;

    // Delays allow the ESP8266 to perform critical tasks
    // defined outside of the sketch. These tasks include
    // setting up, and maintaining, a WiFi connection.
    delay(100);
    // Potentially infinite loops are generally dangerous.
    // Add delays -- allowing the processor to perform other
    // tasks -- wherever possible.
  }
}

void initHardware()
{
  Serial.begin(9600);
  pinMode(DIGITAL_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  // Don't need to set ANALOG_PIN as input, 
  // that's all it can be.
}

int postToPhant()
{
  // LED turns on when we enter, it'll go off when we 
  // successfully post.
  digitalWrite(LED_PIN, HIGH);

  // Declare an object from the Phant library - phant
  Phant phant(PhantHost, PublicKey, PrivateKey);

  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "Thing-":
//  uint8_t mac[WL_MAC_ADDR_LENGTH];
//  WiFi.macAddress(mac);
//  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
//                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
//  macID.toUpperCase();
//  String postedID = "Thing-" + macID;

  // Add the four field/value pairs defined by our stream:
  phant.add("temp", value);
//  phant.add("analog", analogRead(ANALOG_PIN));
//  phant.add("digital", digitalRead(DIGITAL_PIN));
//  phant.add("time", millis());

  // Now connect to data.sparkfun.com, and post our data:
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(PhantHost, httpPort)) 
  {
    // If we fail to connect, return 0.
    return 0;
  }
  // If we successfully connected, print our Phant post:
  client.print(phant.post());

  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    //Serial.print(line); // Trying to avoid using serial
  }

  // Before we exit, turn the LED off.
  digitalWrite(LED_PIN, LOW);

  return 1; // Return success
}

