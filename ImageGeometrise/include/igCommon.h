#ifndef igCommon_h
#define igCommon_h

#include <cstdint>

struct igColour
{
  igColour() = default;
  igColour(uint8_t r, uint8_t b, uint8_t g, uint8_t opacity);

  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;
  uint8_t opacity = 255;
};

struct igVec2
{
  igVec2(int64_t x, int64_t y);
  int64_t x;
  int64_t y;
};

#endif
