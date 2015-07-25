#include "stdafx.h"
#include <Shlwapi.h>
#include <Unknwn.h>
#include <gdiplus.h>
#include <ctime>
#include <array>
#include <list>
#include <map>
#include <memory>
#include "ProgramThread.h"

int GetEncoderClsid(CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;
	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, L"image/jpeg") == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

std::unique_ptr<Gdiplus::Bitmap> GetBitmap(const std::wstring &path, const Gdiplus::Size &size, const std::wstring &fileName)
{
	auto pathWithFileName = path;
	pathWithFileName += L"\\";
	pathWithFileName += GetDirectoryName(size);
	pathWithFileName += L"\\";
	pathWithFileName += fileName;
	return std::unique_ptr<Gdiplus::Bitmap>(Gdiplus::Bitmap::FromFile(pathWithFileName.c_str()));
}

ProgramParams ProgramThread::Params;

void ProgramThread::MakeWallpapers()
{
	ULONG_PTR gdiPlusToken;
	Gdiplus::GdiplusStartupInput gdiPlusStartupInput;
	Gdiplus::GdiplusStartup(&gdiPlusToken, &gdiPlusStartupInput, NULL);

	auto ox = Params.sumMonitorRects.left < 0 ? -Params.sumMonitorRects.left : 0;
	auto oy = Params.sumMonitorRects.top < 0 ? -Params.sumMonitorRects.top : 0;
	auto sx = Params.sumMonitorRects.right - Params.sumMonitorRects.left;
	auto sy = Params.sumMonitorRects.bottom - Params.sumMonitorRects.top;

	CLSID pngClsid;
	GetEncoderClsid(&pngClsid);
	SizeEqualTo sizeEqualTo;
	for (;;)
	{
		::EnterCriticalSection(&Params.cs);
		auto count = Params.totalCount;
		if (Params.totalCount > 0)
			--Params.totalCount;
		::LeaveCriticalSection(&Params.cs);

		if (count == 0)
			break;

		std::wstring logBuffer;
		wchar_t buffer[512] = { 0 };

		wprintf(L"------\n>>(%03d)\n------\n\n", count);
		swprintf_s(buffer, L"------\n>>(%03d)\n------\n", count);
		logBuffer += buffer;

		Gdiplus::Bitmap bmp(sx, sy);
		std::unique_ptr<Gdiplus::Graphics> pGraph(Gdiplus::Graphics::FromImage(&bmp));
		pGraph->SetInterpolationMode(Gdiplus::InterpolationMode::InterpolationModeHighQuality);
		pGraph->SetCompositingQuality(Gdiplus::CompositingQuality::CompositingQualityHighQuality);
		pGraph->SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeHighQuality);

		auto error = false;
		for (auto rect : Params.lMonitorRects)
		{
			Gdiplus::Size gdiSizeScr(rect.right - rect.left, rect.bottom - rect.top);
			Gdiplus::Rect gdiRectScr(rect.left + ox, rect.top + oy, gdiSizeScr.Width, gdiSizeScr.Height);

			::EnterCriticalSection(&Params.cs);
			auto fileName = GetFileName(Params.fileNamesPerResolution.at(gdiSizeScr));
			::LeaveCriticalSection(&Params.cs);

			swprintf_s(buffer, L"=>[%s]\n", fileName.c_str());
			logBuffer += buffer;

			std::unique_ptr<Gdiplus::Bitmap> pBmp = GetBitmap(Params.pathIn, gdiSizeScr, fileName);
			if (pBmp->GetLastStatus() != Gdiplus::Status::Ok)
			{
				error = true;
				break;
			}

			Gdiplus::Size gdiSizeBmp(pBmp->GetWidth(), pBmp->GetHeight());

			swprintf_s(buffer, L"    [%d x %d] <= [%d x %d] ...\n", gdiSizeScr.Width, gdiSizeScr.Height, gdiSizeBmp.Width, gdiSizeBmp.Height);
			logBuffer += buffer;

			_ASSERT(gdiSizeBmp.Width != 0);
			_ASSERT(gdiSizeBmp.Height != 0);

			if (sizeEqualTo(gdiSizeScr, gdiSizeBmp))
				pGraph->DrawImage(pBmp.get(), gdiRectScr);
			else
			{
				auto aspectRatioScr = (double)gdiSizeScr.Width / gdiSizeScr.Height;
				auto aspectRatioBmp = (double)gdiSizeBmp.Width / gdiSizeBmp.Height;
				if (abs(aspectRatioScr - aspectRatioBmp) < 0.001)
				{
					pGraph->DrawImage(pBmp.get(), gdiRectScr);
				}
				else
				{
					INT width = 0;
					INT height = 0;
					auto aspectRatioW = (double)gdiSizeBmp.Width / gdiSizeScr.Width;
					auto aspectRatioH = (double)gdiSizeBmp.Height / gdiSizeScr.Height;
					auto aspectRatio = min(aspectRatioW, aspectRatioH);

					if (abs(aspectRatioW - 1) < abs(aspectRatioH - 1))
					{
						width = gdiSizeBmp.Width;
						height = (INT)(width / aspectRatioScr);
						auto diff = height - gdiSizeBmp.Height;
						if (diff > 0)
						{
							height = gdiSizeBmp.Height;
							width -= diff * aspectRatioScr;
						}
					}
					else
					{
						height = gdiSizeBmp.Height;
						width = (INT)(height * aspectRatioScr);
						auto diff = width - gdiSizeBmp.Width;
						if (diff > 0)
						{
							width = gdiSizeBmp.Width;
							height -= diff / aspectRatioScr;
						}
					}

					INT x = (gdiSizeBmp.Width - width);
					x >>= 1;
					INT y = (gdiSizeBmp.Height - height);
					y >>= 1;
					pGraph->DrawImage(pBmp.get(), gdiRectScr, x, y, width, height, Gdiplus::Unit::UnitPixel);
				}
				if (pGraph->GetLastStatus() != Gdiplus::Status::Ok)
				{
					error = true;
					break;
				}
			}
		}
		if (error) continue;

		auto path = Params.pathOut;
		path += L"\\";
		path += GetFileName(count);
		bmp.Save(path.c_str(), &pngClsid);

		swprintf_s(buffer, L"<=[%s]\n", path.c_str());
		logBuffer += buffer;

		wprintf(L"%s------\n\n", logBuffer.c_str());
	}

	Gdiplus::GdiplusShutdown(gdiPlusToken);
}
