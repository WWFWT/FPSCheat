#pragma once
#include<Windows.h>
#include<vector>
#include<iostream>

using namespace std;

//所有全局变量

#define D3DCOLOR_ARGB(a,r,g,b) \
    ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#define D3DCOLOR_RGBA(r,g,b,a) D3DCOLOR_ARGB(a,r,g,b)

typedef struct UObjectData_ {
	UINT64 addr;
	char name[101] = { 0 };
}UObjectData, * PUObjectData;

typedef struct Box2D_ {
	float x;
	float y;
	float w;
	float h;
	float distance;
}Box2D, * PBox2D;

typedef struct Vector3D_ {
	float x;
	float y;
	float z;
}Vector3D, * PVector3D;

class Global
{
public:
	HWND GameWindow = 0;
	HWND AppWindow = 0;
	DWORD WinHeight = 0;
	DWORD WinWidth = 0;
	DWORD WinXMid = 0;
	DWORD WinYMid = 0;
	DWORD GamePid = 0;
	UINT64 MoudleBase = 0;
	UINT64 MatrixAddr = 0;
	HANDLE GameHandle = 0;
	RECT GameWinRect = { 0 };
	WNDCLASSEX wc = { 0 };
	bool ShowMenu = true;
	bool BanClick = false;
	bool Running = true;
	bool StartESP = true;
	bool AutoAim = true;

	float AutoAimAensitivity = 6;

	vector<const char*> ShowUNameList;
	vector<UObjectData> UObjectList;

	bool InitGlobals(HWND GameWindow,DWORD GamePid);
};

extern Global globals;

