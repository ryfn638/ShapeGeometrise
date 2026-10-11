#ifndef igGenerationBatch_h
#define igGenerationBatch_h

// Included by plain C++ too (igGeneration.cpp), so nothing CUDA-only in here.
// The kernel itself lives in igGenerationBatch.cu.

#include "igCommon.h"
#include <cstdint>
#include <unordered_map>
#include <vector>

class igCanvas;
class igMask;
class igShape;

// What the grade kernel needs from a shape, as plain data the GPU can read
struct igGPUShape
{
  // positional
  float posX, posY, posZ;

  float width, height;
  float angle;
  igColour colour;
  int maskIndex; // into the batch's mask list
};

// Non-owning view of a 2D buffer. Passed to kernels by value (pData is a device pointer there).
template <typename T>
struct ig2DSpan
{
  T* pData;
  igVec2 dims;
  IG_HD T GetAt(int64_t x, int64_t y) const;
  IG_HD T At(int64_t index) const;
  IG_HD T* Data() const;
};

template <typename T>
IG_HD T ig2DSpan<T>::GetAt(int64_t x, int64_t y) const
{
  return pData[x + y * dims.x];
}

template <typename T>
IG_HD T ig2DSpan<T>::At(int64_t index) const
{
  return pData[index];
}

template <typename T>
IG_HD T* ig2DSpan<T>::Data() const
{
  return pData;
}

// GPU version of igCanvasGrader::Grade for a whole generation at once: one kernel launch
// grades every candidate. Keeps the masks, target and canvas resident in VRAM so they're
// only uploaded when they change.
class igGenerationBatch
{
public:
  igGenerationBatch() = default;
  ~igGenerationBatch();

  igGenerationBatch(const igGenerationBatch&) = delete;
  igGenerationBatch& operator=(const igGenerationBatch&) = delete;

  // True if there's a CUDA capable GPU
  static bool Available();

  // Once per run. Shapes must use masks from this list.
  bool SetMasks(const std::vector<igMask>& masks);
  // When the stage (resolution) changes
  bool SetTarget(const igCanvas& target);
  // After every projected shape
  bool SetCanvas(const igCanvas& canvas);

  // Same scoring as igCanvasGrader::Grade for each shape. Within ~0.05% at subsample 1: GPU
  // powf/cosf round slightly differently, which shifts the per-pixel int truncation. With
  // subsample > 1 it skips different pixels than the CPU (every nth of the box, not of the covered pixels).
  // averageColours: also does igCanvasGrader::AverageColour on the GPU first, grades with that
  // colour, and sets it on the shapes. Lets CreateShape skip the per-pixel colour work.
  // False if a CUDA call failed.
  bool Grade(std::vector<igShape>& shapes, int subsample, std::vector<int64_t>& scores, bool averageColours = false);

private:
  bool Upload(igColour*& pDevice, int64_t& capacity, const igCanvas& canvas);
  void Free();

  igColour* m_pCanvas = nullptr;
  igColour* m_pTarget = nullptr;
  int64_t m_canvasCapacity = 0;
  int64_t m_targetCapacity = 0;
  igVec2 m_canvasSize;

  uint8_t* m_pMaskData = nullptr;
  ig2DSpan<uint8_t>* m_pMasks = nullptr;
  std::unordered_map<const igMask*, int> m_maskIndices;

  igGPUShape* m_pShapes = nullptr;
  unsigned long long* m_pScores = nullptr;
  unsigned long long* m_pColourSums = nullptr; // r, g, b, count per shape
  int64_t m_shapeCapacity = 0;

  // Host side staging, reused between calls
  std::vector<igGPUShape> m_gpuShapes;
  std::vector<unsigned long long> m_rawScores;
  std::vector<unsigned long long> m_rawSums;
};

#endif
