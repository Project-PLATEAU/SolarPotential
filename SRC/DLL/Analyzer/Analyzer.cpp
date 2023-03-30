// Analyzer.cpp : DLL 用にエクスポートされる関数を定義します。
//

#include "pch.h"
#include "Analyzer.h"
#include "DataImport.h"
#include "ImportMetpvData.h"
#include "ImportPossibleSunshineData.h"
#include "ImportAverageSunshineData.h"

#include "CalcSolarPotentialMng.h"
#include "ReflectionSimulateMng.h"

#include "../../LIB/CommonUtil/ReadINIParam.h"
#include "../../LIB/CommonUtil/CFileUtil.h"


#include "UIParam.h"

UIParam* pParam = nullptr;

CImportPossibleSunshineData app1;
CImportAverageSunshineData app2;
CImportMetpvData app3;


// 解析処理開始
ANALYZER_API bool AnalyzeStart(const char* outDir)
{
	bool ret = true;

	// 入力データ読み込み
	ret &= app1.ReadData();
	ret &= app2.ReadData();
	ret &= app3.ReadData();
	if (!ret)	return false;	// 入力データ読み込み失敗

	// 設定ファイル読み込み
	GetINIParam()->Initialize();

	// 解析対象年を取得(国立天文台のデータを基準とする)
	int iTargetYear = app1.GetYear();

	// 発電ポテンシャル推計
	if (pParam->bExecSolarPotantial)
	{
		pParam->strOutputDirPath = CStringEx::ToWString(outDir);
		CCalcSolarPotentialMng cSolarPotentiaMng(&app1, &app2, &app3, pParam, iTargetYear);
		ret &= cSolarPotentiaMng.AnalyzeSolarPotential();
	}

	// 反射シミュレーション
	if (pParam->bExecReflection)
	{
		CReflectionSimulateMng reflSimMng;
		reflSimMng.Exec(outDir, pParam, iTargetYear);
	}

	// 解放
	delete pParam;
	pParam = nullptr;

	return ret;
}


// 月毎の可照時間
ANALYZER_API void SetPossibleSunshineDataPath(char* path)
{
	app1.SetReadFilePath(path);
	return;
}

// 毎月の平均日照時間
ANALYZER_API void SetAverageSunshineDataPath(char* path)
{
	app2.SetReadFilePath(path);
	return;
}

// 月毎の積雪深
ANALYZER_API void SetMetpvDataPath(char* path)
{
	app3.SetReadFilePath(path);
	return;
}

ANALYZER_API void InitializeUIParam()
{
	if (!pParam)
	{
		pParam = new UIParam;
	}
}

// 出力フォルダ
ANALYZER_API void SetOutputPath(char* path)
{
	pParam->strOutputDirPath = CStringEx::ToWString(path);
}


// 発電ポテンシャル推計
ANALYZER_API void SetElecPotential(double d1, int e, double d2, double d3)
{
	pParam->_elecPotential = CElecPotential(d1, (eDirections)e, d2, d3);
}

// 屋根面補正
ANALYZER_API void SetRoofSurfaceCorrect(double d1, double d2)
{
	pParam->_roofSurfaceCorrect = CRoofSurfaceCorrect(d1, d2);
}

// 太陽光パネル単位面積当たりの発電容量
ANALYZER_API void SetAreaSolarPower(double d)
{
	pParam->_dAreaSolarPower = d;
}

// 反射シミュレーション時の屋根面の向き・傾き補正(3度未満)
ANALYZER_API void SetReflectRoofCorrect_Lower(bool b1, bool b2, int e, double d)
{
	pParam->_reflectRoofCorrect_Lower = CReflectionRoofCorrect(b1, b2, (eDirections)e, d);
}

// 反射シミュレーション時の屋根面の向き・傾き補正(3度以上)
ANALYZER_API void SetReflectRoofCorrect_Upper(bool b1, bool b2, int e, double d)
{
	pParam->_reflectRoofCorrect_Upper = CReflectionRoofCorrect(b1, b2, (eDirections)e, d);
}

// DEMデータを使用するか
ANALYZER_API void SetEnableDEMData(bool b)
{
	pParam->bEnableDEMData = b;
}

// 発電ポテンシャル推計実行フラグ
ANALYZER_API void SetExecSolarPotantial(bool b)
{
	pParam->bExecSolarPotantial = b;
}


// 反射シミュレーション実行フラグ
ANALYZER_API void SetExecReflection(bool b)
{
	pParam->bExecReflection = b;
}
