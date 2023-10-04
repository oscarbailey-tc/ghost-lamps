#include <Arduino.h>
#include "esp_http_server.h"
#include "esp_camera.h"
#include "BluetoothSerial.h"
#include "wifi_ctrl.h"
#include "bt_ctrl.h"
#include "led_ctrl.h"
#include "http_ctrl.h"
#include "serial_cmds.h"

// Resources used:
// https://robotzero.one/esp32-wi-fi-connection-bluetooth/
// https://docs.espressif.com/projects/arduino-esp32/en/latest/

BluetoothSerial SerialBT;

httpd_handle_t camera_httpd = NULL;

#define TOUCH_PIN T4

int threshold = 60;

void touch_trigger(){
  static bool must_be_lower = true;
  static int touch_start;

  if (touchRead(T4) > threshold) {
    touch_start = millis();
  } else {
    int touch_duration = millis() - touch_start;
    Serial.print("Touch duration: ");
    Serial.println(touch_duration);

    if (touch_duration < 1000 && !led_changing()) {

      // Choose either:
      // - White LED of different brightness
      // - Coloured LED (just 1)
      // - Hue
      // - Random LEDs (all)

      switch (random(4)) {
        case 0: {
          uint8_t brightness = 50 + random(200);
          rgb_t color = rgb_t {
            brightness, brightness, brightness
          };
          upload_color(color);
          break;
        }
        case 1: {
          uint8_t brightness = 50 + random(200);
          switch (random(3)) {
            case 0:
              upload_color(rgb_t {brightness, 0, 0});
              break;
            case 1:
              upload_color(rgb_t {0, brightness, 0});
              break;
            default:
              upload_color(rgb_t {0, 0, brightness});
              break;
          }
          break;
        }
        case 2: {
          rgb_t color = hue_to_rgb(random(255), 255);
          upload_color(color);
          break;
        }
        default: {
          uint8_t r = random(245) + 10;
          uint8_t g = random(245) + 10;
          uint8_t b = random(245) + 10;
          rgb_t color = rgb_t { r, g, b};
          upload_color(color);
          break;
        }
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

  wifi_setup();
  led_setup();
  bt_setup();

  touchAttachInterrupt(TOUCH_PIN, touch_trigger, threshold);
}

void loop()
{
  handle_serial_cmd();
  bt_loop();
  led_loop();
  http_loop();
  wifi_loop();
}
