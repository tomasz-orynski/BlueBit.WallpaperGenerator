// stdafx.cpp : source file that includes just the standard includes
// WallpaperGenerator.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

std::wstring GetFileName(std::list<std::wstring> &fileNames)
{
	auto idx = rand() % fileNames.size();
	auto i = fileNames.begin();
#ifndef _DEBUG
	std::advance(i, idx);
#endif
	auto fileName = *i;
	fileNames.remove(fileName);
	return fileName;
}

std::wstring GetFileName(int number)
{
	std::wstring path = L"_WallpaperGenerator_.";
	wchar_t buf[10];
	wsprintf(buf, L"%03d", number);
	path += buf;
	path += L".jpg";
	return path;
}

std::wstring GetDirectoryName(const Gdiplus::Size &size)
{
	std::wstring name;
	wchar_t buf[10];
	_itow_s(size.Width, buf, 10);
	name += buf;
	name += L"x";
	_itow_s(size.Height, buf, 10);
	name += buf;
	return name;
}
