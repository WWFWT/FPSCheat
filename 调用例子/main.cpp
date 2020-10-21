#include"GameCheat.h"
#pragma comment(lib,"外挂库.lib")

DWORD pid=0;
UINT64 模块基址, 矩阵地址;
vector<UINT64> TempArray,对象数组, 载具数组, 物品数组, 物品箱数组, 空投数组;
HANDLE hProcess;
HWND hwnd;

RGBA BoxColor{ 200, 200, 10, 255 };
RGBA LineColor{ 0, 0, 139, 255 };
RGBA TextColor{ 139, 0, 0, 255 };
RGBA CircleColor{ 0, 139, 139, 255 };

bool 绘制方框 = true;
bool 绘制射线 = true;
bool 绘制物品 = true;


static bool alpha_preview = true;
static bool alpha_half_preview = false;
static bool drag_and_drop = true;
static bool options_menu = true;
static bool hdr = false;
ImGuiColorEditFlags misc_flags = (hdr ? ImGuiColorEditFlags_HDR : 0) | (drag_and_drop ? 0 : ImGuiColorEditFlags_NoDragDrop) | (alpha_half_preview ? ImGuiColorEditFlags_AlphaPreviewHalf : (alpha_preview ? ImGuiColorEditFlags_AlphaPreview : 0)) | (options_menu ? 0 : ImGuiColorEditFlags_NoOptions);

void GetData()
{
	while (1)
	{
		//这里获取对象数据 先放到TempArray，之后再放入对象数组，否则在读取的时候可能出错
		对象数组.clear();
		for (vector<UINT64>::iterator it = TempArray.begin(); it != TempArray.end(); it++)
			对象数组.push_back(*it);
	}
}

void Draw()
{
	UINT64 对象地址;
	Rect rect;
	for (int i=0;i<对象数组.size();i++)
	{
		if(i<对象数组.size())	对象地址 =对象数组[i];

		WorldToScreen(hProcess, 矩阵地址, 对象地址, 1.8, &rect, true);
		if(绘制方框) DrawBox(rect.x, rect.y, rect.w, rect.h, BoxColor,2);
		if(绘制射线) DrawLine(MyImGui::win_width / 2, 0, rect.x, rect.y, LineColor, 2);
	}
}

void DrawMenu()
{
	static ImVec4 color = ImVec4(200.0/255.0,200.0/255.0,10.0/255.0,1);
	//ImGui::ShowDemoWindow(&ShowImGui);
	if (ImGui::CollapsingHeader(u8"功能选项"))
	{
		ImGui::Checkbox(u8"绘制方框", &绘制方框);
		ImGui::Checkbox(u8"绘制射线", &绘制射线); ImGui::SameLine(150);
		ImGui::Checkbox(u8"绘制物品", &绘制物品); ImGui::SameLine(300);
		ImGui::ColorEdit4(u8"方框颜色", (float*)&color, ImGuiColorEditFlags_Float | misc_flags);
		ImGui::ColorEdit4(u8"线条颜色", (float*)&color, ImGuiColorEditFlags_Float | misc_flags);
	}
}

int main()
{
	pid=GetProcessIdByName(L"LdVBoxHeadless.exe");
	hwnd = FindWindowExW(FindWindowW(NULL,L"雷电模拟器"),NULL,L"RenderWindow",L"TheRender");
	hProcess=MyOpenProcess(PROCESS_ALL_ACCESS,false,pid);

	cout << "PID:" << pid << endl;
	cout << "窗口句柄:" << hwnd <<endl;

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GetData, NULL, 0, NULL);
	//ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
	CreateWindowAndInitImGui(hwnd, Draw, DrawMenu);

	return 0;
}