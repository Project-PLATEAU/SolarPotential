#pragma once
#include "CINIFileIO.h"

/*! INIファイル読み込みクラス
	読み取り専用
*/
class CReadINIParam
{
public:
	static CReadINIParam* GetInstance() { return &m_instance; }
	bool Initialize();
	
	// [CoordinateSystem]
	int					GetJPZone();				// 系番号

	// [File]
	std::wstring		GetAzimuthCSVPath();		// 建物IDごとの方位角データCSVファイルパス

	// [SolarRadiation]
	double				GetTransmissivity(const int& month);	// 大気透過率(P)
	double				GetReflectivity();						// 反射率(R)
	double				GetReflectivitySnow();					// 反射率(R)
	double				GetDemHeight();							// 標高しきい値[m]
	double				GetDemDist();							// 判定対象とするDEMの距離[m]
	double				GetNeighborBuildDist_SolarRad();		// 近隣建物の検索範囲[m]

	// [ReflectionSimulator]
	double				GetNeighborBuildDist_Reflection();		// 近隣建物の検索範囲[m]


private:
	CReadINIParam(void);
	virtual ~CReadINIParam(void);

	static CReadINIParam	m_instance;				//!< 自クラスの唯一のインスタンス

	// [CoordinateSystem]
	int				m_iJPZone;				// 系番号

	// [File]
	std::wstring	m_strAzimuthCSVPath;	// 建物IDごとの方位角データCSVファイルパス

	// [SolarRadiation]
	double			m_dTransmissivity[12];	// 大気透過率(P)
	double			m_dReflectivity;		// 反射率(R)
	double			m_dReflectivitySnow;	// 積雪時の反射率(R)
	double			m_dDemHeight;			// 標高しきい値[m]
	double			m_dDemDist;				// 判定対象とするDEMの距離[m]
	double			m_dBuildDist_SolarRad;		// 近隣建物の検索範囲[m]
	double			m_dBuildDist_Reflection;	// 近隣建物の検索範囲[m]

};

#define GetINIParam() (CReadINIParam::GetInstance())

