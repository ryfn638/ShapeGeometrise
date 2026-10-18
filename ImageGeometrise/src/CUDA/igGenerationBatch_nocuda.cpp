// Stand-in for igGenerationBatch.cu when building without the CUDA toolkit (IG_NO_CUDA,
// set by workspace.lua). Every call reports failure, so igGeneration stays on the CPU grader.
#ifdef IG_NO_CUDA

#include "igGenerationBatch.cuh"

igGenerationBatch::~igGenerationBatch()
{
}

void igGenerationBatch::Free()
{
}

bool igGenerationBatch::Available()
{
  return false;
}

bool igGenerationBatch::SetMasks(const std::vector<const igMask*>&)
{
  return false;
}

bool igGenerationBatch::Upload(igColour*&, int64_t&, const igCanvas&)
{
  return false;
}

bool igGenerationBatch::SetTarget(const igCanvas&)
{
  return false;
}

bool igGenerationBatch::SetCanvas(const igCanvas&)
{
  return false;
}

bool igGenerationBatch::Grade(std::vector<igShape>&, int, std::vector<int64_t>&, bool)
{
  return false;
}

#endif
