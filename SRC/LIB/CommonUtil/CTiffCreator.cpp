#include "pch.h"
#include "CTiffCreator.h"
#include "CFileUtil.h"
#include "CFileIO.h"
#include "StringEx.h"
#include "ReadINIParam.h"
#include <windows.h>
#include <gdiplus.h>
#include <locale.h>


#pragma comment (lib,"Gdiplus.lib")

using namespace Gdiplus;

INT GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num = 0;
	UINT size = 0;

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)	return -1;

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)	return -1;

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT i = 0; i < num; ++i)
	{
		if (wcscmp(pImageCodecInfo[i].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[i].Clsid;
			free(pImageCodecInfo);
			return i;
		}
	}

	free(pImageCodecInfo);
	return -1;

};

HINSTANCE g_hInst = NULL;

CTiffCreator::CTiffCreator()
{
}

CTiffCreator::~CTiffCreator()
{

}

bool CTiffCreator::CreateColorTiff(
	const std::wstring& strOutputPath,
	GeoTiffInfo	stGeoTiffInfo,
	float* fBuffer,
	const std::wstring& strColorSetting)
{
	struct stCol
	{
		int iR;
		int iG;
		int iB;
		double maxH;
	};
	std::wstring strMdlPath = CFileUtil::GetModulePathW();
	std::wstring colorSetting = CFileUtil::Combine(strMdlPath, strColorSetting);

	setlocale(LC_ALL, "");

	// ファイルの読込
	CFileIO fio;
	if (!fio.Open(colorSetting, L"r, ccs=UTF-8"))
	{
		return false;
	}

	std::vector<stCol> colorAry;
	{
		wchar_t cBuff[1024];
		std::wstring sbuf;			// 1行単位での情報取得用変数
		while (fio.ReadLineW(cBuff, 1024) != NULL)
		{
			sbuf = cBuff;

			stCol colAndH;
			int idxR = (int)sbuf.find(L" ", 0);
			int idxG = (int)sbuf.find(L" ", (int64_t)idxR + 1);
			int idxB = (int)sbuf.find(L" ", (int64_t)idxG + 1);
			colAndH.maxH = _wtof(sbuf.substr(0, idxR).c_str());
			colAndH.iR = _wtoi(sbuf.substr((int64_t)idxR + 1, (int64_t)idxG - idxR).c_str());
			colAndH.iG = _wtoi(sbuf.substr((int64_t)idxG + 1, (int64_t)idxB - idxG).c_str());
			colAndH.iB = _wtoi(sbuf.substr((int64_t)idxB + 1, 3).c_str());
			colorAry.push_back(colAndH);
		}
		fio.Close();
	}
	int colNum = static_cast<int>(colorAry.size());

	GdiplusStartupInput input;
	ULONG_PTR token;
	Gdiplus::Status status;
	GdiplusStartup(&token, &input, NULL);
	{
		Gdiplus::Bitmap img(stGeoTiffInfo.width, stGeoTiffInfo.height, PixelFormat32bppRGB);
		img.SetResolution(300.0, 300.0);

		double dZ;
		for (UINT ui = 0; ui < (UINT)stGeoTiffInfo.height; ui++)
		{
			for (UINT uj = 0; uj < (UINT)stGeoTiffInfo.width; uj++)
			{
				UINT iIndex = uj + (stGeoTiffInfo.width * ui);
				dZ = fBuffer[iIndex];

				if (dZ == stGeoTiffInfo.noDataValue)
				{
					//背景色を設定
					COLORREF bgClr = RGB(255, 255, 255);
					Gdiplus::Color bgColor;
					bgColor.SetFromCOLORREF(bgClr);
					img.SetPixel(uj, ui, bgColor);
				}
				else
				{
					// 色取得
					int idxColor = -1;
					for (int ic = 0; ic < colNum; ic++)
					{
						if (dZ < colorAry[ic].maxH)
						{
							break;
						}
						idxColor = ic;
					}
					int iR = 255;
					int iG = 255;
					int iB = 255;
					if (0 <= idxColor) {
						iR = colorAry[idxColor].iR;
						iG = colorAry[idxColor].iG;
						iB = colorAry[idxColor].iB;
					}
					// 色設定
					COLORREF clr = RGB(iR, iG, iB);
					Gdiplus::Color color;
					color.SetFromCOLORREF(clr);
					img.SetPixel(uj, ui, color);
				}
			}
		}
		// TIFF画像出力
		EncoderParameters encoderParams;
		ULONG parameterValue;
		encoderParams.Count = 1;
		encoderParams.Parameter[0].Guid = EncoderSaveFlag;
		encoderParams.Parameter[0].Type = EncoderParameterValueTypeLong;
		encoderParams.Parameter[0].NumberOfValues = 1;
		encoderParams.Parameter[0].Value = &parameterValue;

		CLSID encoderClsid;
		INT result = GetEncoderClsid(L"image/tiff", &encoderClsid);
		status = img.Save(strOutputPath.c_str(), &encoderClsid, &encoderParams);
	}
	GdiplusShutdown(token);

	return (status == S_OK) ? true : false;
}

 //ワールドファイル作成
bool
CTiffCreator::CreateWldFile
(
	const std::wstring& strWldFilePath,
	CTiffCreator::GeoTiffInfo	stGeoTiffInfo
)
{
	bool bRet = false;

	CFileIO file;
	if (!file.Open(strWldFilePath, L"w"))
	{
		return false;
	}

	std::wstring strLine;
	strLine = CStringEx::Format(L"%.8f", stGeoTiffInfo.meshSize);
	file.WriteLineW(strLine);

	strLine = CStringEx::Format(L"%.8f", 0.0);
	file.WriteLineW(strLine);

	strLine = CStringEx::Format(L"%.8f", 0.0);
	file.WriteLineW(strLine);

	strLine = CStringEx::Format(L"%.8f", -stGeoTiffInfo.meshSize);
	file.WriteLineW(strLine);

	// 左上ピクセル中央値
	double fLeftTopPixelCenterX = stGeoTiffInfo.offsetX + stGeoTiffInfo.meshSize / 2.0;
	double fLeftTopPixelCenterY = stGeoTiffInfo.offsetY - stGeoTiffInfo.meshSize / 2.0;

	strLine = CStringEx::Format(L"%.8f", fLeftTopPixelCenterX);
	file.WriteLineW(strLine);
	strLine = CStringEx::Format(L"%.8f", fLeftTopPixelCenterY);
	file.WriteLineW(strLine);

	// 測地系のコメント
	int JPZONE = GetINIParam()->GetJPZone();
	strLine = CStringEx::Format(L"#%d系", JPZONE);
	file.WriteLineW(strLine);

	file.Close();

	return true;
}
