#ifndef igCommon_h
#define igCommon_h

#include <cmath>
#include <cstdint>

// Callable from both CPU and CUDA code. Anything a kernel calls needs this, and its body in a header.
#ifdef __CUDACC__
#define IG_HD __host__ __device__
#else
#define IG_HD
#endif

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

  IG_HD igLAB ConvertToLAB() const;

  // This colour drawn over background, using this colour's opacity
  IG_HD igColour BlendOver(const igColour& background) const;
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

// Shared by igCanvasGrader (CPU) and the CUDA grade kernel, so both score identically
IG_HD inline igLAB igColour::ConvertToLAB() const
{
  // RGB to linear
  float r = red / 255.0f;
  float g = green / 255.0f;
  float b = blue / 255.0f;

  // gamma correction
  r = r > 0.04045f ? powf((r + 0.055f) / 1.055f, 2.4f) : r / 12.92f;
  g = g > 0.04045f ? powf((g + 0.055f) / 1.055f, 2.4f) : g / 12.92f;
  b = b > 0.04045f ? powf((b + 0.055f) / 1.055f, 2.4f) : b / 12.92f;

  // RGB to XYZ (D65 illuminant)
  float x = r * 0.4124564f + g * 0.3575761f + b * 0.1804375f;
  float y = r * 0.2126729f + g * 0.7151522f + b * 0.0721750f;
  float z = r * 0.0193339f + g * 0.1191920f + b * 0.9503041f;

  // normalise by D65 white point
  x /= 0.95047f;
  y /= 1.00000f;
  z /= 1.08883f;

  // XYZ to LAB
  auto f = [](float t)
  {
    return t > 0.008856f
               ? powf(t, 1.0f / 3.0f)
               : (7.787f * t + 16.0f / 116.0f);
  };

  igLAB lab;
  lab.L = 116.0f * f(y) - 16.0f;
  lab.a = 500.0f * (f(x) - f(y));
  lab.b = 200.0f * (f(y) - f(z));
  return lab;
}

IG_HD inline igColour igColour::BlendOver(const igColour& background) const
{
  float a = opacity / 255.0f;
  igColour blended;
  blended.red = uint8_t(red * a + background.red * (1.0f - a));
  blended.green = uint8_t(green * a + background.green * (1.0f - a));
  blended.blue = uint8_t(blue * a + background.blue * (1.0f - a));
  blended.opacity = background.opacity;
  return blended;
}

#endif
