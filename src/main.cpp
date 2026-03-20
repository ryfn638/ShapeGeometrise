#include <d3d11.h>
#include <tchar.h>
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include <windows.h>
#include <commdlg.h>
#include <thread>
#include <atomic>
#include <vector>
#include "include/arena.h"
#include "include/params.h"
#include "include/operations.h"
#include "include/generation.h"
#include "include/pipeline.h"
#include "include/draw.h"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include "imgui_impl_win32.h"

using namespace std;

// DirectX globals
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
static ID3D11ShaderResourceView* g_canvasTexture = nullptr;
static ID3D11Texture2D* g_canvasTex2D = nullptr;

// app state
static std::string selectedFilepath = "";
static std::atomic<bool> generating(false);
std::atomic<int>  currentShape(0);
static std::vector<Colour> sharedCanvas;
static std::mutex canvasMutex;

// forward declarations
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

std::string openFileDialog()
{
    char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = "Image Files\0*.jpg;*.jpeg;*.png;*.bmp\0All Files\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (GetOpenFileNameA(&ofn))
        return std::string(filename);
    return "";
}

void updateCanvasTexture(const std::vector<Colour>& canvas, int w, int h)
{
    if (!g_pd3dDevice) return;

    // create texture once, update after
    if (!g_canvasTex2D)
    {
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = w;
        desc.Height = h;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DYNAMIC;        // dynamic for frequent updates
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;     // allow CPU writes

        g_pd3dDevice->CreateTexture2D(&desc, nullptr, &g_canvasTex2D);
        g_pd3dDevice->CreateShaderResourceView(g_canvasTex2D, nullptr, &g_canvasTexture);
    }

    // map texture and write pixels directly without recreating
    D3D11_MAPPED_SUBRESOURCE mapped;
    g_pd3dDeviceContext->Map(g_canvasTex2D, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

    uint8_t* dst = (uint8_t*)mapped.pData;
    for (int y = 0; y < h; y++)
    {
        uint8_t* row = dst + y * mapped.RowPitch;
        for (int x = 0; x < w; x++)
        {
            const Colour& c = canvas[y * w + x];
            row[x * 4 + 0] = c.red;
            row[x * 4 + 1] = c.green;
            row[x * 4 + 2] = c.blue;
            row[x * 4 + 3] = 255;
        }
    }

    g_pd3dDeviceContext->Unmap(g_canvasTex2D, 0);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // create window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L,
                       GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
                       L"Geometrise", nullptr };
    RegisterClassExW(&wc);
    HWND hwnd = CreateWindowW(wc.lpszClassName, L"Geometrise",
        WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800,
        nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    // init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    std::vector<ShapePoint> squareMask;
    std::vector<std::vector<ShapePoint>> all_masks;
    std::vector<Colour> canvas;

    bool done = false;
    int ORIG_W = 0;
    int ORIG_H = 0;
    while (!done)
    {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
        if (done) break;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // main window — fullscreen
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Geometrise", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove);

        // left panel — controls
        ImGui::BeginChild("Controls", ImVec2(300, 0), true);

        ImGui::Text("Geometrise");
        ImGui::Separator();

        // file picker
        if (ImGui::Button("Select Image", ImVec2(260, 40)))
            selectedFilepath = openFileDialog();

        if (!selectedFilepath.empty()) {
            std::string fname = selectedFilepath.substr(
                selectedFilepath.find_last_of("/\\") + 1);
            ImGui::TextWrapped("File: %s", fname.c_str());
        }
        else {
            ImGui::TextDisabled("No image selected");
        }

        ImGui::Separator();
        ImGui::Text("Parameters");
        ImGui::SliderInt("Shapes", &NUM_SHAPES, 1, 500);
        ImGui::SliderInt("Gen Shapes", &GENERATION_SHAPES, 1, 500);
        ImGui::SliderInt("Generations", &NUM_GENERATIONS, 1, 10);

        ImGui::Separator();

     
        // generate button
        if (!generating) {
            // then in the generate button:
            if (ImGui::Button("Generate", ImVec2(260, 50)) && !selectedFilepath.empty()) {
                cv::Mat img = cv::imread(selectedFilepath);
                IMG_WIDTH = img.cols;
                IMG_HEIGHT = img.rows;
                ORIG_W = IMG_WIDTH;   // set here before thread starts
                ORIG_H = IMG_HEIGHT;

                squareMask.clear();
                create_mask(squareMask, "C:/Users/ryanf/Documents/Geometrise/shapes/rectangle.png", 0.9);
                all_masks = { squareMask };
                canvas = createCanvas(IMG_WIDTH, IMG_HEIGHT);
                sharedCanvas = canvas;
                currentShape = 0;
                generating = true;

                std::thread([&]() {
                    draw_shapes(canvas, all_masks, selectedFilepath, NUM_SHAPES);
                    generating = false;
                    }).detach();
            }
        }
        else {
            // progress bar

            ImGui::ProgressBar((float)currentShape / NUM_SHAPES,
                ImVec2(260, 20));
            ImGui::Text("Generating shape %d / %d", (int)currentShape, NUM_SHAPES);
        }

        ImGui::EndChild();

        ImGui::SameLine();

        // right panel — canvas
        ImGui::BeginChild("Canvas", ImVec2(0, 0), true);

        if (g_canvasTexture) {
            ImVec2 available = ImGui::GetContentRegionAvail();
            float aspectRatio = (float)IMG_WIDTH / IMG_HEIGHT;
            float displayW = available.x;
            float displayH = displayW / aspectRatio;
            if (displayH > available.y) {
                displayH = available.y;
                displayW = displayH * aspectRatio;
            }
            ImGui::Image((void*)g_canvasTexture, ImVec2(ORIG_W, ORIG_H));
        }
        else {
            ImGui::TextDisabled("Canvas will appear here");
        }

        ImGui::EndChild();
        ImGui::End();

        // render
        ImVec4 clearColor = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView,
            (float*)&clearColor);
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(1, 0);
    }

    // cleanup
    if (g_canvasTexture) g_canvasTexture->Release();
    if (g_canvasTex2D)   g_canvasTex2D->Release();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}

bool CreateDeviceD3D(HWND hWnd)
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
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {
        D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };

    if (D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        featureLevelArray, 2, D3D11_SDK_VERSION,
        &sd, &g_pSwapChain, &g_pd3dDevice,
        &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release();        g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release();        g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) {
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = nullptr;
    }
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, LOWORD(lParam), HIWORD(lParam),
                DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}