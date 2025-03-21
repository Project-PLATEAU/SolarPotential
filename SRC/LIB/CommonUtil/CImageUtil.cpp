#include "pch.h"
#include "CFileUtil.h"
#include "CFileIO.h"
#include "StringEx.h"
#include <windows.h>
#include <gdiplus.h>
#include <locale.h>
#include "CImageUtil.h"

#pragma comment (lib,"Gdiplus.lib")

using namespace Gdiplus;

INT GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

CImageUtil::CImageUtil()
{

}

CImageUtil::~CImageUtil()
{

}

bool CImageUtil::ConvertTiffToJpeg(
	const std::wstring& strTiffPath
)
{
	GdiplusStartupInput input;
	ULONG_PTR token;
	Gdiplus::Status status;
	GdiplusStartup(&token, &input, NULL);
	{
		// TIFF�t�@�C���ǂݍ���
		Gdiplus::Image* img = new Gdiplus::Image(strTiffPath.c_str());

		std::wstring strJpegPath = GetFUtil()->ChangeFileNameExt(strTiffPath, L".jpg");

		EncoderParameters encoderParams;
		ULONG quality = 100;
		encoderParams.Count = 1;
		encoderParams.Parameter[0].Guid = EncoderQuality;
		encoderParams.Parameter[0].Type = EncoderParameterValueTypeLong;
		encoderParams.Parameter[0].NumberOfValues = 1;
		encoderParams.Parameter[0].Value = &quality;

		CLSID encoderClsid;
		INT result = GetEncoderClsid(L"image/jpeg", &encoderClsid);
		status = img->Save(strJpegPath.c_str(), &encoderClsid, &encoderParams);

		delete img;
	}
	GdiplusShutdown(token);

	return (status == S_OK) ? true : false;
}

bool CImageUtil::CreateLegendImage(
	const std::wstring& strColorSetting,		// �F�ݒ�t�@�C��
	const std::wstring& strHeader				// �w�b�_����
)
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
	std::wstring tmpPath = CFileUtil::Combine(strMdlPath, L"tmpImage.jpg");

	setlocale(LC_ALL, "");

	// �t�@�C���̓Ǎ�
	CFileIO fio;
	if (!fio.Open(colorSetting, L"r, ccs=UTF-8"))
	{
		return false;
	}

	std::vector<stCol> colorAry;
	{
		wchar_t cBuff[1024];
		std::wstring sbuf;			// 1�s�P�ʂł̏��擾�p�ϐ�
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

	INT imgWidth = 140;
	INT imgHeight = colNum * 20 + 30;

	GdiplusStartupInput input;
	ULONG_PTR token;
	Gdiplus::Status status;
	GdiplusStartup(&token, &input, NULL);
	{
		Gdiplus::Bitmap bitmap(imgWidth, imgHeight, PixelFormat32bppRGB);

		// �摜�o��
		EncoderParameters encoderParams;
		ULONG quality = 100;
		encoderParams.Count = 1;
		encoderParams.Parameter[0].Guid = EncoderQuality;
		encoderParams.Parameter[0].Type = EncoderParameterValueTypeLong;
		encoderParams.Parameter[0].NumberOfValues = 1;
		encoderParams.Parameter[0].Value = &quality;

		CLSID encoderClsid;
		INT result = GetEncoderClsid(L"image/jpeg", &encoderClsid);

		status = bitmap.Save(tmpPath.c_str(), &encoderClsid, &encoderParams);

		Gdiplus::Image* img = new Gdiplus::Image(tmpPath.c_str());
		Gdiplus::Graphics graphics(img);

		// �w�i�𔒂œh��ׂ�
		SolidBrush bgClrBrush(Color(255, 255, 255, 255));
		graphics.FillRectangle(&bgClrBrush, 0, 0, imgWidth, imgHeight);

		// �t�H���g����
		FontFamily fontFamily(L"MS Gothic");
		Font font(&fontFamily, 14, FontStyleRegular, UnitPixel);
		PointF pointF(3.0f, 3.0f);
		SolidBrush textBrush(Color(255, 0, 0, 0));
		
		// �w�b�_������`��
		graphics.DrawString(strHeader.c_str(), -1, &font, PointF(3.0f, 3.0f), &textBrush);

		REAL drawPosX = 70.0f;
		REAL drawPosY = 30.0f;

		// �}�ᕔ����`��
		for (int ic = colNum - 1; ic >= 0; --ic)
		{
			// �l
			std::wstring strVal = CStringEx::Format( L"%.1f", colorAry[ic].maxH);
			graphics.DrawString(strVal.c_str(), -1, &font, PointF(10.0f, drawPosY + 3.0f), &textBrush);

			int iR = 255;
			int iG = 255;
			int iB = 255;
			iR = colorAry[ic].iR;
			iG = colorAry[ic].iG;
			iB = colorAry[ic].iB;

			// �F
			RectF rect(drawPosX, drawPosY, 60.0f, 20.0f);
			SolidBrush clrBrush(Color(255, iR, iG, iB));
			graphics.FillRectangle(&clrBrush, rect);

			drawPosY += 20.0f;
		}

		std::wstring outputPath = GetFUtil()->ChangeFileNameExt(strColorSetting, L".jpg");
		status = img->Save(outputPath.c_str(), &encoderClsid, &encoderParams);

		delete img;
	}
	GdiplusShutdown(token);

	// �ꎞ�摜���폜
	DeleteFile(tmpPath.c_str());

	return (status == S_OK) ? true : false;

}