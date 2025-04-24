#ifndef PLATFORM_H
#define PLATFORM_H

#include <cstdint>

typedef struct {
  int32_t x;
  int32_t y; // top-left
  int32_t w;
  int32_t h;
} Platform;

extern Platform platforms1[];
extern Platform platforms2[];
//extern const int numPlatforms;

void drawPlatforms(int);

#endif