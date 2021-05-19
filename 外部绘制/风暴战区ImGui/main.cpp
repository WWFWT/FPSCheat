#include"Draw.h"
#include"Util.h"
#include"offset.h"

Global globals;
Draw draw;
BOOL IsRedCamp = true;
DWORD AutoAimKey = VK_RBUTTON;
UINT64 AimObject = 0;
float MinDistance = 0;

#pragma warning(disable:4244)

int GetUObjectByName(bool ShowAllName = false)
{
	char name[100] = { 0 };

	vector<UObjectData> temp;

	PDWORD UObjectList = nullptr;
	PDWORD UNameList = nullptr;

	DWORD UObjectNum = 0;
	DWORD UNameNum = 0;

	ReadProcessMemory(globals.GameHandle, reinterpret_cast<PVOID>(Offset::GObject + 4), &UObjectNum, 4, nullptr);
	ReadProcessMemory(globals.GameHandle, reinterpret_cast<PVOID>(Offset::GName + 4), &UNameNum, 4, nullptr);

	/*ReadProcessMemory(globals.GameHandle, (PVOID)Offset::GObject, &UObjectList, 4, nullptr);
	ReadProcessMemory(globals.GameHandle, (PVOID)Offset::GName, &UNameList, 4, nullptr);*/

	UObjectList = new DWORD[UObjectNum];
	UNameList = new DWORD[UNameNum];
	if (!UObjectList || !UNameList) return 0;

	ZeroMemory(UObjectList, UObjectNum * 4);
	ZeroMemory(UNameList, UNameNum * 4);

	BOOL bret = true;
	PVOID tempAddr = 0;
	bret &= ReadProcessMemory(globals.GameHandle, reinterpret_cast<PVOID>(Offset::GObject), &tempAddr, 4, nullptr);
	bret &= ReadProcessMemory(globals.GameHandle, tempAddr, UObjectList, UObjectNum * 4, nullptr);
	bret &= ReadProcessMemory(globals.GameHandle, reinterpret_cast<PVOID>(Offset::GName), &tempAddr, 4, nullptr);
	bret &= ReadProcessMemory(globals.GameHandle, tempAddr, UNameList, UNameNum * 4, nullptr);
	if (!bret) {
		cout << "游戏数据读取失败！" << endl;
		return 0;
	}

	for (DWORD i = 0; i < UObjectNum; i++)
	{
		DWORD UObject = 0;
		DWORD UName = 0;
		DWORD index = 0;

		//ReadProcessMemory(globals.GameHandle, (PVOID)((UINT64)UObjectList + i * 4), &UObject, 4, nullptr);
		UObject = UObjectList[i];
		ReadProcessMemory(globals.GameHandle, reinterpret_cast<PVOID>(UObject + 0x2C), &index, 4, nullptr);
		if (index >= UNameNum) continue;

		//ReadProcessMemory(globals.GameHandle, (PVOID)(UNameList + index * 4), &UName, 4, nullptr);
		UName = UNameList[index];
		ReadProcessMemory(globals.GameHandle, reinterpret_cast<PVOID>(UName + 0x10), name, 50, nullptr);

		//cout << "Index:" << index << "  Name:" << name << endl;
		for (const auto& i : globals.ShowUNameList) {
			if (ShowAllName || !strcmp(i, name)) {
				//cout << "addr:" << hex << UObject << "  Name:" << hex << name << endl;
				UObjectData data = { 0 };
				data.addr = UObject;
				strcpy_s(data.name, name);
				temp.push_back(data);
			}
		}

		ZeroMemory(name,100);
	}

	delete[] UObjectList;
	delete[] UNameList;

	globals.UObjectList.clear();
	for (const auto& i : temp) {

		if (ShowAllName) {
			float temp = 0;
			ReadProcessMemory(globals.GameHandle, reinterpret_cast<PVOID>(i.addr + Offset::UObjectVector3D), &temp, 4, nullptr);
			if (abs(temp) > 0.5 && abs(temp) < 100000) globals.UObjectList.push_back(i);
			continue;
		}

		int heath = 0;
		float temp = 0;
		ReadProcessMemory(globals.GameHandle, reinterpret_cast<PVOID>(i.addr + Offset::UObjectVector3D), &temp, 4, nullptr);
		ReadProcessMemory(globals.GameHandle, reinterpret_cast<PVOID>(i.addr + Offset::UObjectHealth), &heath, 4, nullptr);
		if (abs(temp) < 0.1 || heath <= 0 || heath >= 200) continue;

		//cout << hex << i.addr << endl;
		globals.UObjectList.push_back(i);
	}

	return static_cast<size_t>(globals.UObjectList.size());
}

//绘制菜单
void DrawMenu()
{
	//ImGui::ShowDemoWindow(&globals.ShowMenu);

	if (!ImGui::Begin(u8"风暴战区", nullptr, ImGuiWindowFlags_None)){
		ImGui::End();
		return;
	}

	ImGui::Text(u8"HOME键 开/关 菜单 | END 键切换阵营");

	if (ImGui::CollapsingHeader(u8"功能选项"))
	{
		if (ImGui::BeginTable(u8"功能", 3))
		{
			ImGui::TableNextColumn(); ImGui::Checkbox(u8"透视", &globals.StartESP);
			ImGui::TableNextColumn(); ImGui::Checkbox(u8"自瞄", &globals.AutoAim);
			ImGui::EndTable();
		}
	}

	if (ImGui::CollapsingHeader(u8"透视选项"))
	{
		ImGui::ColorEdit4(u8"方框颜色##3", reinterpret_cast<PFLOAT>(&draw.BoxColor), ImGuiColorEditFlags__OptionsDefault);
		ImGui::ColorEdit4(u8"线条颜色##3", reinterpret_cast<PFLOAT>(&draw.LineColor), ImGuiColorEditFlags__OptionsDefault);
		ImGui::ColorEdit4(u8"字体颜色##3", reinterpret_cast<PFLOAT>(&draw.TextColor), ImGuiColorEditFlags__OptionsDefault);
	}

	if (ImGui::CollapsingHeader(u8"自瞄选项"))
	{
		ImGui::Text(u8"数值越大灵敏度越低");
		ImGui::SliderFloat(u8"灵敏度", &globals.AutoAimAensitivity, 20.0f, 1.0f, "%.2f", ImGuiColorEditFlags_None);
		ImGui::RadioButton(u8"鼠标右键", reinterpret_cast<PINT>(&AutoAimKey), VK_RBUTTON); ImGui::SameLine();
		ImGui::RadioButton(u8"Ctrl键", reinterpret_cast<PINT>(&AutoAimKey), VK_LCONTROL); ImGui::SameLine();
		ImGui::RadioButton(u8"Shift键", reinterpret_cast<PINT>(&AutoAimKey), VK_LSHIFT);
	}

	ImGui::End();
}

//透视绘制方框在这里写
void ESP()
{
	//draw.DrawBox(100, 100, 100, 200, draw.BoxColor, 2);
	//draw.DrawTextW(100, 100, "123456", draw.TextColor);

	if (!globals.StartESP) return;

	char show[100] = { 0 };

	for (int i = 0; i < globals.UObjectList.size(); i++) {
		UObjectData enemyData = { 0 };
		if (i > globals.UObjectList.size()) {
			break;
		}
		enemyData = globals.UObjectList[i];

		if (GetKeyState(VK_END) & 0x8000)
		{
			IsRedCamp = !IsRedCamp;
			Sleep(200);
		}

		//cout << hex << isTeam << ":" << hex << enemyData.addr + 0x5e8 << endl;
		DWORD heath = 0;
		BYTE RedCamp = 0;
		ReadProcessMemory(globals.GameHandle, reinterpret_cast<PVOID>(enemyData.addr + Offset::UObjectHealth), &heath, 4, nullptr);
		ReadProcessMemory(globals.GameHandle, reinterpret_cast<PVOID>(enemyData.addr + Offset::Camp), &RedCamp, 1, nullptr);
		if (!heath || RedCamp != IsRedCamp) {
			continue;
		}

		Box2D box = { 0 };
		if (!draw.WorldToScreen(enemyData.addr, &box, 75)) continue;
		draw.DrawBox(box.x - box.w, box.y, box.w * 2, box.h * 2, draw.BoxColor, 2);
		draw.DrawFillRect(box.x - box.w, box.y - box.h / 5, (static_cast<float>(heath) / 100.0)*(box.w * 2), box.h / 8, draw.FillRectColor);
	}
}

//死循环获取数据
void UpDate()
{
	while (globals.Running) 
	{
		DWORD addr = 0;
		ReadProcessMemory(globals.GameHandle, reinterpret_cast<PVOID>(globals.MoudleBase + Offset::MatrixOffset1), &addr, 4, nullptr);
		ReadProcessMemory(globals.GameHandle, reinterpret_cast<PVOID>(addr + Offset::MatrixOffset2), &addr, 4, nullptr);
		globals.MatrixAddr = addr + Offset::MatrixOffset3 + Offset::MatrixOffset4;
		
		GetUObjectByName();
	}
	
}

//自瞄
void AutoAimThread()
{
	while (globals.Running)
	{
		if (!globals.AutoAim) continue;

		for (int i = 0; i < globals.UObjectList.size(); i++) {
			UObjectData enemyData = { 0 };
			if (i > globals.UObjectList.size()) {
				break;
			}
			enemyData = globals.UObjectList[i];


			//cout << hex << isTeam << ":" << hex << enemyData.addr + 0x5e8 << endl;
			DWORD heath = 0;
			BYTE RedCamp = 0;
			ReadProcessMemory(globals.GameHandle, reinterpret_cast<PVOID>(enemyData.addr + Offset::UObjectHealth), &heath, 4, nullptr);
			ReadProcessMemory(globals.GameHandle, reinterpret_cast<PVOID>(enemyData.addr + Offset::Camp), &RedCamp, 1, nullptr);
			if (!heath || RedCamp != IsRedCamp) {
				continue;
			}

			Box2D box = { 0 };
			if (!draw.WorldToScreen(enemyData.addr, &box, 75)) continue;

			if (GetKeyState(AutoAimKey) & 0x8000)
			{
				float distance = 0;
				distance = Util::GetDistance(box.x - box.w, box.y, globals.WinXMid, globals.WinYMid);
				if (static_cast<int>(MinDistance) == 0)
				{
					MinDistance = distance;
					AimObject = enemyData.addr;
				}
				else if (distance < MinDistance)
				{
					MinDistance = distance;
					AimObject = enemyData.addr;
				}
			}
			else {
				MinDistance = 0;
			}

			if ((GetKeyState(AutoAimKey) & 0x8000) && AimObject == enemyData.addr) {
				Util::AutoAim(box.x, box.y + box.h / 2.5, globals.WinXMid, globals.WinYMid, globals.AutoAimAensitivity);
			}
		}
		Sleep(10);
	}
}

int main()
{
	DWORD pid = Util::FindProcessId(L"SFGame.exe");
	cout << "PID:" << pid << endl;

	HWND GameHwnd = FindWindow(L"LaunchUnrealUWindowsClient", L"风暴战区TF");
	cout << "游戏窗口:" << GameHwnd << endl;

	globals.InitGlobals(GameHwnd, pid);

	CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(UpDate), nullptr, 0, nullptr);
	CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(AutoAimThread), nullptr, 0, nullptr);

	draw.CreateTransparenceWindows();
	draw.DrawMenu = DrawMenu;
	draw.ESP = ESP;
	draw.RunImGuiWindow();
	return 0;
}