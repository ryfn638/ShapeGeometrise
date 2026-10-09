#include "igWindow.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include <commdlg.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

igWindow::igWindow(const wchar_t* title, int width, int height)
{
  m_instance = GetModuleHandle(nullptr);

  WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L,
                     m_instance, nullptr, nullptr, nullptr, nullptr,
                     m_className, nullptr };
  RegisterClassExW(&wc);

  m_handle = CreateWindowW(m_className, title,
                           WS_OVERLAPPEDWINDOW, 100, 100, width, height,
                           nullptr, nullptr, m_instance, nullptr);

  // Lets WndProc find this object again
  SetWindowLongPtrW(m_handle, GWLP_USERDATA, (LONG_PTR)this);
}

igWindow::~igWindow()
{
  if (m_handle)
  {
    DestroyWindow(m_handle);
  }
  UnregisterClassW(m_className, m_instance);
}

void igWindow::Show()
{
  ShowWindow(m_handle, SW_SHOWDEFAULT);
  UpdateWindow(m_handle);
}

bool igWindow::PollEvents()
{
  MSG msg;
  bool open = true;
  while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    if (msg.message == WM_QUIT)
    {
      open = false;
    }
  }
  return open;
}

bool igWindow::ConsumeResize(UINT& width, UINT& height)
{
  if (m_resizeWidth == 0 || m_resizeHeight == 0)
  {
    return false;
  }

  width = m_resizeWidth;
  height = m_resizeHeight;
  m_resizeWidth = m_resizeHeight = 0;
  return true;
}

std::string igWindow::OpenFileDialog(const char* filter) const
{
  char filename[MAX_PATH] = "";
  OPENFILENAMEA ofn = {};
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = m_handle;
  ofn.lpstrFilter = filter;
  ofn.lpstrFile = filename;
  ofn.nMaxFile = MAX_PATH;
  ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

  if (GetOpenFileNameA(&ofn))
  {
    return std::string(filename);
  }
  return "";
}

LRESULT WINAPI igWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
  {
    return true;
  }

  igWindow* pWindow = (igWindow*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);

  switch (msg)
  {
  case WM_SIZE:
    // Applied by the renderer next frame, see ConsumeResize()
    if (pWindow && wParam != SIZE_MINIMIZED)
    {
      pWindow->m_resizeWidth = LOWORD(lParam);
      pWindow->m_resizeHeight = HIWORD(lParam);
    }
    return 0;
  case WM_SYSCOMMAND:
    // Disable the ALT application menu
    if ((wParam & 0xfff0) == SC_KEYMENU)
    {
      return 0;
    }
    break;
  case WM_DESTROY:
    if (pWindow)
    {
      pWindow->m_handle = nullptr;
    }
    PostQuitMessage(0);
    return 0;
  }
  return DefWindowProcW(hWnd, msg, wParam, lParam);
}
