#include <Arduino.h>
#include "esp_http_server.h"
#include "esp_camera.h"
#include "BluetoothSerial.h"
#include "wifi_ctrl.h"
#include "bt_ctrl.h"
#include "led_ctrl.h"
#include "http_ctrl.h"
#include <Preferences.h>

// Resources used:
// https://robotzero.one/esp32-wi-fi-connection-bluetooth/
// https://docs.espressif.com/projects/arduino-esp32/en/latest/

BluetoothSerial SerialBT;
Preferences preferences;

httpd_handle_t camera_httpd = NULL;

#define TOUCH_PIN T4

int threshold = 60;

bool config_mode_triggered = false;
bool config_mode_done = false;
bool config_mode = false;

void start_config_mode() {
  wifi_disconnect();
  bt_init();
  led_reset();
  config_mode = true;
  Serial.println("Config mode enabled.");
}

void stop_config_mode() {
  bt_disconnect();
  wifi_setup();
  led_read_brightness();
  config_mode = false;
  Serial.println("Config mode disabled.");
}

void touch_trigger(){
  static bool must_be_lower = true;
  static int touch_start;

  if (touchRead(T4) > threshold) {
    touch_start = millis();
  } else {
    int touch_duration = millis() - touch_start;

    if (touch_duration > 5000) {
      // Enter config mode
      config_mode_triggered = true;
    }

    if (touch_duration < 1000) {
      if (config_mode) {
        config_mode_done = true;
      } else if (!led_changing()) {
        set_and_upload_random_led_color();
      }
    }
  }
  must_be_lower = !must_be_lower;
  touchInterruptSetThresholdDirection(must_be_lower);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Booting...");
  preferences.begin("all", false);

  wifi_setup();
  led_setup();

  touchAttachInterrupt(TOUCH_PIN, touch_trigger, threshold);
}

void loop()
{
  static int led_next = millis();
  static int led_on = false;

  if (config_mode_triggered) {
    start_config_mode();
    config_mode_triggered = false;
  }

  if (config_mode_done) {
    stop_config_mode();
    config_mode_done = false;
  }

  if (config_mode) {
    // Flash LED
    if (led_next < millis()) {
      led_on = !led_on; 
      uint8_t brightness = 255 * led_on;
      write_led(BLUE(brightness), BLUE(brightness));
      led_next += 1000;
    }

    bt_loop();
  } else {
    bt_loop();
    led_loop();
    http_loop();
  }
}
