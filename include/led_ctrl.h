#include <Arduino.h>

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} rgb_t;

void led_setup();
void led_loop();
