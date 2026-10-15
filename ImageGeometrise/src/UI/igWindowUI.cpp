#include "igWindowUI.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <vector>

static const char* IMAGE_FILTER = "Image Files\0*.jpg;*.jpeg;*.png;*.bmp\0All Files\0*.*\0";

namespace
{
  namespace impl
  {
    // Relative to the working directory (the repo root when run from VS)
    inline const char* shapesFolder = "shapes";

    // "C:/a/b\\c.png" -> "c.png", either slash
    std::string FileName(const std::string& path)
    {
      return path.substr(path.find_last_of("/\\") + 1);
    }

    bool IsImage(const std::filesystem::path& path)
    {
      std::string ext = path.extension().string();
      std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return char(std::tolower(c)); });
      return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp";
    }
  }
}

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
}

igWindowUI::~igWindowUI()
{
  m_generation.Stop();
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext(m_pCtx);
}

void igWindowUI::AddMask(std::string path)
{
  if (path.empty())
    return; // cancelled the file dialog

  const std::string name = impl::FileName(path);
  m_maskError = m_generation.AddMask(std::move(path)) ? "" : "Couldn't load " + name;
}

void igWindowUI::DrawMasks()
{
  ImGui::SeparatorText("Masks");

  const auto& masks = m_generation.GetMasks();
  if (masks.empty())
    ImGui::TextDisabled("No masks yet, add one below");

  // One row per mask: name on the left, remove button on the right
  int removeIndex = -1;
  for (size_t i = 0; i < masks.size(); i++)
  {
    const igMask& mask = *masks[i];
    ImGui::PushID(int(i));

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(impl::FileName(mask.id).c_str());
    if (ImGui::IsItemHovered())
      ImGui::SetTooltip("%s\n%lld x %lld", mask.id.c_str(), mask.Width(), mask.Height());

    ImGui::SameLine(ImGui::GetContentRegionAvail().x + ImGui::GetCursorStartPos().x - 60);
    ImGui::BeginDisabled(m_generation.IsRunning());
    if (ImGui::Button("Remove", ImVec2(60, 0)))
      removeIndex = int(i);
    ImGui::EndDisabled();
    if (m_generation.IsRunning() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
      ImGui::SetTooltip("Can't remove masks while generating");

    ImGui::PopID();
  }

  // Remove after the loop so the list doesn't change under it
  if (removeIndex >= 0)
    m_generation.RemoveMask(size_t(removeIndex));

  // Add from the shapes/ folder (listed fresh each time the dropdown opens)
  ImGui::SetNextItemWidth(260 - 70 - ImGui::GetStyle().ItemSpacing.x);
  if (ImGui::BeginCombo("##addShape", "Add from shapes/"))
  {
    std::error_code error;
    bool any = false;
    for (const auto& entry : std::filesystem::directory_iterator(impl::shapesFolder, error))
    {
      if (!entry.is_regular_file() || !impl::IsImage(entry.path()))
        continue;

      any = true;
      if (ImGui::Selectable(entry.path().filename().string().c_str()))
        AddMask(entry.path().string());
    }
    if (!any)
      ImGui::TextDisabled("Nothing in %s/", impl::shapesFolder);
    ImGui::EndCombo();
  }

  ImGui::SameLine();
  if (ImGui::Button("Browse", ImVec2(70, 0)))
    AddMask(m_pWindow->OpenFileDialog(IMAGE_FILTER));

  if (!m_maskError.empty())
    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", m_maskError.c_str());

  if (m_generation.IsRunning())
    ImGui::TextDisabled("New masks are used from the next run");
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
                   ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoBringToFrontOnFocus); // keeps Diagnostics on top

  DrawControls();
  ImGui::SameLine();
  DrawCanvas();

  ImGui::End();

  if (m_showDiagnostics)
  {
    DrawDiagnostics();
  }

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
  ImGui::SliderInt("Shapes", &settings.numShapes, 1, 5000);
  ImGui::SliderInt("Gen Shapes", &settings.generationShapes, 1, 10000);
  ImGui::SliderInt("Generations", &settings.numGenerations, 1, 25);

  static const bool gpuAvailable = igGenerationBatch::Available();
  ImGui::BeginDisabled(!gpuAvailable);
  ImGui::Checkbox("Use GPU (CUDA)", &settings.useGpu);
  ImGui::EndDisabled();
  if (!gpuAvailable)
    ImGui::TextDisabled("No CUDA device found");

  DrawMasks();

  ImGui::Separator();

  if (!m_generation.IsRunning())
  {
    const bool canStart = !m_selectedFilepath.empty() && !m_generation.GetMasks().empty();
    ImGui::BeginDisabled(!canStart);
    if (ImGui::Button("Generate", ImVec2(260, 50)))
    {
      m_generation.Start(m_selectedFilepath);
    }
    ImGui::EndDisabled();
    if (!canStart)
      ImGui::TextDisabled(m_selectedFilepath.empty() ? "Pick an image first" : "Add a mask first");
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

  ImGui::Separator();
  ImGui::Checkbox("Diagnostics", &m_showDiagnostics);

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

void igWindowUI::DrawDiagnostics()
{
  ImGui::SetNextWindowSize(ImVec2(460, 420), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Diagnostics", &m_showDiagnostics))
  {
    ImGui::End();
    return;
  }

  const std::vector<igShapeTiming> timings = m_generation.Timings();
  if (timings.empty())
  {
    ImGui::TextDisabled("Run a generation to see timings");
    ImGui::End();
    return;
  }

  std::vector<float> total, create, grade, gradePerCandidate;
  float sumTotal = 0.0f, sumCreate = 0.0f, sumGrade = 0.0f;
  int sumCandidates = 0;
  for (const igShapeTiming& t : timings)
  {
    total.push_back(t.totalMs);
    create.push_back(t.createMs);
    grade.push_back(t.gradeMs);
    gradePerCandidate.push_back(t.candidates ? t.gradeMs * 1000.0f / t.candidates : 0.0f);
    sumTotal += t.totalMs;
    sumCreate += t.createMs;
    sumGrade += t.gradeMs;
    sumCandidates += t.candidates;
  }

  const float sumOther = (std::max)(0.0f, sumTotal - sumCreate - sumGrade);
  ImGui::Text("Grading on: %s", timings.back().gpu ? "GPU (CUDA)" : "CPU");
  ImGui::Text("%d shapes, %.2f s total, %.1f shapes/s", (int)timings.size(), sumTotal / 1000.0f,
              timings.size() * 1000.0f / (std::max)(sumTotal, 0.001f));
  ImGui::Text("Avg %.2f ms/shape, %.1f us per graded candidate", sumTotal / timings.size(),
              sumGrade * 1000.0f / (std::max)(sumCandidates, 1));
  ImGui::Text("Time split: grade %.0f%%, create %.0f%%, other %.0f%%",
              100.0f * sumGrade / sumTotal, 100.0f * sumCreate / sumTotal, 100.0f * sumOther / sumTotal);

  ImGui::Separator();

  // One graph per series, x = shape index
  auto plot = [](const char* label, const std::vector<float>& values, const char* unit)
  {
    char overlay[64];
    snprintf(overlay, sizeof(overlay), "last %.2f %s", values.back(), unit);
    const float maxValue = *std::max_element(values.begin(), values.end());
    ImGui::Text("%s", label);
    ImGui::PlotLines((std::string("##") + label).c_str(), values.data(), (int)values.size(), 0,
                     overlay, 0.0f, maxValue * 1.1f, ImVec2(-1, 60));
  };

  plot("Total per shape", total, "ms");
  plot("Grading per shape", grade, "ms");
  plot("Creating candidates per shape", create, "ms");
  plot("Grading per candidate", gradePerCandidate, "us");

  ImGui::End();
}
