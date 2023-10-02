#include <Arduino.h>
#include "led_ctrl.h"
#include "http_ctrl.h"

#define LEDC_BASE_FREQ 12000
#define LED_PIN_R 2
#define LED_PIN_G 4
#define LED_PIN_B 5
#define LED_PIN_W 16

#define LEDC_CHANNEL_W 0
#define LEDC_CHANNEL_R 1
#define LEDC_CHANNEL_G 2
#define LEDC_CHANNEL_B 3

typedef enum {
  ON,
  OFF,
  FADE
} led_state_t;
led_state_t led_state = ON;

extern rgb_t led_color;

rgb_t hue_to_rgb(uint8_t hue, uint8_t brightness);

void led_setup() {
  // Setup timer and attach timer to a led pin
  ledcSetup(0, LEDC_BASE_FREQ, 8);
  ledcSetup(1, LEDC_BASE_FREQ, 8);
  ledcSetup(2, LEDC_BASE_FREQ, 8);
  ledcSetup(3, LEDC_BASE_FREQ, 8);

  ledcAttachPin(LED_PIN_W, LEDC_CHANNEL_W);
  ledcAttachPin(LED_PIN_R, LEDC_CHANNEL_R);
  ledcAttachPin(LED_PIN_G, LEDC_CHANNEL_G);
  ledcAttachPin(LED_PIN_B, LEDC_CHANNEL_B);
}

void write_led(rgb_t color) {
  Serial.print("LED: ");
  Serial.print(color.r);
  Serial.print(", ");
  Serial.print(color.g);
  Serial.print(", ");
  Serial.println(color.b);

  // If color is brightness only
  if (color.r == color.g && color.g == color.b) {
    ledcWrite(LEDC_CHANNEL_W, color.r);
    ledcWrite(LEDC_CHANNEL_R, 0);
    ledcWrite(LEDC_CHANNEL_G, 0);
    ledcWrite(LEDC_CHANNEL_B, 0);
    return;
  }

  ledcWrite(LEDC_CHANNEL_R, color.r);
  ledcWrite(LEDC_CHANNEL_G, color.g);
  ledcWrite(LEDC_CHANNEL_B, color.b);
}

void led_loop() {
  static long next = millis();
  static uint8_t hue = 0;

  if (millis() < next) {
    return;
  }

  // Read latest color from supabase
  bool success = read_color_supabase();
  if (success) {
    write_led(led_color);
  }
  next = millis() + 2000;
}

rgb_t hue_to_rgb(uint8_t hue, uint8_t brightness)
{
  uint16_t scaledHue = (hue * 6);
  uint8_t segment = scaledHue / 256; // segment 0 to 5 around the
                                          // color wheel
  uint16_t segmentOffset =
    scaledHue - (segment * 256); // position within the segment

  uint8_t complement = 0;
  uint16_t prev = (brightness * ( 255 -  segmentOffset)) / 256;
  uint16_t next = (brightness *  segmentOffset) / 256;

  //if(invert)
  //{
  //  brightness = 255 - brightness;
  //  complement = 255;
  //  prev = 255 - prev;
  //  next = 255 - next;
  //}
  rgb_t color;

  switch(segment ) {
    case 0:      // red
      color.r = brightness;
      color.g = next;
      color.b = complement;
      break;
    case 1:     // yellow
      color.r = prev;
      color.g = brightness;
      color.b = complement;
      break;
    case 2:     // green
      color.r = complement;
      color.g = brightness;
      color.b = next;
      break;
    case 3:    // cyan
      color.r = complement;
      color.g = prev;
      color.b = brightness;
      break;
    case 4:    // blue
      color.r = next;
      color.g = complement;
      color.b = brightness;
      break;
    case 5:      // magenta
    default:
      color.r = brightness;
      color.g = complement;
      color.b = prev;
      break;
  }
  return color;
}
