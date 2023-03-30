// JudgeSuitablePlace.cpp : DLL 用にエクスポートされる関数を定義します。
//
#include "pch.h"
#include "framework.h"
#include "JudgeSuitablePlace.h"
#include "UIParam.h"
#include "ImportRestrictAreaData.h"
#include "ImportWeatherData.h"
#include "JudgeSuitablePlaceMng.h"
#include "../../LIB/CommonUtil/CFileUtil.h"
#include "../../LIB/CommonUtil/ReadINIParam.h"

#ifdef _DEBUG
#include <sys/timeb.h>
#include <time.h>
#include <psapi.h>
#endif

CUIParam* pParam = NULL;

JUDGESUITABLEPLACE_API void InitializeUIParam()
{
    if (pParam)
    {
        delete pParam;
    }
    pParam = new CUIParam;
}

// 入力パラメータ
JUDGESUITABLEPLACE_API void SetAggregateParam(AggregateParam* aggregateParam)
{
    // 設定ファイル読み込み
    GetINIParam()->Initialize();

    // 集計範囲指定
    if (aggregateParam->bAggregateRange)
    {
        pParam->m_pAggregationRange =
            new CAggregationRange(
                aggregateParam->dMaxLat,
                aggregateParam->dMinLon,
                aggregateParam->dMaxLon,
                aggregateParam->dMinLat
            );
    }

    // 太陽光パネルの設置に関して優先度が低い施設の判定
    unsigned int iBuildingStructure = 0;
    if (aggregateParam->bParam_1_2_1) iBuildingStructure |= (1 << (int)eBuildingStructure::WOOD);
    if (aggregateParam->bParam_1_2_2) iBuildingStructure |= (1 << (int)eBuildingStructure::STEEL_REINFORCED_CONCRETE);
    if (aggregateParam->bParam_1_2_3) iBuildingStructure |= (1 << (int)eBuildingStructure::REINFORCED_CONCRETE);
    if (aggregateParam->bParam_1_2_4) iBuildingStructure |= (1 << (int)eBuildingStructure::STEEL);
    if (aggregateParam->bParam_1_2_5) iBuildingStructure |= (1 << (int)eBuildingStructure::LIGHT_GAUGE_STEEL);
    if (aggregateParam->bParam_1_2_6) iBuildingStructure |= (1 << (int)eBuildingStructure::MASONRY_CONSTRUCTION);
    if (aggregateParam->bParam_1_2_7) iBuildingStructure |= (1 << (int)eBuildingStructure::UNKNOWN);
    if (aggregateParam->bParam_1_2_8) iBuildingStructure |= (1 << (int)eBuildingStructure::NON_WOOD);
    pParam->m_pBuildingParam =
        new CBuildingParam(
            aggregateParam->dParam_1_1_1, 
            aggregateParam->iParam_1_1_2, 
            iBuildingStructure, 
            aggregateParam->iParam_1_3_1, 
            aggregateParam->iParam_1_3_2
        );

    // 災害時に太陽光パネルが破損、消失する危険性の判定
    pParam->m_pHazardParam =
        new CHazardParam(
            aggregateParam->bParam_2_1,
            aggregateParam->bParam_2_2,
            aggregateParam->bParam_2_3,
            aggregateParam->strParam_2_4,
            aggregateParam->dParam_2_4_1,
            aggregateParam->dParam_2_4_2,
            aggregateParam->dParam_2_4_3
        );

    // 太陽光パネルの設置に制限がある施設の判定
    pParam->m_pRestrictParam =
        new CRestrictParam(
            aggregateParam->strParam_3_1,
            aggregateParam->dParam_3_1_1,
            (eDirections)aggregateParam->iParam_3_1_2,
            aggregateParam->strParam_3_2,
            aggregateParam->dParam_3_2_1,
            (eDirections)aggregateParam->iParam_3_2_2,
            aggregateParam->strParam_3_3,
            aggregateParam->dParam_3_3_1,
            (eDirections)aggregateParam->iParam_3_3_2
        );

	return;
}

// 出力フォルダ
JUDGESUITABLEPLACE_API bool SetOutputPath(char* aggregatePath)
{
    setlocale(LC_ALL, "");

    // 出力用フォルダチェック
    std::wstring path = CStringEx::ToWString(aggregatePath);
    pParam->m_strOutputDirPath = GetFUtil()->Combine(path, L"data");
    if (!GetFUtil()->IsExistPath(pParam->m_strOutputDirPath))
    {
        return false;
    }
    return true;
}

// 解析結果フォルダ
JUDGESUITABLEPLACE_API bool SetBldgResultPath(char* analyzePath)
{
    setlocale(LC_ALL, "");

    // 解析結果フォルダチェック
    std::wstring path = CStringEx::ToWString(analyzePath);
    pParam->m_strBldgResultPath = GetFUtil()->GetParentDir(path);   // output直下
    if (!GetFUtil()->IsExistPath(pParam->m_strBldgResultPath))
    {
        return false;
    }
    return true;
}

// 判定処理開始
JUDGESUITABLEPLACE_API int JadgeStart()
{
#ifdef _DEBUG
    struct __timeb32 timebuffer;
    _ftime32_s(&timebuffer);
    double dStart = timebuffer.time + (double)timebuffer.millitm / 1000.0;
#endif

    CImportWeatherData app1;
    CImportRestrictAreaData app2[3];

    bool bRet = true;
    int iRet = 0;

    // 気象データ読み込み
    if (pParam->m_pHazardParam->strWeatherDataPath.size() > 0)
    {
        app1.SetReadFilePath(pParam->m_pHazardParam->strWeatherDataPath);
        bRet &= app1.ReadData();
    }
    // 制限区域データ読み込み
    if (pParam->m_pRestrictParam->strRestrictAreaPath_1.size() > 0)
    {
        app2[0].SetReadFilePath(pParam->m_pRestrictParam->strRestrictAreaPath_1);
        bRet &= app2[0].ReadData();
    }
    if (pParam->m_pRestrictParam->strRestrictAreaPath_2.size() > 0)
    {
        app2[1].SetReadFilePath(pParam->m_pRestrictParam->strRestrictAreaPath_2);
        bRet &= app2[1].ReadData();
    }
    if (pParam->m_pRestrictParam->strRestrictAreaPath_3.size() > 0)
    {
        app2[2].SetReadFilePath(pParam->m_pRestrictParam->strRestrictAreaPath_3);
        bRet &= app2[2].ReadData();
    }

    // 読み込み失敗
    if (!bRet)
    {
#ifdef _DEBUG
        std::cout << "Failed to load SHP file!!" << std::endl;
#endif
        return 1;
    }
    CJudgeSuitablePlaceMng cJudgeSuitablePlaceMng;
    // 入力パラメータを設定
    cJudgeSuitablePlaceMng.SetParameter(pParam);
    // 気象データを設定
    cJudgeSuitablePlaceMng.SetWeatherData(&app1);
    // 制限区域データを設定
    cJudgeSuitablePlaceMng.SetRestrictAreaData(app2);
    // 適地判定実行
    iRet = cJudgeSuitablePlaceMng.Judge();
#ifdef _DEBUG
    if (iRet) std::cout << "Failed to Judge!!" << std::endl;
#endif

    // 解放
    if (pParam) delete pParam;
    pParam = NULL;

#ifdef _DEBUG
    _ftime32_s(&timebuffer);
    double dEndStart = timebuffer.time + (double)timebuffer.millitm / 1000.0 - dStart;
    std::cout << "Jadge AllTime: " << dEndStart << " sec" << std::endl;

    // メモリ使用量
    HANDLE hProc = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS_EX pmc;
    BOOL isSuccess = GetProcessMemoryInfo(
        hProc,
        (PROCESS_MEMORY_COUNTERS*)&pmc,
        sizeof(pmc));
    CloseHandle(hProc);
    if (isSuccess != FALSE)
        std::cout << "PROCESS_MEMORY3: " << pmc.PrivateUsage << std::endl;
#endif

    return iRet;
}