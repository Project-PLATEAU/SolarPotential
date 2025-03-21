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
	, m_dPanelRatio(0.0)
{

}

CCalcSolarPower::~CCalcSolarPower(void)
{

}


// 年間予測発電量(EPY)の算出 [kWh/年]
bool CCalcSolarPower::CalcEPY(CPotentialData& result)
{
	double EPY = 0.0;

	// メッシュごとの発電量
	for (auto& surface : result.mapSurface)
	{
		CSurfaceData& surfaceData = surface.second;

		for (auto& mesh : surfaceData.vecMeshData)
		{
			// 設置可能システム容量(P)
			// パネル面積＊単位面積当たり容量
			double P0 = 1.0 * m_dPperUnit;	// 1m2あたり

			// 年間予測日射量(HAY)
			double HAY0 = mesh.solarRadiationUnit;

			// 設置可能システム容量(P) ＊ 年間予測日射量(HAY) ＊ 基本設計係数(KPY) ＊ １ ／ 標準試験条件における日射強度(GS)
			EPY = P0 * HAY0 * def_KPY * 1 / def_GS;

			// 発電量
			mesh.solarPowerUnit = EPY;
		}
	}

	//// 建物・土地ごとの発電量
	//result.panelArea = result.GetAllArea() * m_dPanelRatio;

	// 設置可能システム容量(P)
	// パネル面積＊単位面積当たり容量
	double P = result.panelArea * m_dPperUnit;

	// 年間予測日射量(HAY)
	double HAY = result.solarRadiationUnit;

	// 設置可能システム容量(P) ＊ 年間予測日射量(HAY) ＊ 基本設計係数(KPY) ＊ １ ／ 標準試験条件における日射強度(GS)
	EPY = P * HAY * def_KPY * 1 / def_GS;

	// 建物ごとに算出した発電量を適用
	result.solarPower = EPY;

	// 1m2あたりの発電量
	result.solarPowerUnit = EPY / result.panelArea;

	return true;

}

