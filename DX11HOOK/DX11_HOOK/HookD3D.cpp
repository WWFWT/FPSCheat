#include "HookD3D.h"

void (*DrawMenuCallBack)();
void (*ESP_CallBack)();

ID3D11Device* pDevice = nullptr;
IDXGISwapChain* pSwapchain = nullptr;
ID3D11DeviceContext* pContext = nullptr;
ID3D11RenderTargetView* pRenderTargetView = nullptr;
ID3D11Buffer* pConstantBuffer = nullptr;
ImDrawList* DrawList = nullptr;

DWORD WinWidth = 0;
DWORD WinHeight = 0;
DWORD WinXMid = 0;
DWORD WinYMid = 0;
HWND GameWindow = 0;
RECT GameWinRect = { 0 };
bool EnableESP = true;
bool ShowMenu = true;
bool ShowCircle = true;
float WinX_Offset = -7.56f;
float WinY_Offset = -18.78f;

static WNDPROC oWndProc = NULL;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Changing this to an array of viewports
#define MAINVP 0
D3D11_VIEWPORT pViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE]{ 0 };
XMMATRIX mOrtho;

HRESULT __stdcall hkPresent(IDXGISwapChain* pThis, UINT SyncInterval, UINT Flags);
using fnPresent = HRESULT(__stdcall*)(IDXGISwapChain* pThis, UINT SyncInterval, UINT Flags);
void* ogPresent;					// Pointer to the original Present function
fnPresent ogPresentTramp;			// Function pointer that calls the Present stub in our trampoline
void* pTrampoline = nullptr;		// Pointer to jmp instruction in our trampoline that leads to hkPresent
char ogBytes[PRESENT_STUB_SIZE];	// Buffer to store original bytes from Present

LRESULT CALLBACK hkWindowProc(
	_In_ HWND   hwnd,
	_In_ UINT   uMsg,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam) > 0)
		return 1L;
	return ::CallWindowProcA(oWndProc, hwnd, uMsg, wParam, lParam);
}

bool HookD3D()
{
	// Create a dummy device, get swapchain vmt, hook present.
	D3D_FEATURE_LEVEL featLevel;
	DXGI_SWAP_CHAIN_DESC sd{ 0 };
	sd.BufferCount = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.Height = 800;
	sd.BufferDesc.Width = 600;
	sd.BufferDesc.RefreshRate = { 60, 1 };
	sd.OutputWindow = GetForegroundWindow();
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_REFERENCE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &sd, &pSwapchain, &pDevice, &featLevel, nullptr);
	if (FAILED(hr))
		return false;

	// Get swapchain vmt
	void** pVMT = *(void***)pSwapchain;

	// Get Present's address out of vmt
	ogPresent = (fnPresent)(pVMT[VMT_PRESENT]);

	// got what we need, we can release device and swapchain now
	// we'll be stealing the game's.
	safe_release(pSwapchain);
	safe_release(pDevice);

	// Create a code cave to trampoline to our hook
	// We'll try to do this close to present to make sure we'll be able to use a 5 byte jmp (important for x64)
	void* pLoc = (void*)((uintptr_t)ogPresent - 0x2000);
	void* pTrampLoc = nullptr;
	while (!pTrampLoc)
	{
		pTrampLoc = VirtualAlloc(pLoc, 1, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		pLoc = (void*)((uintptr_t)pLoc + 0x200);
	}
	ogPresentTramp = (fnPresent)pTrampLoc;

	// write original bytes to trampoline
	// write jmp to hook
	memcpy(ogBytes, ogPresent, PRESENT_STUB_SIZE);
	memcpy(pTrampLoc, ogBytes, PRESENT_STUB_SIZE);

	pTrampLoc = (void*)((uintptr_t)pTrampLoc + PRESENT_STUB_SIZE);

	// write the jmp back into present
	*(char*)pTrampLoc = (char)0xE9;
	pTrampLoc = (void*)((uintptr_t)pTrampLoc + 1);
	uintptr_t ogPresRet = (uintptr_t)ogPresent + 5;
	*(int*)pTrampLoc = (int)(ogPresRet - (uintptr_t)pTrampLoc - 4);

	// write the jmp to our hook
	pTrampoline = pTrampLoc = (void*)((uintptr_t)pTrampLoc + 4);
#ifdef _WIN64
	// if x64, gazzillion byte absolute jmp
	char pJmp[] = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00 };
	WriteMem(pTrampLoc, pJmp, ARRAYSIZE(pJmp));
	pTrampLoc = (void*)((uintptr_t)pTrampLoc + ARRAYSIZE(pJmp));
	*(uintptr_t*)pTrampLoc = (uintptr_t)hkPresent;
#else
	// if x86, normal 0xE9 jmp
	* (char*)pTrampLoc = (char)0xE9;
	pTrampLoc = (void*)((uintptr_t)pTrampLoc + 1);
	*(int*)pTrampLoc = (uintptr_t)hkPresent - (uintptr_t)pTrampLoc - 4;
#endif

	// hook present, place a normal mid-function at the beginning of the Present function
	return Util::Hook(ogPresent, pTrampoline, PRESENT_STUB_SIZE);
}

bool InitD3DHook(IDXGISwapChain* pSwapchain)
{
	HRESULT hr = pSwapchain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice);
	if (FAILED(hr))
		return false;

	pDevice->GetImmediateContext(&pContext);
	pContext->OMGetRenderTargets(1, &pRenderTargetView, nullptr);

	// If for some reason we fail to get a render target, create one.
	// This will probably never happen with a real game but maybe certain test environments... :P
	if (!pRenderTargetView)
	{
		// Get a pointer to the back buffer for the render target view
		ID3D11Texture2D* pBackbuffer = nullptr;
		hr = pSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackbuffer));
		if (FAILED(hr))
			return false;

		// Create render target view
		hr = pDevice->CreateRenderTargetView(pBackbuffer, nullptr, &pRenderTargetView);
		pBackbuffer->Release();
		if (FAILED(hr))
			return false;

		// Make sure our render target is set.
		pContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);
	}


	DXGI_SWAP_CHAIN_DESC desc;
	pSwapchain->GetDesc(&desc);
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(desc.OutputWindow);
	ImGui_ImplDX11_Init(pDevice, pContext);
	GameWindow = desc.OutputWindow;

	ImGui::StyleColorsDark();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\simsun.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
	if (font == nullptr) MessageBox(nullptr, L"×ÖÌå¼ÓÔØ³ö´í", L"error", MB_OK);

	oWndProc = (WNDPROC)::SetWindowLongPtr((HWND)desc.OutputWindow, GWLP_WNDPROC, (LONG)hkWindowProc);
	DrawList = ImGui::GetBackgroundDrawList();


	UINT numViewports = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	float fWidth = 0;
	float fHeight = 0;

	// Apparently this isn't universal. Works on some games
	pContext->RSGetViewports(&numViewports, pViewports);

	if (!numViewports || !pViewports[MAINVP].Width)
	{
		// This should be retrieved dynamically
		//HWND hWnd0 = FindWindowA( "W2ViewportClass", nullptr );
		HWND hWnd = Util::FindMainWindow(GetCurrentProcessId());
		RECT rc{ 0 };
		if (!GetClientRect(hWnd, &rc))
			return false;

		//fWidth = 1600.0f;
		//fHeight = 900.0f;
		fWidth = (float)rc.right;
		fHeight = (float)rc.bottom;

		// Setup viewport
		pViewports[MAINVP].Width = (float)fWidth;
		pViewports[MAINVP].Height = (float)fHeight;
		pViewports[MAINVP].MinDepth = 0.0f;
		pViewports[MAINVP].MaxDepth = 1.0f;

		// Set viewport to context
		pContext->RSSetViewports(1, pViewports);
	}
	else
	{
		fWidth = (float)pViewports[MAINVP].Width;
		fHeight = (float)pViewports[MAINVP].Height;
	}
	
	// Create the constant buffer
	D3D11_BUFFER_DESC bd{ 0 };
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.Usage = D3D11_USAGE_DEFAULT;

	// Setup orthographic projection
	mOrtho = XMMatrixOrthographicLH(fWidth, fHeight, 0.0f, 1.0f);
	ConstantBuffer cb;
	cb.mProjection = mOrtho;

	D3D11_SUBRESOURCE_DATA sr{ 0 };
	sr.pSysMem = &cb;
	hr = pDevice->CreateBuffer(&bd, &sr, &pConstantBuffer);
	if (FAILED(hr))
		return false;

	return true;
}

void CleanupD3D()
{
	safe_release(pConstantBuffer);
}

void Render()
{
	// Make sure our render target is set.
	pContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);

	// Update view
	ConstantBuffer cb;
	cb.mProjection = XMMatrixTranspose(mOrtho);
	pContext->UpdateSubresource(pConstantBuffer, 0, nullptr, &cb, 0, 0);
	pContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);

	// Set viewport to context
	pContext->RSSetViewports(1, pViewports);
}

HRESULT __stdcall hkPresent(IDXGISwapChain* pThis, UINT SyncInterval, UINT Flags)
{
	pSwapchain = pThis;

	if (!pDevice)
	{
		if (!InitD3DHook(pThis))
			return false;

	}

	Render();

	GetWindowRect(GameWindow, &GameWinRect);
	WinWidth = GameWinRect.right - GameWinRect.left;
	WinHeight = GameWinRect.bottom - GameWinRect.top;
	WinYMid = WinHeight / 2 + WinY_Offset;
	WinXMid = WinWidth / 2 + WinX_Offset;
	


	if (GetKeyState(VK_HOME) & 0x8000){
		ShowMenu = !ShowMenu;
		Sleep(200);
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	
	if(ShowMenu && DrawMenuCallBack) DrawMenuCallBack();
	if (EnableESP && ESP_CallBack) ESP_CallBack();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return ogPresentTramp(pThis, SyncInterval, Flags);
}

void DestroyHook()
{
	SetWindowLongPtr(GameWindow, GWLP_WNDPROC, (LONG)oWndProc);
	Util::WriteMem(ogPresent, ogBytes, PRESENT_STUB_SIZE);
	VirtualFree((void*)ogPresentTramp, 0x1000, MEM_RELEASE);
	CleanupD3D();
}

#pragma warning(push)
#pragma warning(disable:4244)
void DrawBox(int x, int y, int w, int h, ImVec4 color, float T)
{
	DrawList->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(color), 0, 0, T);
}

void DrawCircle(int x, int y, float r, ImVec4 color, float T)
{
	DrawList->AddCircle(ImVec2(x, y), r, ImGui::ColorConvertFloat4ToU32(color), 0, T);
}

void DrawLine(int x1, int y1, int x2, int y2, ImVec4 color, float T)
{
	DrawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImGui::ColorConvertFloat4ToU32(color), T);
}

void DrawD3DText(int x, int y, const char* str, ImVec4 color)
{
	DrawList->AddText(ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(color), str);
}

void DrawFillRect(int x, int y, int w, int h, ImVec4 color)
{
	DrawList->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(color), 0, 0);
}

#pragma warning(pop)