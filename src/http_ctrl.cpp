#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "http_ctrl.h"
#include "led_ctrl.h"
#include <ArduinoJson.h>

#include "appcfg.h"

#define SUPABASE_URI "https://" SUPABASE_PROJECT_REF ".supabase.co/rest/v1/lamp_groups?id=eq." LAMP_GROUP

typedef struct {
  String data;
  int code;
} http_resp_t;

http_resp_t http_get_supabase();

rgb_t led_color {
  r: 255,
  g: 255,
  b: 255
};

bool read_color_supabase() {
  http_resp_t resp = http_get_supabase();

  const int capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(3);
  StaticJsonDocument<capacity> doc;
  DeserializationError err = deserializeJson(doc, resp.data);

  if (err != DeserializationError::Ok) {
    Serial.println("Failed to deserialize JSON: ");
    Serial.println(err.c_str());
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

  Serial.print("Set RGB to: ");
  Serial.print(led_color.r);
  Serial.print(", ");
  Serial.print(led_color.g);
  Serial.print(", ");
  Serial.println(led_color.b);
}

// Uses supabase postgrest API to get latest color
http_resp_t http_get_supabase() {
  WiFiClient client;
  HTTPClient http;
    
  http.begin(client, SUPABASE_URI);
  http.addHeader("Authorization", "Bearer " SUPABASE_API_KEY);
  http.addHeader("apikey", SUPABASE_API_KEY);
  
  // Send HTTP GET request
  int resp_code = http.GET();
  String resp_data; 
  
  if (resp_code>0) {
    resp_data = http.getString();
  }
  // Free resources
  http.end();

  return http_resp_t {
    data: resp_data,
    code: resp_code
  };
}

bool http_update_supabase() {
  char* hex_str = "#000000";
  itoa(
    led_color.r << 16 + led_color.g << 8 + led_color.b,
    &hex_str[1], 
    16
  );

  WiFiClient client;
  HTTPClient http;
    
  http.begin(client, SUPABASE_URI);
  http.addHeader("Authorization", "Bearer " SUPABASE_API_KEY);
  http.addHeader("apikey", SUPABASE_API_KEY);

  String payload = "{ \"color\": \"" + String(hex_str) + "\"}";

  // Send HTTP GET request
  int resp_code = http.PATCH(payload);
  
  bool err = resp_code != 200;
  if (err) {
    Serial.println("Failed to update led color on Supabase: ");
    Serial.println(http.getString());
  }

  // Free resources
  http.end();

  return err;

}
