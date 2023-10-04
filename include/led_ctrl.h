#ifndef LED_CTRL_H
#define LED_CTRL_H

#include <Arduino.h>
#include "color.h"

void led_setup();
void led_loop();
bool led_changing();
rgb_t hue_to_rgb(uint8_t hue, uint8_t brightness);

#endif // LED_CTRL_H
