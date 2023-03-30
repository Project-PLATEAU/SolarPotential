#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "../../LIB/CommonUtil/CSunVector.h"
#include "../../LIB/CommonUtil/TiffDataManager.h"
#include "../../LIB/CommonUtil/CLightRay.h"
#include "AnalysisRadiationData.h"
#include "ImportMetpvData.h"
#include "ImportPossibleSunshineData.h"
#include "ImportAverageSunshineData.h"
#include "UIParam.h"

#include "AnalyzeData.h"


class CCalcSolarPotentialMng
{
public:
	// 方位範囲しきい値(仮で16方位で22度未満)
	const double AZ_RANGE_JUDGE_DEGREE = 22.0;

	enum class eOutputImageTarget
	{
		SOLAR_RAD = 0,
		SOLAR_POWER,
	};

	CCalcSolarPotentialMng(
		CImportPossibleSunshineData* pSunshineData,
		CImportAverageSunshineData* pPointData,
		CImportMetpvData* pMetpvData,
		UIParam* m_pUIParam,
		const int& iYear
	);
	~CCalcSolarPotentialMng(void);


public:
	bool AnalyzeSolarPotential();	// 発電ポテンシャル推計メイン処理

	CAnalysisRadiationCommon*	GetRadiationData()	{ return m_pRadiationData; };
	UIParam*					GetUIParam()		{ return m_pUIParam; };
	CImportPossibleSunshineData*	GetSunshineData()	{ return m_pSunshineData; };
	CImportAverageSunshineData*		GetPointData()		{ return m_pPointData; };
	CImportMetpvData*				GetMetpvData()		{ return m_pMetpvData; };
	
	// 周辺の地物に邪魔されず屋根面に光線が当たるかどうかの判定
	bool IntersectRoofSurfaceCenter(
		const CVector3D& inputVec,						// 入射光
		const std::vector<CVector3D>& roofMesh,			// 対象の屋根BB
		const std::string& strId,						// 対象の屋根ID
		const vector<BLDGLIST>& neighborBuildings,		// 周辺の建物リスト
		const vector<DEMLIST>& neighborDems				// 周辺の地形DEMリスト
	);

	// 対象メッシュの隣接するメッシュを取得
	void GetNeighborBuildings(
		const std::string& targetMeshId,				// 対象の3次メッシュID
		const CVector3D& bldCenter,						// 対象の建物中心
		std::vector<BLDGLIST>& neighborBuildings		// 近隣メッシュ
	);

	// 対象の建物に隣接するDEMを取得
	void GetNeighborDems(
		const std::string& targetMeshId,				// 対象の3次メッシュID
		const CVector3D& bldCenter,						// 対象の建物中心
		std::vector<DEMLIST>& neighborDems				// 近隣DEM
	);

	const int GetYear() { return m_iYear; };

	// キャンセル処理
	bool IsCancel();

	// DEMデータの使用有無
	bool IsEnableDEMData();

private:
	double calcLength(double dx, double dy, double dz)
	{
		return dx * dx + dy * dy + dz * dz;
	}

	void initialize();			// 計算用データ等の初期化
	// 日射量推計
	bool calcSolarRadiation(const std::string& Lv3meshId, double bbMinX, double bbMinY, double bbMaxX, double bbMaxY, CBuildingDataMap& bldDataMap);
	// 発電量推計
	bool calcSolarPower(const std::string& Lv3meshId, double bbMinX, double bbMinY, double bbMaxX, double bbMaxY, CBuildingDataMap& bldDataMap);

	// 傾斜角、方位角を算出する
	void calcRoofAspect(const BLDGLIST& bldList, CBuildingDataMap& bldDataMap);
	bool calcRansacPlane(const std::vector<CPointBase>& vecAry, CVector3D& vNormal);

	// 月ごとの日照率を計算
	void calcMonthlyRate();

	// 出力処理
	// 出力用の3Dポイントデータを作成
	bool createPointData(
		std::vector<CPointBase>& vecPoint3d,
		const std::string& Lv3meshId,
		double bbMinX,
		double bbMinY,
		double bbMaxX,
		double bbMaxY,
		double outMeshsize,
		const CBuildingDataMap& bldDataMap,
		const eOutputImageTarget& eTarget
	);
	// 日射量の値に応じて着色した画像を出力
	bool outputImage(
		const std::wstring strFilePath,
		const std::string& Lv3meshId,
		double bbMinX, double bbMinY, double bbMaxX, double bbMaxY,
		const CBuildingDataMap& bldDataMap,
		const eOutputImageTarget& eTarget
	);
	bool outputLegendImage();	// 凡例画像を出力
	bool outputResultCSV();														// 年間日射量・発電量を出力
	bool outputAzimuthDataCSV();												// 適地判定用 方位角中間ファイル出力
	bool outputMonthlyRadCSV(const std::string& Lv3meshId, const CBuildingDataMap& dataMap, const std::wstring& wstrOutDir);	// 月別日射量CSV出力
	bool outputRoofRadCSV(const std::string& Lv3meshId, const CBuildingDataMap& dataMap, const std::wstring& wstrOutDir);		// 屋根面別年間日射量CSV出力

	bool setTotalSolarRadiationToSHP();

	void finalize();

	double calcArea(const std::vector<CPointBase>& vecPos);
	
	// 光線が建物群にあたっているかどうか
	bool intersectBuildings(
		const CLightRay& lightRay,					// 光線
		const std::string& strId,					// 対象の屋根ID
		const std::vector<BLDGLIST>& buildingsList	// 光線があたっているかチェックする建物群
	);
	// 光線が建物にあたっているかどうか
	bool intersectBuilding(
		const CLightRay& lightRay,					// 光線
		const vector<WALLSURFACES>& wallSurfaceList	// 光線があたっているかチェックする建物の壁
	);
	bool intersectBuilding(
		const CLightRay& lightRay,					// 光線
		const std::string& strId,					// 対象の屋根ID
		const BUILDINGS& buildings					// 光線があたっているかチェックする建物群
	);
	// 建物が光線の範囲内か
	bool checkDistance(const CLightRay& lightRay, const vector<WALLSURFACES>& wallSurfaceList);

	// 光線が地形にあたっているかどうか
	bool intersectLandDEM(
		const CLightRay& lightRay,					// 光線
		const vector<DEMLIST>& demList				// 光線があたっているかチェックする地形のDEM
	);

	// 3次メッシュが隣接(周囲8方向)しているか
	bool isNeighborMesh(const std::string& meshId1, const std::string& meshId2);

private:
	// 入力データ
	CImportPossibleSunshineData* m_pSunshineData;
	CImportAverageSunshineData* m_pPointData;
	CImportMetpvData* m_pMetpvData;
	UIParam* m_pUIParam;

	std::vector<BLDGLIST>*		m_pvecAllBuildList;		// CityGMLから取得した全建物データ情報
	std::vector<DEMLIST>*		m_pvecAllDemList;		// CityGMLから取得したDEMデータ情報

	CAnalysisRadiationCommon*	m_pRadiationData;		// 共通の計算パラメータ
	CResultDataMap*				m_pmapResultData;		// 計算結果データマップ

	int m_iYear;

	std::wstring m_strCancelFilePath;

};
