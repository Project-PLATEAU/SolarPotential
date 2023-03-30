#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "UIParam.h"
#include "BuildingData.h"
#include "ImportRestrictAreaData.h"
#include "ImportWeatherData.h"
#include "ResultJudgment.h"

#define CSVFILE L"建物別適地判定結果.csv"
#define GEOTIFFFILE L"建物別適地判定結果.tif"

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
	int OutputGeoTiff( CBuildingData buildings, const std::wstring& filepath);

private:

	CUIParam* m_pUIParam;
	CBuildingData *m_pBuildingData;
	CImportWeatherData *m_pWeatherData;
	CImportRestrictAreaData* m_pRestrictAreaData[3];

	CResultJudgment m_ResultJudgment;

};

