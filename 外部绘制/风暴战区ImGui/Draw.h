#pragma once
#include<Windows.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <dinput.h>
#include <tchar.h>
#include "Global.h"

#define DBT_DEVNODES_CHANGED 0x0007

//»æÖÆ
class Draw
{
public:
	ImVec4 BoxColor = { 1.0f,0.3f,0.6f,0.8f};
	ImVec4 FillRectColor = { 1.0f,0.0f,0.0f,0.8f};
	ImVec4 LineColor = { 0.6f,0.8f,0.2f,0.8f };
	ImVec4 TextColor = { 0.6f,1.0f,0.9f,0.8f };

	ImDrawList* DrawList = nullptr;

	ID3D11Device* g_pd3dDevice = nullptr;
	ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
	IDXGISwapChain* g_pSwapChain = nullptr;
	ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

	void (*DrawMenu)() = nullptr;
	void (*ESP)() = nullptr;

public:
	HWND CreateTransparenceWindows();
	void ClickThrough(bool click);

	void DrawText(int x, int y, const char* str, ImVec4 color);
	void DrawLine(int x1, int y1, int x2, int y2, ImVec4 color, float T);
	void DrawCircle(int x, int y, float r, ImVec4 color, float T);
	void DrawBox(int x, int y, int w, int h, ImVec4 color, float T);
	void DrawFillRect(int x, int y, int w, int h, ImVec4 color);

	bool InitImGui();
	void SetStyle();
	void RunImGuiWindow();
	bool CreateDeviceD3D();
	void CleanupDeviceD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();
	bool WorldToScreen(UINT64 Enemyaddr, PBox2D box, float EnemyHigth);
	
};

extern Draw draw;

