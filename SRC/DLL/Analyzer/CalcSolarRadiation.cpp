#include "pch.h"
#include <math.h>
#include <random>
#include <iostream>
#include <unordered_map>
#include "CalcSolarRadiation.h"
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "../../LIB/CommonUtil/CSunVector.h"
#include "../../LIB/CommonUtil/ReadINIParam.h"

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

// 3.太陽軌道をもとにした日射量の算出
bool CCalcSolarRadiation::Exec(
	CBuildingDataMap& dataMap,				// 計算結果格納用データマップ
	const bool& bModDegree,					// true:補正角度データを使用
	const std::wstring& wstrPath,			// 中間CSV出力ファイル名
	const std::string Lv3meshId				// 3次メッシュID
)
{
	if (!m_pMng)	return false;

	int JPZONE = GetINIParam()->GetJPZone();

	int iYear = m_pMng->GetYear();

	for (auto& bld : dataMap)
	{
		// キャンセル検知
		if (m_pMng->IsCancel())
		{
			return false;
		};

		CBuildingData& bldData = bld.second;
		const CVector3D bldCenter = bldData.center;

		// 隣接する建物を取得
		std::vector<BLDGLIST> neighborBuildings;
		m_pMng->GetNeighborBuildings(Lv3meshId, bldCenter, neighborBuildings);

		// 隣接するDEMを取得
		std::vector<DEMLIST> neighborDems;
		if (m_pMng->IsEnableDEMData())
		{
			m_pMng->GetNeighborDems(Lv3meshId, bldCenter, neighborDems);
		}

		for (auto& surface : bldData.mapRoofSurface)
		{
			std::string surfaceId = surface.first;
			CRoofSurfaceData& surfaceData = surface.second;

			// 直達成分の有無を判定
			std::unordered_map<int, bool> mapSunDir;
			double dLat = 0.0, dLon = 0.0;
			CGeoUtil::XYToLatLon(JPZONE, surfaceData.center.y, surfaceData.center.x, dLat, dLon);

			for (auto& mesh : surfaceData.vecRoofMesh)
			{
				double slopeDegree = 0.0, azDegree = 0.0;	// 傾斜角、方位角

				if (!bModDegree)
				{	// 補正しない
					slopeDegree = surfaceData.slopeDegreeAve;
					azDegree = surfaceData.azDegreeAve;
				}
				else
				{	// 補正する
					slopeDegree = surfaceData.slopeModDegree;
					azDegree = surfaceData.azModDegree;
				}

				double surfaceAngle = slopeDegree * _COEF_DEG_TO_RAD;	// 斜面の傾斜角
				double surfaceAz = azDegree * _COEF_DEG_TO_RAD;			// 斜面の方位角

				for (int month = 1; month <= 12; month++)
				{
					double dSunnyVal = 0.0; double dCloudVal = 0.0;

					double p = GetINIParam()->GetTransmissivity(month);		// 大気透過率
					double pdifCloud = 1.0 - p;								// 透過率(曇天時)

					int dnum = CTime::GetDayNum(month);
					for (int day = 1; day <= dnum; day++)
					{
						CTime date(iYear, month, day, 0, 0, 0);
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
							if (mapSunDir.count(idx) == 0)
							{
								mapSunDir[idx] = m_pMng->IntersectRoofSurfaceCenter(sunVector, surfaceData.bbPos, surfaceId, neighborBuildings, neighborDems);
							}

							bool bDirect = mapSunDir.at(idx);

							dSunnyVal += calcSlopeDif(surfaceAngle, angIn, sunAngle, refRate, bDirect, p);
							dCloudVal += calcSlopeDif(surfaceAngle, angIn, sunAngle, refRate, bDirect, pdifCloud);

						}
					}

					// ひと月ごとに値を適用
					mesh.solarRadiationSunny[month - 1] = dSunnyVal;
					mesh.solarRadiationCloud[month - 1] = dCloudVal;
				}

			}
			mapSunDir.clear();
		}
	}

	return true;
}

bool CCalcSolarRadiation::ModifySunRate
(
	CBuildingDataMap& dataMap,				// 計算結果格納用データマップ
	const std::wstring& wstrPath			// 中間CSV出力ファイル名
)
{
	if (!m_pMng)	return false;

	for (auto& bld : dataMap)
	{
		// キャンセル検知
		if (m_pMng->IsCancel())
		{
			return false;
		};

		std::string buildId = bld.first;
		CBuildingData& bldData = bld.second;

		for (auto& surface : bldData.mapRoofSurface)
		{
			std::string surfaceId = surface.first;
			CRoofSurfaceData& surfaceData = surface.second;

			for (auto& mesh : surfaceData.vecRoofMesh)
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
	}

	return true;
}

// 斜面日射量の算出
double CCalcSolarRadiation::calcSlopeDif(
	const double& surfaceAngle,	// 傾斜角
	const double& angIn,		// 入射角
	const double& sunAngle,		// 太陽高度
	const double& refRate,		// 反射率
	const bool& bDirect,		// 直達成分の有無
	const double& pdif			// 透過率
)
{
	// 法線面直達日射量(W/m2)
	double dDN = 0.0;
	if (bDirect)	dDN = calcDirectNormal(sunAngle, pdif);

	// 水平面天空日射量(W/m2)
	double dSH = calcSkyHorizon(sunAngle, pdif);

	// 斜面直達日射量
	double dDT = 0.0;
	if (bDirect)	dDT = calcDirectSlope(dDN, angIn);

	// 斜面天空日射量
	double dST = calcSkySlope(dSH, surfaceAngle);

	// 水平面全天日射量
	double dTH = calcSolarHorizon(dDN, dSH, sunAngle);

	// 斜面に入射する反射日射量
	double dRT = calcRefrectSlope(dTH, surfaceAngle, refRate);

	// 斜面全天日射量
	double dTT = dDT + dST + dRT;

	return dTT;
}

// 入射角算出
inline double CCalcSolarRadiation::calcAngleIn(double sunAngle, double surfaceAngle, double surfaceAz, double alpha)
{
	return sin(sunAngle) * cos(surfaceAngle) + cos(sunAngle) * sin(surfaceAngle) * cos(alpha - surfaceAz);
}

// 法線面直達日射量(W/m2)
inline double CCalcSolarRadiation::calcDirectNormal(double sunAngle, double pdif)
{
	double m = 1 / sin(sunAngle);
	double betaM = pow(pdif, m);
	return def_Sconst * betaM;
}

// 水平面天空日射量(W/m2)
inline double CCalcSolarRadiation::calcSkyHorizon(double sunAngle, double pdif)
{
	double m = 1 / sin(sunAngle);
	double betaM = pow(pdif, m);
	double dN = 1.0 - betaM;
	double dD = 1.0 - 1.4 * log(pdif);
	return def_Sconst * sin(sunAngle) * (dN / dD) * 0.5;
}

// 斜面直達日射量
inline double CCalcSolarRadiation::calcDirectSlope(double dDN, double angIn)
{
	return dDN * angIn;
}

// 斜面天空日射量
inline double CCalcSolarRadiation::calcSkySlope(double dSH, double surfaceAngle)
{
	return dSH * (1 + cos(surfaceAngle)) * 0.5;
}

// 水平面全天日射量
inline double CCalcSolarRadiation::calcSolarHorizon(double dDN, double dSH, double sunAngle)
{
	return dDN * sin(sunAngle) + dSH;
}

// 斜面に入射する反射日射量
inline double CCalcSolarRadiation::calcRefrectSlope(double dTH, double surfaceAngle, double refRate)
{
	return dTH * (1 - cos(surfaceAngle)) * 0.5 * refRate;
}

// 建物別年間日射量
bool CCalcSolarRadiation::CalcBuildSolarRadiation(CBuildingDataMap& dataMap, const std::wstring& wstrOutDir)
{
	if (!m_pMng)	return false;

	// 集計
	calcTotalSolarRadiation(dataMap, wstrOutDir);

	return true;
}


// 集計
void CCalcSolarRadiation::calcTotalSolarRadiation(CBuildingDataMap& dataMap, const std::wstring& wstrOutDir)
{
	for (auto& bld : dataMap)
	{
		std::string buildId = bld.first;
		CBuildingData& build = bld.second;

		for ( auto& surface : build.mapRoofSurface)
		{
			// キャンセル検知
			if (m_pMng->IsCancel())
			{
				return;
			};

			std::string surfaceId = surface.first;
			CRoofSurfaceData& surfaceData = surface.second;

			double sumVal = 0.0;

			for (const auto& mesh : surfaceData.vecRoofMesh)
			{
				for (int month = 0; month < 12; month++)
				{
					sumVal += mesh.solarRadiation[month];
				}
			}

			int meshsize = (int)surfaceData.vecRoofMesh.size();

			// 1m2当たりの日射量
			surfaceData.solarRadiationUnit = sumVal / meshsize;	// 平均
			surfaceData.solarRadiationUnit *= 0.001;			// kWhに変換

			// 屋根面全体の日射量(kWh)
			surfaceData.solarRadiation = surfaceData.solarRadiationUnit * surfaceData.GetRoofArea();

			// 総計(kWh)
			build.solarRadiationTotal += surfaceData.solarRadiation;
		}

		// 建物毎の1m2当たりの年間日射量を算出
		double area = build.GetAllRoofArea();
		if (area > 0.0)
		{
			build.solarRadiationUnit = build.solarRadiationTotal / build.GetAllRoofArea();
		}

	}

}

