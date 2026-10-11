#define NOMINMAX

#include "igGeneration.h"
#include "igCanvasGrader.h"
#include "igCanvasRenderer.h"

#include <algorithm>
#include <chrono>
#include <climits>
#include <math.h>

using Clock = std::chrono::steady_clock;

static float MsSince(Clock::time_point start)
{
  return std::chrono::duration<float, std::milli>(Clock::now() - start).count();
}

igGeneration::igGeneration(igCanvasRenderer* pRenderer)
  : m_pRenderer(pRenderer)
{
}

igGeneration::~igGeneration()
{
  Stop();
}

void igGeneration::AddMask(const std::string& path, double backgroundLenience)
{
  m_masks.emplace_back(path, backgroundLenience);
}

bool igGeneration::Start(const std::string& targetPath)
{
  if (m_running || m_masks.empty())
    return false;

  if (m_thread.joinable())
    m_thread.join();

  m_target = igCanvas::LoadImageCanvas(targetPath);
  if (m_target.Empty())
    return false;

  m_runSettings = m_settings;
  if (m_runSettings.useGpu && !igGenerationBatch::Available())
    m_runSettings.useGpu = false; // no CUDA device, fall back to the CPU grader
  m_currentShape = 0;
  {
    std::lock_guard<std::mutex> lock(m_timingMutex);
    m_timings.clear();
  }
  m_stop = false;
  m_running = true;

  // The renderer's canvas is the full resolution one that's shown
  m_pRenderer->ResetCanvas(m_target.Size());

  m_thread = std::thread(&igGeneration::Run, this);
  return true;
}

void igGeneration::Stop()
{
  m_stop = true;
  if (m_thread.joinable())
  {
    m_thread.join();
  }
}

std::vector<igShapeTiming> igGeneration::Timings() const
{
  std::lock_guard<std::mutex> lock(m_timingMutex);
  return m_timings;
}

void igGeneration::Run()
{
  // Coarse to fine: early shapes are found at a lower resolution for speed
  struct Stage
  {
    float progress, scale;
    int subsample;
  };

  const Stage stages[] = {
    { 0.0f, 0.25f, 8 },
    { 0.75f, 0.5f, 4 },
    { 0.85f, 0.75f, 2 },
    { 0.9f, 1.0f, 1 },
  };

  const igVec2 fullSize = m_target.Size();
  igCanvas canvas(fullSize);
  igCanvas target;
  float currentScale = -1.0f;

  if (m_runSettings.useGpu && !m_gpu.SetMasks(m_masks))
    m_runSettings.useGpu = false;

  const int numShapes = m_runSettings.numShapes;
  for (int s = 0; s < numShapes && !m_stop; s++)
  {
    m_currentShape = s;
    const Clock::time_point shapeStart = Clock::now();
    m_currentTiming = igShapeTiming();
    float progress = (float)s / numShapes;

    Stage currentStage = stages[0];
    for (const Stage& stage : stages)
      if (progress >= stage.progress)
        currentStage = stage;

    // Only one working canvas: resample it (and the target) when the stage changes
    if (currentStage.scale != currentScale)
    {
      currentScale = currentStage.scale;
      igVec2 scaledSize(std::max<int64_t>(1, int64_t(fullSize.x * currentScale)),
                        std::max<int64_t>(1, int64_t(fullSize.y * currentScale)));

      canvas = canvas.Resized(scaledSize);
      target = m_target.Resized(scaledSize);

      if (m_runSettings.useGpu && !m_gpu.SetTarget(target))
        m_runSettings.useGpu = false;
    }

    // The GPU grades against its own copy of the canvas, so it needs the latest one
    m_currentTiming.gpu = m_runSettings.useGpu;
    if (m_runSettings.useGpu)
    {
      const Clock::time_point uploadStart = Clock::now();
      if (!m_gpu.SetCanvas(canvas))
        m_runSettings.useGpu = m_currentTiming.gpu = false;
      m_currentTiming.gradeMs += MsSince(uploadStart);
    }

    igShape bestShape = FindBestShape(canvas, target, 2);
    canvas.Project(bestShape);

    // Same shape at full resolution for the renderer's canvas
    const float upscale = 1.0f / currentScale;
    igShape displayShape = bestShape;
    displayShape.SetPosition(igVec2(int64_t(bestShape.Position().x * upscale), int64_t(bestShape.Position().y * upscale)));
    displayShape.Scale(upscale, upscale);
    m_pRenderer->ProjectShape(displayShape);

    m_currentTiming.totalMs = MsSince(shapeStart);
    std::lock_guard<std::mutex> lock(m_timingMutex);
    m_timings.push_back(m_currentTiming);
  }

  m_currentShape = numShapes;
  m_running = false;
}

igShape igGeneration::FindBestShape(const igCanvas& canvas, const igCanvas& target, int subsample)
{
  // TODO: Grader for whatever reason works out the average colour, I think this makes more sense it igCanvas was the one to do it
  const igCanvasGrader grader(&target, &canvas);
  const int generationShapes = m_runSettings.generationShapes;

  Clock::time_point start = Clock::now();
  igShape topShape = CreateShape(nullptr, &m_masks[0], target);
  m_currentTiming.createMs += MsSince(start);

  std::vector<igShape> candidates;
  std::vector<int64_t> scores;
  for (int g = 0; g < m_runSettings.numGenerations; g++)
  {
    // Make the whole generation first...
    start = Clock::now();
    candidates.clear();
    for (int i = 0; i < generationShapes; i++)
    {
      // Spread the masks evenly over the generation
      const igMask* pMask = &m_masks[(i * m_masks.size()) / generationShapes];
      // On the GPU path the average colour is filled in by the GPU during grading
      const bool cpuColour = !m_runSettings.useGpu;
      candidates.push_back((g == 0) ? CreateShape(nullptr, pMask, target, cpuColour)
                                    : CreateShape(&topShape, pMask, target, cpuColour));
    }
    m_currentTiming.createMs += MsSince(start);

    // ...then grade it: one kernel launch on the GPU, or one shape at a time on the CPU
    start = Clock::now();
    if (!m_runSettings.useGpu || !m_gpu.Grade(candidates, subsample, scores, true))
    {
      // CPU path, or a CUDA call failed (then stay on the CPU, colouring the shapes it skipped)
      const bool needColours = m_runSettings.useGpu;
      m_runSettings.useGpu = m_currentTiming.gpu = false;
      scores.resize(candidates.size());
      for (size_t i = 0; i < candidates.size(); i++)
      {
        if (needColours)
          candidates[i].SetColour(grader.AverageColour(candidates[i]));
        scores[i] = grader.Grade(candidates[i], subsample);
      }
    }
    m_currentTiming.gradeMs += MsSince(start);
    m_currentTiming.candidates += int(candidates.size());

    int64_t maxImprovement = INT64_MIN;
    size_t bestIndex = 0;
    for (size_t i = 0; i < candidates.size(); i++)
    {
      if (scores[i] > maxImprovement)
      {
        maxImprovement = scores[i];
        bestIndex = i;
      }
    }

    if (!candidates.empty())
      topShape = candidates[bestIndex];
  }
  return topShape;
}

igShape igGeneration::CreateShape(const igShape* pBaseShape, const igMask* pMask, const igCanvas& target, bool averageColour)
{
  const igCanvasGrader grader(&target, nullptr);
  const igVec2 canvasSize = target.Size();

  if (!pBaseShape)
  {
    std::uniform_real_distribution<float> disSize(0.02f, 1.0f);
    std::uniform_real_distribution<float> disPos(0.0f, 1.0f);

    int64_t highestAxis = std::max(canvasSize.x, canvasSize.y);
    igVec2 size(std::max<int64_t>(1, int64_t(disSize(m_rng) * highestAxis)),
                std::max<int64_t>(1, int64_t(disSize(m_rng) * highestAxis)));
    igVec2 position(int64_t(disPos(m_rng) * (canvasSize.x - size.x)),
                    int64_t(disPos(m_rng) * (canvasSize.y - size.y)));

    igShape newShape(size, position, 0.0f, 1.0f, igColour(), pMask);
    if (averageColour)
      newShape.SetColour(grader.AverageColour(newShape));
    return newShape;
  }

  std::uniform_real_distribution<float> dis(0.8f, 1.2f);
  std::uniform_real_distribution<float> disAngle(0.0f, 2.0f);
  std::uniform_real_distribution<float> disShift(-0.1f, 0.1f);

  igShape newShape = *pBaseShape;
  newShape.Scale(dis(m_rng), dis(m_rng), false);
  newShape.ScaleOpacity(dis(m_rng));
  newShape.ShiftPosition(disShift(m_rng), disShift(m_rng));
  newShape.Rotate(disAngle(m_rng));

  if (averageColour)
    newShape.SetColour(grader.AverageColour(newShape));
  return newShape;
}
