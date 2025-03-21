#include "pch.h"
#include <math.h>
#include <random>
#include <iostream>
#include <unordered_map>
#include "CalcSolarRadiation.h"
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "../../LIB/CommonUtil/CSunVector.h"
#include "../../LIB/CommonUtil/ReadINIParam.h"

#include <fstream>
#include <filesystem>

#ifdef CHRONO
#include <chrono>
#endif

// 日射量計算パラメータ
// 太陽定数
const double def_Sconst = 1367.0;

CCalcSolarRadiation::CCalcSolarRadiation
(
	CCalcSolarPotentialMng* pMng
)
	: m_pMng(pMng)
{

}

CCalcSolarRadiation::~CCalcSolarRadiation(void)
{

}

// 建物の日射量算出
bool CCalcSolarRadiation::ExecBuild(
	CPotentialData& result,					// 計算結果格納用データ
	const CTime& startDate,					// 解析する期間の開始日時
	const CTime& endDate					// 解析する期間の終了日時
)
{
	if (!m_pMng)	return false;

#ifdef CHRONO
	std::filesystem::path p = std::filesystem::path(m_pMng->GetUIParam()->strOutputDirPath) / "time2.log";
	ofstream ofs(p, ios::ios_base::app);

	ofs << "SolarRad::ExecBuild Start " << std::endl;
	chrono::system_clock::time_point start, end;
	start = std::chrono::system_clock::now();
	double time;
#endif

	int JPZONE = GetINIParam()->GetJPZone();

	int iYear = m_pMng->GetYear();

	const CVector3D center = result.center;

	// 隣接する建物を取得
#ifdef CHRONO
	ofs << "GetNeighborBuildings Start " << std::endl;
	start = std::chrono::system_clock::now();
#endif
	std::vector<BLDGLIST> neighborBuildings;
	m_pMng->GetNeighborBuildings(center, neighborBuildings);
#ifdef CHRONO
	end = std::chrono::system_clock::now();
	time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 0.001);
	ofs << "GetNeighborBuildings Time: " << time << " sec" << std::endl;
#endif

	// 隣接するTINを取得
	std::vector<CTriangle> neighborDems;
	if (m_pMng->IsEnableDEMData())
	{
		m_pMng->GetNeighborDems(center, neighborDems, CCalcSolarPotentialMng::eAnalyzeTarget::ROOF);
	}

#ifdef CHRONO
	ofs << "Calc Start " << std::endl;
	auto cstart = std::chrono::system_clock::now();
#endif

	for (auto& surface : result.mapSurface)
	{
		std::string surfaceId = surface.first;
		CSurfaceData& surfaceData = surface.second;

#ifdef CHRONO
		ofs << "--- Target Surface: " << surfaceId << std::endl;
		auto tstart = std::chrono::system_clock::now();
#endif
		// 直達成分の有無を判定
		std::unordered_map<int, bool> mapSunDir;
		double dLat = 0.0, dLon = 0.0;
		CGeoUtil::XYToLatLon(JPZONE, surfaceData.center.y, surfaceData.center.x, dLat, dLon);

		double slopeDegree = surfaceData.slopeModDegree;	// 傾斜角
		double azDegree = surfaceData.azModDegree;			// 方位角

		double surfaceAngle = slopeDegree * _COEF_DEG_TO_RAD;	// 斜面の傾斜角
		double surfaceAz = azDegree * _COEF_DEG_TO_RAD;			// 斜面の方位角

		CTime date = startDate;
		do {

#ifdef CHRONO
			start = std::chrono::system_clock::now();
#endif
			if (m_pMng->IsCancel())	return false;

			double dSunnyVal = 0.0; double dCloudVal = 0.0;
			int month = date.GetMonth();

			double p = GetINIParam()->GetTransmissivity(month);		// 大気透過率
			double pdifCloud = 1.0 - p;								// 透過率(曇天時)

			const CSunVector outSun(dLat, dLon, date);

			for (int hour = 0; hour < 24; hour++)
			{
				HorizontalCoordinate pos;
				outSun.GetPos(hour, pos);
				double sunAngle = pos.altitude;	// 太陽天頂角(太陽高度)	
				if (sunAngle < 0)
				{
					//太陽高度が0未満
					continue;
				}
				double alpha = pos.azimuth + _PI;		// 太陽方位

				int idx = date.iYDayCnt * 24 + hour;

				// 反射率
				double refRate = 0.0;	// 反射率
				date.iHour = hour;
				double depth = this->m_pMng->GetMetpvData()->GetSnowDepth(date);
				if (depth > 10.0)		// 10cm以上
				{
					refRate = GetINIParam()->GetReflectivitySnow();
				}
				else
				{
					refRate = GetINIParam()->GetReflectivity();
				}

				// 入射角
				double angIn = calcAngleIn(sunAngle, surfaceAngle, surfaceAz, alpha);
				if (angIn < 0.0)	angIn = 0.0;

				CVector3D sunVector;
				outSun.GetVector(hour, sunVector);

				// 直達成分の有無を判定
				if (mapSunDir.find(idx) == mapSunDir.end())
				{
					mapSunDir[idx] = m_pMng->IntersectSurfaceCenter(sunVector, surfaceData.bbPos, surfaceData.center, surfaceId, neighborBuildings, neighborDems);
				}
				bool bDirect = mapSunDir.at(idx);

				dSunnyVal += calcSlopeDif(surfaceAngle, angIn, sunAngle, refRate, bDirect, p);
				dCloudVal += calcSlopeDif(surfaceAngle, angIn, sunAngle, refRate, bDirect, pdifCloud);

			}

			int mIdx = month - 1;
			for (auto& mesh : surfaceData.vecMeshData)
			{
				// ひと月ごとに値を適用
				mesh.solarRadiationSunny[mIdx] += dSunnyVal;
				mesh.solarRadiationCloud[mIdx] += dCloudVal;
			}

#ifdef CHRONO
			end = std::chrono::system_clock::now();
			time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 0.001);
			ofs << month << "/" << date.iDay << " Time : " << time << " sec" << std::endl;
#endif
			++date;

		} while (date <= endDate);

		mapSunDir.clear();

#ifdef CHRONO
		auto tend = std::chrono::system_clock::now();
		time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(tend - tstart).count() * 0.001);
		ofs << "Surface Time: " << time << " sec" << std::endl;
#endif
	}

#ifdef CHRONO
	auto cend = std::chrono::system_clock::now();
	time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(cend - cstart).count() * 0.001);
	ofs << "Calc Time: " << time << " sec" << std::endl;
#endif

	return true;
}

// 土地の日射量算出
bool CCalcSolarRadiation::ExecLand(
	CPotentialData& result,					// 計算結果格納用データマップ
	const CTime& startDate,					// 解析する期間の開始日時
	const CTime& endDate					// 解析する期間の終了日時
)
{
	if (!m_pMng)	return false;

	int JPZONE = GetINIParam()->GetJPZone();

	int iYear = m_pMng->GetYear();

	for (auto& [surfaceId, surfaceData] : result.mapSurface)
	{
		// 傾斜角、方位角
		double slopeDegree = surfaceData.slopeModDegree;
		double azDegree = surfaceData.azModDegree;

		double surfaceAngle = slopeDegree * _COEF_DEG_TO_RAD;	// 斜面の傾斜角
		double surfaceAz = azDegree * _COEF_DEG_TO_RAD;			// 斜面の方位角

		for (auto& mesh : surfaceData.vecMeshData)
		{
			// 隣接する建物を取得
			std::vector<BLDGLIST> neighborBuildings;
			m_pMng->GetNeighborBuildings(mesh.center, neighborBuildings);

			// 隣接するDEMを取得
			std::vector<CTriangle> neighborDems;
			m_pMng->GetNeighborDems(mesh.center, neighborDems, CCalcSolarPotentialMng::eAnalyzeTarget::LAND);

			double dLat = 0.0, dLon = 0.0;
			CGeoUtil::XYToLatLon(JPZONE, mesh.center.y, mesh.center.x, dLat, dLon);

			CTime date = startDate;
			do {
				if (m_pMng->IsCancel())	return false;

				double dSunnyVal = 0.0; double dCloudVal = 0.0;
				int month = date.GetMonth();

				double p = GetINIParam()->GetTransmissivity(month);		// 大気透過率
				double pdifCloud = 1.0 - p;								// 透過率(曇天時)

				const CSunVector outSun(dLat, dLon, date);

				for (int hour = 0; hour < 24; hour++)
				{
					HorizontalCoordinate pos;
					outSun.GetPos(hour, pos);
					double sunAngle = pos.altitude;	// 太陽天頂角(太陽高度)	
					if (sunAngle < 0)
					{
						//太陽高度が0未満
						continue;
					}
					double alpha = pos.azimuth + _PI;		// 太陽方位

					int idx = date.iYDayCnt * 24 + hour;

					// 反射率
					double refRate = 0.0;	// 反射率
					date.iHour = hour;
					double depth = this->m_pMng->GetMetpvData()->GetSnowDepth(date);
					if (depth > 10.0)		// 10cm以上
					{
						refRate = GetINIParam()->GetReflectivitySnow();
					}
					else
					{
						refRate = GetINIParam()->GetReflectivity();
					}

					// 入射角
					double angIn = calcAngleIn(sunAngle, surfaceAngle, surfaceAz, alpha);
					if (angIn < 0.0)	angIn = 0.0;

					CVector3D sunVector;
					outSun.GetVector(hour, sunVector);

					// 直達成分の有無を判定
					bool bDirect = m_pMng->IntersectSurfaceCenter(sunVector, mesh.meshPos, mesh.centerMod, surfaceId, neighborBuildings, neighborDems);

					dSunnyVal += calcSlopeDif(surfaceAngle, angIn, sunAngle, refRate, bDirect, p);
					dCloudVal += calcSlopeDif(surfaceAngle, angIn, sunAngle, refRate, bDirect, pdifCloud);
				}

				// ひと月ごとに値を適用
				mesh.solarRadiationSunny[month - 1] += dSunnyVal;
				mesh.solarRadiationCloud[month - 1] += dCloudVal;

				++date;

			} while (date <= endDate);

		}
	}

	return true;
}

bool CCalcSolarRadiation::ModifySunRate(CPotentialData& result)
{
	if (!m_pMng)	return false;

	for (auto& surface : result.mapSurface)
	{
		CSurfaceData& surfaceData = surface.second;

		for (auto& mesh : surfaceData.vecMeshData)
		{
			for (int month = 1; month <= 12; month++)
			{
				// 晴天時の日射量 × 晴天時の日照率(Ⅰ)
				double sunnyVal = mesh.solarRadiationSunny[month - 1] * m_pMng->GetRadiationData()->sunnyRate[month - 1];
				// 曇天時の日射量 × 曇天時の日照率(Ⅱ)
				double cloudVal = mesh.solarRadiationCloud[month - 1] * m_pMng->GetRadiationData()->cloudRate[month - 1];
				double val = sunnyVal + cloudVal;
				mesh.solarRadiation[month - 1] = val;	// 補正した日射量を適用
			}
		}
	}

	return true;
}

// 斜面日射量の算出
inline double CCalcSolarRadiation::calcSlopeDif(
	const double& surfaceAngle,	// 傾斜角
	const double& angIn,		// 入射角
	const double& sunAngle,		// 太陽高度
	const double& refRate,		// 反射率
	const bool& bDirect,		// 直達成分の有無
	const double& pdif			// 透過率
)
{
	double sinA = sin(sunAngle);
	double m = 1 / sinA;
	double betaM = pow(pdif, m);

	// 法線面直達日射量(W/m2)
	double dDN = 0.0;
	if (bDirect)
	{
		dDN = def_Sconst * betaM;
	}

	// 水平面天空日射量(W/m2)
	double dN = 1.0 - betaM;
	double dD = 1.0 - 1.4 * log(pdif);
	double dSH = def_Sconst * sinA * (dN / dD) * 0.5;

	// 斜面直達日射量
	double dDT = 0.0;
	if (bDirect)
	{
		dDT = dDN * angIn;
	}

	// 斜面天空日射量
	double dST = dSH * (1 + cos(surfaceAngle)) * 0.5;

	// 水平面全天日射量
	double dTH = dDN * sinA + dSH;

	// 斜面に入射する反射日射量
	double dRT = dTH * (1 - cos(surfaceAngle)) * 0.5 * refRate;

	// 斜面全天日射量
	double dTT = dDT + dST + dRT;

	return dTT;
}

// 入射角算出
inline double CCalcSolarRadiation::calcAngleIn(double sunAngle, double surfaceAngle, double surfaceAz, double alpha)
{
	return sin(sunAngle) * cos(surfaceAngle) + cos(sunAngle) * sin(surfaceAngle) * cos(alpha - surfaceAz);
}

// 年間日射量
bool CCalcSolarRadiation::CalcAreaSolarRadiation(CPotentialData& result)
{
	if (!m_pMng)	return false;

	// 集計
	calcTotalSolarRadiation(result);

	return true;
}


// 集計
void CCalcSolarRadiation::calcTotalSolarRadiation(CPotentialData& result)
{
	for ( auto& surface : result.mapSurface)
	{
		// キャンセル検知
		if (m_pMng->IsCancel())
		{
			return;
		};

		std::string surfaceId = surface.first;
		CSurfaceData& surfaceData = surface.second;

		double sumVal = 0.0;

		for (auto& mesh : surfaceData.vecMeshData)
		{
			for (int month = 0; month < 12; month++)
			{
				sumVal += mesh.solarRadiation[month];
				mesh.solarRadiationUnit += mesh.solarRadiation[month];
			}

			mesh.solarRadiationUnit *= 0.001;	// kWhに変換
		}

		double areaunit = (double)surfaceData.vecMeshData.size();

		// 単位面積当たりの日射量
		surfaceData.solarRadiationUnit = sumVal / areaunit;		// 平均(1m2あたり)
		surfaceData.solarRadiationUnit *= 0.001;				// kWhに変換

		// 対象面全体の日射量(kWh)
		double area = surfaceData.GetArea() * m_pMng->GetUIParam()->pSolarPotentialParam->dPanelRatio;	// PV設置割合を適用した面積
		surfaceData.solarRadiation = surfaceData.solarRadiationUnit * area;

		// 総計(kWh)
		result.solarRadiationTotal += surfaceData.solarRadiation;
	}

	// 1m2当たりの年間日射量を算出
	result.panelArea = result.GetAllArea() * m_pMng->GetUIParam()->pSolarPotentialParam->dPanelRatio;	// PV設置割合を適用した総面積
	if (result.panelArea > 0.0)
	{
		result.solarRadiationUnit = result.solarRadiationTotal / result.panelArea;
	}

}

