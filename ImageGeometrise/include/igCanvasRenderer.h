#ifndef igCanvasRenderer_h
#define igCanvasRenderer_h

#include "igWindow.h"
#include "operations.h" // Colour
#include <d3d11.h>
#include <vector>

// Owns the D3D11 device/swap chain for a window, plus the texture the canvas is drawn into
class igCanvasRenderer
{
public:
  igCanvasRenderer(igWindow* pWindow);
  ~igCanvasRenderer();

  igCanvasRenderer(const igCanvasRenderer&) = delete;
  igCanvasRenderer& operator=(const igCanvasRenderer&) = delete;

  // False if the device couldn't be created
  bool IsValid() const { return m_pd3dDevice != nullptr; }

  void Resize(UINT width, UINT height);

  // Clears the back buffer and binds it
  void BeginFrame(float r, float g, float b);
  void EndFrame();

  // Uploads the canvas pixels, (re)creating the texture if the size changed
  void UpdateCanvasTexture(const std::vector<Colour>& canvas, int width, int height);
  ID3D11ShaderResourceView* CanvasTexture() const { return m_canvasTexture; }

  ID3D11Device* Device() const { return m_pd3dDevice; }
  ID3D11DeviceContext* Context() const { return m_pd3dDeviceContext; }

private:
  bool CreateDeviceD3D();
  void CleanupDeviceD3D();
  void CreateRenderTarget();
  void CleanupRenderTarget();
  void CleanupCanvasTexture();

  igWindow* m_pWindow;

  // DirectX vars
  ID3D11Device* m_pd3dDevice = nullptr;
  ID3D11DeviceContext* m_pd3dDeviceContext = nullptr;
  IDXGISwapChain* m_pSwapChain = nullptr;
  ID3D11RenderTargetView* m_mainRenderTargetView = nullptr;

  ID3D11ShaderResourceView* m_canvasTexture = nullptr;
  ID3D11Texture2D* m_canvasTex2D = nullptr;
  int m_canvasWidth = 0;
  int m_canvasHeight = 0;
};

#endif
