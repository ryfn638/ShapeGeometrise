#include "igShape.h"

igShape::igShape(const igVec2 dimensions, const igVec2 position, float angle, float scale, igColour colour, const igMask* pMask)
  : m_dimensions(dimensions),
    m_position(position),
    m_angle(angle),
    m_colour(colour),
    m_pMask(pMask)
{
  if (scale != 1.0f)
  {
    Scale(scale, scale, true);
  }
}

igVec2 igShape::Size() const
{
  return m_dimensions;
}

igVec2 igShape::Position() const
{
  return m_position;
}

float igShape::Angle() const
{
  return m_angle;
}

igColour igShape::Colour() const
{
  return m_colour;
}

void igShape::SetPosition(const igVec2& position)
{
  m_position = position;
}

void igShape::SetColour(const igColour& colour)
{
  m_colour = colour;
}

void igShape::Rotate(float angle)
{
  m_angle = angle;
}

void igShape::Scale(float scaleX, float scaleY, bool lockedScaling)
{
  if (m_dimensions.x == 0 || m_dimensions.y == 0)
  {
    return;
  }

  int64_t newWidth = std::max(static_cast<int64_t>(m_dimensions.x * scaleX), static_cast<int64_t>(1));
  int64_t newHeight = std::max(static_cast<int64_t>(m_dimensions.y * (lockedScaling ? scaleX : scaleY)), static_cast<int64_t>(1));

  m_dimensions = igVec2(newWidth, newHeight);
}

void igShape::ScaleColour(float scaleR, float scaleG, float scaleB)
{
  // do this to prevent truncatiosn
  int64_t redScaled = m_colour.red * scaleR;
  int64_t greenScaled = m_colour.green * scaleG;
  int64_t blueScaled = m_colour.blue * scaleB;

  m_colour.red = std::clamp(static_cast<uint8_t>(redScaled), (uint8_t)0, (uint8_t)255);
  m_colour.green = std::clamp(static_cast<uint8_t>(greenScaled), (uint8_t)0, (uint8_t)255);
  m_colour.blue = std::clamp(static_cast<uint8_t>(blueScaled), (uint8_t)0, (uint8_t)255);
}

void igShape::ScaleOpacity(float scale)
{
  m_colour.opacity = uint8_t(std::min(255, int(m_colour.opacity * scale)));
}

void igShape::ShiftPosition(float movePercX, float movePercY)
{
  igVec2 move(
   static_cast<int64_t>(m_dimensions.x * movePercX),
   static_cast<int64_t>(m_dimensions.y * movePercY)
   );

  m_position += move;
}
