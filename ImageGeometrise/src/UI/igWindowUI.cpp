#include "igWindowUI.h"
#include "generation.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "params.h"
#include "pipeline.h"
#include <mutex>
#include <opencv2/imgcodecs.hpp>
#include <thread>

static const char* IMAGE_FILTER = "Image Files\0*.jpg;*.jpeg;*.png;*.bmp\0All Files\0*.*\0";

igWindowUI::igWindowUI(igWindow* pWindow, igCanvasRenderer* pRenderer)
  : m_pWindow(pWindow), m_pRenderer(pRenderer)
{
  IMGUI_CHECKVERSION();
  m_pCtx = ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsDark();
  ImGui_ImplWin32_Init(m_pWindow->Handle());
  ImGui_ImplDX11_Init(m_pRenderer->Device(), m_pRenderer->Context());
}

igWindowUI::~igWindowUI()
{
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext(m_pCtx);
}

void igWindowUI::Render()
{
  // Pick up whatever the generation thread drew since last frame
  if (canvasDirty)
  {
    std::lock_guard<std::mutex> lock(canvasMutex);
    m_pRenderer->UpdateCanvasTexture(pendingCanvas, pendingW, pendingH);
    canvasDirty = false;
  }

  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  // main window, fullscreen
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
  ImGui::Begin("Geometrise", nullptr,
               ImGuiWindowFlags_NoTitleBar |
               ImGuiWindowFlags_NoResize |
               ImGuiWindowFlags_NoMove);

  DrawControls();
  ImGui::SameLine();
  DrawCanvas();

  ImGui::End();

  m_pRenderer->BeginFrame(0.1f, 0.1f, 0.1f);
  ImGui::Render();
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
  m_pRenderer->EndFrame();
}

void igWindowUI::DrawControls()
{
  ImGui::BeginChild("Controls", ImVec2(300, 0), true);

  ImGui::Text("Geometrise");
  ImGui::Separator();

  // file picker
  if (ImGui::Button("Select Image", ImVec2(260, 40)))
  {
    m_selectedFilepath = m_pWindow->OpenFileDialog(IMAGE_FILTER);
  }

  if (!m_selectedFilepath.empty())
  {
    std::string fname = m_selectedFilepath.substr(m_selectedFilepath.find_last_of("/\\") + 1);
    ImGui::TextWrapped("File: %s", fname.c_str());
  }
  else
  {
    ImGui::TextDisabled("No image selected");
  }

  ImGui::Separator();
  ImGui::Text("Parameters");
  ImGui::SliderInt("Shapes", &NUM_SHAPES, 1, 500);
  ImGui::SliderInt("Gen Shapes", &GENERATION_SHAPES, 1, 500);
  ImGui::SliderInt("Generations", &NUM_GENERATIONS, 1, 10);

  ImGui::Separator();

  if (!m_generating)
  {
    if (ImGui::Button("Generate", ImVec2(260, 50)) && !m_selectedFilepath.empty())
    {
      StartGeneration();
    }
  }
  else
  {
    ImGui::ProgressBar((float)currentShape / NUM_SHAPES, ImVec2(260, 20));
    ImGui::Text("Generating shape %d / %d", (int)currentShape, NUM_SHAPES);
  }

  ImGui::EndChild();
}

void igWindowUI::DrawCanvas()
{
  ImGui::BeginChild("Canvas", ImVec2(0, 0), true);

  if (m_pRenderer->CanvasTexture())
  {
    // Fit the image to the panel, keeping its aspect ratio
    ImVec2 available = ImGui::GetContentRegionAvail();
    float aspectRatio = (float)m_imageWidth / m_imageHeight;
    float displayW = available.x;
    float displayH = displayW / aspectRatio;
    if (displayH > available.y)
    {
      displayH = available.y;
      displayW = displayH * aspectRatio;
    }
    ImGui::Image((ImTextureID)(intptr_t)m_pRenderer->CanvasTexture(), ImVec2(displayW, displayH));
  }
  else
  {
    ImGui::TextDisabled("Canvas will appear here");
  }

  ImGui::EndChild();
}

void igWindowUI::StartGeneration()
{
  cv::Mat img = cv::imread(m_selectedFilepath);
  if (img.empty())
  {
    return;
  }

  IMG_WIDTH = img.cols;
  IMG_HEIGHT = img.rows;
  m_imageWidth = IMG_WIDTH;
  m_imageHeight = IMG_HEIGHT;

  std::vector<ShapePoint> squareMask;
  create_mask(squareMask, "shapes/rectangle.png", 0.9);
  m_masks = { squareMask };
  m_canvas = createCanvas(IMG_WIDTH, IMG_HEIGHT);
  currentShape = 0;
  m_generating = true;

  // Detached: draw_shapes can't be cancelled, so closing mid-generation just exits the process
  std::thread([this]() {
    draw_shapes(m_canvas, m_masks, m_selectedFilepath, NUM_SHAPES);
    m_generating = false;
  }).detach();
}
