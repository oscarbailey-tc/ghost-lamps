#include <Arduino.h>
#include <BluetoothSerial.h>
#include <WiFi.h>
#include "wifi_ctrl.h"
#include "bt_ctrl.h"
#include "appcfg.h"
#include <Preferences.h>

extern bool config_mode_done;

extern BluetoothSerial SerialBT;

extern Preferences preferences;

int cfg_index = -1;
String cfg_val;

unsigned int len_cfg_options = 4;
String cfg_options[] = {
  "wifi",
  "led_brightness",
  "lamp_group",
  "bluetooth_name",
};

typedef enum {
  NO_CFG,
  WAIT_CONN,
  PRINT_CFG_OPTIONS,
  WAIT_CFG_OPTION,
  WIFI_SETUP,
  CFG_OPTION_SELECTED,
  WAIT_CFG_VAL,
  CFG_VAL_SELECTED,
  END_CFG_MODE,
} bt_state_t;
bt_state_t bt_state = NO_CFG;

enum wifi_setup_stages { 
  NONE,
  SCAN_START,
  SCAN_COMPLETE,
  SSID_ENTERED,
  WAIT_PASS,
  PASS_ENTERED,
  WAIT_CONNECT,
  LOGIN_FAILED,
  CONNECTED
};
enum wifi_setup_stages wifi_stage = NONE;

String client_wifi_ssid;
String client_wifi_password;
String connected_string;
String ssids_array[50];
String network_string;

void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    bt_state = PRINT_CFG_OPTIONS;
  }

  if (event == ESP_SPP_DATA_IND_EVT) {
    switch (bt_state) {
      case WAIT_CFG_OPTION: {
        cfg_index = SerialBT.readString().toInt();
        if (cfg_index == len_cfg_options) {
          bt_state = END_CFG_MODE;
        } else if (cfg_index == 0) {
          // WiFi
          bt_state = WIFI_SETUP;
          wifi_stage = SCAN_START;
        } else if (cfg_index < 0 || cfg_index > len_cfg_options) {
          SerialBT.println("Invalid cfg option.");
          bt_state = PRINT_CFG_OPTIONS;
        } else {
          bt_state = CFG_OPTION_SELECTED;
        }
        break;
      }
      case WAIT_CFG_VAL: {
        cfg_val = SerialBT.readString();
        bt_state = CFG_VAL_SELECTED;
        break;
      }
      case WIFI_SETUP: {
        if (wifi_stage == SCAN_COMPLETE) { // data from phone is SSID
          int client_wifi_ssid_id = SerialBT.readString().toInt();
          client_wifi_ssid = ssids_array[client_wifi_ssid_id];
          wifi_stage = SSID_ENTERED;
        }

        if (wifi_stage == WAIT_PASS) { // data from phone is password
          client_wifi_password = SerialBT.readString();
          client_wifi_password.trim();
          wifi_stage = PASS_ENTERED;
        }
        break;
      }
    }
  }
}

void bt_init() {
  String bt_name = preferences.getString("bluetooth_name", String(BLUETOOTH_NAME));
  SerialBT.begin(bt_name);
  SerialBT.register_callback(callback);
}

void bt_disconnect() {
  bt_state = NO_CFG;
  delay(1000);
  Serial.println("BT stopping");
  SerialBT.println("Bluetooth disconnecting...");
  delay(1000);
  SerialBT.flush();
  SerialBT.disconnect();
  SerialBT.end();
  Serial.println("BT stopped");
  delay(1000);
}

void scan_wifi_networks()
{
  WiFi.mode(WIFI_STA);
  // WiFi.scanNetworks will return the number of networks found
  int n =  WiFi.scanNetworks();
  if (n == 0) {
    SerialBT.println("no networks found");
  } else {
    SerialBT.println();
    SerialBT.print(n);
    SerialBT.println(" networks found");
    delay(1000);
    for (int i = 0; i < n; ++i) {
      ssids_array[i + 1] = WiFi.SSID(i);
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.println(ssids_array[i + 1]);
      network_string = i + 1;
      network_string = network_string + ": " + WiFi.SSID(i) + " (Strength:" + WiFi.RSSI(i) + ")";
      SerialBT.println(network_string);
    }
    wifi_stage = SCAN_COMPLETE;
  }
}

void wifi_setup_loop() {
  switch (wifi_stage)
  {
    case SCAN_START:
      SerialBT.println("Scanning Wi-Fi networks");
      Serial.println("Scanning Wi-Fi networks");
      scan_wifi_networks();
      SerialBT.println("Please enter the number for your Wi-Fi");
      wifi_stage = SCAN_COMPLETE;
      break;

    case SSID_ENTERED:
      SerialBT.println("Please enter your Wi-Fi password");
      Serial.println("Please enter your Wi-Fi password");
      wifi_stage = WAIT_PASS;
      break;

    case PASS_ENTERED:
      SerialBT.println("Please wait for Wi-Fi connection...");
      Serial.println("Please wait for Wi_Fi connection...");
      wifi_stage = WAIT_CONNECT;
      preferences.putString("wifi_ssid", client_wifi_ssid);
      preferences.putString("wifi_pass", client_wifi_password);
      wifi_test_connection();
      wifi_stage = NONE;
      bt_state = PRINT_CFG_OPTIONS;
      break;
  }
}

void bt_loop() {
  switch (bt_state) {
    case PRINT_CFG_OPTIONS: {
      SerialBT.println("Config options: ");
      for (int i=0; i<len_cfg_options; i++) {
        SerialBT.println(String(i) + ": " + cfg_options[i]);
      }
      SerialBT.println(String(len_cfg_options) + ": Exit config mode");
      SerialBT.println("Select the config index you want to change: ");
      bt_state = WAIT_CFG_OPTION;
      break;
    }
    case WIFI_SETUP: {
      wifi_setup_loop();
      break;
    }
    case CFG_OPTION_SELECTED: {
      SerialBT.println("Selected  " + String(cfg_options[cfg_index]) + ". Current Value: ");
      SerialBT.println(preferences.getString(cfg_options[cfg_index].c_str()));
      SerialBT.println("Please enter a new value: ");
      bt_state = WAIT_CFG_VAL;
      break;
    }
    case CFG_VAL_SELECTED: {
      if (cfg_index == 1) {
        // led_brightness
        int brightness = cfg_val.toInt();
        SerialBT.print("LED Brightness as int: ");
        SerialBT.println(brightness);
      }
      SerialBT.println("Setting " + String(cfg_options[cfg_index]) + " to " + cfg_val);
      preferences.putString(cfg_options[cfg_index].c_str(), cfg_val);
      bt_state = PRINT_CFG_OPTIONS;
      break;
    }
    case END_CFG_MODE: {
      config_mode_done = true;
      bt_state = NO_CFG;
      break;
    }
  }
}
