#include "igGenerationBatch.cuh"
#include "igCanvas.h"
#include "igMask.h"
#include "igShape.h"
#include <cstdio>
#include <algorithm>
#include <cmath>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>

// CUDA calls fail silently unless you check them
#define CUDA_CHECK(x)                                                                         \
  do                                                                                          \
  {                                                                                           \
    cudaError_t err = (x);                                                                    \
    if (err != cudaSuccess)                                                                   \
    {                                                                                         \
      fprintf(stderr, "[cuda] %s:%d %s\n", __FILE__, __LINE__, cudaGetErrorString(err)); \
      return false;                                                                           \
    }                                                                                         \
  } while (0)

static constexpr int THREADS_PER_BLOCK = 256;
static constexpr int MAX_BLOCKS_PER_SHAPE = 64;

// Same as igCanvasGrader::ColourDistance
__device__ int64_t ColourDistance(const igColour& a, const igColour& b)
{
  igLAB lab1 = a.ConvertToLAB();
  igLAB lab2 = b.ConvertToLAB();

  float dL = lab2.L - lab1.L;
  float da = lab2.a - lab1.a;
  float db = lab2.b - lab1.b;

  return int64_t(dL * dL + da * da + db * db);
}

// Shape layout for both kernels: one row of blocks per candidate (blockIdx.y), and the threads in
// that row walk the candidate's rotated bounding box, exactly like igShape::ForEachPixel.
// Calls fn(x, y) for each covered pixel this thread owns.
template <typename Fn>
__device__ void ForEachCoveredPixel(const igGPUShape& s, const ig2DSpan<uint8_t>& mask, const igVec2& canvasDims, int subsample, Fn fn)
{
  if (s.width <= 0 || s.height <= 0 || mask.dims.x == 0)
    return;

  // Same bounding box maths as ForEachPixel (every thread recomputes it, it's cheap)
  const float cosA = cosf(s.angle), sinA = sinf(s.angle);
  const float cx = s.posX + s.width / 2, cy = s.posY + s.height / 2;
  const float ex = fabsf(s.width / 2 * cosA) + fabsf(s.height / 2 * sinA);
  const float ey = fabsf(s.width / 2 * sinA) + fabsf(s.height / 2 * cosA);
  const int minX = (int)floorf(cx - ex), minY = (int)floorf(cy - ey);
  const int boxW = (int)ceilf(cx + ex) - minX, boxH = (int)ceilf(cy + ey) - minY;

  // Shape space -> mask space
  const float toMaskX = mask.dims.x / s.width;
  const float toMaskY = mask.dims.y / s.height;

  // Grid-stride loop: this thread handles pixel i, then i + (all threads on this candidate), ...
  for (int i = blockIdx.x * blockDim.x + threadIdx.x; i < boxW * boxH; i += gridDim.x * blockDim.x)
  {
    if (subsample > 1 && i % subsample != 0)
      continue;

    const int x = minX + i % boxW, y = minY + i / boxW;
    if (x < 0 || y < 0 || x >= canvasDims.x || y >= canvasDims.y)
      continue;

    // Undo the rotation around the centre and sample the mask, like ForEachPixel
    const float dx = x + 0.5f - cx;
    const float dy = y + 0.5f - cy;
    const float localX = dx * cosA - dy * sinA + s.width / 2;
    const float localY = dx * sinA + dy * cosA + s.height / 2;
    const int mx = (int)floorf(localX * toMaskX);
    const int my = (int)floorf(localY * toMaskY);
    if (mx < 0 || my < 0 || mx >= mask.dims.x || my >= mask.dims.y || !mask.GetAt(mx, my))
      continue;

    fn(x, y);
  }
}

// Sum a value across the warp (32 threads), then one atomic per warp.
// Unsigned add is two's complement, so negative values still add up correctly.
__device__ void WarpSumAtomic(unsigned long long* pTotal, long long value)
{
  for (int offset = 16; offset > 0; offset >>= 1)
    value += __shfl_down_sync(0xffffffff, value, offset);
  if ((threadIdx.x & 31) == 0)
    atomicAdd(pTotal, (unsigned long long)value);
}

// GPU version of igCanvasGrader::AverageColour: sums r, g, b and pixel count of the target
// under each candidate into sums[candidate * 4 + 0..3]
__global__ void AverageColourKernel(
  const igGPUShape* shapes,
  const ig2DSpan<igColour> target,
  const ig2DSpan<uint8_t>* masks,
  unsigned long long* sums)
{
  const igGPUShape s = shapes[blockIdx.y];
  long long r = 0, g = 0, b = 0, count = 0;

  ForEachCoveredPixel(s, masks[s.maskIndex], target.dims, 1, [&](int x, int y)
  {
    const igColour pixel = target.GetAt(x, y);
    r += pixel.red;
    g += pixel.green;
    b += pixel.blue;
    count++;
  });

  unsigned long long* pSums = sums + blockIdx.y * 4;
  WarpSumAtomic(pSums + 0, r);
  WarpSumAtomic(pSums + 1, g);
  WarpSumAtomic(pSums + 2, b);
  WarpSumAtomic(pSums + 3, count);
}

// Everything is passed by value: kernel arguments are copied to the GPU, references wouldn't be.
// If colourSums isn't null, each shape's colour is its average from AverageColourKernel
// (opaque, same as igCanvasGrader::AverageColour) instead of the colour it was uploaded with.
__global__ void GradeKernel(
  const igGPUShape* shapes,
  const ig2DSpan<igColour> canvas,
  const ig2DSpan<igColour> target,
  const ig2DSpan<uint8_t>* masks,
  const unsigned long long* colourSums,
  int subsample,
  unsigned long long* scores)
{
  const igGPUShape s = shapes[blockIdx.y];

  igColour colour = s.colour;
  if (colourSums)
  {
    const unsigned long long* pSums = colourSums + blockIdx.y * 4;
    const unsigned long long count = pSums[3];
    colour.red = count ? uint8_t(pSums[0] / count) : 0;
    colour.green = count ? uint8_t(pSums[1] / count) : 0;
    colour.blue = count ? uint8_t(pSums[2] / count) : 0;
    colour.opacity = 255;
  }

  long long local = 0;
  ForEachCoveredPixel(s, masks[s.maskIndex], canvas.dims, subsample, [&](int x, int y)
  {
    const igColour current = canvas.GetAt(x, y);
    const igColour goal = target.GetAt(x, y);
    local += ColourDistance(current, goal) - ColourDistance(colour.BlendOver(current), goal);
  });

  WarpSumAtomic(&scores[blockIdx.y], local);
}

igGenerationBatch::~igGenerationBatch()
{
  Free();
}

void igGenerationBatch::Free()
{
  cudaFree(m_pCanvas);
  cudaFree(m_pTarget);
  cudaFree(m_pMaskData);
  cudaFree(m_pMasks);
  cudaFree(m_pShapes);
  cudaFree(m_pScores);
  cudaFree(m_pColourSums);
  m_pCanvas = m_pTarget = nullptr;
  m_pColourSums = nullptr;
  m_pMaskData = nullptr;
  m_pMasks = nullptr;
  m_pShapes = nullptr;
  m_pScores = nullptr;
  m_canvasCapacity = m_targetCapacity = m_shapeCapacity = 0;
}

bool igGenerationBatch::Available()
{
  int count = 0;
  return cudaGetDeviceCount(&count) == cudaSuccess && count > 0;
}

bool igGenerationBatch::SetMasks(const std::vector<const igMask*>& masks)
{
  cudaFree(m_pMaskData);
  cudaFree(m_pMasks);
  m_pMaskData = nullptr;
  m_pMasks = nullptr;
  m_maskIndices.clear();

  // All masks in one buffer, plus a span per mask pointing into it
  size_t totalBytes = 0;
  for (const igMask* pMask : masks)
    totalBytes += pMask->GetData().size();

  CUDA_CHECK(cudaMalloc(&m_pMaskData, (std::max)(totalBytes, size_t(1))));

  std::vector<ig2DSpan<uint8_t>> spans;
  size_t offset = 0;
  for (size_t i = 0; i < masks.size(); i++)
  {
    const std::vector<uint8_t>& data = masks[i]->GetData();
    CUDA_CHECK(cudaMemcpy(m_pMaskData + offset, data.data(), data.size(), cudaMemcpyHostToDevice));
    spans.push_back({ m_pMaskData + offset, igVec2(masks[i]->Width(), masks[i]->Height()) });
    m_maskIndices[masks[i]] = int(i);
    offset += data.size();
  }

  CUDA_CHECK(cudaMalloc(&m_pMasks, (std::max)(spans.size(), size_t(1)) * sizeof(ig2DSpan<uint8_t>)));
  CUDA_CHECK(cudaMemcpy(m_pMasks, spans.data(), spans.size() * sizeof(ig2DSpan<uint8_t>), cudaMemcpyHostToDevice));
  return true;
}

bool igGenerationBatch::Upload(igColour*& pDevice, int64_t& capacity, const igCanvas& canvas)
{
  const int64_t pixels = canvas.Size().x * canvas.Size().y;
  if (pixels > capacity)
  {
    cudaFree(pDevice);
    pDevice = nullptr;
    capacity = 0;
    CUDA_CHECK(cudaMalloc(&pDevice, pixels * sizeof(igColour)));
    capacity = pixels;
  }
  CUDA_CHECK(cudaMemcpy(pDevice, canvas.Data(), pixels * sizeof(igColour), cudaMemcpyHostToDevice));
  return true;
}

bool igGenerationBatch::SetTarget(const igCanvas& target)
{
  return Upload(m_pTarget, m_targetCapacity, target);
}

bool igGenerationBatch::SetCanvas(const igCanvas& canvas)
{
  m_canvasSize = canvas.Size();
  return Upload(m_pCanvas, m_canvasCapacity, canvas);
}

bool igGenerationBatch::Grade(std::vector<igShape>& shapes, int subsample, std::vector<int64_t>& scores, bool averageColours)
{
  const int64_t count = int64_t(shapes.size());
  scores.assign(count, 0);
  if (count == 0)
    return true;

  if (count > m_shapeCapacity)
  {
    cudaFree(m_pShapes);
    cudaFree(m_pScores);
    cudaFree(m_pColourSums);
    m_pShapes = nullptr;
    m_pScores = nullptr;
    m_pColourSums = nullptr;
    m_shapeCapacity = 0;
    CUDA_CHECK(cudaMalloc(&m_pShapes, count * sizeof(igGPUShape)));
    CUDA_CHECK(cudaMalloc(&m_pScores, count * sizeof(unsigned long long)));
    CUDA_CHECK(cudaMalloc(&m_pColourSums, count * 4 * sizeof(unsigned long long)));
    m_shapeCapacity = count;
  }

  // Flatten the shapes and size the grid for the biggest bounding box
  m_gpuShapes.resize(count);
  int64_t biggestBox = 1;
  for (int64_t i = 0; i < count; i++)
  {
    const igShape& shape = shapes[i];
    auto it = m_maskIndices.find(shape.Mask());
    if (it == m_maskIndices.end())
    {
      fprintf(stderr, "[cuda] shape uses a mask that wasn't passed to SetMasks\n");
      return false;
    }

    igGPUShape& g = m_gpuShapes[i];
    g.posX = float(shape.Position().x);
    g.posY = float(shape.Position().y);
    g.posZ = 0.0f;
    g.width = float(shape.Size().x);
    g.height = float(shape.Size().y);
    g.angle = shape.Angle();
    g.colour = shape.Colour();
    g.maskIndex = it->second;

    // A rotated box is at most the diagonal squared
    const int64_t diagonal = int64_t(std::sqrt(g.width * g.width + g.height * g.height)) + 2;
    biggestBox = (std::max)(biggestBox, diagonal * diagonal);
  }

  CUDA_CHECK(cudaMemcpy(m_pShapes, m_gpuShapes.data(), count * sizeof(igGPUShape), cudaMemcpyHostToDevice));
  CUDA_CHECK(cudaMemset(m_pScores, 0, count * sizeof(unsigned long long)));

  const int64_t blocksNeeded = (biggestBox + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK;
  const dim3 block(THREADS_PER_BLOCK);
  const dim3 grid(unsigned((std::min)(blocksNeeded, int64_t(MAX_BLOCKS_PER_SHAPE))), unsigned(count));

  const ig2DSpan<igColour> canvas = { m_pCanvas, m_canvasSize };
  const ig2DSpan<igColour> target = { m_pTarget, m_canvasSize };

  // Kernels on the same stream run in order, so the grade kernel sees the finished colour sums
  if (averageColours)
  {
    CUDA_CHECK(cudaMemset(m_pColourSums, 0, count * 4 * sizeof(unsigned long long)));
    AverageColourKernel<<<grid, block>>>(m_pShapes, target, m_pMasks, m_pColourSums);
    CUDA_CHECK(cudaGetLastError()); // bad launch config
  }

  GradeKernel<<<grid, block>>>(m_pShapes, canvas, target, m_pMasks, averageColours ? m_pColourSums : nullptr, subsample, m_pScores);
  CUDA_CHECK(cudaGetLastError());

  // Waits for the kernels to finish
  m_rawScores.resize(count);
  CUDA_CHECK(cudaMemcpy(m_rawScores.data(), m_pScores, count * sizeof(unsigned long long), cudaMemcpyDeviceToHost));

  for (int64_t i = 0; i < count; i++)
    scores[i] = int64_t(m_rawScores[i]) * subsample;

  // Give the shapes the colours they were graded with
  if (averageColours)
  {
    m_rawSums.resize(count * 4);
    CUDA_CHECK(cudaMemcpy(m_rawSums.data(), m_pColourSums, count * 4 * sizeof(unsigned long long), cudaMemcpyDeviceToHost));
    for (int64_t i = 0; i < count; i++)
    {
      const unsigned long long* pSums = &m_rawSums[i * 4];
      const unsigned long long pixels = pSums[3];
      shapes[i].SetColour(pixels ? igColour(uint8_t(pSums[0] / pixels), uint8_t(pSums[1] / pixels), uint8_t(pSums[2] / pixels))
                                 : igColour(0, 0, 0));
    }
  }
  return true;
}
