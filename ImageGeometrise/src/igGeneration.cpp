#include "igGeneration.h"
#include "igCanvasGrader.h"
#include "igCanvasRenderer.h"
#include <algorithm>
#include <climits>

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
  {
    return false;
  }

  if (m_thread.joinable())
  {
    m_thread.join();
  }

  m_target = igCanvas::LoadImageCanvas(targetPath);
  if (m_target.Empty())
  {
    return false;
  }

  m_runSettings = m_settings;
  m_currentShape = 0;
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

  const int numShapes = m_runSettings.numShapes;
  for (int s = 0; s < numShapes && !m_stop; s++)
  {
    m_currentShape = s;
    float progress = (float)s / numShapes;

    Stage currentStage = stages[0];
    for (const Stage& stage : stages)
    {
      if (progress >= stage.progress)
      {
        currentStage = stage;
      }
    }

    // Only one working canvas: resample it (and the target) when the stage changes
    if (currentStage.scale != currentScale)
    {
      currentScale = currentStage.scale;
      igVec2 scaledSize(std::max<int64_t>(1, int64_t(fullSize.x * currentScale)),
                        std::max<int64_t>(1, int64_t(fullSize.y * currentScale)));

      canvas = canvas.Resized(scaledSize);
      target = m_target.Resized(scaledSize);
    }

    igShape bestShape = FindBestShape(canvas, target, 2);
    canvas.Project(bestShape);

    // Same shape at full resolution for the renderer's canvas
    const float upscale = 1.0f / currentScale;
    igShape displayShape = bestShape;
    displayShape.SetPosition(igVec2(int64_t(bestShape.Position().x * upscale), int64_t(bestShape.Position().y * upscale)));
    displayShape.Scale(upscale, upscale);
    m_pRenderer->ProjectShape(displayShape);
  }

  m_currentShape = numShapes;
  m_running = false;
}

igShape igGeneration::FindBestShape(const igCanvas& canvas, const igCanvas& target, int subsample)
{
  const igCanvasGrader grader(&target, &canvas);
  const int generationShapes = m_runSettings.generationShapes;

  igShape topShape = CreateShape(nullptr, &m_masks[0], target);
  for (int g = 0; g < m_runSettings.numGenerations; g++)
  {
    int64_t maxImprovement = INT64_MIN;
    igShape generationBest = topShape;

    for (int i = 0; i < generationShapes; i++)
    {
      // Spread the masks evenly over the generation
      const igMask* pMask = &m_masks[(i * m_masks.size()) / generationShapes];
      igShape shape = (g == 0) ? CreateShape(nullptr, pMask, target)
                               : CreateShape(&topShape, pMask, target);

      int64_t improvement = grader.Grade(shape, subsample);
      if (improvement > maxImprovement)
      {
        maxImprovement = improvement;
        generationBest = shape;
      }
    }

    topShape = generationBest;
  }
  return topShape;
}

igShape igGeneration::CreateShape(const igShape* pBaseShape, const igMask* pMask, const igCanvas& target)
{
  const igCanvasGrader grader(&target, nullptr);
  const igVec2 canvasSize = target.Size();

  if (!pBaseShape)
  {
    std::uniform_real_distribution<float> dis(0.1f, 0.9f);
    igVec2 size(std::max<int64_t>(1, int64_t(dis(m_rng) * canvasSize.x)),
                std::max<int64_t>(1, int64_t(dis(m_rng) * canvasSize.y)));
    igVec2 position(int64_t(dis(m_rng) * (canvasSize.x - size.x)),
                    int64_t(dis(m_rng) * (canvasSize.y - size.y)));

    igShape newShape(size, position, 0.0f, 1.0f, igColour(), pMask);
    newShape.SetColour(grader.AverageColour(newShape));
    return newShape;
  }

  std::uniform_real_distribution<float> dis(0.8f, 1.2f);
  std::uniform_real_distribution<float> disAngle(0.0f, 2.0f);
  std::uniform_real_distribution<float> disShift(-0.1f, 0.1f);

  igShape newShape = *pBaseShape;
  newShape.Scale(dis(m_rng), dis(m_rng));
  newShape.ScaleOpacity(dis(m_rng));
  newShape.ShiftPosition(disShift(m_rng), disShift(m_rng));
  newShape.Rotate(disAngle(m_rng));

  newShape.SetColour(grader.AverageColour(newShape));
  return newShape;
}
