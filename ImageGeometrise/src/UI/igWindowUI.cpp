#include "igWindowUI.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

static const char* IMAGE_FILTER = "Image Files\0*.jpg;*.jpeg;*.png;*.bmp\0All Files\0*.*\0";

igWindowUI::igWindowUI(igWindow* pWindow, igCanvasRenderer* pRenderer)
  : m_pWindow(pWindow),
    m_pRenderer(pRenderer),
    m_generation(pRenderer)
{
  IMGUI_CHECKVERSION();
  m_pCtx = ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsDark();
  ImGui_ImplWin32_Init(m_pWindow->Handle());
  ImGui_ImplDX11_Init(m_pRenderer->Device(), m_pRenderer->Context());

  m_generation.AddMask("shapes/rectangle.png");
}

igWindowUI::~igWindowUI()
{
  m_generation.Stop();
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext(m_pCtx);
}

void igWindowUI::Render()
{
  // Pick up whatever the generation thread drew since last frame
  m_pRenderer->UpdateCanvas();

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
  igGenerationSettings& settings = m_generation.Settings();
  ImGui::SliderInt("Shapes", &settings.numShapes, 1, 500);
  ImGui::SliderInt("Gen Shapes", &settings.generationShapes, 1, 500);
  ImGui::SliderInt("Generations", &settings.numGenerations, 1, 10);

  ImGui::Separator();

  if (!m_generation.IsRunning())
  {
    if (ImGui::Button("Generate", ImVec2(260, 50)) && !m_selectedFilepath.empty())
    {
      m_generation.Start(m_selectedFilepath);
    }
  }
  else
  {
    const int current = m_generation.CurrentShape();
    const int total = m_generation.TotalShapes();
    ImGui::ProgressBar((float)current / total, ImVec2(260, 20));
    ImGui::Text("Generating shape %d / %d", current, total);
    if (ImGui::Button("Stop", ImVec2(260, 30)))
    {
      m_generation.Stop();
    }
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
    const igVec2 canvasSize = m_pRenderer->CanvasSize();
    float aspectRatio = (float)canvasSize.x / canvasSize.y;
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
