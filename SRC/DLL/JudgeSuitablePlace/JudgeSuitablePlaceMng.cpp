
#include "pch.h"
#include "JudgeSuitablePlaceMng.h"
#include "ExtendedAttributeIniFile.h"
#include "AggregateData.h"
#include "../../LIB/CommonUtil/TiffDataManager.h"
#include "../../LIB/CommonUtil/CEpsUtil.h"
#include "../../LIB/CommonUtil/CFileUtil.h"
#include "../../LIB/CommonUtil/ReadINIParam.h"
#include "../../LIB/CommonUtil/CImageUtil.h"
#include <cmath>

#ifdef _DEBUG
#include <sys/timeb.h>
#include <time.h>
#include <psapi.h>
#endif

CJudgeSuitablePlaceMng::CJudgeSuitablePlaceMng(void)
    : m_pUIParam(NULL)
    , m_pBuildingData(NULL)
    , m_pWeatherData(NULL)
    , m_pRestrictAreaData{NULL}
{
}

CJudgeSuitablePlaceMng::~CJudgeSuitablePlaceMng(void)
{
}
eBuildingStructure CJudgeSuitablePlaceMng::GetBuildingStructureType1(int iBldStructureType)
{
    eBuildingStructure e;
    switch (iBldStructureType)
    {
    case 601:
        e = eBuildingStructure::WOOD;
        break;
    case 602:
        e = eBuildingStructure::STEEL_REINFORCED_CONCRETE;
        break;
    case 603:
        e = eBuildingStructure::REINFORCED_CONCRETE;
        break;
    case 604:
        e = eBuildingStructure::STEEL;
        break;
    case 605:
        e = eBuildingStructure::LIGHT_GAUGE_STEEL;
        break;
    case 606:
        e = eBuildingStructure::MASONRY_CONSTRUCTION;
        break;
    case 610:
        e = eBuildingStructure::NON_WOOD;
        break;
    case 611:
        e = eBuildingStructure::UNKNOWN;
        break;
    default:
        e = eBuildingStructure::UNKNOWN;
        break;
    }
    return e;
}


int CJudgeSuitablePlaceMng::Judge()
{
#ifdef _DEBUG
    std::cout << "Jadge Start " << std::endl;
    struct __timeb32 timebuffer;
    _ftime32_s(&timebuffer);
    double dStart = timebuffer.time + (double)timebuffer.millitm / 1000.0;
#endif

    m_allAreaList = reinterpret_cast<vector<AREADATA>*>(GetAllAreaList());

    // 集計範囲(UI指定)
    if (m_pUIParam->m_pAggregationRange != NULL)
    {
        m_aggregationRange.push_back(CPoint2D(m_pUIParam->m_pAggregationRange->dMinX, m_pUIParam->m_pAggregationRange->dMinY));
        m_aggregationRange.push_back(CPoint2D(m_pUIParam->m_pAggregationRange->dMaxX, m_pUIParam->m_pAggregationRange->dMinY));
        m_aggregationRange.push_back(CPoint2D(m_pUIParam->m_pAggregationRange->dMaxX, m_pUIParam->m_pAggregationRange->dMaxY));
        m_aggregationRange.push_back(CPoint2D(m_pUIParam->m_pAggregationRange->dMinX, m_pUIParam->m_pAggregationRange->dMaxY));
    }

    int iRet = 0;

    // 建物実行
    if (m_pUIParam->m_bExecBuild)
    {
        iRet = JudgeBuild();
        if (iRet != 0)  return iRet;
    }

    // 土地実行
    if (m_pUIParam->m_bExecLand)
    {
        iRet = JudgeLand();
    }

    if (iRet == 0)
    {
        // 凡例出力
        OutputLegendImage();
    }

    return iRet;
};

int CJudgeSuitablePlaceMng::JudgeBuild()
{
#ifdef _DEBUG
    std::cout << "JudgeBuild Start " << std::endl;
    struct __timeb32 timebuffer;
    _ftime32_s(&timebuffer);
    double dStart = timebuffer.time + (double)timebuffer.millitm / 1000.0;
#endif

    CBuildingData targetBuilding;

    // エリアごと
    for (int iAreaCount = 0; iAreaCount < m_allAreaList->size(); iAreaCount++)
    {
        vector<BLDGLIST>* allList = &(*m_allAreaList)[iAreaCount].buildList;
        std::string areaID = (*m_allAreaList)[iAreaCount].areaID;

        // メッシュごと
        for (int iMeshCount = 0; iMeshCount < allList->size(); iMeshCount++)
        {
            BLDGLIST* pBldList = &allList->at(iMeshCount);

#ifdef _DEBUG
            std::cout << "BuildingList Size: " << pBldList->buildingList.size() << std::endl;
#endif

            // 建物ごと
            for (int iBldCount = 0; iBldCount < pBldList->buildingList.size(); iBldCount++)
            {
                BUILDING* pBuilding = &pBldList->buildingList.at(iBldCount);

                CBuilding building;
                building.m_strAreaId = areaID;                                       // エリアID
                building.m_iMeshId = pBldList->meshID;                               // メッシュID
                building.m_strBuildingId = pBuilding->strBuildingId;                 // 建物ID
                building.dBldHeight = pBuilding->dBuildingHeight;
                building.m_pRoofSurfaceList = &pBuilding->roofSurfaceList;           // 屋根リスト
                building.dSolorRadiation = pBuilding->dSolorRadiation;
                building.iBldStructureType = pBuilding->iBldStructureType;
                building.dFloodDepth = pBuilding->dFloodDepth;
                building.dTsunamiHeight = pBuilding->dTsunamiHeight;
                building.bLandslideArea = pBuilding->bLandslideArea;
                building.iBldStructureType2 = pBuilding->iBldStructureType2;
                building.iFloorType = pBuilding->iFloorType;

                // 屋根リストがない場合は飛ばす
                if (building.m_pRoofSurfaceList->size() == 0) continue;

                // 集計範囲指定
                if (m_pUIParam->m_pAggregationRange != NULL)
                {
                    // 集計範囲内の建物か判定
                    bool bRet = building.IsBuildingInPolygon((unsigned int)m_aggregationRange.size(), m_aggregationRange.data());
                    if (bRet) targetBuilding.Add(building);
                }
                // 全範囲集計
                else
                {
                    targetBuilding.Add(building);
                }
            }
        }
    }

    if (targetBuilding.GetBuildingSize() == 0)
    {
#ifdef _DEBUG
        std::cout << "No target Building!! " << std::endl;
#endif
        return 0;
    }

    // 建物IDごとの方位角データを読み込む
    bool bReadAzimuth = false;
    std::wstring strCSVName= GetINIParam()->GetAzimuthCSVPath();
    if (strCSVName != L"")
    {
        std::wstring strFilePath = GetFUtil()->Combine(m_pUIParam->m_strAnalyzeResultPath, strCSVName);
        bReadAzimuth = targetBuilding.ReadAzimuthCSV(strFilePath);
    }

    // 拡張属性設定ファイルを読み込む
    std::string strFilePath = GetFUtil()->Combine(GetFUtil()->GetModulePath(), "extended_attribute.ini");
    CExtendedAttributeIniFile ExtendedAttributeIniFile(strFilePath);

    CResultJudgment result;     // 判定結果
    result.SetTarget(eTarget::TARGET_BUILD);

    // 建物ごとに判定する
    for (int i = 0; i < targetBuilding.GetBuildingSize(); i++)
    {
        CBuilding Building = *targetBuilding.GetBuildingAt(i);

        ResultJudgment resultJudgment;
        // エリアID/メッシュID/建物IDを設定
        resultJudgment.m_strAreaId = Building.m_strAreaId;
        resultJudgment.m_iMeshId = Building.m_iMeshId;
        resultJudgment.m_strBuildingId = Building.m_strBuildingId;

        // バウンディングの取得
        double dMinX = 0.0, dMinY = 0.0, dMinZ = 0.0;
        double dMaxX = 0.0, dMaxY = 0.0, dMaxZ = 0.0;
        Building.CalcBounding(&dMinX, &dMinY, &dMaxX, &dMaxY, &dMinZ, &dMaxZ);
        // 建物の中心座標を取得
        CPoint2D posCenter = CPoint2D((dMinX + dMaxX) * 0.5, (dMaxY + dMaxY) * 0.5);

        /////////
        // 太陽光パネルの設置に関して優先度が低い施設の判定
        /////////
        // 日射量が少ない施設を除外
        {
            // 年間予測日射量が建物属性にない場合は判定しない
            if (Building.dSolorRadiation >= 0.0)
            {
                double dSolorRadiation = m_pUIParam->m_pBuildingParam->dSolorRadiation;
                if (dSolorRadiation > 0.0)
                {   // kWh/m2 未満は不適
                    resultJudgment.m_strSuitable1_1_1 = Building.dSolorRadiation < dSolorRadiation ? "×" : "○";
                }
                double dLowerPercent = m_pUIParam->m_pBuildingParam->dLowerPercent;
                if (dLowerPercent > 0.0)
                {   // 日射量を昇順でソートした時の順位
                    double dTargetOrder = targetBuilding.GetSolorRadiationOrder(Building.m_strBuildingId);
                    // 下位％は不適
                    resultJudgment.m_strSuitable1_1_2 = (dTargetOrder / (double)targetBuilding.GetBuildingSize() <= dLowerPercent / 100.0) ? "×" : "○";
                }
            }
        }

        // 建物構造による除外
        {
            int iBuildingStructure = m_pUIParam->m_pBuildingParam->iBuildingStructure;
            if (iBuildingStructure > 0)
            {
                // 建物構造を取得
                eBuildingStructure eBldStrct = eBuildingStructure::UNKNOWN;
                if (Building.iBldStructureType > 0)
                {
                    eBldStrct = this->GetBuildingStructureType1(Building.iBldStructureType);
                }
                else if (Building.iBldStructureType2 >= 0)
                {
                    eBldStrct = ExtendedAttributeIniFile.GetBuildingStructureType2(Building.iBldStructureType2);
                }

                bool bRet = true;
                if (iBuildingStructure & (1 << (int)eBuildingStructure::WOOD))
                {   // 木造・土蔵造 
                    bRet &= (eBldStrct == eBuildingStructure::WOOD) ? false : true;
                }
                if (iBuildingStructure & (1 << (int)eBuildingStructure::STEEL_REINFORCED_CONCRETE))
                {   // 鉄骨鉄筋コンクリート造
                    bRet &= (eBldStrct == eBuildingStructure::STEEL_REINFORCED_CONCRETE) ? false : true;
                }
                if (iBuildingStructure & (1 << (int)eBuildingStructure::REINFORCED_CONCRETE))
                {   // 鉄筋コンクリート造
                    bRet &= (eBldStrct == eBuildingStructure::REINFORCED_CONCRETE) ? false : true;
                }
                if (iBuildingStructure & (1 << (int)eBuildingStructure::STEEL))
                {   // 鉄骨造
                    bRet &= (eBldStrct == eBuildingStructure::STEEL) ? false : true;
                }
                if (iBuildingStructure & (1 << (int)eBuildingStructure::LIGHT_GAUGE_STEEL))
                {   // 軽量鉄骨造
                    bRet &= (eBldStrct == eBuildingStructure::LIGHT_GAUGE_STEEL) ? false : true;
                }
                if (iBuildingStructure & (1 << (int)eBuildingStructure::MASONRY_CONSTRUCTION))
                {   // レンガ造・コンクリートブロック造・石造
                    bRet &= (eBldStrct == eBuildingStructure::MASONRY_CONSTRUCTION) ? false : true;
                }
                if (iBuildingStructure & (1 << (int)eBuildingStructure::UNKNOWN))
                {   // 不明
                    bRet &= (eBldStrct == eBuildingStructure::UNKNOWN) ? false : true;
                }
                if (iBuildingStructure & (1 << (int)eBuildingStructure::NON_WOOD))
                {   // 非木造
                    bRet &= (eBldStrct == eBuildingStructure::NON_WOOD) ? false : true;
                }
                resultJudgment.m_strSuitable1_2 = bRet ? "○" : "×";
            }
        }

        // 特定の階層の施設を除外
        {
            // 建物階数が建物属性にない場合は判定しない
            if (Building.iFloorType > 0)
            {
                int iFloorsBelow = m_pUIParam->m_pBuildingParam->iFloorsBelow;
                int iUpperFloors = m_pUIParam->m_pBuildingParam->iUpperFloors;
                // 建物階数を取得
                int iFloorType = ExtendedAttributeIniFile.GetFloorType(Building.iFloorType);
                if (iFloorsBelow != -1 || iUpperFloors != 9999)
                {
                    bool bRet = true;
                    if (iFloorsBelow != -1)
                    {
                        bRet &= (iFloorType <= iFloorsBelow) ? false : true;
                    }
                    if (iUpperFloors != 9999)
                    {
                        bRet &= (iUpperFloors <= iFloorType) ? false : true;
                    }
                    resultJudgment.m_strSuitable1_3 = bRet ? "○" : "×";
                }
            }
        }

        /////////
        // 災害時に太陽光パネルが破損、消失する危険性の判定
        /////////
        // 建物高さが建物属性にない場合は判定しない
        if (Building.dBldHeight >= 0.0)
        {
            // 高さが想定される最大津波高さを下回る(未満)建物を除外
            {
                // 最大津波高さが建物属性にない場合は判定しない
                if (Building.dTsunamiHeight > 0.0)
                {
                    if (m_pUIParam->m_pHazardParam->bBelowTsunamiHeight)
                    {
                        resultJudgment.m_strSuitable2_1 = (Building.dBldHeight < Building.dTsunamiHeight) ? "×" : "○";
                    }
                }
            }
            // 建物高さが想定される河川浸水想定浸水深を下回る建物を除外
            {
                // 浸水深が建物属性にない場合は判定しない
                if (Building.dFloodDepth > 0.0)
                {
                    if (m_pUIParam->m_pHazardParam->bBelowFloodDepth)
                    {
                        resultJudgment.m_strSuitable2_2 = (Building.dBldHeight < Building.dFloodDepth) ? "×" : "○";
                    }
                }
            }
        }
        // 土砂災害警戒区域内に存在する建物を除外
        {
            if (m_pUIParam->m_pHazardParam->bLandslideWarningArea)
            {
                resultJudgment.m_strSuitable2_3 = (Building.bLandslideArea) ? "×" : "○";
            }
        }
        // 積雪の多い地域の建物を除外
        {
            if (!m_pUIParam->m_pHazardParam->strWeatherDataPath.empty())
            {   // cm以上は除外
                double dSnowDepth = m_pUIParam->m_pHazardParam->dSnowDepth;
                if (dSnowDepth > 0.0)
                {
                    // 積雪深を取得
                    double dTargetDepth = m_pWeatherData->GetSnowDepth(posCenter);
                    resultJudgment.m_strSuitable2_4 = (dTargetDepth >= dSnowDepth) ? "×" : "○";
                }
                // 積雪荷重(kgf/m2)以上は除外
                double dSnowLoad = m_pUIParam->m_pHazardParam->dS;
                if (dSnowLoad > 0.0)
                {
                    double dTargetSnowLoad = m_pWeatherData->CalSnowLoad(posCenter, m_pUIParam->m_pHazardParam->dp);
                    resultJudgment.m_strSuitable2_4 = (dTargetSnowLoad >= dSnowLoad) ? "×" : "○";
                }
            }
        }
        
        /////////
        // 太陽光パネルの設置に制限がある施設の判定
        /////////
        //if( bReadAzimuth )
        {
            // 制限を設ける範囲のシェープファイル_1
            if (!m_pUIParam->m_pRestrictParam->strRestrictAreaPath_1.empty())
            {
                // 高さ
                double dHeight = m_pUIParam->m_pRestrictParam->dHeight_1;
                if (dHeight != -1)
                {
                    bool bRet = true;
                    // 制限区域かどうか
                    if (m_pRestrictAreaData[0]->IsBuildingInRestrictArea(*Building.m_pRoofSurfaceList))
                    {
                        if (Building.dBldHeight >= dHeight)
                        {
                            // 方位
                            eDirections eDirection = m_pUIParam->m_pRestrictParam->eDirections_1;
                            if (eDirection > eDirections::UNKNOWN && bReadAzimuth)
                            {
                                for (double dAzimuth : Building.vecRoofAzimuth)
                                {
                                    eDirections eTargetDirection = GetDirection(dAzimuth);
                                    bRet &= (eTargetDirection == eDirection) ? false : true;
                                }
                            }
                            else
                            {
                                bRet = false;
                            }
                        }
                    }
                    resultJudgment.m_strSuitable3_1 = bRet ? "○" : "×";
                }
            }
            // 制限を設ける範囲のシェープファイル_2
            if (!m_pUIParam->m_pRestrictParam->strRestrictAreaPath_2.empty())
            {
                // 高さ
                double dHeight = m_pUIParam->m_pRestrictParam->dHeight_2;
                if (dHeight != -1)
                {
                    bool bRet = true;
                    // 制限区域かどうか
                    if (m_pRestrictAreaData[1]->IsBuildingInRestrictArea(*Building.m_pRoofSurfaceList))
                    {
                        if (Building.dBldHeight >= dHeight)
                        {
                            // 方位
                            eDirections eDirection = m_pUIParam->m_pRestrictParam->eDirections_2;
                            if (eDirection > eDirections::UNKNOWN && bReadAzimuth)
                            {
                                for (double dAzimuth : Building.vecRoofAzimuth)
                                {
                                    eDirections eTargetDirection = GetDirection(dAzimuth);
                                    bRet &= (eTargetDirection == eDirection) ? false : true;
                                }
                            }
                            else
                            {
                                bRet = false;
                            }
                        }
                    }
                    resultJudgment.m_strSuitable3_2 = bRet ? "○" : "×";
                }
            }
            // 制限を設ける範囲のシェープ    _3
            if (!m_pUIParam->m_pRestrictParam->strRestrictAreaPath_3.empty())
            {
                // 高さ
                double dHeight = m_pUIParam->m_pRestrictParam->dHeight_3;
                if (dHeight != -1)
                {
                    bool bRet = true;
                    // 制限区域かどうか
                    if (m_pRestrictAreaData[2]->IsBuildingInRestrictArea(*Building.m_pRoofSurfaceList))
                    {
                        if (Building.dBldHeight >= dHeight)
                        {
                            // 方位
                            eDirections eDirection = m_pUIParam->m_pRestrictParam->eDirections_3;
                            if (eDirection > eDirections::UNKNOWN && bReadAzimuth)
                            {
                                for (double dAzimuth : Building.vecRoofAzimuth)
                                {
                                    eDirections eTargetDirection = GetDirection(dAzimuth);
                                    bRet &= (eTargetDirection == eDirection) ? false : true;
                                }
                            }
                            else
                            {
                                bRet = false;
                            }
                        }
                    }
                    resultJudgment.m_strSuitable3_3 = bRet ? "○" : "×";
                }
            }
        }
        // 判定結果を格納
        result.Add(resultJudgment);
    }

    // 判定結果から優先度を決定する
    result.Prioritization();

    // 判定結果をCSV出力する
    std::wstring strCSVFilePath = GetFUtil()->Combine(m_pUIParam->m_strOutputDirPath, CSVFILE);
    if (!result.OutputResultCSV(strCSVFilePath))
    {
#ifdef _DEBUG
        std::cout << "Failed to output CSV file!! " << std::endl;
#endif
        return 1;
    }

#ifdef _DEBUG
    _ftime32_s(&timebuffer);
    double dEndStart = timebuffer.time + (double)timebuffer.millitm / 1000.0 - dStart;
    std::cout << "Jadge Time: " << dEndStart << " sec" << std::endl;
    dStart = timebuffer.time + (double)timebuffer.millitm / 1000.0;
#endif

    // キャンセルチェック
    std::wstring strCancelFilePath = GetFUtil()->Combine(m_pUIParam->m_strOutputDirPath, CStringEx::ToWString(CANCELFILE));
    if (GetFUtil()->IsExistPath(strCancelFilePath))
    {
        return 2; // キャンセル
    }

    // GeoTiff画像を出力する
    std::wstring strGeoTiffFilePath = GetFUtil()->Combine(m_pUIParam->m_strOutputDirPath, GEOTIFFFILE);
    int iRet = OutputGeoTiff(result, targetBuilding, strGeoTiffFilePath);
#ifdef _DEBUG
    if (iRet == 1)
    {
        std::cout << "Failed to output GeoTiff file!! " << std::endl;
    }
    else if (iRet == 2)
    {
        std::cout << "Cancel the Judgment!! " << std::endl;
    }
    _ftime32_s(&timebuffer);
    dEndStart = timebuffer.time + (double)timebuffer.millitm / 1000.0 - dStart;
    std::cout << "OutputGeoTiff Time: " << dEndStart << " sec" << std::endl;
#endif

    return iRet;
};

int CJudgeSuitablePlaceMng::JudgeLand()
{
#ifdef _DEBUG
    std::cout << "JudgeLand Start " << std::endl;
    struct __timeb32 timebuffer;
    _ftime32_s(&timebuffer);
    double dStart = timebuffer.time + (double)timebuffer.millitm / 1000.0;
#endif

    CLandData targetLand;

    // エリアごと
    for (int iAreaCount = 0; iAreaCount < m_allAreaList->size(); iAreaCount++)
    {
        std::string areaID = (*m_allAreaList)[iAreaCount].areaID;
        LANDSURFACE* pLand = &(*m_allAreaList)[iAreaCount].landData;

        CLand land;
        land.m_strAreaId = areaID;                                       // エリアID
        land.dLandHeight = pLand->dLandHeight;
        land.m_pSurface = &(*m_allAreaList)[iAreaCount].pos2dList;       // 屋根リスト
        land.dSolorRadiation = pLand->dSolorRadiation;
        land.dFloodDepth = pLand->dFloodDepth;
        land.dTsunamiHeight = pLand->dTsunamiHeight;
        land.bLandslideArea = pLand->bLandslideArea;

        // 屋根リストがない場合は飛ばす
        if (land.m_pSurface->size() == 0) continue;

        // 集計範囲指定
        if (m_pUIParam->m_pAggregationRange != NULL)
        {
            // 集計範囲内の建物か判定
            bool bRet = land.IsLandInPolygon((unsigned int)m_aggregationRange.size(), m_aggregationRange.data());
            if (bRet) targetLand.Add(land);
        }
        // 全範囲集計
        else
        {
            targetLand.Add(land);
        }
    }

    if (targetLand.GetLandSize() == 0)
    {
#ifdef _DEBUG
        std::cout << "No target Land!! " << std::endl;
#endif
        return 0;
    }

    // エリアIDごとの方位角データを読み込む
    bool bReadAzimuth = false;
    std::wstring strCSVName = GetINIParam()->GetAzimuthCSVPath();
    if (strCSVName != L"")
    {
        std::wstring strFilePath = GetFUtil()->Combine(m_pUIParam->m_strAnalyzeResultPath, strCSVName);
        bReadAzimuth = targetLand.ReadAzimuthCSV(strFilePath);
    }

    CResultJudgment result;     // 判定結果
    result.SetTarget(eTarget::TARGET_LAND);

    // エリアごとに判定する
    for (int i = 0; i < targetLand.GetLandSize(); i++)
    {
        CLand Land = *targetLand.GetLandAt(i);

        ResultJudgment resultJudgment;
        // エリアIDを設定
        resultJudgment.m_strAreaId = Land.m_strAreaId;

        // バウンディングの取得
        double dMinX = 0.0, dMinY = 0.0, dMinZ = 0.0;
        double dMaxX = 0.0, dMaxY = 0.0, dMaxZ = 0.0;
        Land.CalcBounding(&dMinX, &dMinY, &dMaxX, &dMaxY, &dMinZ, &dMaxZ);
        // 中心座標を取得
        CPoint2D posCenter = CPoint2D((dMinX + dMaxX) * 0.5, (dMaxY + dMaxY) * 0.5);

        /////////
        // 太陽光パネルの設置に関して優先度が低い施設の判定
        /////////
        // 日射量が少ない施設を除外
        {
            // 年間予測日射量が建物属性にない場合は判定しない
            if (Land.dSolorRadiation >= 0.0)
            {
                double dSolorRadiation = m_pUIParam->m_pBuildingParam->dSolorRadiation;
                if (dSolorRadiation > 0.0)
                {   // kWh/m2 未満は不適
                    resultJudgment.m_strSuitable1_1_1 = Land.dSolorRadiation < dSolorRadiation ? "×" : "○";
                }
                double dLowerPercent = m_pUIParam->m_pBuildingParam->dLowerPercent;
                if (dLowerPercent > 0.0)
                {   // 日射量を昇順でソートした時の順位
                    double dTargetOrder = targetLand.GetSolorRadiationOrder(Land.m_strAreaId);
                    // 下位％は不適
                    resultJudgment.m_strSuitable1_1_2 = (dTargetOrder / (double)targetLand.GetLandSize() <= dLowerPercent / 100.0) ? "×" : "○";
                }
            }
        }

        /////////
        // 災害時に太陽光パネルが破損、消失する危険性の判定
        /////////
        if (Land.dLandHeight >= 0.0)
        {
            // 高さが想定される最大津波高さを下回る(未満)土地を除外
            {
                // 最大津波高さが土地属性にない場合は判定しない
                if (Land.dTsunamiHeight > 0.0)
                {
                    if (m_pUIParam->m_pHazardParam->bBelowTsunamiHeight)
                    {
                        resultJudgment.m_strSuitable2_1 = (Land.dLandHeight < Land.dTsunamiHeight) ? "×" : "○";
                    }
                }
            }
            // 建物高さが想定される河川浸水想定浸水深を下回る建物を除外
            {
                // 浸水深が建物属性にない場合は判定しない
                if (Land.dFloodDepth > 0.0)
                {
                    if (m_pUIParam->m_pHazardParam->bBelowFloodDepth)
                    {
                        resultJudgment.m_strSuitable2_2 = (Land.dLandHeight < Land.dFloodDepth) ? "×" : "○";
                    }
                }
            }
        }
        // 土砂災害警戒区域内に存在する建物を除外
        {
            if (m_pUIParam->m_pHazardParam->bLandslideWarningArea)
            {
                resultJudgment.m_strSuitable2_3 = (Land.bLandslideArea) ? "×" : "○";
            }
        }
        // 積雪の多い地域の建物を除外
        {
            if (!m_pUIParam->m_pHazardParam->strWeatherDataPath.empty())
            {   // cm以上は除外
                double dSnowDepth = m_pUIParam->m_pHazardParam->dSnowDepth;
                if (dSnowDepth > 0.0)
                {
                    // 積雪深を取得
                    double dTargetDepth = m_pWeatherData->GetSnowDepth(posCenter);
                    resultJudgment.m_strSuitable2_4 = (dTargetDepth >= dSnowDepth) ? "×" : "○";
                }
                // 積雪荷重(kgf/m2)以上は除外
                double dSnowLoad = m_pUIParam->m_pHazardParam->dS;
                if (dSnowLoad > 0.0)
                {
                    double dTargetSnowLoad = m_pWeatherData->CalSnowLoad(posCenter, m_pUIParam->m_pHazardParam->dp);
                    resultJudgment.m_strSuitable2_4 = (dTargetSnowLoad >= dSnowLoad) ? "×" : "○";
                }
            }
        }

        /////////
        // 太陽光パネルの設置に制限がある施設の判定
        /////////
        //if( bReadAzimuth )
        {
            // 制限を設ける範囲のシェープファイル_1
            if (!m_pUIParam->m_pRestrictParam->strRestrictAreaPath_1.empty())
            {
                // 高さ
                double dHeight = m_pUIParam->m_pRestrictParam->dHeight_1;
                if (dHeight != -1)
                {
                    bool bRet = true;
                    // 制限区域かどうか
                    if (m_pRestrictAreaData[0]->IsLandInRestrictArea(*Land.m_pSurface))
                    {
                        if (Land.dLandHeight >= dHeight)
                        {
                            // 方位
                            eDirections eDirection = m_pUIParam->m_pRestrictParam->eDirections_1;
                            if (eDirection > eDirections::UNKNOWN && bReadAzimuth)
                            {
                                for (double dAzimuth : Land.vecAzimuth)
                                {
                                    eDirections eTargetDirection = GetDirection(dAzimuth);
                                    bRet &= (eTargetDirection == eDirection) ? false : true;
                                }
                            }
                            else
                            {
                                bRet = false;
                            }
                        }
                    }
                    resultJudgment.m_strSuitable3_1 = bRet ? "○" : "×";
                }
            }
            // 制限を設ける範囲のシェープファイル_2
            if (!m_pUIParam->m_pRestrictParam->strRestrictAreaPath_2.empty())
            {
                // 高さ
                double dHeight = m_pUIParam->m_pRestrictParam->dHeight_2;
                if (dHeight != -1)
                {
                    bool bRet = true;
                    // 制限区域かどうか
                    if (m_pRestrictAreaData[0]->IsLandInRestrictArea(*Land.m_pSurface))
                    {
                        if (Land.dLandHeight >= dHeight)
                        {
                            // 方位
                            eDirections eDirection = m_pUIParam->m_pRestrictParam->eDirections_2;
                            if (eDirection > eDirections::UNKNOWN && bReadAzimuth)
                            {
                                for (double dAzimuth : Land.vecAzimuth)
                                {
                                    eDirections eTargetDirection = GetDirection(dAzimuth);
                                    bRet &= (eTargetDirection == eDirection) ? false : true;
                                }
                            }
                            else
                            {
                                bRet = false;
                            }
                        }
                    }
                    resultJudgment.m_strSuitable3_2 = bRet ? "○" : "×";
                }
            }
            // 制限を設ける範囲のシェープ    _3
            if (!m_pUIParam->m_pRestrictParam->strRestrictAreaPath_3.empty())
            {
                // 高さ
                double dHeight = m_pUIParam->m_pRestrictParam->dHeight_3;
                if (dHeight != -1)
                {
                    bool bRet = true;
                    // 制限区域かどうか
                    if (m_pRestrictAreaData[0]->IsLandInRestrictArea(*Land.m_pSurface))
                    {
                        if (Land.dLandHeight >= dHeight)
                        {
                            // 方位
                            eDirections eDirection = m_pUIParam->m_pRestrictParam->eDirections_3;
                            if (eDirection > eDirections::UNKNOWN && bReadAzimuth)
                            {
                                for (double dAzimuth : Land.vecAzimuth)
                                {
                                    eDirections eTargetDirection = GetDirection(dAzimuth);
                                    bRet &= (eTargetDirection == eDirection) ? false : true;
                                }
                            }
                            else
                            {
                                bRet = false;
                            }
                        }
                    }
                    resultJudgment.m_strSuitable3_3 = bRet ? "○" : "×";
                }
            }
        }
        // 判定結果を格納
        result.Add(resultJudgment);
    }

    // 判定結果から優先度を決定する
    result.Prioritization();

    // 判定結果をCSV出力する
    std::wstring strCSVFilePath = GetFUtil()->Combine(m_pUIParam->m_strOutputDirPath, CSVFILE_LAND);
    if (!result.OutputResultCSV(strCSVFilePath))
    {
#ifdef _DEBUG
        std::cout << "Failed to output CSV file!! " << std::endl;
#endif
        return 1;
    }

#ifdef _DEBUG
    _ftime32_s(&timebuffer);
    double dEndStart = timebuffer.time + (double)timebuffer.millitm / 1000.0 - dStart;
    std::cout << "Jadge Time: " << dEndStart << " sec" << std::endl;
    dStart = timebuffer.time + (double)timebuffer.millitm / 1000.0;
#endif

    // キャンセルチェック
    std::wstring strCancelFilePath = GetFUtil()->Combine(m_pUIParam->m_strOutputDirPath, CStringEx::ToWString(CANCELFILE));
    if (GetFUtil()->IsExistPath(strCancelFilePath))
    {
        return 2; // キャンセル
    }

    // GeoTiff画像を出力する
    std::wstring strGeoTiffFilePath = GetFUtil()->Combine(m_pUIParam->m_strOutputDirPath, GEOTIFFFILE_LAND);
    int iRet = OutputGeoTiff(result, targetLand, strGeoTiffFilePath);
#ifdef _DEBUG
    if (iRet == 1)
    {
        std::cout << "Failed to output GeoTiff file!! " << std::endl;
    }
    else if (iRet == 2)
    {
        std::cout << "Cancel the Judgment!! " << std::endl;
    }
    _ftime32_s(&timebuffer);
    dEndStart = timebuffer.time + (double)timebuffer.millitm / 1000.0 - dStart;
    std::cout << "OutputGeoTiff Time: " << dEndStart << " sec" << std::endl;
#endif

    return iRet;
};

// 方位角を取得する
eDirections CJudgeSuitablePlaceMng::GetDirection(double dAzimuth)
{
    // 北		0°(337.5°～22.5°)
    if (CEpsUtil::LessEqual( 0, dAzimuth ) && CEpsUtil::Less( dAzimuth, 22.5 ) )
    {
        return eDirections::NORTH;
    }
    // 北北東	22.5°
    // 北東		45°(22.5°～67.5°)
    else if (CEpsUtil::Less(dAzimuth, 67.5))
    {
        return eDirections::NORTHEAST;
    }
    // 東北東	67.5°		    
    //  東		90°(67.5°～112.5°)
    else if (CEpsUtil::Less(dAzimuth, 112.5))
    {
        return eDirections::EAST;
    }
    // 東南東	112.5°
    // 南東		135°(112.5°～157.5°)
    else if (CEpsUtil::Less(dAzimuth, 157.5))
    {
        return eDirections::SOUTHEAST;
    }
    // 南南東	157.5°
    // 南		180°(157.5°～202.5°)
    else if (CEpsUtil::Less(dAzimuth, 202.5))
    {
        return eDirections::SOUTH;
    }
    // 南南西	202.5°
    // 南西	225°(202.5°～247.5°)
    else if (CEpsUtil::Less(dAzimuth, 247.5))
    {
        return eDirections::SOUTHWEST;
    }
    // 西南西	247.5°
    // 西		270°(247.5°～292.5°)
    else if (CEpsUtil::Less(dAzimuth, 292.5))
    {
        return eDirections::WEST;
    }
    // 西北西	292.5°
    // 北西		315°(292.5°～337.5°)
    else if (CEpsUtil::Less(dAzimuth, 337.5))
    {
        return eDirections::NORTHWEST;
    }
    // 北北西	337.5°
    // 北		0°(337.5°～22.5°)
    else if (CEpsUtil::Less(dAzimuth, 360))
    {
        return eDirections::NORTH;
    }
    return eDirections::NORTH;
}

// GeoTiff画像を出力する
int CJudgeSuitablePlaceMng::OutputGeoTiff(CResultJudgment result, CBuildingData buildings, const std::wstring& filepath)
{
    int JPZONE = GetINIParam()->GetJPZone();
    float dMeshSize = CTRLVALUE_MESH_SIZE_DEFAULT;

    CTiffDataManager cTiffDataManager;
    cTiffDataManager.SetFilePath(filepath);
    cTiffDataManager.SetEPSGCode(JGD2011_EPSG_CODE_TABLE[JPZONE - 1]);
    cTiffDataManager.SetMeshSize(dMeshSize);
    cTiffDataManager.SetNoDataVal(CTRLVALUE_NO_DATA_VALUE_DEFAULT);
    cTiffDataManager.SetColorSetting(L"ColorSetting_JudgeSuitablePlace.txt");

    // 建物ごとのデータを作成する
    for (int i = 0; i < buildings.GetBuildingSize(); i++)
    {
        CBuilding Building = *buildings.GetBuildingAt(i);

        // 建物の優先度ランクを取得する
        int iPriority = (int)result.GetPriority(Building.m_strBuildingId);

        // 建物のバウンディングの取得
        double dMinX = 0.0, dMinY = 0.0, dMinZ = 0.0;
        double dMaxX = 0.0, dMaxY = 0.0, dMaxZ = 0.0;
        Building.CalcBounding(&dMinX, &dMinY, &dMaxX, &dMaxY, &dMinZ, &dMaxZ);

        int iH = (int)std::ceil((dMaxY - dMinY) / dMeshSize);
        int iW = (int)std::ceil((dMaxX - dMinX) / dMeshSize);

        std::vector<CPointBase>* vecData = new std::vector<CPointBase>;
        for (int h = 0; h < iH; h++)
        {
            double curtY = dMinY + h * (double)dMeshSize;
            if (CEpsUtil::Less(dMaxY, curtY)) curtY = dMaxY;

            for (int w = 0; w < iW; w++)
            {
                double curtX = dMinX + w * (double)dMeshSize;
                if (CEpsUtil::Less(dMaxX, curtX)) curtX = dMaxX;

                // 屋根ごとに内外判定する
                for (const auto& roofSurfaces : *Building.m_pRoofSurfaceList)
                {
                    for (const auto& surfaceMembers : roofSurfaces.roofSurfaceList)
                    {
                        // 内外判定用
                        int iCountPoint = (int)surfaceMembers.posList.size();
                        CPoint2D* ppoint = new CPoint2D[iCountPoint];
                        for (int n = 0; n < iCountPoint; n++)
                        {
                            ppoint[n] = CPoint2D(surfaceMembers.posList[n].x, surfaceMembers.posList[n].y);
                        }
                        CPoint2D target2d(curtX, curtY);
                        bool bRet = CGeoUtil::IsPointInPolygon(target2d, iCountPoint, ppoint);

                        // Z値に優先度を設定する
                        if (bRet)
                        {
                            vecData->push_back(CPointBase(curtX, curtY, iPriority));
                        }
                        delete[] ppoint;
                    }
                }
            }
            // キャンセルチェック
            std::wstring strCancelFilePath = GetFUtil()->Combine(m_pUIParam->m_strOutputDirPath, CStringEx::ToWString(CANCELFILE));
            if (GetFUtil()->IsExistPath(strCancelFilePath))
            {
                return 2; // キャンセル
            }
        }
        if (vecData->size() > 0)
        {
            cTiffDataManager.AddTiffData(vecData);
        }
    }

    if (!cTiffDataManager.Create()) return 1;

#ifdef _DEBUG
    HANDLE hProc = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS_EX pmc;
    BOOL isSuccess = GetProcessMemoryInfo(
        hProc,
        (PROCESS_MEMORY_COUNTERS*)&pmc,
        sizeof(pmc));
    CloseHandle(hProc);
    if (isSuccess != FALSE)
        std::cout << "PROCESS_MEMORY2: " << pmc.PrivateUsage << std::endl;
#endif
    return 0;
}

// GeoTiff画像を出力する
int CJudgeSuitablePlaceMng::OutputGeoTiff(CResultJudgment result, CLandData lands, const std::wstring& filepath)
{
    int JPZONE = GetINIParam()->GetJPZone();
    float dMeshSize = CTRLVALUE_MESH_SIZE_DEFAULT;

    CTiffDataManager cTiffDataManager;
    cTiffDataManager.SetFilePath(filepath);
    cTiffDataManager.SetEPSGCode(JGD2011_EPSG_CODE_TABLE[JPZONE - 1]);
    cTiffDataManager.SetMeshSize(dMeshSize);
    cTiffDataManager.SetNoDataVal(CTRLVALUE_NO_DATA_VALUE_DEFAULT);
    cTiffDataManager.SetColorSetting(L"ColorSetting_JudgeSuitablePlace.txt");

    // 土地ごとのデータを作成する
    for (int i = 0; i < lands.GetLandSize(); i++)
    {
        CLand Land = *lands.GetLandAt(i);

        // 優先度ランクを取得する
        int iPriority = (int)result.GetPriority(Land.m_strAreaId);

        // バウンディングの取得
        double dMinX = 0.0, dMinY = 0.0, dMinZ = 0.0;
        double dMaxX = 0.0, dMaxY = 0.0, dMaxZ = 0.0;
        Land.CalcBounding(&dMinX, &dMinY, &dMaxX, &dMaxY, &dMinZ, &dMaxZ);

        int iH = (int)std::ceil((dMaxY - dMinY) / dMeshSize);
        int iW = (int)std::ceil((dMaxX - dMinX) / dMeshSize);

        // 内外判定用
        int iCountPoint = (int)Land.m_pSurface->size();
        CPoint2D* ppoint = new CPoint2D[iCountPoint];
        for (int n = 0; n < iCountPoint; n++)
        {
            ppoint[n] = (*Land.m_pSurface)[n];
        }

        std::vector<CPointBase>* vecData = new std::vector<CPointBase>;
        for (int h = 0; h < iH; h++)
        {
            double curtY = dMinY + h * (double)dMeshSize;
            if (CEpsUtil::Less(dMaxY, curtY)) curtY = dMaxY;

            for (int w = 0; w < iW; w++)
            {
                double curtX = dMinX + w * (double)dMeshSize;
                if (CEpsUtil::Less(dMaxX, curtX)) curtX = dMaxX;

                // 屋根ごとに内外判定する
                CPoint2D target2d(curtX, curtY);
                bool bRet = CGeoUtil::IsPointInPolygon(target2d, iCountPoint, ppoint);

                // Z値に優先度を設定する
                if (bRet)
                {
                    vecData->push_back(CPointBase(curtX, curtY, iPriority));
                }
            }
            // キャンセルチェック
            std::wstring strCancelFilePath = GetFUtil()->Combine(m_pUIParam->m_strOutputDirPath, CStringEx::ToWString(CANCELFILE));
            if (GetFUtil()->IsExistPath(strCancelFilePath))
            {
                return 2; // キャンセル
            }
        }
        delete[] ppoint;

        if (vecData->size() > 0)
        {
            cTiffDataManager.AddTiffData(vecData);
        }
    }

    if (!cTiffDataManager.Create()) return 1;

#ifdef _DEBUG
    HANDLE hProc = GetCurrentProcess();
    PROCESS_MEMORY_COUNTERS_EX pmc;
    BOOL isSuccess = GetProcessMemoryInfo(
        hProc,
        (PROCESS_MEMORY_COUNTERS*)&pmc,
        sizeof(pmc));
    CloseHandle(hProc);
    if (isSuccess != FALSE)
        std::cout << "PROCESS_MEMORY2: " << pmc.PrivateUsage << std::endl;
#endif
    return 0;
}

bool CJudgeSuitablePlaceMng::OutputLegendImage()
{
    bool ret = false;

    std::wstring strColorSettingPath = L"";
    std::wstring strMdlPath = CFileUtil::GetModulePathW();

    strColorSettingPath = L"ColorSetting_JudgeSuitablePlace.txt";
    ret = CImageUtil::CreateLegendImage(strColorSettingPath, L"優先度(Rank)");
    if (ret)
    {
        std::wstring colorSetting = CFileUtil::Combine(strMdlPath, strColorSettingPath);
        std::wstring srcPath = GetFUtil()->ChangeFileNameExt(colorSetting, L".jpg");

        std::wstring tmpPath = CFileUtil::Combine(m_pUIParam->m_strOutputDirPath, strColorSettingPath);
        std::wstring dstPath = GetFUtil()->ChangeFileNameExt(tmpPath, L".jpg");

        if (MoveFile(srcPath.c_str(), dstPath.c_str()) == FALSE)
        {
            return false;
        }
    }

    return ret;
}

