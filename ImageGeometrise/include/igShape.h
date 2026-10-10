#ifndef igShape_h
#define igShape_h

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "igCommon.h"
#include "igMask.h"

// A mask plus a transform. Scale/Rotate only change the transform (O(1)); the mask is
// shared and never touched. Which pixels the shape covers is worked out on demand in
// ForEachPixel, by mapping each canvas pixel back into the mask.
class igShape
{
public:
  igShape(const igVec2 dimensions, const igVec2 position, float angle, float scale, igColour colour, const igMask* pMask);

  igVec2 Size() const;
  igVec2 Position() const;
  float Angle() const;
  igColour Colour() const;

  void SetPosition(const igVec2& position);
  void SetColour(const igColour& colour);

  // Sets the rotation in radians (absolute, not added to the current one)
  void Rotate(float angle);
  void Scale(float scaleX, float scaleY, bool lockedScaling = false);

  void ScaleColour(float r, float g, float b);
  void ScaleOpacity(float scale);
  void ShiftPosition(float movePercX, float movePercY);

  // Calls fn(x, y) for every canvas pixel the shape covers.
  // Walks the rotated bounding box and samples the mask at each pixel (inverse mapping), so
  // there are no holes when scaling up. One iteration = one CUDA thread when this moves to the GPU.
  template <typename Fn>
  void ForEachPixel(Fn&& fn) const;

private:
  igVec2 m_dimensions; // size on the canvas, after scaling
  igVec2 m_position;   // top left of the unrotated shape on the canvas

  float m_angle = 0.0f;

  igColour m_colour;
  const igMask* m_pMask;
};

template <typename Fn>
void igShape::ForEachPixel(Fn&& fn) const
{
  const float w = float(m_dimensions.x);
  const float h = float(m_dimensions.y);
  if (w <= 0 || h <= 0 || m_pMask == nullptr || m_pMask->Width() == 0)
  {
    return;
  }

  const float cosA = std::cos(m_angle);
  const float sinA = std::sin(m_angle);
  const float centerX = m_position.x + w / 2.0f;
  const float centerY = m_position.y + h / 2.0f;

  // Half extents of the rotated rectangle
  const float extentX = std::abs(w / 2.0f * cosA) + std::abs(h / 2.0f * sinA);
  const float extentY = std::abs(w / 2.0f * sinA) + std::abs(h / 2.0f * cosA);

  const int64_t minX = int64_t(std::floor(centerX - extentX));
  const int64_t maxX = int64_t(std::ceil(centerX + extentX));
  const int64_t minY = int64_t(std::floor(centerY - extentY));
  const int64_t maxY = int64_t(std::ceil(centerY + extentY));

  // Shape space -> mask space
  const float toMaskX = m_pMask->Width() / w;
  const float toMaskY = m_pMask->Height() / h;

  for (int64_t y = minY; y < maxY; y++)
  {
    for (int64_t x = minX; x < maxX; x++)
    {
      // Undo the rotation around the centre (sampling at the pixel centre)
      const float dx = x + 0.5f - centerX;
      const float dy = y + 0.5f - centerY;
      const float localX = dx * cosA - dy * sinA + w / 2.0f;
      const float localY = dx * sinA + dy * cosA + h / 2.0f;

      if (m_pMask->Contains(int64_t(std::floor(localX * toMaskX)), int64_t(std::floor(localY * toMaskY))))
      {
        fn(x, y);
      }
    }
  }
}

#endif
