#include "igCanvasRenderer.h"
#include "igWindow.h"
#include "igWindowUI.h"
#include <Windows.h>
#include <string>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  // Always run from the exe's folder: shapes/ and targets/ are copied next to it at build time,
  // so they resolve however the program is launched (VS, VS Code, Explorer, a shortcut)
  wchar_t exePath[MAX_PATH];
  GetModuleFileNameW(nullptr, exePath, MAX_PATH);
  std::wstring exeDir(exePath);
  exeDir = exeDir.substr(0, exeDir.find_last_of(L"\\/"));
  SetCurrentDirectoryW(exeDir.c_str());

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
