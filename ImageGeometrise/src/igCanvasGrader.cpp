#include "igCanvasGrader.h"

igCanvasGrader::igCanvasGrader(const igCanvas* pTarget, const igCanvas* pCanvas)
  : m_pTarget(pTarget),
    m_pCanvas(pCanvas)
{
}

int64_t igCanvasGrader::Grade(const igShape& shape, int subsample) const
{
  const igColour colour = shape.Colour();
  int64_t improvement = 0;
  int64_t i = 0;

  shape.ForEachPixel([&](int64_t x, int64_t y)
                     {
    if (i++ % subsample != 0 || !m_pCanvas->Contains(x, y))
    {
      return;
    }

    const igColour current = m_pCanvas->GetPixel(x, y);
    const igColour target = m_pTarget->GetPixel(x, y);
    improvement += ColourDistance(current, target) - ColourDistance(colour.BlendOver(current), target); });

  return improvement * subsample;
}

igColour igCanvasGrader::AverageColour(const igShape& shape) const
{
  uint64_t totalR = 0, totalG = 0, totalB = 0;
  uint64_t count = 0;

  shape.ForEachPixel([&](int64_t x, int64_t y)
                     {
    if (!m_pTarget->Contains(x, y))
    {
      return;
    }

    const igColour pixel = m_pTarget->GetPixel(x, y);
    totalR += pixel.red;
    totalG += pixel.green;
    totalB += pixel.blue;
    count++; });

  if (count == 0)
  {
    return igColour(0, 0, 0);
  }
  return igColour(uint8_t(totalR / count), uint8_t(totalG / count), uint8_t(totalB / count));
}

int64_t igCanvasGrader::ColourDistance(const igColour& a, const igColour& b)
{
  igLAB lab1 = a.ConvertToLAB();
  igLAB lab2 = b.ConvertToLAB();

  float dL = lab2.L - lab1.L;
  float da = lab2.a - lab1.a;
  float db = lab2.b - lab1.b;

  return int64_t(dL * dL + da * da + db * db);
}
