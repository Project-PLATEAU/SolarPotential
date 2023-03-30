#pragma once
#include <list>
#include "StringEx.h"

class CTiffCreator
{
public:
	CTiffCreator();
	~CTiffCreator();

	typedef struct GeoTiffInfo
	{
		double offsetX;
		double offsetY;

		double meshSize;

		int width;
		int height;

		double noDataValue;

		int EPSGCode;

		GeoTiffInfo()
		{
			offsetX = 0;
			offsetY = 0;
			meshSize = 0;
			width = 0;
			height = 0;
			noDataValue = 0;
			EPSGCode = 0;
		}

		GeoTiffInfo(const GeoTiffInfo& x)
		{
			offsetX = x.offsetX;
			offsetY = x.offsetY;
			meshSize = x.meshSize;
			width = x.width;
			height = x.height;
			noDataValue = x.noDataValue;
			EPSGCode = x.EPSGCode;
		}

		GeoTiffInfo& operator=(const GeoTiffInfo& x)
		{
			offsetX = x.offsetX;
			offsetY = x.offsetY;
			meshSize = x.meshSize;
			width = x.width;
			height = x.height;
			noDataValue = x.noDataValue;
			EPSGCode = x.EPSGCode;

			return *this;
		}
	} GeoTiffInfo_tag;

	bool CreateColorTiff(const std::wstring& strOutputPath, GeoTiffInfo stGeoTiffInfo, float* fBuffer, const std::wstring& strColorSetting);
	bool CreateWldFile(const std::wstring& strWldFilePath, GeoTiffInfo stGeoTiffInfo);

};
