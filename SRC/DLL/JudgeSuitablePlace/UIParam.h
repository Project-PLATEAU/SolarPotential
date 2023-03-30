#pragma once
#include <string>
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "../../LIB/CommonUtil/ReadINIParam.h"

// 建築構造の種類
enum class eBuildingStructure
{
	WOOD						= 0,	// 木造・土蔵造
	STEEL_REINFORCED_CONCRETE,			// 鉄骨鉄筋コンクリート造
	REINFORCED_CONCRETE,				// 鉄筋コンクリート造
	STEEL,								// 鉄骨造
	LIGHT_GAUGE_STEEL,					// 軽量鉄骨造
	MASONRY_CONSTRUCTION,				// レンガ造・コンクリートブロック造・石造
	NON_WOOD,							// 非木造
	UNKNOWN,							// 不明
};

// 方位
enum class eDirections
{
	UNKNOWN = 0,			// 不明
	NORTH = 1,				// 北		0°
	NORTH_NORTHEAST,		// 北北東	22.5°
	NORTHEAST,				// 北東		45°
	EAST_NORTHEAST,			// 東北東	67.5°
	EAST,					// 東		90°
	EAST_SOUTHEAST,			// 東南東	112.5°
	SOUTHEAST,				// 南東		135°
	SOUTH_SOUTHEAST,		// 南南東	157.5°
	SOUTH,					// 南		180°
	SOUTH_SOUTHWEST,		// 南南西	202.5°
	SOUTHWEST,				// 南西		225°
	WEST_SOUTHWEST,			// 西南西	247.5°
	WEST,					// 西		270°
	WEST_NORTHWEST,			// 西北西	292.5°
	NORTHWEST,				// 北西		315°
	NORTH_NORTHWEST,		// 北北西	337.5°
};


// 集計範囲
class CAggregationRange
{
public:
	double dMaxLat;		// 最大緯度
	double dMinLon;		// 最小経度
	double dMaxLon;		// 最大経度
	double dMinLat;		// 最小緯度
	double dMaxY;		// 平面直角座標最大Y座標
	double dMinX;		// 平面直角座標最小X座標
	double dMaxX;		// 平面直角座標最大X座標
	double dMinY;		// 平面直角座標最小Y座標
	CAggregationRange(const double& d1, const double& d2, const double& d3, const double& d4)
		: dMaxLat(-DBL_MAX)
		, dMinLon(DBL_MAX)
		, dMaxLon(-DBL_MAX)
		, dMinLat(DBL_MAX)
		, dMaxY(-DBL_MAX)
		, dMinX(DBL_MAX)
		, dMaxX(-DBL_MAX)
		, dMinY(DBL_MAX)
	{
		dMaxLat = d1; dMinLon = d2; dMaxLon = d3; dMinLat = d4;
		// 緯度経度　→　平面直角座標系に変換
		int iJPZone = GetINIParam()->GetJPZone(); // 座標系
		double dEast, dNorth;
		CGeoUtil::LonLatToXY(dMinLon, dMinLat, iJPZone, dEast, dNorth);
		dMinX = dEast; dMinY = dNorth;
		CGeoUtil::LonLatToXY(dMaxLon, dMaxLat, iJPZone, dEast, dNorth);
		dMaxX = dEast; dMaxY = dNorth;
	}

	CAggregationRange& operator = (const CAggregationRange& v) {
		if (&v != this) { dMaxLat = v.dMaxLat; dMinLon = v.dMinLon; dMaxLon = v.dMaxLon; dMinLat = v.dMinLat;
		}
		return *this;
	}
};

// 太陽光パネルの設置に関して優先度が低い施設の判定パラメータ
class CBuildingParam
{
public:
	// 日射量が少ない施設を除外
	double dSolorRadiation;				// kWh/m2 未満
	double dLowerPercent;				// 下位％
	// 建物構造による除外
	int    iBuildingStructure;			// 建築構造の種類
	// 特定の階層の施設を除外
	int iFloorsBelow;					// 〇階以下
	int iUpperFloors;					// 〇階以上

	CBuildingParam(const double& d1, const double& d2, const int& d3, const int& d4, const int& d5)
		: dSolorRadiation(0.0)
		, dLowerPercent(0.0)
		, iBuildingStructure(0)
		, iFloorsBelow(-1)
		, iUpperFloors(-1)
	{
		dSolorRadiation = d1; dLowerPercent = d2; iBuildingStructure = d3; iFloorsBelow = d4; iUpperFloors = d5;
	}

	CBuildingParam& operator = (const CBuildingParam& v) {
		if (&v != this) {
			dSolorRadiation = v.dSolorRadiation; dLowerPercent = v.dLowerPercent;
			iBuildingStructure = v.iBuildingStructure;
			iFloorsBelow = v.iUpperFloors; iFloorsBelow = v.iFloorsBelow;
		}
		return *this;
	}
};

// 災害時に太陽光パネルが破損、消失する危険性の判定
class CHazardParam
{
public:
	bool bBelowTsunamiHeight;			// 高さが想定される最大津波高さを下回る建物を除外
	bool bBelowFloodDepth;				// 建物高さが想定される河川浸水想定浸水深を下回る建物を除外
	bool bLandslideWarningArea;			// 土砂災害警戒区域内に存在する建物を除外
	std::string strWeatherDataPath;		// 気象データ(積雪)フォルダパス
	double dSnowDepth;					// 気象データ(積雪)_cm以上
	double dS;							// 積雪荷重(kgf / m3)
	double dp;							// 単位荷重(N / m3)

	CHazardParam(const bool& d1, const bool& d2, const bool& d3, const std::string& d4, const double& d5, const double& d6, const double& d7)
		: bBelowTsunamiHeight(false)
		, bBelowFloodDepth(false)
		, bLandslideWarningArea(false)
		, strWeatherDataPath("")
		, dSnowDepth(-1.0)
		, dS(0.0)
		, dp(0.0)
	{
		bBelowTsunamiHeight = d1; bBelowFloodDepth = d2; bLandslideWarningArea = d3;
		strWeatherDataPath = d4; dSnowDepth = d5; dS = d6; dp = d7;
	}

	CHazardParam& operator = (const CHazardParam& v) {
		if (&v != this) {
			bBelowTsunamiHeight = v.bBelowTsunamiHeight; bBelowFloodDepth = v.bBelowFloodDepth; bLandslideWarningArea = v.bLandslideWarningArea;
			strWeatherDataPath = v.strWeatherDataPath; dSnowDepth = v.dSnowDepth; dS = v.dS; dp = v.dp;
		}
		return *this;
	}
};


// 太陽光パネルの設置に制限がある施設の判定パラメータ
class CRestrictParam
{
public:
	// 制限を設ける範囲のシェープファイル_1
	std::string strRestrictAreaPath_1;		// フォルダパス１
	double		dHeight_1;					// 高さ１
	eDirections eDirections_1;				// 方位１
	// 制限を設ける範囲のシェープファイル_2
	std::string strRestrictAreaPath_2;		// フォルダパス２
	double		dHeight_2;					// 高さ２
	eDirections eDirections_2;				// 方位２
	// 制限を設ける範囲のシェープファイル_3
	std::string strRestrictAreaPath_3;		// フォルダパス３
	double		dHeight_3;					// 高さ３
	eDirections eDirections_3;				// 方位３

	CRestrictParam(const std::string& d1, const double& d2, const eDirections& d3, const std::string& d4, const double& d5, const eDirections& d6, const std::string& d7, const double& d8, const eDirections& d9)
		: strRestrictAreaPath_1("")
		, dHeight_1(0.0)
		, eDirections_1(eDirections::UNKNOWN)
		, strRestrictAreaPath_2("")
		, dHeight_2(0.0)
		, eDirections_2(eDirections::UNKNOWN)
		, strRestrictAreaPath_3("")
		, dHeight_3(0.0)
		, eDirections_3(eDirections::UNKNOWN)
	{
		strRestrictAreaPath_1 = d1; dHeight_1 = d2; eDirections_1 = d3;
		strRestrictAreaPath_2 = d4; dHeight_2 = d5; eDirections_2 = d6;
		strRestrictAreaPath_3 = d7; dHeight_3 = d8; eDirections_3 = d9;
	}

	CRestrictParam& operator = (const CRestrictParam& v) {
		if (&v != this) {
			strRestrictAreaPath_1 = v.strRestrictAreaPath_1; dHeight_1 = v.dHeight_1; eDirections_1 = v.eDirections_1;
			strRestrictAreaPath_2 = v.strRestrictAreaPath_2; dHeight_2 = v.dHeight_2; eDirections_2 = v.eDirections_2;
			strRestrictAreaPath_3 = v.strRestrictAreaPath_3; dHeight_3 = v.dHeight_3; eDirections_3 = v.eDirections_3;
		}
		return *this;
	}
};

class CUIParam
{
public:
	CUIParam()
		: m_pAggregationRange(NULL)
		, m_pBuildingParam(NULL)
		, m_pHazardParam(NULL)
		, m_pRestrictParam(NULL)
		, m_strOutputDirPath(L"")
		, m_strBldgResultPath(L"")
	{
	
	};
	~CUIParam()
	{
		delete m_pAggregationRange;
		delete m_pBuildingParam;
		delete m_pHazardParam;
		delete m_pRestrictParam;
	};

	CAggregationRange*	m_pAggregationRange;
	CBuildingParam*		m_pBuildingParam;
	CHazardParam*		m_pHazardParam;
	CRestrictParam*		m_pRestrictParam;

	std::wstring		m_strOutputDirPath;
	std::wstring		m_strBldgResultPath;
}; 