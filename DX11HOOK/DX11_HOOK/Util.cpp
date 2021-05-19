#include "Util.h"
#include <TlHelp32.h>
#include <psapi.h>

HWND Util::FindMainWindow(DWORD dwPID)
{
	HandleData handleData{ 0 };
	handleData.pid = dwPID;
	EnumWindows(EnumWindowsCallback, (LPARAM)&handleData);
	return handleData.hWnd;
}

BOOL CALLBACK Util::EnumWindowsCallback(HWND hWnd, LPARAM lParam)
{
	HandleData& data = *(HandleData*)lParam;
	DWORD pid = 0;
	GetWindowThreadProcessId(hWnd, &pid);
	if (pid == data.pid && GetWindow(hWnd, GW_OWNER) == HWND(0) && IsWindowVisible(hWnd))
	{
		data.hWnd = hWnd;
		return FALSE;
	}

	return TRUE;
}

bool Util::Hook(void* pSrc, void* pDst, size_t size)
{
	DWORD dwOld;
	uintptr_t src = (uintptr_t)pSrc;
	uintptr_t dst = (uintptr_t)pDst;

	if (!VirtualProtect(pSrc, size, PAGE_EXECUTE_READWRITE, &dwOld))
		return false;

	*(char*)src = (char)0xE9;
	*(int*)(src + 1) = (int)(dst - src - 5);

	VirtualProtect(pSrc, size, dwOld, &dwOld);
	return true;
}

bool Util::WriteMem(void* pDst, char* pBytes, size_t size)
{
	DWORD dwOld;
	if (!VirtualProtect(pDst, size, PAGE_EXECUTE_READWRITE, &dwOld))
		return false;

	memcpy(pDst, pBytes, PRESENT_STUB_SIZE);

	VirtualProtect(pDst, size, dwOld, &dwOld);
	return true;
}

uintptr_t Util::GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 modEntry;
		modEntry.dwSize = sizeof(modEntry);
		if (Module32First(hSnap, &modEntry))
		{
			do
			{
				if (!_wcsicmp(modEntry.szModule, modName))
				{
					modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
					//wprintf(L"%s | %s | %p\n", modName, modEntry.szModule, modBaseAddr);
					break;
				}
			} while (Module32Next(hSnap, &modEntry));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

bool Util::WorldToScreen(int WinXMid,int WinYMid, uintptr_t MatrixAddr,Vector3D pos, PBox2D box, float EnemyHigth)
{
	float x, y1, y2;
	float Internal;

	static float MatrixArray[4][4] = { 0 };

	memcpy(MatrixArray, (PVOID)MatrixAddr, 64);
	Internal = MatrixArray[0][3] * pos.x + MatrixArray[1][3] * pos.y + MatrixArray[2][3] * pos.z + MatrixArray[3][3];
	if (Internal < -0.01f) {
		return false;
	}
	box->distance = Internal;
	Internal = 1 / Internal;

	x = WinXMid + (MatrixArray[0][0] * pos.x + MatrixArray[1][0] * pos.y + MatrixArray[2][0] * pos.z + MatrixArray[3][0]) * Internal * WinXMid;
	y1 = WinYMid - (MatrixArray[0][1] * pos.x + MatrixArray[1][1] * pos.y + MatrixArray[2][1] * pos.z + MatrixArray[3][1]) * Internal * WinYMid;
	y2 = WinYMid - (MatrixArray[0][1] * pos.x + MatrixArray[1][1] * (pos.y + EnemyHigth) + MatrixArray[2][1] * pos.z + MatrixArray[3][1]) * Internal * WinYMid;

	box->x = x;
	box->y = y1;
	box->h = y2 - y1;
	box->w = box->h / 2;

	return true;
}

void Util::AutoAim(float x, float y, float ScreenCenterX, float ScreenCenterY, int AimSpeed)
{

	float	TargetX = 0;
	float	TargetY = 0;

	if (x != 0)
	{
		if (x > ScreenCenterX)
		{
			TargetX = -(ScreenCenterX - x);
			TargetX /= AimSpeed;
			if (TargetX + ScreenCenterX > ScreenCenterX * 2) TargetX = 0;
		}

		if (x < ScreenCenterX)
		{
			TargetX = x - ScreenCenterX;
			TargetX /= AimSpeed;
			if (TargetX + ScreenCenterX < 0) TargetX = 0;
		}
	}

	if (y != 0)
	{
		if (y > ScreenCenterY)
		{
			TargetY = -(ScreenCenterY - y);
			TargetY /= AimSpeed;
			if (TargetY + ScreenCenterY > ScreenCenterY * 2) TargetY = 0;
		}

		if (y < ScreenCenterY)
		{
			TargetY = y - ScreenCenterY;
			TargetY /= AimSpeed;
			if (TargetY + ScreenCenterY < 0) TargetY = 0;
		}
	}

	//WriteAngles(TargetX / 3.5f, TargetY / 3.5f);
	mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(TargetX), static_cast<DWORD>(TargetY), NULL, NULL);

	return;
}

float Util::GetDistance(float Target_X, float Target_Y, float ScreenCenterX, float ScreenCenterY)
{
	float Len_X, Len_Y, Len;
	Len_X = ScreenCenterX - Target_X;
	Len_Y = ScreenCenterY - Target_Y;
	Len = sqrt(Len_X * Len_X + Len_Y * Len_Y);
	return Len;
}