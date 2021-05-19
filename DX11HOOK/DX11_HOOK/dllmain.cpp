// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "HookD3D.h"
#include <set>
#include <process.h>
#include <mutex>
#include "../minhook/MinHook.h"

#if defined _M_X64
#pragma comment(lib, "../minhook/libMinHook.x64.lib")
#elif defined _M_IX86
#pragma comment(lib, "../minhook/libMinHook.x86.lib")
#endif

#define GetKeyDown(keycode) GetKeyState(keycode) & 0x8000



set<DWORD> players;
DWORD ModuleBase = 0;
ImVec4 BoxColor = { 1.0f,0.3f,0.6f,0.8f };
ImVec4 FillRectColor = { 1.0f,0.0f,0.0f,0.8f };
ImVec4 LineColor = { 0.6f,0.8f,0.2f,0.8f };
ImVec4 TextColor = { 0.6f,1.0f,0.9f,0.8f };
DWORD MatrixAddr = 0;
bool AutoAim = true;
float AutoAimAensitivity = 6;
DWORD AutoAimKey = VK_RBUTTON;
CRITICAL_SECTION PlayerListCS;
float RGD = 5.0f;




PVOID PGetPlayerVecAddr = nullptr;
PVOID PSetLocalPlayerVec = nullptr;
typedef void (*FGetPlayerVecAddr)(DWORD, DWORD, DWORD, DWORD, DWORD);



//HOOK游戏内某个读取所有玩家坐标的函数，这样就不需要去找对象数组了
void GetPlayerVecAddr(DWORD VecAddr,DWORD p2,DWORD p3,DWORD p4,DWORD p5)
{
	EnterCriticalSection(&PlayerListCS);
	if(strcmp((char*)(VecAddr+0x30),"player_avatar")) players.insert(VecAddr);
	LeaveCriticalSection(&PlayerListCS);
	((FGetPlayerVecAddr)PGetPlayerVecAddr)(VecAddr, p2, p3, p4, p5);
}

//这里进行菜单绘制
void DrawMenu()
{
	if (!ImGui::Begin(u8"badlanders", nullptr, ImGuiWindowFlags_None))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

	ImGui::Text(u8"HOME键 开/关 菜单 | END键退出 | %.1f FPS", ImGui::GetIO().Framerate);

	if (ImGui::CollapsingHeader(u8"功能选项"))
	{
		ImGui::Checkbox(u8"透视", &EnableESP);
		ImGui::Checkbox(u8"自瞄", &AutoAim);
	}	

	if (ImGui::CollapsingHeader(u8"透视选项"))
	{
		ImGui::ColorEdit4(u8"方框颜色##3", reinterpret_cast<PFLOAT>(&BoxColor), ImGuiColorEditFlags__OptionsDefault);
		ImGui::ColorEdit4(u8"线条颜色##3", reinterpret_cast<PFLOAT>(&LineColor), ImGuiColorEditFlags__OptionsDefault);
		ImGui::ColorEdit4(u8"字体颜色##3", reinterpret_cast<PFLOAT>(&TextColor), ImGuiColorEditFlags__OptionsDefault);
	}

	if (ImGui::CollapsingHeader(u8"自瞄选项"))
	{
		ImGui::Text(u8"数值越大灵敏度越低");
		ImGui::SliderFloat(u8"灵敏度", &AutoAimAensitivity, 20.0f, 1.0f, "%.2f");
		ImGui::RadioButton(u8"鼠标右键", reinterpret_cast<PINT>(&AutoAimKey), VK_RBUTTON); ImGui::SameLine();
		ImGui::RadioButton(u8"Ctrl键", reinterpret_cast<PINT>(&AutoAimKey), VK_LCONTROL); ImGui::SameLine();
		ImGui::RadioButton(u8"Shift键", reinterpret_cast<PINT>(&AutoAimKey), VK_LSHIFT);
	}

	if (ImGui::CollapsingHeader(u8"校准"))
	{
		ImGui::Checkbox(u8"显示中心圆", &ShowCircle);
		ImGui::SliderFloat(u8"屏幕X偏移", &WinX_Offset, -30.0f, 30.0f, "%.2f");
		ImGui::SliderFloat(u8"屏幕Y偏移", &WinY_Offset, -30.0f, 30.0f, "%.2f");
	}

	ImGui::End();
}

void ESP()
{
	static DWORD move = 0;
	static Box2D box = { 0 };
	static float MinDistance = 0;
	static DWORD target = 0;

	for (DWORD i : players)
	{
		Vector3D pos = *(PVector3D)i;
		if ((int)pos.x == 0 && (int)pos.y==0 && (int)pos.z==0) {
			move = i;
			continue;
		}

		//矩阵地址
		MatrixAddr = *(PDWORD)(ModuleBase + 0x293DC0C);

		//画框画线
		if (!Util::WorldToScreen(WinXMid,WinYMid, MatrixAddr,pos, &box, 2)) continue;
		DrawBox(box.x - box.w/2, box.y, box.w, box.h,BoxColor,2);
		DrawLine(WinXMid,WinYMid,box.x, box.y+box.h,LineColor,1);

#ifdef _DEBUG
		char caddr[20] = { 0 };
		_i64toa_s(i-0x50, caddr, 20, 16);
		DrawD3DText(box.x, box.y, caddr, TextColor);
#endif // _DEBUG

		if ((GetKeyState(AutoAimKey) & 0x8000)) {
			static float distance = 0;
			distance = Util::GetDistance(box.x - box.w, box.y, WinXMid, WinYMid);
			if (static_cast<int>(MinDistance) == 0)
			{
				MinDistance = distance;
				target = i;
			}
			else if (distance < MinDistance)
			{
				MinDistance = distance;
				target = i;
			}
		} else {
			MinDistance = 0;
		}
	}

	//自瞄
	if ((GetKeyState(AutoAimKey) & 0x8000) && AutoAim) {
		Vector3D pos = *(PVector3D)target;
		Box2D TargetBox = { 0 };
		if (Util::WorldToScreen(WinXMid, WinYMid, MatrixAddr, pos, &TargetBox, 2))
		{
			for (int i = 0; i < 12; i++) {
				Util::AutoAim(TargetBox.x, TargetBox.y + TargetBox.h / 1.5, WinXMid, WinYMid, AutoAimAensitivity);
			}
		}
	}

	//去除已经无效的地址
	if (move) {
		EnterCriticalSection(&PlayerListCS);
		players.erase(move);
		LeaveCriticalSection(&PlayerListCS);
	}

	if (ShowCircle)	DrawCircle(WinXMid, WinYMid, 23, LineColor, 3);
}

void HookGame()
{
	if (MH_Initialize() != MH_OK) {
		DBG_LOG("MinHook初始化失败\n");
		return;
	}

	
	MH_CreateHook((PVOID)(ModuleBase + 目标函数偏移), &GetPlayerVecAddr(代理函数), &PGetPlayerVecAddr（原函数）);

	if (MH_EnableHook((PVOID)(ModuleBase + 目标函数偏移)) != MH_OK
		) {
		DBG_LOG("Hook 失败\n");
		return;
	}
	
}

void UnHookGame()
{
	MH_DisableHook((PVOID)(ModuleBase + 目标函数偏移));
	MH_Uninitialize();
}

void MainThread(HINSTANCE hinstDLL)
{
//DEBUG模式下开启控制台窗口进行打印
#ifdef _DEBUG
	AllocConsole();
	freopen("CON", "w", stdout);
	SetConsoleTitleA("Debug");
#endif // _DEBUG

	DrawMenuCallBack = DrawMenu;
	ESP_CallBack = ESP;
	ModuleBase = (DWORD)GetModuleHandle(nullptr);
	if (!ModuleBase) {
		DBG_LOG("模块地址获取失败!");
		FreeLibraryAndExitThread(hinstDLL,0);
		return;
	}
	InitializeCriticalSection(&PlayerListCS);

	//HOOK游戏的一些函数
	HookGame();



	// Hook d3d
	if (HookD3D())
	{
		// END key to unload
		while (!GetAsyncKeyState(VK_END)) 
		{
			if (GetKeyState(VK_F1) & 0x8000) {
				//监视F1是否按下
			}
		}
	}



	UnHookGame();
	DestroyHook();
#ifdef _DEBUG
	FreeConsole();
#endif // _DEBUG

	//卸载DLL并退出线程
	FreeLibraryAndExitThread(hinstDLL, 0);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hinstDLL);
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hinstDLL, 0, nullptr);
		break;
	case DLL_PROCESS_DETACH:

		break;
	}
	return TRUE;
}

