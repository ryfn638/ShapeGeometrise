#ifndef igCanvas_h
#define igCanvas_h

#include <vector>
#include <string>

#include "igCommon.h"

class igShape;

// wrapper for the canvas/ accessing specific pixels
class igCanvas
{
public:
  igCanvas() = default;
  igCanvas(const igVec2& size);

  igVec2 Size() const;
  bool Empty() const;

  // Empty canvas if the image couldn't be loaded
  static igCanvas LoadImageCanvas(const std::string& path);

  bool Contains(int64_t x, int64_t y) const;
  igColour GetPixel(int64_t x, int64_t y) const;
  void SetPixel(int64_t x, int64_t y, const igColour& colour);

  // Blends the shape onto the canvas. Anything outside the canvas is skipped.
  void Project(const igShape& shape);

  // Resamples to a new size (used by the coarse to fine stages)
  igCanvas Resized(const igVec2& size) const;

private:
  igVec2 m_dimensions;
  std::vector<igColour> m_data;
};

#endif
