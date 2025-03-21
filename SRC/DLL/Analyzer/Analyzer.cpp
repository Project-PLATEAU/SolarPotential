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
#include "../../LIB/CommonUtil/ExitCode.h"


#include "UIParam.h"

CUIParam* pParam = NULL;

CImportPossibleSunshineData KashoData;
CImportAverageSunshineData NisshoData;
CImportMetpvData SnowDepthData;

// 解析処理開始
ANALYZER_API int AnalyzeStart(const char* outDir)
{
	eExitCode code = eExitCode::Normal;

	try
	{
		// 入力データ読み込み
		if (!KashoData.ReadData())
		{
			delete pParam;
			pParam = nullptr;
			return (int)eExitCode::Err_KashoData;
		}
		if (!NisshoData.ReadData())
		{
			delete pParam;
			pParam = nullptr;
			return (int)eExitCode::Err_NisshoData;
		}
		if (pParam->pInputData->strSnowDepthData != "")
		{
			if (!SnowDepthData.ReadData())
			{
				delete pParam;
				pParam = nullptr;
				return (int)eExitCode::Err_SnowDepthData;
			}
		}

		// 解析対象年を取得(国立天文台のデータを基準とする)
		int iTargetYear = KashoData.GetYear();

		// 出力パス
		pParam->strOutputDirPath = CStringEx::ToWString(outDir);

		// 発電ポテンシャル推計
		if (pParam->bExecSolarPotantial)
		{
			CCalcSolarPotentialMng cSolarPotentiaMng(&KashoData, &NisshoData, &SnowDepthData, pParam, iTargetYear);
			bool ret = cSolarPotentiaMng.AnalyzeSolarPotential();

			if (!ret)
			{
				code = cSolarPotentiaMng.GetExitCode();
				delete pParam;
				pParam = nullptr;
				return (int)code;
			}
		}

		// 反射シミュレーション
		if (pParam->bExecReflection)
		{
			CReflectionSimulateMng reflSimMng;
			bool ret = reflSimMng.Exec(outDir, pParam, iTargetYear);
			if (!ret)
			{
				code = reflSimMng.GetExitCode();
				delete pParam;
				pParam = nullptr;
				return (int)code;
			}
		}
	}
	catch (...)
	{
		code = eExitCode::Error;
	}

	// 解放
	delete pParam;
	pParam = nullptr;

	return (int)code;
}

ANALYZER_API void InitializeUIParam()
{
	if (pParam)
	{
		delete pParam;
	}
	pParam = new CUIParam;

	// 設定ファイル読み込み
	GetINIParam()->Initialize();
}

// 入力パラメータ
ANALYZER_API void SetAnalyzeParam(AnalyzeParam* analyzeParam)
{
	assert(pParam);

	// 解析対象
	pParam->bExecSolarPotantial = analyzeParam->bExecSolarPotantial;
	pParam->bExecReflection = analyzeParam->bExecReflection;
	pParam->bExecBuild = analyzeParam->bExecBuild;
	pParam->bExecLand = analyzeParam->bExecLand;

	return;
}

ANALYZER_API void SetAnalyzeInputData(AnalyzeInputData* p)
{
	assert(pParam);

	pParam->pInputData = new CInputData(
		p->strKashoData,
		p->strNisshoData,
		p->strSnowDepthData,
		p->strLandData,
		p->bUseDemData
	);

	// 各クラスのファイルパスを設定
	KashoData.SetReadFilePath(p->strKashoData);
	NisshoData.SetReadFilePath(p->strNisshoData);
	SnowDepthData.SetReadFilePath(p->strSnowDepthData);

}

ANALYZER_API void SetSolarPotentialParam(SolarPotentialParam* p)
{
	assert(pParam);

	CSolarPotential_Roof* pRoof = new CSolarPotential_Roof(
		p->dArea2D_Roof,
		(eDirections)p->eDirection_Roof,
		p->dDirectionDegree_Roof,
		p->dSlopeDegree_Roof,
		p->dCorrectionCaseDeg_Roof,
		(eDirections)p->eCorrectionDirection_Roof,
		p->dCorrectionDirectionDegree_Roof,
		p->bExclusionInterior_Roof
	);

	CSolarPotential_Land* pLand = new CSolarPotential_Land(
		p->dArea2D_Land,
		p->dSlopeDegree_Land,
		(eDirections)p->eCorrectionDirection_Land,
		p->dCorrectionDirectionDegree_Land
	);

	pParam->pSolarPotentialParam = new CSolarPotentialParam(
		pRoof, pLand,
		p->dPanelMakerSolarPower,
		p->dPanelRatio
	);

}

ANALYZER_API void SetReflectionParam(ReflectionParam* p)
{
	assert(pParam);

	CReflectionCorrect* pRoof_Lower = new CReflectionCorrect(
		p->bCustom_Roof_Lower,
		(eDirections)p->eAzimuth_Roof_Lower,
		p->dSlopeAngle_Roof_Lower
	);

	CReflectionCorrect* pRoof_Upper = new CReflectionCorrect(
		p->bCustom_Roof_Upper,
		(eDirections)p->eAzimuth_Roof_Upper,
		p->dSlopeAngle_Roof_Upper
	);

	CReflectionCorrect* pLand_Lower = new CReflectionCorrect(
		p->bCustom_Land_Lower,
		(eDirections)p->eAzimuth_Land_Lower,
		p->dSlopeAngle_Land_Lower
	);

	CReflectionCorrect* pLand_Upper = new CReflectionCorrect(
		p->bCustom_Land_Upper,
		(eDirections)p->eAzimuth_Land_Upper,
		p->dSlopeAngle_Land_Upper
	);

	pParam->pReflectionParam = new CReflectionParam(
		pRoof_Lower, pRoof_Upper, pLand_Lower, pLand_Upper, p->dReflectionRange
	);

}

ANALYZER_API void SetAnalyzeDateParam(AnalyzeDateParam* date)
{
	assert(pParam);

	// 期間
	pParam->eAnalyzeDate = (eDateType)date->eDateType;
	pParam->nMonth = date->iMonth;
	pParam->nDay = date->iDay;

}

