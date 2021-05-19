#include "Draw.h"
#include "offset.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (draw.g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
            draw.CleanupRenderTarget();
            draw.g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            draw.CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

HWND Draw::CreateTransparenceWindows()
{
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, TEXT(" "), nullptr };
    ::RegisterClassEx(&wc);
    globals.wc = wc;

    globals.AppWindow = ::CreateWindow(wc.lpszClassName, TEXT(" "), WS_EX_TOPMOST | WS_POPUP, static_cast<int>(globals.GameWinRect.left * 0.05f),
        static_cast<int>(globals.GameWinRect.top * 0.05f), globals.WinWidth, globals.WinHeight, nullptr, nullptr, wc.hInstance, nullptr);

    //globals.AppWindow = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED, L" ", L" ", WS_POPUP, 1, 1, globals.WinWidth, globals.WinHeight, 0, 0, 0, 0);

    ClickThrough(false);
    SetLayeredWindowAttributes(globals.AppWindow, RGB(0, 0, 0), 0, ULW_COLORKEY);
    //SetLayeredWindowAttributes(globals.AppWindow, 0, 255, LWA_ALPHA);

    if (!InitImGui()) {
        return 0;
    }

    // Show the window
    ::ShowWindow(globals.AppWindow, SW_SHOWDEFAULT);
    ::UpdateWindow(globals.AppWindow);

    return globals.AppWindow;
}

bool Draw::InitImGui()
{
    // Initialize Direct3D
    if (!CreateDeviceD3D())
    {
        CleanupDeviceD3D();
        ::UnregisterClass(globals.wc.lpszClassName, globals.wc.hInstance);
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\simsun.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
    if (font == nullptr) MessageBox(nullptr, L"×ÖÌå¼ÓÔØ³ö´í", L"error", MB_OK);
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(globals.AppWindow);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(globals.WinWidth / 3.0f, globals.WinHeight / 2.0f), ImGuiCond_FirstUseEver);

    DrawList = ImGui::GetBackgroundDrawList();

    return true;
}

bool Draw::CreateDeviceD3D()
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = globals.AppWindow;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void Draw::CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void Draw::RunImGuiWindow()
{
    MSG msg;
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.00f);

    SetStyle();

    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        if (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            continue;
        }

        ::SetWindowPos(globals.AppWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
       
        if (GetKeyState(VK_HOME) & 0x8000)
        {
            globals.ShowMenu = !globals.ShowMenu;
            ClickThrough(!globals.ShowMenu);
            Sleep(200);
        }

        if (globals.GameWindow)
        {
            GetWindowRect(globals.GameWindow, &globals.GameWinRect);
            globals.WinWidth = globals.GameWinRect.right - globals.GameWinRect.left;
            globals.WinHeight = globals.GameWinRect.bottom - globals.GameWinRect.top;
            DWORD dwStyle = GetWindowLong(globals.GameWindow, GWL_STYLE);
            if (dwStyle & WS_BORDER)
            {
                globals.GameWinRect.top += 23;
                globals.WinHeight -= 23;
            }
            MoveWindow(globals.AppWindow, globals.GameWinRect.left, globals.GameWinRect.top, globals.WinWidth, globals.WinHeight, true);
        }

        if (this->DrawMenu && globals.ShowMenu) this->DrawMenu();
        if (this->ESP) this->ESP();

        // Rendering
        ImGui::Render();
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * 255.0f), (int)(clear_color.y * 255.0f), (int)(clear_color.z * 255.0f), (int)(clear_color.w * 255.0f));
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clear_col_dx);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    globals.Running = false;

    CleanupDeviceD3D();
    ::DestroyWindow(globals.AppWindow);
    ::UnregisterClass(globals.wc.lpszClassName, globals.wc.hInstance);
}

void Draw::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void Draw::CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

void Draw::ClickThrough(bool click)
{
    if (click)
    {
        SetWindowLong(globals.AppWindow, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT);
    }
    else
    {
        SetWindowLong(globals.AppWindow, GWL_EXSTYLE, WS_EX_LAYERED);
    }
}

void Draw::SetStyle()
{
    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    style->FrameBorderSize = 1.0f;
    style->WindowRounding = 7.0f;
    colors[ImGuiCol_Text] = { 129.0f / 255.0f,166.0f / 255.0f,24.0f / 255.0f,0.8f };
    //colors[ImGuiCol_WindowBg] = { 0.7f,0,0,0 };
    colors[ImGuiCol_Border] = { 20.0f / 255.0f,20.0f / 255.0f,142.0f / 255.0f,169.0f / 255.0f };
    colors[ImGuiCol_FrameBg] = { 6.0f / 255.0f,40.0f / 255.0f,82.0f / 255.0f,228.0f / 255.0f };
}

bool Draw::WorldToScreen(UINT64 Enemyaddr, PBox2D box, float EnemyHigth)
{
    float x, y1, y2;
    float Internal;
    float MatrixArray[4][4];

    ReadProcessMemory(globals.GameHandle, (PVOID)(globals.MatrixAddr), MatrixArray, 64, nullptr);

    Vector3D Enemy = { 0 };
    ReadProcessMemory(globals.GameHandle, (PVOID)(Enemyaddr + Offset::UObjectVector3D), &Enemy, 12, nullptr);

    Internal = MatrixArray[0][3] * Enemy.x + MatrixArray[1][3] * Enemy.y + MatrixArray[2][3] * Enemy.z + MatrixArray[3][3];
    if (Internal < -0.01f) {
        return false;
    }
    box->distance = Internal;
    Internal = 1 / Internal;

    x = globals.WinXMid + (MatrixArray[0][0] * Enemy.x + MatrixArray[1][0] * Enemy.y + MatrixArray[2][0] * Enemy.z + MatrixArray[3][0]) * Internal * globals.WinXMid;
    y1 = globals.WinYMid - (MatrixArray[0][1] * Enemy.x + MatrixArray[1][1] * Enemy.y + MatrixArray[2][1] * (Enemy.z + EnemyHigth) + MatrixArray[3][1]) * Internal * globals.WinYMid;
    y2 = globals.WinYMid - (MatrixArray[0][1] * Enemy.x + MatrixArray[1][1] * Enemy.y + MatrixArray[2][1] * Enemy.z + MatrixArray[3][1]) * Internal * globals.WinYMid;

    box->x = x;
    box->y = y1;
    box->h = y2 - y1;
    box->w = box->h / 2;
    return true;
}

#pragma warning(push)
#pragma warning(disable:4244)
void Draw::DrawBox(int x, int y, int w, int h, ImVec4 color, float T)
{
    DrawList->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(color), 0, 0, T);
}

void Draw::DrawCircle(int x, int y, float r, ImVec4 color, float T)
{
    DrawList->AddCircle(ImVec2(x, y), r, ImGui::ColorConvertFloat4ToU32(color), 0, T);
}

void Draw::DrawLine(int x1, int y1, int x2, int y2, ImVec4 color, float T)
{
    DrawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImGui::ColorConvertFloat4ToU32(color), T);
}

void Draw::DrawText(int x, int y, const char* str, ImVec4 color)
{
    DrawList->AddText(ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(color), str);
}

void Draw::DrawFillRect(int x, int y, int w, int h, ImVec4 color)
{
    DrawList->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(color), 0, 0);
}

#pragma warning(pop)