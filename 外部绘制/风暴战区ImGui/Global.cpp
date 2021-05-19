#include "Global.h"
#include"Util.h"

bool Global::InitGlobals(HWND GameWindow, DWORD GamePid)
{
	this->GameWindow = GameWindow;
	this->GamePid = GamePid;

	GetWindowRect(GameWindow, &GameWinRect);
	WinWidth = GameWinRect.right - GameWinRect.left;
	WinHeight = GameWinRect.bottom - GameWinRect.top;
	WinXMid = WinWidth / 2;
	WinYMid = WinHeight / 2;

	static const char* SFCPawnAI_Dummy = "SFCPawnAI_Dummy";
	ShowUNameList.push_back(SFCPawnAI_Dummy);
	static const char* SFCPawn = "SFCPawn";
	ShowUNameList.push_back(SFCPawn);

	MoudleBase = Util::GetModuleBaseAddress(GamePid, L"SFGame.exe");
	if (!MoudleBase) return false;

	GameHandle = OpenProcess(PROCESS_ALL_ACCESS, false, GamePid);
	if (!GameHandle) return false;

	return true;
}