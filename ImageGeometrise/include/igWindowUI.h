#ifndef igWindowUI_h
#define igWindowUI_h

#include "igCanvasRenderer.h"
#include "igWindow.h"
#include "imgui.h"
#include "operations.h" // Colour, ShapePoint
#include <atomic>
#include <string>
#include <vector>

// The ImGui front end: controls on the left, the canvas on the right.
// Also owns the generation that the Generate button kicks off.
class igWindowUI
{
public:
  igWindowUI(igWindow* pWindow, igCanvasRenderer* pRenderer);
  ~igWindowUI();

  igWindowUI(const igWindowUI&) = delete;
  igWindowUI& operator=(const igWindowUI&) = delete;

  // Builds and draws one frame
  void Render();

private:
  void DrawControls();
  void DrawCanvas();
  void StartGeneration();

  igWindow* m_pWindow;
  igCanvasRenderer* m_pRenderer;
  ImGuiContext* m_pCtx;

  std::string m_selectedFilepath;
  std::atomic<bool> m_generating = false;

  std::vector<std::vector<ShapePoint>> m_masks;
  std::vector<Colour> m_canvas;
  int m_imageWidth = 0;
  int m_imageHeight = 0;
};
#endif
