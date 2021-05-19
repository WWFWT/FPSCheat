#pragma once
#include<Windows.h>
#include <string>
#include <vector>

//π§æﬂ¿‡
using namespace std;
class Util
{
public:
	static DWORD FindProcessId(const std::wstring& processName);
	static uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName);
	static void AutoAim(float x, float y, float ScreenCenterX, float ScreenCenterY, int AimSpeed);
	static float GetDistance(float Target_X, float Target_Y, float ScreenCenterX, float ScreenCenterY);
};

