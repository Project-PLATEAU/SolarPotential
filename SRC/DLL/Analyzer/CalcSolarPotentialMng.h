#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "../../LIB/CommonUtil/CSunVector.h"
#include "../../LIB/CommonUtil/TiffDataManager.h"
#include "../../LIB/CommonUtil/CLightRay.h"
#include "../../LIB/CommonUtil/ExitCode.h"
#include "AnalysisRadiationData.h"
#include "ImportMetpvData.h"
#include "ImportPossibleSunshineData.h"
#include "ImportAverageSunshineData.h"
#include "UIParam.h"

#include "AnalyzeData.h"

#include <iostream>
#include <fstream>

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

	/// <summary>
	/// 解析対象
	/// 名前は対象ごとの出力フォルダ名
	/// </summary>
	enum class eAnalyzeTarget
	{
		NONE = -1,
		ROOF = 0,
		LAND,

		// 解析対象数
		TARGET_COUNT,
	};

	CCalcSolarPotentialMng(
		CImportPossibleSunshineData* pSunshineData,
		CImportAverageSunshineData* pPointData,
		CImportMetpvData* pMetpvData,
		CUIParam* m_pUIParam,
		const int& iYear
	);
	~CCalcSolarPotentialMng(void);


	typedef std::map<std::string, CResultData> CResultDataMap;

public:
	bool AnalyzeSolarPotential();	// 発電ポテンシャル推計メイン処理
	void AnalyzeBuild(const AREADATA& areaData, CBuildListDataMap*& resultDataMap);	// 建物解析
	void AnalyzeLand(const AREADATA& areaData, CPotentialData*& resultData);		// 土地解析

	eExitCode GetExitCode()				{ return m_eExitCode; };
	void SetExitCode(eExitCode code)	{ m_eExitCode = code; };

	CAnalysisRadiationCommon*	GetRadiationData()	{ return m_pRadiationData; };
	CUIParam*					GetUIParam()		{ return m_pUIParam; };
	CImportPossibleSunshineData*	GetSunshineData()	{ return m_pSunshineData; };
	CImportAverageSunshineData*		GetPointData()		{ return m_pPointData; };
	CImportMetpvData*				GetMetpvData()		{ return m_pMetpvData; };
	
	// 周辺の地物に邪魔されず対象面に光線が当たるかどうかの判定
	bool IntersectSurfaceCenter(
		const CVector3D& inputVec,						// 入射光
		const std::vector<CVector3D>& surfaceBB,		// 対象面BB
		const CVector3D& center,						// 対象面中心
		const std::string& strId,						// 対象の屋根ID
		const vector<BLDGLIST>& neighborBuildings,		// 周辺の建物リスト
		const vector<CTriangle>& neighborDems			// 周辺の地形TINリスト
	);

	// 対象の建物に隣接する建物を取得
	void GetNeighborBuildings(
		const CVector3D& bldCenter,						// 対象の建物中心
		std::vector<BLDGLIST>& neighborBuildings		// 近隣建物
	);

	// 対象の建物に隣接するDEMを取得
	void GetNeighborDems(
		const CVector3D& bldCenter,						// 対象の建物中心
		std::vector<CTriangle>& neighborDems,			// 近隣TIN
		eAnalyzeTarget target
	);

	const int GetYear() { return m_iYear; };

	// キャンセル処理
	bool IsCancel();

	// DEMデータの使用有無
	bool IsEnableDEMData();

private:
	double calcLength(double dx, double dy, double dz)
	{
		return sqrt(dx * dx + dy * dy + dz * dz);
	}

	void initialize();			// 計算用データ等の初期化

	// 傾斜角、方位角を算出する
	void calcRoofAspect(const std::vector<BUILDINGS*>& targetBuildings, CPotentialDataMap& bldDataMap);
	bool calcRansacPlane(const std::vector<CPointBase>& vecAry, CVector3D& vNormal);
	void calcLandAspect(const AREADATA& area, CPotentialData& landData);

	// 月ごとの日照率を計算
	void calcMonthlyRate();

	// エリアごとの出力処理
	bool outputAreaBuildResult(const AREADATA& areaData);
	bool outputAreaLandResult(const AREADATA& areaData);

	// 出力用の3Dポイントデータを作成
	bool createPointData_Build(
		std::vector<CPointBase>& vecPoint3d,
		const AREADATA& areaData,
		const BLDGLIST& bldList,
		const CPotentialDataMap& bldDataMap,
		double outMeshsize,
		const eOutputImageTarget& eTarget
	);
	// 出力用の3Dポイントデータを作成
	bool createPointData_Land(
		std::vector<CPointBase>& vecPoint3d,
		const AREADATA& areaData,
		const CPotentialData& landData,
		const eOutputImageTarget& eTarget
	);
	// 日射量の値に応じて着色した画像を出力
	bool outputImage(
		const std::wstring strFilePath,
		std::vector<CPointBase>* pvecPoint3d,
		double outMeshsize,
		const eOutputImageTarget& eTarget
	);

	bool outputLegendImage();			// 凡例画像を出力
	bool outputAzimuthDataCSV();		// 適地判定用 方位角中間ファイル出力
	bool outputAllAreaResultCSV();		// 全範囲における日射量・発電量CSVを出力
	bool outputLandShape();					// シェープファイルに出力

	bool outputMonthlyRadCSV(const CPotentialDataMap& dataMap, const std::wstring& wstrOutDir);	// 月別日射量CSV出力
	bool outputSurfaceRadCSV(const eAnalyzeTarget analyzeTarget, const CPotentialDataMap& dataMap, const std::wstring& wstrOutDir);	// メッシュごとの日射量CSV出力

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
		const vector<CTriangle>& tinList,			// 光線があたっているかチェックする地形のTIN
		const std::vector<CVector3D>& surfaceBB		// 対象面のBB
	);

	// 色設定ファイルパスを取得
	std::wstring getColorSettingFileName(const eOutputImageTarget& eTarget);

	// フォルダ・ファイル名称
	const std::wstring GetDirName_AnalyzeTargetDir(eAnalyzeTarget target);
	const std::wstring GetDirName_SolarRadImage() { return L"日射量画像"; }
	const std::wstring GetDirName_SolarPotentialImage() { return L"発電ポテンシャル画像"; }
	const std::wstring GetDirName_LandShape() { return L"シェープファイル"; }
	const std::wstring GetFileName_SolarPotentialCsv(eAnalyzeTarget target);	// 建物・土地ごと予測発電量CSV
	const std::wstring GetFileName_MeshPotentialCsv(eAnalyzeTarget target);		// (土地面のみ)メッシュごと予測発電量CSV
	const std::wstring GetFileName_SolarPotentialShp();							// (土地面のみ)土地ごと予測発電量SHP
	const std::wstring GetFileName_MeshSolarRadCsv(eAnalyzeTarget target);		// 屋根面別・土地面メッシュ別日射量CSV

private:
	// 入力データ
	CImportPossibleSunshineData* m_pSunshineData;
	CImportAverageSunshineData* m_pPointData;
	CImportMetpvData* m_pMetpvData;
	CUIParam* m_pUIParam;

	std::vector<AREADATA>*		m_pvecAllAreaList;		// 解析エリア情報(建物、土地データ)
	AREADATA*					m_targetArea;			// 処理中エリアデータ

	CAnalysisRadiationCommon*	m_pRadiationData;		// 共通の計算パラメータ
	CResultDataMap*				m_pmapResultData;		// 計算結果データマップ

	int m_iYear;

	CTime						m_dateStart;
	CTime						m_dateEnd;

	std::wstring				m_strCancelFilePath;

	eExitCode					m_eExitCode;				// 終了コード
	bool						m_isCancel;					// キャンセル状態

#ifdef CHRONO
	std::ofstream m_ofs;
#endif

};
