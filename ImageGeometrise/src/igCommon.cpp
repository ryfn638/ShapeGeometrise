#include "igCommon.h"
#include <algorithm>
#include <math.h>
#include <iostream>

igColour::igColour(uint8_t r, uint8_t g, uint8_t b, uint8_t opacity)
  : red(r),
    green(g),
    blue(b),
    opacity(opacity)
{
}

igVec2::igVec2(int64_t x, int64_t y)
  : x(x),
    y(y)
{
}

bool igVec2::operator==(igVec2&& rhs) const
{
  return x == rhs.x && y == rhs.y;
}

bool igVec2::operator==(const igVec2& rhs) const
{
  return x == rhs.x && y == rhs.y;
}

bool igVec2::operator!=(igVec2&& rhs) const
{
  return !(*this == rhs);
}

bool igVec2::operator!=(const igVec2& rhs) const
{
  return !(*this == rhs);
}

igVec2 igVec2::operator+(igVec2&& rhs)
{
  return igVec2(x + rhs.x, y + rhs.y);
}

igVec2 igVec2::operator+(const igVec2& rhs)
{
  return igVec2(x + rhs.x, y + rhs.y);
}

igVec2 &igVec2::operator+=(igVec2&& rhs)
{
  x += rhs.x;
  y += rhs.y;
  return *this;
}

igVec2 &igVec2::operator+=(const igVec2& rhs)
{
  x += rhs.x;
  y += rhs.y;
  return *this;
}

igVec2 igVec2::operator-(igVec2&& rhs)
{
  return igVec2(x - rhs.x, y - rhs.y);
}

igVec2 igVec2::operator-(const igVec2& rhs)
{
  return igVec2(x - rhs.x, y - rhs.y);
}

igVec2 &igVec2::operator-=(igVec2&& rhs)
{
  x -= rhs.x;
  y -= rhs.y;
  return *this;
}

igVec2 &igVec2::operator-=(const igVec2& rhs)
{
  x -= rhs.x;
  y -= rhs.y;
  return *this;
}
