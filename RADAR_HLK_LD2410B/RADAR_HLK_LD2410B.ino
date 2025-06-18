#define BLYNK_TEMPLATE_ID "TMPL6zvf9FWww"
#define BLYNK_TEMPLATE_NAME "RADAR HLK LD 2410"
#define BLYNK_AUTH_TOKEN "LjSkrMdEQd2c9hR5UkdYB-vmfpLk6Wr1"

#define MONITOR_SERIAL Serial
#define RADAR_SERIAL Serial1
#define RADAR_TX_PIN 32
#define RADAR_RX_PIN 33

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include <ld2410.h>

char auth [] = BLYNK_AUTH_TOKEN ;
char ssid[] = " NY " ;
char pass[] = " 01234567 " ;

ld2410 radar;

uint32_t lastReading = 0;
bool radarConnected = false;

void setup(void)
{
  MONITOR_SERIAL.begin(115200);
  RADAR_SERIAL.begin(256000, SERIAL_8N1, RADAR_TX_PIN, RADAR_RX_PIN);
  delay(10);
  MONITOR_SERIAL.print(F("\nConnect LD2410 radar TX to GPIO:"));
  MONITOR_SERIAL.println(RADAR_TX_PIN);
  MONITOR_SERIAL.print(F("Connect LD2410 radar RX to GPIO:"));
  MONITOR_SERIAL.println(RADAR_RX_PIN);
  MONITOR_SERIAL.print(F("LD2410 radar sensor initialising: "));
  if (radar.begin(RADAR_SERIAL))
  {
    MONITOR_SERIAL.println(F("OK"));
    MONITOR_SERIAL.print(F("LD2410 firmware version: "));
    MONITOR_SERIAL.print(radar.firmware_major_version);
    MONITOR_SERIAL.print('.');
    MONITOR_SERIAL.print(radar.firmware_minor_version);
    MONITOR_SERIAL.print('.');
    MONITOR_SERIAL.println(radar.firmware_bugfix_version, HEX);
  }
  else
  {
    MONITOR_SERIAL.println(F("not connected"));
  }
}

void loop()
{
  radar.read();
  if (radar.isConnected() && millis() - lastReading > 10) //Report every 1000ms
  {
    lastReading = millis();
    if (radar.presenceDetected())
    {
      if (radar.stationaryTargetDetected())
      {
        Serial.print(F("Stationary target: "));
        Serial.print(radar.stationaryTargetDistance());
        Serial.print(F("cm energy:"));
        Serial.print(radar.stationaryTargetEnergy());
        Serial.print(' ');
      }
      if (radar.movingTargetDetected())
      {
        Serial.print(F("Moving target: "));
        Serial.print(radar.movingTargetDistance());
        Serial.print(F("cm energy:"));
        Serial.print(radar.movingTargetEnergy());
      }
      Serial.println();
    }
    else
    {
      Serial.println(F("No target"));
    }
  }
  float a = radar.stationaryTargetDistance();
  float b = radar.stationaryTargetEnergy();
  float c = radar.movingTargetDistance();
  float d = radar.movingTargetEnergy();
  Blynk.virtualWrite(V0, a);
  Blynk.virtualWrite(V1, b);
  Blynk.virtualWrite(V2, c);
  Blynk.virtualWrite(V3, d);
}
