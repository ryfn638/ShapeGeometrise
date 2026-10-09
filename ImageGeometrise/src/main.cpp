#include "igCanvasRenderer.h"
#include "igWindow.h"
#include "igWindowUI.h"
#include <Windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  // Declared in order, so they're torn down in reverse (UI, then D3D, then the window)
  igWindow window;
  igCanvasRenderer renderer(&window);
  if (!renderer.IsValid())
  {
    return 1;
  }
  window.Show();

  igWindowUI ui(&window, &renderer);

  while (window.PollEvents())
  {
    UINT width, height;
    if (window.ConsumeResize(width, height))
    {
      renderer.Resize(width, height);
    }
    ui.Render();
  }

  return 0;
}
