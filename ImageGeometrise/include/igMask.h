#ifndef igMask_h
#define igMask_h

#include "igCommon.h"
#include <cstdint>
#include <string>
#include <vector>

// Small wrapper for what is essentially a bitmap
// parses images using opencv
//
// Never changes once loaded, so one mask can be shared by every shape that uses it.
// Shapes scale/rotate by sampling it (see igShape::ForEachPixel), not by rebuilding it.
class igMask
{
public:
  // Solid rectangle
  igMask(int64_t width, int64_t height);
  igMask(const std::string& filePath, double backgroundLenience);

  int64_t Width() const
  {
    return m_width;
  }
  int64_t Height() const
  {
    return m_height;
  }

  // Out of range is just "not in the shape"
  bool Contains(int64_t x, int64_t y) const
  {
    return x >= 0 && y >= 0 && x < m_width && y < m_height && m_data[y * m_width + x];
  }

  // Row major, 1 = inside the shape. Flat on purpose so it can go straight to the GPU.
  const std::vector<uint8_t>& GetData() const
  {
    return m_data;
  }

private:
  int64_t m_width = 0;
  int64_t m_height = 0;
  std::vector<uint8_t> m_data;
};

#endif
