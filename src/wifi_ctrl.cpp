#include <Arduino.h>
#include "BluetoothSerial.h"
#include <WiFi.h>
#include <Preferences.h>
#include "wifi_ctrl.h"
#include "bt_ctrl.h"

long wifi_timeout = 10000;

extern BluetoothSerial SerialBT;
extern Preferences preferences;

const char* pref_ssid = "";
const char* pref_pass = "";

bool init_wifi();

void wifi_setup() {
  init_wifi();
}

bool init_wifi()
{
  bool auto_reconnect = WiFi.getAutoReconnect();
  Serial.println("WiFi Reconnect" + auto_reconnect ? "enabled" : "disabled");

  String temp_pref_ssid = preferences.getString("wifi_ssid");
  String temp_pref_pass = preferences.getString("wifi_pass");
  pref_ssid = temp_pref_ssid.c_str();
  pref_pass = temp_pref_pass.c_str();

  Serial.println(pref_ssid);
  Serial.println(pref_pass);

  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);

  long start_wifi_millis = millis();
  WiFi.begin(pref_ssid, pref_pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - start_wifi_millis > wifi_timeout) {
      WiFi.disconnect(true, true);
      return false;
    }
  }
  return true;
}

bool wifi_test_connection() {
  bool success = init_wifi();
  if (success) {
    String connected_string = "ESP32 IP: ";
    connected_string = connected_string + WiFi.localIP().toString();
    SerialBT.println(connected_string);
    Serial.println(connected_string);
  } else {
    SerialBT.println("Failed to connect to wifi.");
    Serial.println("Failed to connect to wifi.");
  }

  WiFi.disconnect(true, true);

  return success;
}

void wifi_disconnect() {
  WiFi.disconnect(true, true);
}
