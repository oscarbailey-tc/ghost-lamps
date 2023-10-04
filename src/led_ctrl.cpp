#include <Arduino.h>
#include "led_ctrl.h"
#include "http_ctrl.h"
#include <Preferences.h>

#define LEDC_BASE_FREQ 12000
#define LED_PIN_R 2
#define LED_PIN_G 4
#define LED_PIN_B 5
#define LED_PIN_W 16

#define LEDC_CHANNEL_W 0
#define LEDC_CHANNEL_R 1
#define LEDC_CHANNEL_G 2
#define LEDC_CHANNEL_B 3

extern rgb_t led_color;

int fade_direction = -1;
rgb_t old_led_color = led_color;
rgb_t cur_led_color;

extern Preferences preferences;
int led_brightness = 10;

#define B(x) ((uint8_t) ((int) x * 10) / 10)

rgb_t hue_to_rgb(uint8_t hue, uint8_t brightness);

bool led_changing() {
  return memcmp(&led_color, &cur_led_color, sizeof led_color);
}

void led_reset() {
  led_color = WHITE(255);
  cur_led_color = WHITE(0);
  old_led_color = led_color;
  fade_direction = 1;
}

void led_read_brightness() {
  String brightness = preferences.getString("led_brightness", "10");
  led_brightness = brightness.toInt();
}

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

  led_read_brightness();
}

void write_led(rgb_t color, rgb_t target) {
#ifdef DEBUG
  Serial.print("LED: ");
  Serial.print(color.r);
  Serial.print(", ");
  Serial.print(color.g);
  Serial.print(", ");
  Serial.println(color.b);
#endif

  // If target color is brightness only
  if (target.r == target.g && target.g == target.b) {
    ledcWrite(LEDC_CHANNEL_W, B(color.r));
    ledcWrite(LEDC_CHANNEL_R, 0);
    ledcWrite(LEDC_CHANNEL_G, 0);
    ledcWrite(LEDC_CHANNEL_B, 0);
    return;
  }

  ledcWrite(LEDC_CHANNEL_W, 0);
  ledcWrite(LEDC_CHANNEL_R, B(color.r));
  ledcWrite(LEDC_CHANNEL_G, B(color.g));
  ledcWrite(LEDC_CHANNEL_B, B(color.b));
}

void led_loop() {
  static long next = millis();

  if (millis() < next) {
    return;
  }
  next = millis() + 50;

  if (memcmp(&led_color, &cur_led_color, sizeof led_color) == 0) {
    old_led_color = led_color;
    return;
  }

  // Fade into next led_color
  if (memcmp(&old_led_color, &led_color, sizeof led_color) != 0) {
    if (fade_direction > 0) {
      Serial.print("Fading: ");
      Serial.print(old_led_color.r);
      Serial.print(", ");
      Serial.print(old_led_color.g);
      Serial.print(", ");
      Serial.print(old_led_color.b);
      Serial.print(" -> ");
      Serial.print(led_color.r);
      Serial.print(", ");
      Serial.print(led_color.g);
      Serial.print(", ");
      Serial.println(led_color.b);
    }
    fade_direction = -1;
  }

  // If we reach 0, start fading into the new color
  if (cur_led_color.r == 0 && cur_led_color.g == 0 && cur_led_color.b == 0) {
    fade_direction = 1;
    old_led_color = led_color;
  }

  int new_r = (int) cur_led_color.r + (fade_direction * 5);
  int new_g = (int) cur_led_color.g + (fade_direction * 5);
  int new_b = (int) cur_led_color.b + (fade_direction * 5);

  // Saturating sub/add
  if (fade_direction < 0) {
    if (new_r < 0) {
      new_r = 0;
    }
    if (new_g < 0) {
      new_g = 0;
    }
    if (new_b < 0) {
      new_b = 0;
    }
  } else {
    if (new_r > led_color.r) {
      new_r = led_color.r;
    }
    if (new_g > led_color.g) {
      new_g = led_color.g;
    }
    if (new_b > led_color.b) {
      new_b = led_color.b;
    }
  }

  cur_led_color.r = new_r;
  cur_led_color.g = new_g;
  cur_led_color.b = new_b;

  write_led(cur_led_color, fade_direction < 0 ? old_led_color : led_color);
}

// Courtesy http://www.instructables.com/id/How-to-Use-an-RGB-LED/?ALLSTEPS
// function to convert a color to its Red, Green, and Blue components.
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
