#ifndef igShape_h
#define igShape_h

#include <cstddef>
#include <new>
#include <utility>
#include <vector>
#include <cstdint>
#include <string>
#include <memory>

#include "igCommon.h"
#include "igMask.h"

// This should be able to take a completely new in
class igShape
{
public:
  igShape(const igVec2 dimensions, const igVec2 position, float angle, float scale, igMask *pMask);

  igVec2 Size();
  igVec2 Position();

  void Rotate(float angleScale);
  void Scale(float scale);

  void ChangeColour(float x, float y, float z);
private:
  igVec2 m_dimensions;
  igVec2 m_position;

  float m_angle;
  float m_scale;

  std::shared_ptr<igMask> m_mask;
};

#endif
