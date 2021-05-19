#pragma once
#include "Util.h"

void DestroyHook();
bool HookD3D();

void DrawD3DText(int x, int y, const char* str, ImVec4 color);
void DrawLine(int x1, int y1, int x2, int y2, ImVec4 color, float T);
void DrawCircle(int x, int y, float r, ImVec4 color, float T);
void DrawBox(int x, int y, int w, int h, ImVec4 color, float T);
void DrawFillRect(int x, int y, int w, int h, ImVec4 color);

extern void (*DrawMenuCallBack)();
extern void (*ESP_CallBack)();
extern DWORD WinXMid;
extern DWORD WinYMid;
extern bool EnableESP;
extern bool ShowMenu;
extern bool ShowCircle;
extern float WinX_Offset;
extern float WinY_Offset;
