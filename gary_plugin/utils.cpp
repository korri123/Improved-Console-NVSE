#include "utils.h"

#include <algorithm>
#include <vector>

void ShowErrorMessageBox(const char* message)
{
	int msgboxID = MessageBox(
		NULL,
		message,
		"Error",
		MB_ICONWARNING | MB_OK
	);
}

std::string GetCurPath()
{
	char path[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, path);
	return path;
}

std::vector<void*> GetCallStack(int i)
{
	std::vector<void*> vecTrace(i, nullptr);
	CaptureStackBackTrace(1, i, vecTrace.data(), nullptr);
	return vecTrace;
}

bool IsInFunction(UInt32 beginAddr, UInt32 endAddr)
{
	auto v = GetCallStack(4);
	return std::any_of(v.begin(), v.end(), [&](void* ptr) {return (UInt32)ptr >= beginAddr && (UInt32)ptr <= endAddr; });
}