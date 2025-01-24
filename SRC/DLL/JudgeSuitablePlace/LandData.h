#pragma once
#include <string>
#include <algorithm>
#include <iterator>
#include <vector>
#include "AggregateData.h"
#include "../../LIB/CommonUtil/CGeoUtil.h"

// 建物
class CLand
{
public:
	std::string	m_strAreaId;			// エリア(土地)ID
	std::vector<CPoint2D>* m_pSurface;	// 土地面

	double dLandHeight;					// 建物高さ
	double dSolorRadiation;				// 予測日射量
	double dFloodDepth;					// 洪水浸水想定の浸水深(メートル) 
										// 複数の川の洪水浸水想定のタグを持つ場合は深い方の浸水深を受け取る
	double dTsunamiHeight;				// 津波浸水想定(メートル)
	bool bLandslideArea;				// 土砂災害警戒区域

	// 日射量計算時の方位角
	std::vector<double> vecAzimuth;		// 解析処理の対象となった土地面の方位角

	// コンストラクタ
	CLand()
		: m_strAreaId("")
		, m_pSurface(NULL)
		, dLandHeight(-1.0)
		, dSolorRadiation(-1.0)
		, dFloodDepth(-1.0)
		, dTsunamiHeight(-1.0)
		, bLandslideArea(false)
	{
	};
	void CalcBounding(double* pMinX = NULL, double* pMinY = NULL, double* pMaxX = NULL, double* pMaxY = NULL, double* pMinZ = NULL, double* pMaxZ = NULL) const;
	bool IsLandInPolygon(unsigned int uiCountPoint, const CPoint2D* pPoint);
};

// 建物群
class CLandData
{
public:
	CLandData(void);
	~CLandData(void);

	size_t GetLandSize()
	{
		return m_vecLand.size();
	};
	CLand* GetLandAt(int idx)
	{
		return &m_vecLand.at(idx);
	};
	void Add(CLand land)
	{
		m_vecLand.push_back(land);
	};
	int GetSolorRadiationOrder(std::string strLandID);
	bool ReadAzimuthCSV(std::wstring strFilePath);

private:

	std::vector<CLand> m_vecLand;			// データ配列
	std::vector<CLand> m_vecSortCopy;		// ソート済みデータ配列

	// ソート(日射量)
	void sortSolorRadiation(std::vector<CLand>* vecSortData);
	static int compareSolorRadiation(void* context, const void* a1, const void* a2);

};

