#include "igCanvasRenderer.h"
#include <cstdint>

igCanvasRenderer::igCanvasRenderer(igWindow* pWindow)
  : m_pWindow(pWindow)
{
  if (!CreateDeviceD3D())
  {
    CleanupDeviceD3D();
  }
}

igCanvasRenderer::~igCanvasRenderer()
{
  CleanupCanvasTexture();
  CleanupDeviceD3D();
}

bool igCanvasRenderer::CreateDeviceD3D()
{
  DXGI_SWAP_CHAIN_DESC sd = {};
  sd.BufferCount = 2;
  sd.BufferDesc.Width = 0;
  sd.BufferDesc.Height = 0;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = m_pWindow->Handle();
  sd.SampleDesc.Count = 1;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  D3D_FEATURE_LEVEL featureLevel;
  const D3D_FEATURE_LEVEL featureLevelArray[2] = {
    D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0
  };

  if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
                                    featureLevelArray, 2, D3D11_SDK_VERSION,
                                    &sd, &m_pSwapChain, &m_pd3dDevice,
                                    &featureLevel, &m_pd3dDeviceContext) != S_OK)
  {
    return false;
  }

  CreateRenderTarget();
  return true;
}

void igCanvasRenderer::CleanupDeviceD3D()
{
  CleanupRenderTarget();
  if (m_pSwapChain)
  {
    m_pSwapChain->Release();
    m_pSwapChain = nullptr;
  }

  if (m_pd3dDeviceContext)
  {
    m_pd3dDeviceContext->Release();
    m_pd3dDeviceContext = nullptr;
  }

  if (m_pd3dDevice)
  {
    m_pd3dDevice->Release();
    m_pd3dDevice = nullptr;
  }
}

void igCanvasRenderer::CreateRenderTarget()
{
  ID3D11Texture2D* pBackBuffer;
  m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
  m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_mainRenderTargetView);
  pBackBuffer->Release();
}

void igCanvasRenderer::CleanupRenderTarget()
{
  if (m_mainRenderTargetView)
  {
    m_mainRenderTargetView->Release();
    m_mainRenderTargetView = nullptr;
  }
}

void igCanvasRenderer::CleanupCanvasTexture()
{
  if (m_canvasTexture)
  {
    m_canvasTexture->Release();
    m_canvasTexture = nullptr;
  }

  if (m_canvasTex2D)
  {
    m_canvasTex2D->Release();
    m_canvasTex2D = nullptr;
  }
}

void igCanvasRenderer::Resize(UINT width, UINT height)
{
  CleanupRenderTarget();
  m_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
  CreateRenderTarget();
}

void igCanvasRenderer::BeginFrame(float r, float g, float b)
{
  const float clearColor[4] = { r, g, b, 1.0f };
  m_pd3dDeviceContext->OMSetRenderTargets(1, &m_mainRenderTargetView, nullptr);
  m_pd3dDeviceContext->ClearRenderTargetView(m_mainRenderTargetView, clearColor);
}

void igCanvasRenderer::EndFrame()
{
  m_pSwapChain->Present(1, 0); // vsync
}

void igCanvasRenderer::ResetCanvas(const igVec2& size)
{
  std::lock_guard<std::mutex> lock(m_canvasMutex);
  m_canvas = igCanvas(size);
  m_canvasDirty = true;
}

void igCanvasRenderer::ProjectShape(const igShape& shape)
{
  std::lock_guard<std::mutex> lock(m_canvasMutex);
  m_canvas.Project(shape);
  m_canvasDirty = true;
}

igVec2 igCanvasRenderer::CanvasSize() const
{
  std::lock_guard<std::mutex> lock(m_canvasMutex);
  return m_canvas.Size();
}

void igCanvasRenderer::UpdateCanvas()
{
  if (!m_pd3dDevice || !m_canvasDirty)
  {
    return;
  }

  std::lock_guard<std::mutex> lock(m_canvasMutex);
  m_canvasDirty = false;

  const int64_t width = m_canvas.Size().x;
  const int64_t height = m_canvas.Size().y;
  if (m_canvas.Empty())
  {
    return;
  }

  // Create the texture once per size, then just update it
  if (!m_canvasTex2D || m_canvas.Size() != m_textureSize)
  {
    CleanupCanvasTexture();

    D3D11_TEXTURE2D_DESC desc = {};

    desc.Width = UINT(width);
    desc.Height = UINT(height);
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DYNAMIC; // dynamic for frequent updates
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // allow CPU writes

    m_pd3dDevice->CreateTexture2D(&desc, nullptr, &m_canvasTex2D);
    m_pd3dDevice->CreateShaderResourceView(m_canvasTex2D, nullptr, &m_canvasTexture);
    m_textureSize = m_canvas.Size();
  }

  // Map the texture and write pixels directly without recreating
  D3D11_MAPPED_SUBRESOURCE mapped;
  if (m_pd3dDeviceContext->Map(m_canvasTex2D, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped) != S_OK)
  {
    return;
  }

  uint8_t* dst = (uint8_t*)mapped.pData;
  for (int64_t y = 0; y < height; y++)
  {
    uint8_t* row = dst + y * mapped.RowPitch;
    for (int64_t x = 0; x < width; x++)
    {
      const igColour c = m_canvas.GetPixel(x, y);
      row[x * 4 + 0] = c.red;
      row[x * 4 + 1] = c.green;
      row[x * 4 + 2] = c.blue;
      row[x * 4 + 3] = 255;
    }
  }

  m_pd3dDeviceContext->Unmap(m_canvasTex2D, 0);
}
