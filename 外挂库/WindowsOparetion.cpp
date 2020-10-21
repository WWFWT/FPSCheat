#include"WindowsOperation.h"

WORD GetTzmArray(char* Tzm, WORD* TzmArray)
{
	int len = 0;
	WORD TzmLength = strlen(Tzm) / 3 + 1;

	for (int i = 0; i < strlen(Tzm); )//将十六进制特征码转为十进制
	{
		char num[2];
		num[0] = Tzm[i++];
		num[1] = Tzm[i++];
		i++;
		if (num[0] != '?' && num[1] != '?')
		{
			int sum = 0;
			WORD a[2];
			for (int i = 0; i < 2; i++)
			{
				if (num[i] >= '0' && num[i] <= '9')
				{
					a[i] = num[i] - '0';
				}
				else if (num[i] >= 'a' && num[i] <= 'z')
				{
					a[i] = num[i] - 87;
				}
				else if (num[i] >= 'A' && num[i] <= 'Z')
				{
					a[i] = num[i] - 55;
				}

			}
			sum = a[0] * 16 + a[1];
			TzmArray[len++] = sum;
		}
		else
		{
			TzmArray[len++] = 256;
		}
	}
	return TzmLength;
}

void GetSundayNextArray(short* next, WORD* Tzm, WORD TzmLength)
{
	for (int i = 0; i < 260; i++)
		next[i] = -1;
	for (int i = 0; i < TzmLength; i++)
		next[Tzm[i]] = i;
}

void SearchMemoryBlock(HANDLE hProcess, WORD* Tzm, WORD TzmLength, unsigned __int64 StartAddress, unsigned long size, BYTE* MemoryData,short *Next,vector<unsigned __int64>& ResultArray)
{
	if (!ReadProcessMemory(hProcess, (LPCVOID)StartAddress, MemoryData, size, NULL))
	{
		return;
	}

	for (int i = 0, j, k; i < size;)
	{
		j = i; k = 0;

		for (; k < TzmLength && j < size && (Tzm[k] == MemoryData[j] || Tzm[k] == 256); k++, j++);

		if (k == TzmLength)
		{
			ResultArray.push_back(StartAddress + i);
		}

		if ((i + TzmLength) >= size)
		{
			return;
		}

		int num = Next[MemoryData[i + TzmLength]];
		if (num == -1)
			i += (TzmLength - Next[256]);//如果特征码有问号，就从问号处开始匹配，如果没有就i+=-1
		else
			i += (TzmLength - num);
	}
}

DWORD SearchMemory(HANDLE hProcess, char* Tzm, unsigned __int64 StartAddress, unsigned __int64 EndAddress, int InitSize, vector<unsigned __int64>& ResultArray)
{
	int i = 0;
	unsigned long BlockSize;
	MEMORY_BASIC_INFORMATION mbi;
	BYTE *MemoryData=new BYTE[BLOCKMAXSIZE];
	short Next[260];

	WORD TzmLength = strlen(Tzm) / 3 + 1;
	WORD* TzmArray = new WORD[TzmLength];

	GetTzmArray(Tzm, TzmArray);
	GetSundayNextArray(Next, TzmArray, TzmLength);

	//初始化结果数组
	ResultArray.clear();
	ResultArray.reserve(InitSize);

	while (VirtualQueryEx(hProcess, (LPCVOID)StartAddress, &mbi, sizeof(mbi)) != 0)
	{
		//获取可读可写和可读可写可执行的内存块
		if (mbi.Protect == PAGE_READWRITE || mbi.Protect == PAGE_EXECUTE_READWRITE)
		{
			i = 0;
			BlockSize = mbi.RegionSize;
			//搜索这块内存
			while (BlockSize >= BLOCKMAXSIZE)
			{
				SearchMemoryBlock(hProcess, TzmArray, TzmLength, StartAddress + (BLOCKMAXSIZE * i), BLOCKMAXSIZE,MemoryData,Next,ResultArray);
				BlockSize -= BLOCKMAXSIZE; i++;
			}
			SearchMemoryBlock(hProcess, TzmArray, TzmLength, StartAddress + (BLOCKMAXSIZE * i), BlockSize, MemoryData, Next,ResultArray);

		}
		StartAddress += mbi.RegionSize;

		if (EndAddress != 0 && StartAddress > EndAddress)
		{
			return ResultArray.size();
		}
	}
	delete TzmArray;
	delete MemoryData;
	return ResultArray.size();
}

UINT GetProcessIdByName(LPCTSTR ProcessName)
{
	UINT nProcessID = 0;
	PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		if (Process32First(hSnapshot, &pe))
		{
			while (Process32Next(hSnapshot, &pe))
			{
				if (lstrcmpi(ProcessName, pe.szExeFile) == 0)
				{
					nProcessID = pe.th32ProcessID;
					break;
				}
			}
		}
	}
	CloseHandle(hSnapshot);
	return nProcessID;
}

void AutoAim(float ScreenMid_X, float ScreenMid_Y, float Target_X, float Target_Y, int Radius, int Speed)
{
	float x, y;
	x = abs(Target_X - ScreenMid_X);
	y = abs(Target_Y - ScreenMid_Y);
	if (x > Radius || y > Radius || Speed <= 0) return;
	mouse_event(1, (Target_X - ScreenMid_X) / Speed, (Target_Y - ScreenMid_Y) / Speed, 3, 3);
}

int WorldToScreen(HANDLE hprocess, unsigned __int64 MatrixAddress, unsigned __int64 EnemyAddress_X, float EnemyHigth, Rect* rect, bool Middle_Z)
{
	float Internal;
	float MatrixArray[4][4];
	D3D_3D EnemyCoordinates;
	
	ReadProcessMemory(hprocess, (LPVOID)MatrixAddress, (LPVOID)MatrixArray, 64, NULL);
	ReadProcessMemory(hprocess, (LPVOID)EnemyAddress_X, &EnemyCoordinates, sizeof(D3D_3D), NULL);

	int ScreenMiddle_x = MyImGui::win_width / 2;
	int ScreenMiddle_y = MyImGui::win_height / 2;

	if (Middle_Z)
	{
		float a = EnemyCoordinates.y;
		EnemyCoordinates.y = EnemyCoordinates.z;
		EnemyCoordinates.z = a;
	}

	float x, y1, y2;
	if (Middle_Z)
	{
		Internal = MatrixArray[0][3] * EnemyCoordinates.x + MatrixArray[1][3] * EnemyCoordinates.z + MatrixArray[2][3] * EnemyCoordinates.y + MatrixArray[3][3];
		if (Internal < 0.01) return -1;
		rect->distance = Internal;
		Internal = 1 / Internal;

		x = ScreenMiddle_x + (MatrixArray[0][0] * EnemyCoordinates.x + MatrixArray[1][0] * EnemyCoordinates.z + MatrixArray[2][0] * EnemyCoordinates.y + MatrixArray[3][0]) * Internal * ScreenMiddle_x;
		y1 = ScreenMiddle_y - (MatrixArray[0][1] * EnemyCoordinates.x + MatrixArray[1][1] * (EnemyCoordinates.z + EnemyHigth) + MatrixArray[2][1] * EnemyCoordinates.y + MatrixArray[3][1]) * Internal * ScreenMiddle_y;
		y2 = ScreenMiddle_y - (MatrixArray[0][1] * EnemyCoordinates.x + MatrixArray[1][1] * EnemyCoordinates.z + MatrixArray[2][1] * EnemyCoordinates.y + MatrixArray[3][1]) * Internal * ScreenMiddle_y;
	}
	else
	{
		Internal = MatrixArray[0][3] * EnemyCoordinates.x + MatrixArray[1][3] * EnemyCoordinates.y + MatrixArray[2][3] * EnemyCoordinates.z + MatrixArray[3][3];

		if (Internal < 0.01)return -1;
		rect->distance = Internal;
		Internal = 1 / Internal;

		x = ScreenMiddle_x + (MatrixArray[0][0] * EnemyCoordinates.x + MatrixArray[1][0] * EnemyCoordinates.y + MatrixArray[2][0] * EnemyCoordinates.z + MatrixArray[3][0]) * Internal * ScreenMiddle_x;
		y1 = ScreenMiddle_y - (MatrixArray[0][1] * EnemyCoordinates.x + MatrixArray[1][1] * EnemyCoordinates.y + MatrixArray[2][1] * (EnemyCoordinates.z + EnemyHigth) + MatrixArray[3][1]) * Internal * ScreenMiddle_y;
		y2 = ScreenMiddle_y - (MatrixArray[0][1] * EnemyCoordinates.x + MatrixArray[1][1] * EnemyCoordinates.y + MatrixArray[2][1] * EnemyCoordinates.z + MatrixArray[3][1]) * Internal * ScreenMiddle_y;
	}


	rect->h = y2 - y1;
	rect->w = rect->h / 2;
	rect->x = x - rect->h / 4;
	rect->y = y1;
	return 0;
}