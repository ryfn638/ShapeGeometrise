#ifndef igGeneration_h
#define igGeneration_h

#include "igCanvas.h"
#include "igGenerationBatch.cuh"
#include "igMask.h"
#include "igShape.h"
#include <atomic>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

class igCanvasRenderer;

struct igGenerationSettings
{
  int numShapes = 300;       // total shapes placed on the canvas
  int generationShapes = 50; // shapes evaluated per generation
  int numGenerations = 2;    // generations per placed shape
  bool useGpu = false;       // grade with CUDA (igGenerationBatch) instead of igCanvasGrader
};

// How long one placed shape took, for the diagnostics panel
struct igShapeTiming
{
  float totalMs = 0.0f;  // whole iteration, including projecting
  float createMs = 0.0f; // making candidates (includes AverageColour on the CPU; on the GPU that's in gradeMs)
  float gradeMs = 0.0f;  // igCanvasGrader::Grade
  int candidates = 0;    // shapes graded for this placement
  bool gpu = false;      // graded on the GPU (gradeMs then includes uploads/readback)
};

// Generation manager: finds the best shape for the canvas, projects it, repeats.
// Runs on its own thread and updates the igCanvasRenderer's view of the current canvas.
class igGeneration
{
public:
  igGeneration(igCanvasRenderer* pRenderer);
  ~igGeneration();

  igGeneration(const igGeneration&) = delete;
  igGeneration& operator=(const igGeneration&) = delete;

  // Edit before Start(); a running generation uses the copy taken when it started
  igGenerationSettings& Settings()
  {
    return m_settings;
  }

  void AddMask(const std::string& path, double backgroundLenience = 0.9);

  // Loads the target and starts generating. False if it's already running or the image didn't load.
  bool Start(const std::string& targetPath);
  void Stop();

  bool IsRunning() const
  {
    return m_running;
  }
  int CurrentShape() const
  {
    return m_currentShape;
  }
  bool UsingGpu() const
  {
    return m_runSettings.useGpu;
  }
  int TotalShapes() const
  {
    return m_runSettings.numShapes;
  }

  // Copy of the timings for the current/last run, one per placed shape
  std::vector<igShapeTiming> Timings() const;

private:
  void Run();

  // Best shape after numGenerations rounds of mutating the previous best
  igShape FindBestShape(const igCanvas& canvas, const igCanvas& target, int subsample);
  // averageColour = false leaves the colour for the GPU to fill in (igGenerationBatch::Grade)
  igShape CreateShape(const igShape* pBaseShape, const igMask* pMask, const igCanvas& target, bool averageColour = true);

  igCanvasRenderer* m_pRenderer;

  igGenerationSettings m_settings;
  igGenerationSettings m_runSettings;

  std::vector<igMask> m_masks;
  igGenerationBatch m_gpu;
  igCanvas m_target; // full resolution

  std::mt19937 m_rng { std::random_device {}() };

  std::thread m_thread;
  std::atomic<bool> m_running = false;
  std::atomic<bool> m_stop = false;
  std::atomic<int> m_currentShape = 0;

  // Filled in by FindBestShape for the shape being placed (worker thread only)
  igShapeTiming m_currentTiming;

  mutable std::mutex m_timingMutex;
  std::vector<igShapeTiming> m_timings;
};

#endif
