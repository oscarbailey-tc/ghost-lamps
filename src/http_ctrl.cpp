#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "http_ctrl.h"
#include "led_ctrl.h"
#include <ArduinoJson.h>

#include "appcfg.h"

#define SUPABASE_URI "https://" SUPABASE_PROJECT_REF ".supabase.co/rest/v1/lamp_groups?id=eq." LAMP_GROUP

// #define DEBUG

typedef struct {
  String data;
  int code;
} http_resp_t;

http_resp_t http_get_supabase();
bool read_color_supabase();

rgb_t led_color {
  r: 255,
  g: 255,
  b: 255
};

bool push_color = false;

HTTPClient https;
WiFiClientSecure client;

void upload_color(rgb_t color) {
  push_color = true;
  led_color = color;
}

void http_setup() {
  https.setReuse(true);
}

void http_loop() {
  static long next = millis();

  if (millis() < next) {
    return;
  }

  if (led_changing()) {
    return;
  }

  // Read latest color from supabase
  if (push_color) {
    bool success = http_update_supabase();
    if (success) {
      push_color = false;
    }
  } else {
    read_color_supabase();
  }
  next = millis() + 2000;
}

bool read_color_supabase() {
  http_resp_t resp = http_get_supabase();
#ifdef DEBUG
  Serial.printf("Free Heap: %d\n", ESP.getFreeHeap());
#endif

  if (resp.code != 200) {
    Serial.print("HTTP GET request failed. Code: ");
    Serial.print(resp.code);
    Serial.print(" Data: ");
    Serial.println(resp.data);
    return false;
  }

  const int capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(3);
  StaticJsonDocument<capacity + 100> doc;
  DeserializationError err = deserializeJson(doc, resp.data);

  if (err != DeserializationError::Ok) {
    Serial.print("Failed to deserialize JSON: ");
    Serial.println(err.c_str());
    Serial.println("Data: ");
    Serial.println(resp.data);
    return false;
  }

  String hex_color = doc[0]["color"].as<String>();
  if (hex_color.isEmpty()) {
    Serial.println("Failed to get color from JSON: ");
    Serial.println(resp.data);
    return false;
  }

  hex_color.toLowerCase();
  for (int i=1; i<7; i++) {
    char c = hex_color[i];
    if (!((c >= '0' && c <= '9') || (c >= 'a' || c <= 'f'))) {
      Serial.print("Invalid hex code: ");
      Serial.println(hex_color);
    }
  }

  uint64_t hex_int = (uint64_t) strtoull(hex_color.substring(1).c_str(), 0, 16);

  led_color.r = hex_int >> 16;
  led_color.g = hex_int >> 8;
  led_color.b = hex_int;

  return true;
}

// Uses supabase postgrest API to get latest color
http_resp_t http_get_supabase() {
  client.setInsecure();

  bool success = false;
  int retry_count = 1;
  while (!success) {
    success = https.begin(client, SUPABASE_URI);
    if (!success) {
      Serial.print("HTTPS Failed to start, retry ");
      Serial.println(retry_count++);
    }
    delay(100);
  }

  https.addHeader("Authorization", "Bearer " SUPABASE_API_KEY);
  https.addHeader("apikey", SUPABASE_API_KEY);

  // Send HTTP GET request
  int resp_code = https.GET();
  String resp_data; 
  
  if (resp_code>0) {
    resp_data = https.getString();
  }

  https.end();

  return http_resp_t {
    data: resp_data,
    code: resp_code
  };
}

bool http_update_supabase() {
  unsigned int led_val = led_color.r;
  led_val <<= 8;
  led_val += led_color.g;
  led_val <<= 8;
  led_val += led_color.b;
  String hex_str = String(led_val, HEX);
  String pad = "000000";
  hex_str = "#" + pad.substring(0, 6 - hex_str.length()) + hex_str;

  client.setInsecure();

  bool success = false;
  int retry_count = 1;
  while (!success) {
    success = https.begin(client, SUPABASE_URI);
    if (!success) {
      Serial.print("HTTPS Failed to start, retry ");
      Serial.println(retry_count++);
    }
    delay(100);
  }

  String payload = "{ \"color\": \"" + hex_str + "\"}";

  https.addHeader("Authorization", "Bearer " SUPABASE_API_KEY);
  https.addHeader("apikey", SUPABASE_API_KEY);

  // Send HTTP GET request
  int resp_code = https.PATCH(payload);
  
  bool err = !(resp_code >= 200 && resp_code < 300);
  if (err) {
    Serial.println("Failed to update led color on Supabase: ");
    Serial.println(resp_code);
    Serial.println(https.getString());
  } else {
    Serial.print("Updated to new color: ");
    Serial.println(hex_str);
  }

  https.end();


  return !err;
}
