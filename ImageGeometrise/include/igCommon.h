#ifndef igCommon_h
#define igCommon_h

#include <cstdint>

struct igLAB
{
  float L, a, b;
};

struct igColour
{
  igColour() = default;
  igColour(uint8_t r, uint8_t g, uint8_t b, uint8_t opacity = 255);

  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;
  uint8_t opacity = 255;

  igLAB ConvertToLAB() const;

  // This colour drawn over background, using this colour's opacity
  igColour BlendOver(const igColour& background) const;
};

struct igVec2
{
  igVec2() = default;
  igVec2(int64_t x, int64_t y);
  int64_t x = 0;
  int64_t y = 0;

  bool operator==(igVec2 &&rhs) const;
  bool operator==(const igVec2 &rhs) const;

  bool operator!=(igVec2 &&rhs) const;
  bool operator!=(const igVec2 &rhs) const;

  igVec2 operator+(igVec2 &&rhs);
  igVec2 operator+(const igVec2 &rhs);

  igVec2 operator-(igVec2 &&rhs);
  igVec2 operator-(const igVec2 &rhs);

  igVec2 &operator+=(igVec2 &&rhs);
  igVec2 &operator+=(const igVec2 &rhs);

  igVec2 &operator-=(igVec2 &&rhs);
  igVec2 &operator-=(const igVec2 &rhs);
};

#endif
