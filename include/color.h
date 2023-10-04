#ifndef COLOR_H
#define COLOR_H
#include <Arduino.h>

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} rgb_t;

#define RED(x) rgb_t { x, 0, 0}
#define GREEN(x) rgb_t { 0, x, 0}
#define BLUE(x) rgb_t { 0, 0, x}
#define WHITE(x) rgb_t { x, x, x}

#endif // COLOR_H
