#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "UIParam.h"
#include "BuildingData.h"
#include "LandData.h"
#include "ImportRestrictAreaData.h"
#include "ImportWeatherData.h"
#include "ResultJudgment.h"

#define CSVFILE L"建物別適地判定結果.csv"
#define GEOTIFFFILE L"建物別適地判定結果.tif"
#define CSVFILE_LAND L"土地別適地判定結果.csv"
#define GEOTIFFFILE_LAND L"土地別適地判定結果.tif"

class CJudgeSuitablePlaceMng
{
public:
	CJudgeSuitablePlaceMng(void);
	~CJudgeSuitablePlaceMng(void);

	void SetParameter(CUIParam* pParam) { m_pUIParam = pParam; };
	void SetWeatherData(CImportWeatherData* pWeatherData) { m_pWeatherData = pWeatherData; };
	void SetRestrictAreaData(CImportRestrictAreaData* pRestrictAreaData)
	{
		m_pRestrictAreaData[0] = &pRestrictAreaData[0];
		m_pRestrictAreaData[1] = &pRestrictAreaData[1];
		m_pRestrictAreaData[2] = &pRestrictAreaData[2];
	};
	int Judge();
	eBuildingStructure GetBuildingStructureType1(int iBldStructureType);
	eDirections GetDirection(double dAzimuth);
	int OutputGeoTiff(CResultJudgment result, CBuildingData buildings, const std::wstring& filepath);
	int OutputGeoTiff(CResultJudgment result, CLandData lands, const std::wstring& filepath);
	bool OutputLegendImage();

private:

	int JudgeBuild();
	int JudgeLand();

	CUIParam* m_pUIParam;
	CBuildingData* m_pBuildingData;
	CLandData* m_pLandData;
	CImportWeatherData *m_pWeatherData;
	CImportRestrictAreaData* m_pRestrictAreaData[3];

	//CResultJudgment m_ResultJudgment;

	vector<AREADATA>* m_allAreaList;
	std::vector<CPoint2D> m_aggregationRange;

};

