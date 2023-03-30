#pragma once
#include "StringEx.h"

class CImageUtil
{
public:
	CImageUtil();
	~CImageUtil();

	// GeoTIFF->Jpeg•ÏŠ·
	static bool ConvertTiffToJpeg(const std::wstring& strTiffPath);

	// –}—á‰æ‘œ‚Ìì¬
	static bool CreateLegendImage(const std::wstring& strColorSetting, const std::wstring& strHeader);

private:

};

