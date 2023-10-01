#include <Arduino.h>
#include "esp_http_server.h"
#include "esp_camera.h"
#include "BluetoothSerial.h"
#include "wifi_ctrl.h"
#include "bt_ctrl.h"
#include "led_ctrl.h"
#include "serial_cmds.h"

BluetoothSerial SerialBT;

httpd_handle_t camera_httpd = NULL;

void setup()
{
  Serial.begin(115200);
  Serial.println("Booting...");

  wifi_setup();
  led_setup();
  bt_setup();
}

void loop()
{
  handle_serial_cmd();
  bt_loop();
  led_loop();
  wifi_loop();
}
