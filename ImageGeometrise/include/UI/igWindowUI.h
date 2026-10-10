#ifndef igWindowUI_h
#define igWindowUI_h

#include "igCanvasRenderer.h"
#include "igWindow.h"
#include "igGeneration.h"
#include "imgui.h"
#include <string>

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

  igWindow* m_pWindow;
  igCanvasRenderer* m_pRenderer;
  ImGuiContext* m_pCtx;

  std::string m_selectedFilepath;
  igGeneration m_generation;
};
#endif
