#ifndef igGeneration_h
#define igGeneration_h

#include "igCanvas.h"
#include "igMask.h"
#include "igShape.h"
#include <atomic>
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
  int TotalShapes() const
  {
    return m_runSettings.numShapes;
  }

private:
  void Run();

  // Best shape after numGenerations rounds of mutating the previous best
  igShape FindBestShape(const igCanvas& canvas, const igCanvas& target, int subsample);
  igShape CreateShape(const igShape* pBaseShape, const igMask* pMask, const igCanvas& target);

  igCanvasRenderer* m_pRenderer;

  igGenerationSettings m_settings;
  igGenerationSettings m_runSettings;

  std::vector<igMask> m_masks;
  igCanvas m_target; // full resolution

  std::mt19937 m_rng { std::random_device {}() };

  std::thread m_thread;
  std::atomic<bool> m_running = false;
  std::atomic<bool> m_stop = false;
  std::atomic<int> m_currentShape = 0;
};

#endif
