#include "pch.h"
#include "CalcSolarPower.h"
#include "AnalysisRadiationData.h"



// 発電量算出パラメータ
// 基本設計係数
double def_KPY = 0.88;

// 標準試験条件における日射強度
double def_GS = 1;


CCalcSolarPower::CCalcSolarPower
()
	: m_dPperUnit(0.0)
{

}

CCalcSolarPower::~CCalcSolarPower(void)
{

}


// 年間予測発電量(EPY)の算出 [kWh/年]
bool CCalcSolarPower::CalcEPY(CBuildingDataMap& dataMap)
{
	double EPY = 0.0;

	for (auto& val1 : dataMap)
	{
		CBuildingData& build = val1.second;

		int meshCount = 0;
		for (auto& val2 : build.mapRoofSurface)
		{
			CRoofSurfaceData& surface = val2.second;
			meshCount += (int)surface.vecRoofMesh.size();
		}

		double panelArea = meshCount * 1.0;		// 1mメッシュ数 = 面積(m2)
		build.panelArea = panelArea;
		
		// 設置可能システム容量(P)
		// パネル面積＊単位面積当たり容量
		double P = panelArea * m_dPperUnit;

		// 年間予測日射量(HAY)
		double HAY = build.solarRadiationUnit;

		// 設置可能システム容量(P) ＊ 年間予測日射量(HAY) ＊ 基本設計係数(KPY) ＊ １ ／ 標準試験条件における日射強度(GS)
		EPY = P * HAY * def_KPY * 1 / def_GS;

		// 建物ごとに算出した発電量を適用
		build.solarPower = EPY;

		// 1m2あたりの発電量
		build.solarPowerUnit = EPY / panelArea;

	}

	return true;

}

