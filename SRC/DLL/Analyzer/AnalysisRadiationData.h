#pragma once
#include <map>
#include "..\..\LIB\CommonUtil\CSunVector.h"

// 発電ポテンシャル推計
// 日射量共通
class CAnalysisRadiationCommon
{
public:
	// 晴天時の日照率（都市ごと）
	double sunnyRate[12]; // 月ごとの日照時間/可照時間
	// 曇天時の日照率（都市ごと）
	double cloudRate[12]; // 月ごと 1 - 晴天時の日照率


};

// 1メッシュごとの発電ポテンシャル推計データ
class CMeshData
{
public:
	std::string meshId;		// ID

	std::vector<CVector3D> meshPos;		// メッシュ情報
	CVector3D center;		// メッシュ中心
	CVector3D centerMod;	// メッシュ中心(角度補正後)

	// 日射計算結果
	double solarRadiationSunny[12];		// 日射量(WH/m2) 月ごと　晴天
	double solarRadiationCloud[12];		// 日射量(WH/m2) 月ごと　曇天
	double solarRadiation[12];			// 日照率による補正した日射量(WH/m2)　月ごと

	double solarRadiationUnit;			// 予測日射量(kWh/m2)
	double solarPowerUnit;				// 予測発電量(kWh/m2)

	double area;						// メッシュの面積(m2)

	CMeshData()
	{
		meshId = "";
		for (int n = 0; n < 12; n++) solarRadiationSunny[n] = 0;
		for (int n = 0; n < 12; n++) solarRadiationCloud[n] = 0;
		for (int n = 0; n < 12; n++) solarRadiation[n] = 0;
		solarRadiationUnit = 0.0;
		area = 0.0;

	};

};

// 面ごとの発電ポテンシャル推計データ
class CSurfaceData
{
public:
	std::vector<CMeshData> vecMeshData;		// メッシュごとのデータ
	double meshSize;						// メッシュサイズ(m)

	std::vector<CVector3D> bbPos;			// 対象面のBB
	CVector3D center;						// BB中心

	double slopeDegreeAve;					// 傾斜角(平均値)(度)
	double azDegreeAve;						// 方位角(平均値)(度)
	double slopeModDegree;					// 補正した傾斜角(度)
	double azModDegree;						// 補正した方位角(度)

	double solarRadiation;					// 予測日射量(kWh)
	double solarRadiationUnit;				// 1m2あたりの予測日射量(kWh/m2)

	double area;							// 対象面の面積(m2)


	// 面積を取得する
	double GetArea() { return area; };

	CSurfaceData()
	{
		slopeDegreeAve = 0.0; azDegreeAve = 0.0;
		slopeModDegree = 0.0; azModDegree = 0.0;
		solarRadiation = 0.0; solarRadiationUnit = 0.0;
		area = 0.0;
		meshSize = 0.0;
	};

};

typedef std::map<std::string, CSurfaceData> CSurfaceDataMap;	// 面ID, データマップ

// 
class CPotentialData
{
public:

	CSurfaceDataMap	mapSurface;			// 対象面ごとのデータマップ

	std::vector<CVector3D> bbPos;		// 対象面全体のBB
	CVector3D center;					// BB中心

	// 解析結果(CityGMLに属性付与)
	double solarRadiationTotal;			// 予測日射量 総計
	double solarRadiationUnit;			// 予測日射量 1m2あたりの日射量(平均値)
	double solarPower;					// 予測発電量
	double solarPowerUnit;				// 予測発電量 1m2あたりの発電量(平均値)

	double panelArea;					// パネル面積(設置割合を適用した面積)

	// 総面積
	double GetAllArea()
	{
		double area = 0.0;
		for (auto val : mapSurface)
		{
			area += val.second.GetArea();
		}
		return area;
	};

	CPotentialData()
	{
		solarRadiationTotal = 0.0; solarRadiationUnit = 0.0; solarPower = 0.0;
		solarPowerUnit = 0.0; panelArea = 0.0;
	};
};

typedef std::map<std::string, CPotentialData> CPotentialDataMap;	// 建物ID, データ
typedef std::map<std::string, CPotentialDataMap> CBuildListDataMap;	// 3次メッシュID, データ

class CResultData
{
public:

	CBuildListDataMap*	pBuildMap;			// 建物ごとのデータマップ
	CPotentialData*		pLandData;			// 土地ごとのデータ

	CResultData()
	: pBuildMap(NULL), pLandData(NULL)
	{}
};

