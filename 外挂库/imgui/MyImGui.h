#pragma once
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include<D3DX11tex.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>
#include<iostream>
#include<vector>
#include<map>
#pragma comment ( lib, "D3D11.lib")
#pragma comment ( lib, "d3dx11.lib")
using namespace std;

#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)

typedef void(*Render)();
typedef void(*Response)();

typedef struct {
	BYTE R,G,B,A;
}RGBA;

static ID3D11Device* g_pd3dDevice = NULL;
static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
static IDXGISwapChain* g_pSwapChain = NULL;
static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

namespace MyImGui
{
	static HWND hwnd;
	static RECT window_rect;
	static int win_height, win_width;
	static WNDCLASSEX wClass;
	static HWND GameHwnd;
	static Render Draw,DrawMenu;
	static ImVec4 clear_color = ImVec4(0, 0, 0, 0);
	static vector<HANDLE> ThreadHandle;
	static bool ShowMenu = true;
	static map<const wchar_t*,ImTextureID> ImagePath;
}

void DrawBox(int x, int y, int w, int h, RGBA color, float T);
void DrawCircle(int x, int y, float r, RGBA color, float T);
void DrawLine(int x1, int y1, int x2, int y2, RGBA color, float T);
void Draw_Text(int x,int y,const char *str, RGBA color);
void DrawImage(wchar_t *path, int x, int y, int w, int h);
ImTextureID Load_Image(wchar_t* path);

void CreateWindowAndInitImGui(HWND GameWindow, Render Draw_, Render Draw_Menu);
void MessageWhile();
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void MonitoringHotkey(Response fun, int key_code, int time);
ID3D11ShaderResourceView* DX11LoadTextureImageFromFile(wchar_t* lpszFilePath);