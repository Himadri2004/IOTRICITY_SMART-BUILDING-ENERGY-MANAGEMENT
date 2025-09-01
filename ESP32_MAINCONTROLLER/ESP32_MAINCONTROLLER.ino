#include <WiFi.h>

#include <SinricPro.h>

#include <SinricProSwitch.h>

#include <SinricProTemperaturesensor.h>

#include "DHT.h"   // if using DHT22/DHT11 for temp+humidity


/********************************

   Credentials

 ********************************/

#define WIFI_SSID       "YOUR_WIFI_SSID"

#define WIFI_PASS       "YOUR_WIFI_PASSWORD"


#define APP_KEY         "YOUR_APP_KEY"       // from SinricPro

#define APP_SECRET      "YOUR_APP_SECRET"    // from SinricPro


/********************************

   Device IDs

 ********************************/

#define SWITCH_LIGHT_ID   "YOUR_LIGHT_DEVICE_ID"

#define SWITCH_FAN_ID     "YOUR_FAN_DEVICE_ID"

#define SWITCH_AC_ID      "YOUR_AC_DEVICE_ID"

#define SWITCH_HEATER_ID  "YOUR_HEATER_DEVICE_ID"

#define TEMP_SENSOR_ID    "YOUR_TEMP_SENSOR_DEVICE_ID"


/********************************

   DHT Sensor

 ********************************/

#define DHTPIN  4

#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);


/********************************

   State Variables

 ********************************/

bool lightState = false;

bool fanState = false;

bool acState = false;

bool heaterState = false;


/********************************

   Callbacks

 ********************************/

bool onPowerState(const String &deviceId, bool &state) {

  if (deviceId == SWITCH_LIGHT_ID) {

    lightState = state;

    Serial.printf("Light turned %s\r\n", state ? "ON" : "OFF");

  } else if (deviceId == SWITCH_FAN_ID) {

    fanState = state;

    Serial.printf("Fan turned %s\r\n", state ? "ON" : "OFF");

  } else if (deviceId == SWITCH_AC_ID) {

    acState = state;

    Serial.printf("AC turned %s\r\n", state ? "ON" : "OFF");

  } else if (deviceId == SWITCH_HEATER_ID) {

    heaterState = state;

    Serial.printf("Heater turned %s\r\n", state ? "ON" : "OFF");

  }

  return true; // request handled properly

}


/********************************

   WiFi + SinricPro setup

 ********************************/

void setupWiFi() {

  Serial.printf("[WiFi] Connecting to %s", WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {

    delay(250);

    Serial.print(".");

  }

  Serial.printf("\r\n[WiFi] Connected. IP: %s\r\n", WiFi.localIP().toString().c_str());

}


void setupSinricPro() {

  // register switch devices

  SinricProSwitch &swLight  = (SinricProSwitch&)SinricPro[SWITCH_LIGHT_ID];

  SinricProSwitch &swFan    = (SinricProSwitch&)SinricPro[SWITCH_FAN_ID];

  SinricProSwitch &swAC     = (SinricProSwitch&)SinricPro[SWITCH_AC_ID];

  SinricProSwitch &swHeater = (SinricProSwitch&)SinricPro[SWITCH_HEATER_ID];


  swLight.onPowerState(onPowerState);

  swFan.onPowerState(onPowerState);

  swAC.onPowerState(onPowerState);

  swHeater.onPowerState(onPowerState);


  // init SinricPro

  SinricPro.onConnected([](){ Serial.println("Connected to SinricPro"); });

  SinricPro.onDisconnected([](){ Serial.println("Disconnected from SinricPro"); });


  SinricPro.begin(APP_KEY, APP_SECRET);

  SinricPro.restoreDeviceStates(true);

}


/********************************

   Automation Example

 ********************************/

void applyAutomation(float temperature, float humidity) {

  if (temperature > 30.0 && !fanState) {

    fanState = true;

    ((SinricProSwitch&)SinricPro[SWITCH_FAN_ID]).sendPowerStateEvent(true);

    Serial.println("Automation: Fan turned ON (temp > 30°C)");

  }

  if (temperature < 20.0 && !heaterState) {

    heaterState = true;

    ((SinricProSwitch&)SinricPro[SWITCH_HEATER_ID]).sendPowerStateEvent(true);

    Serial.println("Automation: Heater turned ON (temp < 20°C)");

  }

}


/********************************

   Arduino setup + loop

 ********************************/

unsigned long lastTempUpdate = 0;

const unsigned long TEMP_INTERVAL = 60000; // send temp every 60s


void setup() {

  Serial.begin(115200);

  dht.begin();

  setupWiFi();

  setupSinricPro();

}


void loop() {

  SinricPro.handle();


  unsigned long now = millis();

  if (now - lastTempUpdate > TEMP_INTERVAL) {

    lastTempUpdate = now;


    float temperature = dht.readTemperature();

    float humidity = dht.readHumidity();


    if (!isnan(temperature) && !isnan(humidity)) {

      Serial.printf("Temp: %.1f °C | Humidity: %.1f %%\r\n", temperature, humidity);

      ((SinricProTemperaturesensor&)SinricPro[TEMP_SENSOR_ID])

        .sendTemperatureEvent(temperature, humidity);


      applyAutomation(temperature, humidity);

    } else {

      Serial.println("Failed to read from DHT sensor!");

    }

  }

}
