#include <Windows.h>
#include <iostream>
#include <vector>
#include <fstream>

enum em_Chess_Length
{
	Max_Row = 19,
	Max_Column = 11
};
BYTE ArrChess[Max_Column][Max_Row];

// Get GameWindow Handle
HWND GetGameWnd()
{
	return ::FindWindowW(NULL, L"QQ游戏 - 连连看角色版");
}

BOOL ReadDebugFile()
{
	WCHAR wszPath[MAX_PATH];
	::GetCurrentDirectoryW(MAX_PATH, wszPath);
	lstrcatW(wszPath, L"\\debug.dat");

	std::ifstream is(wszPath, std::ios::in | std::ios::out | std::ios::binary);
	if (!is.is_open())
	{
		is.close();
		return FALSE;
	}
	
	is.seekg(0, std::ios::beg);
	is.read(reinterpret_cast<CHAR*>(ArrChess[0]), sizeof(ArrChess));
	is.close();
	return TRUE;
}

void WriteDebugFile()
{
	WCHAR wszPath[MAX_PATH];
	::GetCurrentDirectoryW(MAX_PATH, wszPath);
	lstrcatW(wszPath, L"\\debug.dat");

	std::ofstream of(wszPath, std::ios::out | std::ios::binary | std::ios::trunc);
	if (!of.is_open())
	{
		std::wcout << L"Create File Faild:" << wszPath << std::endl;
		return;
	}
	of.seekp(std::ios::beg);
	of.write(reinterpret_cast<CONST CHAR*>(&ArrChess[0][0]), sizeof(ArrChess));
	of.close();
}

BOOL InitializeChess()
{
	DWORD dwPid = NULL;
	if (!::GetWindowThreadProcessId(GetGameWnd(), &dwPid))
	{
		std::wcout << L"Can't find Pid, Err=" << ::GetLastError() << std::endl;
		return FALSE;
	}

	HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
	if (hProcess == NULL)
	{
		std::wcout << L"Can't OpenProcess, Err=" << ::GetLastError() << std::endl;
		return FALSE;
	}

	if (!::ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(0x00189F78), static_cast<LPVOID>(ArrChess), sizeof(ArrChess), NULL))
	{
		std::wcout << L"ReadProcessMemory = false, Err=" << ::GetLastError() << std::endl;
		return FALSE;
	}
	WriteDebugFile();
	return TRUE;
}

struct ChessPoint
{
	int X, Y;
	ChessPoint(int X_, int Y_)
	{
		X = X_;
		Y = Y_;
	}
	ChessPoint()
	{
		X = Y = 0;
	}
	bool operator == (CONST ChessPoint& ChessPoint_) CONST
	{
		return this->X == ChessPoint_.X && this->Y == ChessPoint_.Y;
	}
};
BOOL GetEqualChessValue(_In_ CONST ChessPoint& p1, _Out_ std::vector<ChessPoint>& v2)
{
	for (int i = 0; i < Max_Row; ++i)
	{
		for (int j = 0; j < Max_Column; ++j)
		{
			if (ArrChess[j][i] == ArrChess[p1.Y][p1.X] && !(p1.X == i && p1.Y == j))
				v2.push_back(ChessPoint(i, j));
		}
	}
	return !v2.empty();
}

VOID GetChessPointLine(_In_ CONST ChessPoint& ChessPoint_, _Out_ std::vector<ChessPoint>& VecPoint)
{
	// Top
	for (int i = ChessPoint_.Y - 1; i >= 0; --i)
	{
		if (ArrChess[i][ChessPoint_.X] != 0)
			break;

		ChessPoint NewPoint(ChessPoint_.X, i);
		VecPoint.push_back(NewPoint);
	}

	// Buttom
	for (int i = ChessPoint_.Y + 1; i < Max_Column; ++i)
	{
		if (ArrChess[i][ChessPoint_.X] != 0)
			break;

		ChessPoint NewPoint(ChessPoint_.X, i);
		VecPoint.push_back(NewPoint);
	}

	// Left
	for (int i = ChessPoint_.X - 1; i >= 0; --i)
	{
		if (ArrChess[ChessPoint_.Y][i] != 0)
			break;

		ChessPoint NewPoint(i, ChessPoint_.Y);
		VecPoint.push_back(NewPoint);
	}

	// Right
	for (int i = ChessPoint_.X + 1; i < Max_Row; ++i)
	{
		if (ArrChess[ChessPoint_.Y][i] != 0)
			break;

		ChessPoint NewPoint(i, ChessPoint_.Y);
		VecPoint.push_back(NewPoint);
	}
}

// Comp Intersection
BOOL CheckIntersection(_In_ CONST ChessPoint& p1, _In_ CONST ChessPoint& p2)
{
	if (p1.X == p2.X) // Same X
	{
		for (int i = min(p1.Y, p2.Y) + 1; i < max(p1.Y, p2.Y); ++i)
		{
			if (ArrChess[i][p1.X] != 0)
				return FALSE;
		}
		return TRUE;
	}
	else if (p1.Y == p2.Y) // Same Y
	{
		for (int i = min(p1.X, p2.X) + 1; i < max(p1.X, p2.X); ++i)
		{
			if (ArrChess[p1.Y][i] != 0)
				return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

BOOL IsClean_Near(_In_ CONST ChessPoint& p1, _In_ CONST ChessPoint& p2)
{
	return (p1.Y == p2.Y && p1.X + 1 == p2.X) || (p1.X == p2.X && p1.Y + 1 == p2.Y);
}

BOOL IsClean_One_Line(_In_ CONST ChessPoint& p1, _In_ CONST ChessPoint& p2)
{
	return CheckIntersection(p1, p2);
}

BOOL IsClean_Two_ThreeLine(_In_ CONST ChessPoint& p1, _In_ CONST ChessPoint& p2)
{
	std::vector<ChessPoint> VecPoint1;
	GetChessPointLine(p1, VecPoint1);

	std::vector<ChessPoint> VecPoint2;
	GetChessPointLine(p2, VecPoint2);

	// Check Intersection
	for (CONST auto& line1 : VecPoint1)
	{
		for (CONST auto& lin2 : VecPoint2)
		{
			if (CheckIntersection(line1, lin2))
				return TRUE;
		}
	}
	return FALSE;
}

BOOL IsClean(_In_ CONST ChessPoint& p1, _In_ CONST ChessPoint& p2)
{
	return IsClean_Near(p1, p2) || IsClean_One_Line(p1, p2) || IsClean_Two_ThreeLine(p1, p2);
}

VOID ClickMouse(_In_ int X, _In_ int Y)
{
	if (GetGameWnd() != NULL)
	{
		RECT r;
		::GetWindowRect(GetGameWnd(), &r);

		//::SetCursorPos(r.left + X, r.top + Y);
		//::mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, NULL, NULL);
		//::mouse_event(MOUSEEVENTF_LEFTDOWN, NULL, NULL, NULL, NULL);
		int lparam =  (Y << 16) +  X;
		::PostMessage(GetGameWnd(), WM_LBUTTONDOWN, 0, lparam);
		::PostMessage(GetGameWnd(), WM_LBUTTONUP, 0, lparam);
	}
}

VOID ClickPicture(_In_ CONST ChessPoint& p1, _In_ CONST ChessPoint& p2)
{
	std::wcout << L"p1:" << p1.X << L"," << p1.Y << std::endl;
	std::wcout << L"p2:" << p2.X << L"," << p2.Y << std::endl;
	ClickMouse(22 + p1.X * 31, 194 + p1.Y * 35);
	ClickMouse(22 + p2.X * 31, 194 + p2.Y * 35);
}

BOOL CleanPicture()
{
	for (int i = 0; i < Max_Row; ++i)
	{
		for (int j = 0; j < Max_Column; ++j)
		{
			if (ArrChess[j][i] == NULL)
				continue;

			ChessPoint p1(i, j);
			std::vector<ChessPoint> v2;
			if (!GetEqualChessValue(p1, v2))
				continue;

			for (CONST auto& p2 : v2)
			{
				if (IsClean(p1, p2))
				{
					// Click Picture in Game
					ClickPicture(p1, p2);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}



int main()
{
	BOOL bDebugFile = ReadDebugFile();
	if (!bDebugFile && GetGameWnd() == NULL)
	{
		std::wcout << L"UnExist Game!" << std::endl;
		return 0;
	}

	::SwitchToThisWindow(GetGameWnd(), FALSE);
	while (bDebugFile || InitializeChess())
	{
		if (!CleanPicture())
			break;

		::Sleep(100);
	}

	return 0;
}