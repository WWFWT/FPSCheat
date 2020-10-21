#pragma once
#include<windows.h>
#include<iostream>
#include<vector>
#include<Tlhelp32.h>
#include"./imgui/MyImGui.h"

using namespace std;

#define BLOCKMAXSIZE 409600//每次读取内存的最大大小

typedef struct
{
	float x;
	float y;
	float z;
}D3D_3D;

typedef struct
{
	int x;
	int y;
	int w;
	int h;
	float distance;
}Rect;

EXTERN_C HANDLE MyOpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId);
EXTERN_C BOOL MyReadProcessMemory(
	HANDLE hProcess,
	LPCVOID lpBaseAddress,
	LPVOID lpBuffer,
	DWORD nSize,
	LPDWORD lpNumberOfBytesRead
);

EXTERN_C BOOL MyNtWriteVirtualMemory(
	HANDLE hProcess,
	LPCVOID lpBaseAddress,
	LPVOID lpBuffer,
	DWORD nSize,
	LPDWORD lpNumberOfBytesRead
);

static HMODULE hNtdll, hKernelBase;

int WorldToScreen(HANDLE hprocess, unsigned __int64 MatrixAddress, unsigned __int64 EnemyAddress_X, float EnemyHigth, Rect* rect, bool Middle_Z);
UINT GetProcessIdByName(LPCTSTR ProcessName);
void AutoAim(float ScreenMid_X, float ScreenMid_Y, float Target_X, float Target_Y, int Radius, int Speed);
WORD GetTzmArray(char* Tzm, WORD* TzmArray);
void GetSundayNextArray(short* next, WORD* Tzm, WORD TzmLength);
void SearchMemoryBlock(HANDLE hProcess, WORD* Tzm, WORD TzmLength, unsigned __int64 StartAddress, unsigned long size, BYTE* MemoryData, short* Next, vector<unsigned __int64>& ResultArray);
DWORD SearchMemory(HANDLE hProcess, char* Tzm, unsigned __int64 StartAddress, unsigned __int64 EndAddress, int InitSize, vector<unsigned __int64>& ResultArray);