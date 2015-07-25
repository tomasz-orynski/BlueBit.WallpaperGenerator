#pragma once

struct ProgramParams
{
	RECT sumMonitorRects;
	std::list<RECT> lMonitorRects;
	std::map<Gdiplus::Size, std::list<std::wstring>, SizeLessThan> fileNamesPerResolution;
	std::wstring pathIn;
	std::wstring pathOut;
	volatile unsigned int totalCount;
	CRITICAL_SECTION cs;
};

class ProgramThread :
	public std::thread
{
public:
	static ProgramParams Params;

private:
	static void MakeWallpapers();

public:
	ProgramThread() : std::thread(ProgramThread::MakeWallpapers) {}
};

