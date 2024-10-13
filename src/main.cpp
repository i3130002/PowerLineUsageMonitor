#include <Arduino.h>

#define ADAFRUIT_IO_USERNAME = "USERNAME"
const char *ADAFRUIT_IO_KEY = "KEY";
#define LED D0 

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char *ssid = "NAME";
const char *password = "PASSWORD";

String serverName = "http://io.adafruit.com/api/v2/i3130002/feeds/poweline-kw-slash-h/data";

const int analogInPin = A0; // ESP8266 Analog Pin ADC0 = A0

int sensorValue = 0; // value read from the pot
int kwh = 0;         // value read from the pot
unsigned long lastMillis;
WiFiServer server(23);
WiFiClient client;
bool fallingEdge = true;

void setup()
{
  // initialize serial communication at 115200
  Serial.begin(115200);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  lastMillis = millis();
}

void publishData(int newSensorValue)
{

  kwh = kwh + 1;

  if (kwh % 2 != 0)
    return;

  unsigned long newMillis = millis();
  float seconds = ((float)(newMillis - lastMillis)) / 1000;
  seconds = 2 * 3600.0 / seconds;
  WiFiClient client;
  HTTPClient http;

  http.begin(client, serverName);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-AIO-Key", ADAFRUIT_IO_KEY);
  String data = "{\"value\":";
  data = data + String(seconds) + "}";

  int httpCode = http.POST(data);
  Serial.print("HTTP Response code: ");
  Serial.println(httpCode);
  String payload = http.getString();
  Serial.println(payload); //Print request response payload
  // Free resources
  http.end();
  lastMillis = newMillis;
}

void triggerChecker(int newSensorValue)
{
  if (newSensorValue > sensorValue + 50 && fallingEdge)
  {
    digitalWrite(LED, LOW);
    fallingEdge = false;
    // print the readings in the Serial Monitor
    publishData(newSensorValue);
    Serial.print(String(newSensorValue));
    Serial.print('\n');
    Serial.flush();

    delay(100);
    digitalWrite(LED, HIGH);
  }
  if(newSensorValue<sensorValue+50 && !fallingEdge){
    fallingEdge = true;
  }
}

void loop()
{
  // read the analog in value
  int newSensorValue = analogRead(analogInPin);
  
  triggerChecker(newSensorValue);
  sensorValue = newSensorValue;
  delay(70);
}
