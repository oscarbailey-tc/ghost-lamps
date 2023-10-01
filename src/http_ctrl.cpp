#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "http_ctrl.h"
#include "led_ctrl.h"
#include <ArduinoJson.h>

typedef struct {
  String data;
  int code;
} http_resp_t;

http_resp_t http_get(const char* uri);

rgb_t led_color;

bool read_color() {
  http_resp_t resp = http_get(HTTP_READ);

  if (resp.code != 200) {
    Serial.println("Failed to get color from:");
    Serial.println(HTTP_READ);
    Serial.println("resp code: " + resp.code);
    Serial.println("resp data: " + resp.data);
    return false;
  }

  // Parse string as 3 bytes
  uint8_t colors[3];
  size_t str_len = resp.data.length() + 1;
  char *resp_str = (char*) malloc(str_len);
  char *resp_str_to_free = resp_str;
  memcpy(resp_str, resp.data.c_str(), str_len);

  size_t i = 0;
  char *token = strtok(resp_str, ",");
  do {
    colors[i++] = atoi(token);
    token = strtok(NULL, ",");
  } while (token != NULL && i < 3);
  free(resp_str_to_free);

  if (i < 3) {
    Serial.println("Failed to parse colors.");
    Serial.println("resp data: " + String(resp_str));
    return false;
  }

  // Set color
  led_color.r = colors[0];
  led_color.g = colors[1];
  led_color.b = colors[2];

  return true;

}

http_resp_t http_get(const char* uri) {
  WiFiClient client;
  HTTPClient http;
    
  // Your Domain name with URL path or IP address with path
  http.begin(client, uri);
  
  // If you need Node-RED/server authentication, insert user and password below
  //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
  
  // Send HTTP POST request
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
