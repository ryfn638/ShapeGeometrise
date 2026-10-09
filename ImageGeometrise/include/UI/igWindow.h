#ifndef igWindow_h
#define igWindow_h

#include <Windows.h>
#include <string>

// Window Context Wrapper
// Because I really don't like window's API
class igWindow
{
public:
  igWindow(const wchar_t* title = L"Geometrise", int width = 1280, int height = 800);
  ~igWindow();

  igWindow(const igWindow&) = delete;
  igWindow& operator=(const igWindow&) = delete;

  void Show();

  // Pumps the message queue. Returns false once the window has been closed.
  bool PollEvents();

  // True (once) if the window was resized since the last call
  bool ConsumeResize(UINT& width, UINT& height);

  // Native file picker. Returns "" if cancelled.
  std::string OpenFileDialog(const char* filter) const;

  HWND Handle() const { return m_handle; }

private:
  static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

  HWND m_handle = nullptr;
  HINSTANCE m_instance = nullptr;
  const wchar_t* m_className = L"Geometrise";

  UINT m_resizeWidth = 0;
  UINT m_resizeHeight = 0;
};

#endif
