// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include <stdio.h>
#include <tchar.h>
#include <Shlwapi.h>
#include <Unknwn.h>
#include <gdiplus.h>
#include <ppl.h>
#include <ctime>
#include <array>
#include <list>
#include <vector>
#include <map>
#include <memory>
#include <thread>

#define THREADS_COUNT 4

struct SizeLessThan :
	public std::binary_function < Gdiplus::Size, Gdiplus::Size, bool >
{
	bool operator() (Gdiplus::Size const &a, Gdiplus::Size const &b) const { return a.Width < b.Width || (a.Width == b.Width && a.Height < b.Height); }
};

struct SizeEqualTo :
	public std::binary_function < Gdiplus::Size, Gdiplus::Size, bool >
{
	bool operator() (Gdiplus::Size const &a, Gdiplus::Size const &b) const { return a.Width == b.Width && a.Height == b.Height; }
};

std::wstring GetFileName(std::list<std::wstring> &fileNames);
std::wstring GetFileName(int number);
std::wstring GetDirectoryName(const Gdiplus::Size &size);
