#ifndef igCanvasGrader_h
#define igCanvasGrader_h

#include "igCanvas.h"
#include "igCommon.h"
#include "igShape.h"

// Middle man between a shape and the canvas: scores how much a shape would improve the
// canvas against the target, without projecting it or copying the canvas.
// Only the pixels under the shape change, so only those are compared.
class igCanvasGrader
{
public:
  igCanvasGrader(const igCanvas* pTarget, const igCanvas* pCanvas);

  // Sum over covered pixels of dist(canvas, target) - dist(canvas + shape, target).
  // > 0 means the shape makes the canvas closer to the target.
  // subsample only looks at every nth pixel and scales the result back up.
  int64_t Grade(const igShape& shape, int subsample = 1) const;

  // Average target colour under the shape
  igColour AverageColour(const igShape& shape) const;

private:
  // Squared distance in LAB space
  static int64_t ColourDistance(const igColour& a, const igColour& b);

  const igCanvas* m_pTarget;
  const igCanvas* m_pCanvas;
};

#endif
