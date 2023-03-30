#pragma once
#include <string>
#include <algorithm>
#include <iterator>
#include <vector>
#include "AggregateData.h"
#include "../../LIB/CommonUtil/CGeoUtil.h"

// 建物
class CBuilding
{
public:
	int			m_iMeshId;						// メッシュID
	std::string m_strBuildingId;				// 建物ID
	std::vector<ROOFSURFACES>* m_pRoofSurfaceList;	// 屋根リスト

	double dBldHeight;					// 建物高さ
	double dSolorRadiation;				// 年間予測日射量
	int iBldStructureType;				// 構造種別
	double dFloodDepth;					// 洪水浸水想定の浸水深(メートル) 
										// 複数の川の洪水浸水想定のタグを持つ場合は深い方の浸水深を受け取る
	double dTsunamiHeight;				// 津波浸水想定(メートル)
	bool bLandslideArea;				// 土砂災害警戒区域
	int iBldStructureType2;				// 都市ごとの独自区分に基づく建築構造の種類
	int iFloorType;						// 都市ごとの独自区分に基づく地上階数の範囲

	// 日射量計算時の屋根の方位角
	std::vector<double> vecRoofAzimuth;	// 解析処理の対象となった屋根面の方位角

	// コンストラクタ
	CBuilding()
		: m_iMeshId(0)
		, m_strBuildingId("")
		, m_pRoofSurfaceList(NULL)
		, dBldHeight(-1.0)
		, dSolorRadiation(-1.0)
		, iBldStructureType(-1)
		, dFloodDepth(-1.0)
		, dTsunamiHeight(-1.0)
		, bLandslideArea(false)
		, iBldStructureType2(-1)
		, iFloorType(-1)
	{
	};
	void CalcBounding(double* pMinX = NULL, double* pMinY = NULL, double* pMaxX = NULL, double* pMaxY = NULL, double* pMinZ = NULL, double* pMaxZ = NULL) const;
	bool IsBuildingInPolygon(unsigned int uiCountPoint, const CPoint2D* pPoint);
};

// 建物群
class CBuildingData
{
public:
	CBuildingData(void);
	~CBuildingData(void);

	size_t GetBuildingSize()
	{
		return m_vecBuilding.size();
	};
	CBuilding* GetBuildingAt(int idx)
	{
		return &m_vecBuilding.at(idx);
	};
	void Add(CBuilding building)
	{
		m_vecBuilding.push_back(building);
	};
	int GetSolorRadiationOrder(std::string strBuildingID);
	bool ReadAzimuthCSV(std::wstring strFilePath);

private:
	
	std::vector<CBuilding> m_vecBuilding;	// データ配列
	std::vector<CBuilding> m_vecSortCopy;			// ソート済みデータ配列

	// ソート(日射量)
	void sortSolorRadiation(std::vector<CBuilding>* vecSortData);
	static int compareSolorRadiation(void* context, const void* a1, const void* a2);

};

