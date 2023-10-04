#ifndef LED_CTRL_H
#define LED_CTRL_H

#include <Arduino.h>
#include "color.h"

void led_setup();
void led_loop();
void led_reset();
void led_read_brightness();

bool led_changing();
rgb_t hue_to_rgb(uint8_t hue, uint8_t brightness);
void write_led(rgb_t color, rgb_t target);

#endif // LED_CTRL_H
