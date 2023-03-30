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
	double solarRadiationSunny[12];			// 日射量(WH/m2) 月ごと　晴天
	double solarRadiationCloud[12];			// 日射量(WH/m2) 月ごと　曇天
	double solarRadiation[12];			// 日照率による補正した日射量(WH/m2)　月ごと

	CMeshData()
	{
		meshId = "";
		for (int n = 0; n < 12; n++) solarRadiationSunny[n] = 0;
		for (int n = 0; n < 12; n++) solarRadiationCloud[n] = 0;
		for (int n = 0; n < 12; n++) solarRadiation[n] = 0;

	};

};

// 屋根面ごとの発電ポテンシャル推計データ
class CRoofSurfaceData
{
public:
	std::vector<CMeshData> vecRoofMesh;		// メッシュごとのデータ

	std::vector<CVector3D> bbPos;			// 屋根面のBB
	CVector3D center;						// BB中心

	double slopeDegreeAve;					// 傾斜角(平均値)(度)
	double azDegreeAve;						// 方位角(平均値)(度)
	double slopeModDegree;					// 補正した傾斜角(度)
	double azModDegree;						// 補正した方位角(度)

	double solarRadiation;					// 年間予測日射量(kWh)
	double solarRadiationUnit;				// 年間予測日射量(kWh/m2)

	double area;							// 屋根面の面積(m2)


	// 面積を算出する
	double GetRoofArea() { return area; };

	CRoofSurfaceData()
	{
		slopeDegreeAve = 0.0; azDegreeAve = 0.0;
		slopeModDegree = 0.0; azModDegree = 0.0;
		solarRadiation = 0.0; solarRadiationUnit = 0.0;
		area = 0.0;
	};

};

typedef std::map<std::string, CRoofSurfaceData> CRoofSurfaceDataMap;


// 建物ごとの発電ポテンシャル推計データ
class CBuildingData
{
public:
	CRoofSurfaceDataMap	mapRoofSurface;	// 屋根面ごとのデータマップ

	std::vector<CVector3D> bbPos;		// 屋根面全体のBB
	CVector3D center;					// BB中心

	// 解析結果(CityGMLに属性付与)
	double solarRadiationTotal;			// 年間予測日射量 総計
	double solarRadiationUnit;			// 年間予測日射量 1m2当たりの日射量
	double solarPower;					// 年間予測発電量
	double solarPowerUnit;				// 年間予測発電量 1m2当たりの発電量

	double panelArea;					// パネル面積(1mメッシュ数)

	// 建物の総屋根面積
	double GetAllRoofArea()
	{
		double area = 0.0;
		for (auto val : mapRoofSurface)
		{
			area += val.second.GetRoofArea();
		}
		return area;
	};

	CBuildingData()
	{
		solarRadiationTotal = 0; solarRadiationUnit = 0; solarPower = 0;
		solarPowerUnit = 0; panelArea = 0;
	};
};

typedef std::map<std::string, CBuildingData> CBuildingDataMap;

typedef std::map<std::string, CBuildingDataMap> CResultDataMap;


