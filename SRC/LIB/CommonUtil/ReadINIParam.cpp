#include "pch.h"
#include "ReadINIParam.h"
#include "CFileUtil.h"

CReadINIParam	CReadINIParam::m_instance;		// インスタンス

/*! コンストラクタ
*/
CReadINIParam::CReadINIParam(void)
	: m_iJPZone(0)
	, m_strAzimuthCSVPath(L"")
	, m_dReflectivity(0.0)
	, m_dReflectivitySnow(0.0)
	, m_dDemHeight_Build(0.0)
	, m_dDemHeight_Land(0.0)
	, m_dDemDist(0.0)
{
	for (int i = 0; i < 12; i++)
	{
		m_dTransmissivity[i] = 0.0;
	}
}

/*! デストラクタ
*/
CReadINIParam::~CReadINIParam(void)
{
}

/*!	初期化
@retval	true	成功
@retval	false	失敗
@note	Iniファイルの読み込みを行います。
*/
bool CReadINIParam::Initialize()
{
	bool		bRet = false;
	CINIFileIO	inifile;

	// ファイルOPEN
	std::string strFilePath = GetFUtil()->Combine(GetFUtil()->GetModulePath(), "SolarPotential.ini");
	bRet = inifile.Open(strFilePath);

	// [CoordinateSystem]
	// 系番号 デフォルト値: 7
	m_iJPZone = inifile.GetInt("CoordinateSystem", "JPZone", 7);

	// [File]
	std::string str = inifile.GetString("File", "AzimuthCSVPath", "");
	m_strAzimuthCSVPath = CStringEx::ToWString(str);
	
	// [SolarRadiation]
	// 大気透過率(P) デフォルト値: 0.6
	for (int i = 0; i < 12; i++)
	{
		std::string strKey;
		strKey = CStringEx::Format("Transmissivity%d", i+1);
		m_dTransmissivity[i] = inifile.GetDouble("SolarRadiation", strKey, 0.6);
	};

	// 反射率(R) デフォルト値: 0.1
	m_dReflectivity = inifile.GetDouble("SolarRadiation", "Reflectivity", 0.1);

	// 積雪時の反射率(R)
	m_dReflectivitySnow = inifile.GetDouble("SolarRadiation", "ReflectivitySnow", 0.7);

	// 標高しきい値[m]
	m_dDemHeight_Build = inifile.GetDouble("SolarRadiation", "DemHeight_Build", 10.0);

	// 標高しきい値[m]
	m_dDemHeight_Land = inifile.GetDouble("SolarRadiation", "DemHeight_Land", 1.0);

	// 判定対象とするDEMの距離[m]
	m_dDemDist = inifile.GetDouble("SolarRadiation", "DemDist", 100.0);

	// 近隣建物の検索範囲[m]
	m_dBuildDist_SolarRad = inifile.GetDouble("SolarRadiation", "NeighborBuildDist", 500.0);

	// [ReflectionSimulator]
	// 近隣建物の検索範囲[m]
	m_dBuildDist_Reflection = inifile.GetDouble("ReflectionSimulator", "NeighborBuildDist", 500.0);

	return bRet;
}

#pragma region "平面直角座標系設定"

int CReadINIParam::GetJPZone()
{
	return	m_iJPZone;
}

#pragma endregion

#pragma region "入出力ファイルパス設定(中間ファイル等)"

std::wstring CReadINIParam::GetAzimuthCSVPath()
{
	return	m_strAzimuthCSVPath;
}

#pragma endregion

#pragma region "日射量推計"

// 大気透過率(P)の取得
double CReadINIParam::GetTransmissivity(const int& month)
{
	if (month >= 1 && month <= 12)
	{
		return m_dTransmissivity[month-1];

	}
	return 0.0;
}

// 反射率(R)の取得
double CReadINIParam::GetReflectivity()
{
	return	m_dReflectivity;
}

// 反射率(R)の取得
double CReadINIParam::GetReflectivitySnow()
{
	return	m_dReflectivitySnow;
}

// 建物解析用のDEM標高しきい値[m]
double CReadINIParam::GetDemHeight_Build()
{
	return	m_dDemHeight_Build;
}

// 建物解析用のDEM標高しきい値[m]
double CReadINIParam::GetDemHeight_Land()
{
	return	m_dDemHeight_Land;
}

// 判定対象とするDEMの距離[m]
double CReadINIParam::GetDemDist()
{
	return	m_dDemDist;
}

// 近隣建物の検索範囲[m]
double CReadINIParam::GetNeighborBuildDist_SolarRad()
{
	return	m_dBuildDist_SolarRad;

}

#pragma endregion

#pragma region "反射シミュレーション"


// [ReflectionSimulator]
// 近隣建物の検索範囲[m]
double CReadINIParam::GetNeighborBuildDist_Reflection()
{
	return	m_dBuildDist_Reflection;
}

#pragma endregion
