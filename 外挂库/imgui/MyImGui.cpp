#include"MyImGui.h"
ImTextureID Load_Image(wchar_t* path)
{
    ImTextureID tid;
    auto iter=MyImGui::ImagePath.find(path);

    if (iter == MyImGui::ImagePath.end())
    {
        tid= DX11LoadTextureImageFromFile(path);
        MyImGui::ImagePath.insert(map<const wchar_t*, ImTextureID>::value_type(path, tid));
        return tid;
    }
    else
    {
        return MyImGui::ImagePath[path];
    }
}

void DrawImage(wchar_t* path,int x,int y,int w,int h)
{
    ImGui::GetOverlayDrawList()->AddImage(Load_Image(path), ImVec2(w, h),ImVec2(x,y));
}

void DrawBox(int x, int y, int w, int h, RGBA color, float T)
{
    ImGui::GetOverlayDrawList()->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(color.R / 255.0, color.G / 255.0, color.B / 255.0, color.A / 255.0)), 0, 0, T);
}

void DrawCircle(int x, int y, float r, RGBA color, float T)
{
    ImGui::GetOverlayDrawList()->AddCircle(ImVec2(x,y),r, ImGui::ColorConvertFloat4ToU32(ImVec4(color.R / 255.0, color.G / 255.0, color.B / 255.0, color.A / 255.0)), 0, T);
}

void DrawLine(int x1, int y1, int x2, int y2, RGBA color, float T)
{
    ImGui::GetOverlayDrawList()->AddLine(ImVec2(x1,y1),ImVec2(x2,y2),ImGui::ColorConvertFloat4ToU32(ImVec4(color.R / 255.0, color.G / 255.0, color.B / 255.0, color.A / 255.0)),T);
}

void Draw_Text(int x, int y,const char* str, RGBA color)
{
    ImGui::GetBackgroundDrawList()->AddText(ImVec2(x,y), ImGui::ColorConvertFloat4ToU32(ImVec4(color.R / 255.0, color.G / 255.0, color.B / 255.0, color.A / 255.0)),str);
}

void CreateWindowAndInitImGui(HWND GameWindow,Render Draw_,Render Draw_Menu)
{
    MyImGui::GameHwnd = GameWindow;
    MyImGui::Draw = Draw_;
    MyImGui::DrawMenu = Draw_Menu;
    //初始化窗口类
    MyImGui::wClass.cbClsExtra = NULL;
    MyImGui::wClass.cbSize = sizeof(WNDCLASSEX);
    MyImGui::wClass.cbWndExtra = NULL;
    MyImGui::wClass.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
    MyImGui::wClass.hCursor = LoadCursor(0, IDC_ARROW);
    MyImGui::wClass.hIcon = LoadIcon(0, IDI_APPLICATION);
    MyImGui::wClass.hIconSm = LoadIcon(0, IDI_APPLICATION);
    MyImGui::wClass.hInstance = GetModuleHandle(NULL);
    MyImGui::wClass.lpfnWndProc = (WNDPROC)WndProc;
    MyImGui::wClass.lpszClassName = L" ";
    MyImGui::wClass.lpszMenuName = L" ";
    MyImGui::wClass.style = CS_VREDRAW | CS_HREDRAW;
    if (RegisterClassEx(&MyImGui::wClass) == 0)
    {
        MessageBox(NULL, L"创建窗口出错！", L"提示！", 0);
        exit(1);
    }

    //创建窗口
    GetWindowRect(MyImGui::GameHwnd, &MyImGui::window_rect);
    MyImGui::win_width = MyImGui::window_rect.right - MyImGui::window_rect.left;
    MyImGui::win_height = MyImGui::window_rect.bottom - MyImGui::window_rect.top;
    MyImGui::hwnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST, L" ", L" ", WS_POPUP, 1, 1, MyImGui::win_width, MyImGui::win_height, 0, 0, 0, 0);

    if (!CreateDeviceD3D(MyImGui::hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(MyImGui::wClass.lpszClassName, MyImGui::wClass.hInstance);
        MessageBox(NULL, L"ImGui初始化失败!", L"error", MB_OK);
        exit(1);
    }

    SetLayeredWindowAttributes(MyImGui::hwnd, 0, RGB(0, 0, 0), LWA_COLORKEY);
    ShowWindow(MyImGui::hwnd, SW_SHOW);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\simsun.ttc", 20.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
    if(font == NULL) MessageBox(NULL, L"字体加载出错", L"error", MB_OK);
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(MyImGui::hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
    MessageWhile();
    return;
}

bool laststate = true;
void ChangeWindowStyle()
{
    bool estate = KEY_DOWN(VK_HOME);
    
    if (!laststate && estate)
    {
        MyImGui::ShowMenu = !MyImGui::ShowMenu;
        if (MyImGui::ShowMenu)
        {
            SetWindowLong(MyImGui::hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TOPMOST);
            SetWindowPos(MyImGui::hwnd, HWND_TOPMOST, 100, 100, 100, 100, SWP_NOMOVE | SWP_NOSIZE);
        }
        else
        {
            SetWindowLong(MyImGui::hwnd, GWL_EXSTYLE, WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOPMOST);
            SetWindowPos(MyImGui::hwnd, HWND_TOPMOST, 100, 100, 100, 100, SWP_NOMOVE | SWP_NOSIZE);
        }
    }
    laststate = estate;
}

void MessageWhile()
{
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        if (MyImGui::GameHwnd)
        {
            GetWindowRect(MyImGui::GameHwnd, &MyImGui::window_rect);
            MyImGui::win_width = MyImGui::window_rect.right - MyImGui::window_rect.left;
            MyImGui::win_height = MyImGui::window_rect.bottom - MyImGui::window_rect.top;
            DWORD dwStyle = GetWindowLong(MyImGui::GameHwnd, GWL_STYLE);
            if (dwStyle & WS_BORDER)
            {
                MyImGui::window_rect.top += 23;
                MyImGui::win_height -= 23;
            }
            MoveWindow(MyImGui::hwnd, MyImGui::window_rect.left, MyImGui::window_rect.top, MyImGui::win_width, MyImGui::win_height, true);
        }

        if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

       //这里绘制
        ChangeWindowStyle();
        MyImGui::Draw();
        if (MyImGui::ShowMenu) MyImGui::DrawMenu();

        // Rendering
        ImGui::Render();
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&MyImGui::clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(MyImGui::hwnd);
    ::UnregisterClass(MyImGui::wClass.lpszClassName, MyImGui::wClass.hInstance);

    for (vector<HANDLE>::iterator it = MyImGui::ThreadHandle.begin(); it != MyImGui::ThreadHandle.end(); it++)
    {
        TerminateThread(*it,1);
        CloseHandle(*it);
    }
    MyImGui::ThreadHandle.clear();
}

bool CreateDeviceD3D(HWND hWnd)
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
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
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


ID3D11ShaderResourceView* DX11LoadTextureImageFromFile(wchar_t* lpszFilePath)
{
    ID3D11Texture2D* pTexture2D = NULL;
    D3D11_TEXTURE2D_DESC dec;

    HRESULT result;
    D3DX11_IMAGE_LOAD_INFO loadInfo;
    ZeroMemory(&loadInfo, sizeof(D3DX11_IMAGE_LOAD_INFO));
    loadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    loadInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    loadInfo.MipLevels = D3DX11_DEFAULT; //这时会产生最大的mipmaps层。 
    loadInfo.MipFilter = D3DX11_FILTER_LINEAR;
    result = D3DX11CreateTextureFromFile(g_pd3dDevice, lpszFilePath, &loadInfo, NULL, (ID3D11Resource**)(&pTexture2D), NULL);
    pTexture2D->GetDesc(&dec);

    if (result != S_OK)
    {
        return NULL;
    }

    ID3D11ShaderResourceView* pFontTextureView = NULL;

    // Create texture view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = dec.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    g_pd3dDevice->CreateShaderResourceView(pTexture2D, &srvDesc, &pFontTextureView);


    return pFontTextureView;
}
