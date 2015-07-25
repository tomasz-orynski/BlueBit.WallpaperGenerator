#include "stdafx.h"
#include "ProgramThread.h"

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	auto plMonitorData = (std::list<RECT>*)dwData;
	plMonitorData->push_back(RECT(*lprcMonitor));
	return TRUE;
}

std::list<std::wstring> GetFileNames(const std::wstring &path, const Gdiplus::Size &size)
{
	auto pathWithExt = path;
	pathWithExt += L"\\";
	pathWithExt += GetDirectoryName(size);
	pathWithExt += L"\\*.jpg";

	std::list<std::wstring> fileNames;
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile(pathWithExt.c_str(), &findFileData);
	if (hFind != INVALID_HANDLE_VALUE) do
	{
		fileNames.push_back(findFileData.cFileName);
	} while (FindNextFile(hFind, &findFileData));
	FindClose(hFind);
	fileNames.sort();
	return fileNames;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 4) return -1;
	ProgramThread::Params.pathIn = argv[1];
	ProgramThread::Params.pathOut = argv[2];
	auto count = _wtoi(argv[3]);
	::InitializeCriticalSection(&ProgramThread::Params.cs);

	srand((unsigned int)time(NULL));

	::EnumDisplayMonitors(NULL, NULL, &MonitorEnumProc, (LPARAM)&ProgramThread::Params.lMonitorRects);
	_ASSERT(ProgramThread::Params.lMonitorRects.size() != 0);

	{
		auto sumMonitorRects = *ProgramThread::Params.lMonitorRects.begin();
		for (auto rect : ProgramThread::Params.lMonitorRects)
		{
			sumMonitorRects.left = min(rect.left, sumMonitorRects.left);
			sumMonitorRects.top = min(rect.top, sumMonitorRects.top);
			sumMonitorRects.right = max(rect.right, sumMonitorRects.right);
			sumMonitorRects.bottom = max(rect.bottom, sumMonitorRects.bottom);

			Gdiplus::Size gdiSize(rect.right - rect.left, rect.bottom - rect.top);
			if (ProgramThread::Params.fileNamesPerResolution.find(gdiSize) == std::end(ProgramThread::Params.fileNamesPerResolution))
			{
				auto fileNames = GetFileNames(ProgramThread::Params.pathIn, gdiSize);
				_ASSERT(fileNames.size() != 0);

				count = min(count, (int)fileNames.size());
				ProgramThread::Params.fileNamesPerResolution.insert(std::pair<Gdiplus::Size, std::list<std::wstring>>(gdiSize, fileNames));
			}
		}
		ProgramThread::Params.sumMonitorRects = sumMonitorRects;
		ProgramThread::Params.totalCount = count;
	}
	_ASSERT(ProgramThread::Params.fileNamesPerResolution.size() != 0);

	std::vector<ProgramThread> threads(min(count, THREADS_COUNT));
	for (auto&& thread : threads)
		thread.join();

	return 0;
}

