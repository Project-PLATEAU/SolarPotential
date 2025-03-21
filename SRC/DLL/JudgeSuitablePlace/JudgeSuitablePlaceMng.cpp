
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

    // �W�v�͈�(UI�w��)
    if (m_pUIParam->m_pAggregationRange != NULL)
    {
        m_aggregationRange.push_back(CPoint2D(m_pUIParam->m_pAggregationRange->dMinX, m_pUIParam->m_pAggregationRange->dMinY));
        m_aggregationRange.push_back(CPoint2D(m_pUIParam->m_pAggregationRange->dMaxX, m_pUIParam->m_pAggregationRange->dMinY));
        m_aggregationRange.push_back(CPoint2D(m_pUIParam->m_pAggregationRange->dMaxX, m_pUIParam->m_pAggregationRange->dMaxY));
        m_aggregationRange.push_back(CPoint2D(m_pUIParam->m_pAggregationRange->dMinX, m_pUIParam->m_pAggregationRange->dMaxY));
    }

    int iRet = 0;

    // �������s
    if (m_pUIParam->m_bExecBuild)
    {
        iRet = JudgeBuild();
        if (iRet != 0)  return iRet;
    }

    // �y�n���s
    if (m_pUIParam->m_bExecLand)
    {
        iRet = JudgeLand();
    }

    if (iRet == 0)
    {
        // �}��o��
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

    // �G���A����
    for (int iAreaCount = 0; iAreaCount < m_allAreaList->size(); iAreaCount++)
    {
        vector<BLDGLIST>* allList = &(*m_allAreaList)[iAreaCount].buildList;
        std::string areaID = (*m_allAreaList)[iAreaCount].areaID;

        // ���b�V������
        for (int iMeshCount = 0; iMeshCount < allList->size(); iMeshCount++)
        {
            BLDGLIST* pBldList = &allList->at(iMeshCount);

#ifdef _DEBUG
            std::cout << "BuildingList Size: " << pBldList->buildingList.size() << std::endl;
#endif

            // ��������
            for (int iBldCount = 0; iBldCount < pBldList->buildingList.size(); iBldCount++)
            {
                BUILDING* pBuilding = &pBldList->buildingList.at(iBldCount);

                CBuilding building;
                building.m_strAreaId = areaID;                                       // �G���AID
                building.m_iMeshId = pBldList->meshID;                               // ���b�V��ID
                building.m_strBuildingId = pBuilding->strBuildingId;                 // ����ID
                building.dBldHeight = pBuilding->dBuildingHeight;
                building.m_pRoofSurfaceList = &pBuilding->roofSurfaceList;           // �������X�g
                building.dSolorRadiation = pBuilding->dSolorRadiation;
                building.iBldStructureType = pBuilding->iBldStructureType;
                building.dFloodDepth = pBuilding->dFloodDepth;
                building.dTsunamiHeight = pBuilding->dTsunamiHeight;
                building.bLandslideArea = pBuilding->bLandslideArea;
                building.iBldStructureType2 = pBuilding->iBldStructureType2;
                building.iFloorType = pBuilding->iFloorType;

                // �������X�g���Ȃ��ꍇ�͔�΂�
                if (building.m_pRoofSurfaceList->size() == 0) continue;

                // �W�v�͈͎w��
                if (m_pUIParam->m_pAggregationRange != NULL)
                {
                    // �W�v�͈͓��̌���������
                    bool bRet = building.IsBuildingInPolygon((unsigned int)m_aggregationRange.size(), m_aggregationRange.data());
                    if (bRet) targetBuilding.Add(building);
                }
                // �S�͈͏W�v
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

    // ����ID���Ƃ̕��ʊp�f�[�^��ǂݍ���
    bool bReadAzimuth = false;
    std::wstring strCSVName= GetINIParam()->GetAzimuthCSVPath();
    if (strCSVName != L"")
    {
        std::wstring strFilePath = GetFUtil()->Combine(m_pUIParam->m_strAnalyzeResultPath, strCSVName);
        bReadAzimuth = targetBuilding.ReadAzimuthCSV(strFilePath);
    }

    // �g�������ݒ�t�@�C����ǂݍ���
    std::string strFilePath = GetFUtil()->Combine(GetFUtil()->GetModulePath(), "extended_attribute.ini");
    CExtendedAttributeIniFile ExtendedAttributeIniFile(strFilePath);

    CResultJudgment result;     // ���茋��
    result.SetTarget(eTarget::TARGET_BUILD);

    // �������Ƃɔ��肷��
    for (int i = 0; i < targetBuilding.GetBuildingSize(); i++)
    {
        CBuilding Building = *targetBuilding.GetBuildingAt(i);

        ResultJudgment resultJudgment;
        // �G���AID/���b�V��ID/����ID��ݒ�
        resultJudgment.m_strAreaId = Building.m_strAreaId;
        resultJudgment.m_iMeshId = Building.m_iMeshId;
        resultJudgment.m_strBuildingId = Building.m_strBuildingId;

        // �o�E���f�B���O�̎擾
        double dMinX = 0.0, dMinY = 0.0, dMinZ = 0.0;
        double dMaxX = 0.0, dMaxY = 0.0, dMaxZ = 0.0;
        Building.CalcBounding(&dMinX, &dMinY, &dMaxX, &dMaxY, &dMinZ, &dMaxZ);
        // �����̒��S���W���擾
        CPoint2D posCenter = CPoint2D((dMinX + dMaxX) * 0.5, (dMaxY + dMaxY) * 0.5);

        /////////
        // ���z���p�l���̐ݒu�Ɋւ��ėD��x���Ⴂ�{�݂̔���
        /////////
        // ���˗ʂ����Ȃ��{�݂����O
        {
            // �N�ԗ\�����˗ʂ����������ɂȂ��ꍇ�͔��肵�Ȃ�
            if (Building.dSolorRadiation >= 0.0)
            {
                double dSolorRadiation = m_pUIParam->m_pBuildingParam->dSolorRadiation;
                if (dSolorRadiation > 0.0)
                {   // kWh/m2 �����͕s�K
                    resultJudgment.m_strSuitable1_1_1 = Building.dSolorRadiation < dSolorRadiation ? "�~" : "��";
                }
                double dLowerPercent = m_pUIParam->m_pBuildingParam->dLowerPercent;
                if (dLowerPercent > 0.0)
                {   // ���˗ʂ������Ń\�[�g�������̏���
                    double dTargetOrder = targetBuilding.GetSolorRadiationOrder(Building.m_strBuildingId);
                    // ���ʁ��͕s�K
                    resultJudgment.m_strSuitable1_1_2 = (dTargetOrder / (double)targetBuilding.GetBuildingSize() <= dLowerPercent / 100.0) ? "�~" : "��";
                }
            }
        }

        // �����\���ɂ�鏜�O
        {
            int iBuildingStructure = m_pUIParam->m_pBuildingParam->iBuildingStructure;
            if (iBuildingStructure > 0)
            {
                // �����\�����擾
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
                {   // �ؑ��E�y���� 
                    bRet &= (eBldStrct == eBuildingStructure::WOOD) ? false : true;
                }
                if (iBuildingStructure & (1 << (int)eBuildingStructure::STEEL_REINFORCED_CONCRETE))
                {   // �S���S�؃R���N���[�g��
                    bRet &= (eBldStrct == eBuildingStructure::STEEL_REINFORCED_CONCRETE) ? false : true;
                }
                if (iBuildingStructure & (1 << (int)eBuildingStructure::REINFORCED_CONCRETE))
                {   // �S�؃R���N���[�g��
                    bRet &= (eBldStrct == eBuildingStructure::REINFORCED_CONCRETE) ? false : true;
                }
                if (iBuildingStructure & (1 << (int)eBuildingStructure::STEEL))
                {   // �S����
                    bRet &= (eBldStrct == eBuildingStructure::STEEL) ? false : true;
                }
                if (iBuildingStructure & (1 << (int)eBuildingStructure::LIGHT_GAUGE_STEEL))
                {   // �y�ʓS����
                    bRet &= (eBldStrct == eBuildingStructure::LIGHT_GAUGE_STEEL) ? false : true;
                }
                if (iBuildingStructure & (1 << (int)eBuildingStructure::MASONRY_CONSTRUCTION))
                {   // �����K���E�R���N���[�g�u���b�N���E�Α�
                    bRet &= (eBldStrct == eBuildingStructure::MASONRY_CONSTRUCTION) ? false : true;
                }
                if (iBuildingStructure & (1 << (int)eBuildingStructure::UNKNOWN))
                {   // �s��
                    bRet &= (eBldStrct == eBuildingStructure::UNKNOWN) ? false : true;
                }
                if (iBuildingStructure & (1 << (int)eBuildingStructure::NON_WOOD))
                {   // ��ؑ�
                    bRet &= (eBldStrct == eBuildingStructure::NON_WOOD) ? false : true;
                }
                resultJudgment.m_strSuitable1_2 = bRet ? "��" : "�~";
            }
        }

        // ����̊K�w�̎{�݂����O
        {
            // �����K�������������ɂȂ��ꍇ�͔��肵�Ȃ�
            if (Building.iFloorType > 0)
            {
                int iFloorsBelow = m_pUIParam->m_pBuildingParam->iFloorsBelow;
                int iUpperFloors = m_pUIParam->m_pBuildingParam->iUpperFloors;
                // �����K�����擾
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
                    resultJudgment.m_strSuitable1_3 = bRet ? "��" : "�~";
                }
            }
        }

        /////////
        // �ЊQ���ɑ��z���p�l�����j���A��������댯���̔���
        /////////
        // �������������������ɂȂ��ꍇ�͔��肵�Ȃ�
        if (Building.dBldHeight >= 0.0)
        {
            // �������z�肳���ő�Ôg�����������(����)���������O
            {
                // �ő�Ôg���������������ɂȂ��ꍇ�͔��肵�Ȃ�
                if (Building.dTsunamiHeight > 0.0)
                {
                    if (m_pUIParam->m_pHazardParam->bBelowTsunamiHeight)
                    {
                        resultJudgment.m_strSuitable2_1 = (Building.dBldHeight < Building.dTsunamiHeight) ? "�~" : "��";
                    }
                }
            }
            // �����������z�肳���͐�Z���z��Z���[������錚�������O
            {
                // �Z���[�����������ɂȂ��ꍇ�͔��肵�Ȃ�
                if (Building.dFloodDepth > 0.0)
                {
                    if (m_pUIParam->m_pHazardParam->bBelowFloodDepth)
                    {
                        resultJudgment.m_strSuitable2_2 = (Building.dBldHeight < Building.dFloodDepth) ? "�~" : "��";
                    }
                }
            }
        }
        // �y���ЊQ�x�������ɑ��݂��錚�������O
        {
            if (m_pUIParam->m_pHazardParam->bLandslideWarningArea)
            {
                resultJudgment.m_strSuitable2_3 = (Building.bLandslideArea) ? "�~" : "��";
            }
        }
        // �ϐ�̑����n��̌��������O
        {
            if (!m_pUIParam->m_pHazardParam->strWeatherDataPath.empty())
            {   // cm�ȏ�͏��O
                double dSnowDepth = m_pUIParam->m_pHazardParam->dSnowDepth;
                if (dSnowDepth > 0.0)
                {
                    // �ϐ�[���擾
                    double dTargetDepth = m_pWeatherData->GetSnowDepth(posCenter);
                    resultJudgment.m_strSuitable2_4 = (dTargetDepth >= dSnowDepth) ? "�~" : "��";
                }
                // �ϐ�׏d(kgf/m2)�ȏ�͏��O
                double dSnowLoad = m_pUIParam->m_pHazardParam->dS;
                if (dSnowLoad > 0.0)
                {
                    double dTargetSnowLoad = m_pWeatherData->CalSnowLoad(posCenter, m_pUIParam->m_pHazardParam->dp);
                    resultJudgment.m_strSuitable2_4 = (dTargetSnowLoad >= dSnowLoad) ? "�~" : "��";
                }
            }
        }
        
        /////////
        // ���z���p�l���̐ݒu�ɐ���������{�݂̔���
        /////////
        //if( bReadAzimuth )
        {
            // ������݂���͈͂̃V�F�[�v�t�@�C��_1
            if (!m_pUIParam->m_pRestrictParam->strRestrictAreaPath_1.empty())
            {
                // ����
                double dHeight = m_pUIParam->m_pRestrictParam->dHeight_1;
                if (dHeight != -1)
                {
                    bool bRet = true;
                    // ������悩�ǂ���
                    if (m_pRestrictAreaData[0]->IsBuildingInRestrictArea(*Building.m_pRoofSurfaceList))
                    {
                        if (Building.dBldHeight >= dHeight)
                        {
                            // ����
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
                    resultJudgment.m_strSuitable3_1 = bRet ? "��" : "�~";
                }
            }
            // ������݂���͈͂̃V�F�[�v�t�@�C��_2
            if (!m_pUIParam->m_pRestrictParam->strRestrictAreaPath_2.empty())
            {
                // ����
                double dHeight = m_pUIParam->m_pRestrictParam->dHeight_2;
                if (dHeight != -1)
                {
                    bool bRet = true;
                    // ������悩�ǂ���
                    if (m_pRestrictAreaData[1]->IsBuildingInRestrictArea(*Building.m_pRoofSurfaceList))
                    {
                        if (Building.dBldHeight >= dHeight)
                        {
                            // ����
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
                    resultJudgment.m_strSuitable3_2 = bRet ? "��" : "�~";
                }
            }
            // ������݂���͈͂̃V�F�[�v    _3
            if (!m_pUIParam->m_pRestrictParam->strRestrictAreaPath_3.empty())
            {
                // ����
                double dHeight = m_pUIParam->m_pRestrictParam->dHeight_3;
                if (dHeight != -1)
                {
                    bool bRet = true;
                    // ������悩�ǂ���
                    if (m_pRestrictAreaData[2]->IsBuildingInRestrictArea(*Building.m_pRoofSurfaceList))
                    {
                        if (Building.dBldHeight >= dHeight)
                        {
                            // ����
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
                    resultJudgment.m_strSuitable3_3 = bRet ? "��" : "�~";
                }
            }
        }
        // ���茋�ʂ��i�[
        result.Add(resultJudgment);
    }

    // ���茋�ʂ���D��x�����肷��
    result.Prioritization();

    // ���茋�ʂ�CSV�o�͂���
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

    // �L�����Z���`�F�b�N
    std::wstring strCancelFilePath = GetFUtil()->Combine(m_pUIParam->m_strOutputDirPath, CStringEx::ToWString(CANCELFILE));
    if (GetFUtil()->IsExistPath(strCancelFilePath))
    {
        return 2; // �L�����Z��
    }

    // GeoTiff�摜���o�͂���
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

    // �G���A����
    for (int iAreaCount = 0; iAreaCount < m_allAreaList->size(); iAreaCount++)
    {
        std::string areaID = (*m_allAreaList)[iAreaCount].areaID;
        LANDSURFACE* pLand = &(*m_allAreaList)[iAreaCount].landData;

        CLand land;
        land.m_strAreaId = areaID;                                       // �G���AID
        land.dLandHeight = pLand->dLandHeight;
        land.m_pSurface = &(*m_allAreaList)[iAreaCount].pos2dList;       // �������X�g
        land.dSolorRadiation = pLand->dSolorRadiation;
        land.dFloodDepth = pLand->dFloodDepth;
        land.dTsunamiHeight = pLand->dTsunamiHeight;
        land.bLandslideArea = pLand->bLandslideArea;

        // �������X�g���Ȃ��ꍇ�͔�΂�
        if (land.m_pSurface->size() == 0) continue;

        // �W�v�͈͎w��
        if (m_pUIParam->m_pAggregationRange != NULL)
        {
            // �W�v�͈͓��̌���������
            bool bRet = land.IsLandInPolygon((unsigned int)m_aggregationRange.size(), m_aggregationRange.data());
            if (bRet) targetLand.Add(land);
        }
        // �S�͈͏W�v
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

    // �G���AID���Ƃ̕��ʊp�f�[�^��ǂݍ���
    bool bReadAzimuth = false;
    std::wstring strCSVName = GetINIParam()->GetAzimuthCSVPath();
    if (strCSVName != L"")
    {
        std::wstring strFilePath = GetFUtil()->Combine(m_pUIParam->m_strAnalyzeResultPath, strCSVName);
        bReadAzimuth = targetLand.ReadAzimuthCSV(strFilePath);
    }

    CResultJudgment result;     // ���茋��
    result.SetTarget(eTarget::TARGET_LAND);

    // �G���A���Ƃɔ��肷��
    for (int i = 0; i < targetLand.GetLandSize(); i++)
    {
        CLand Land = *targetLand.GetLandAt(i);

        ResultJudgment resultJudgment;
        // �G���AID��ݒ�
        resultJudgment.m_strAreaId = Land.m_strAreaId;

        // �o�E���f�B���O�̎擾
        double dMinX = 0.0, dMinY = 0.0, dMinZ = 0.0;
        double dMaxX = 0.0, dMaxY = 0.0, dMaxZ = 0.0;
        Land.CalcBounding(&dMinX, &dMinY, &dMaxX, &dMaxY, &dMinZ, &dMaxZ);
        // ���S���W���擾
        CPoint2D posCenter = CPoint2D((dMinX + dMaxX) * 0.5, (dMaxY + dMaxY) * 0.5);

        /////////
        // ���z���p�l���̐ݒu�Ɋւ��ėD��x���Ⴂ�{�݂̔���
        /////////
        // ���˗ʂ����Ȃ��{�݂����O
        {
            // �N�ԗ\�����˗ʂ����������ɂȂ��ꍇ�͔��肵�Ȃ�
            if (Land.dSolorRadiation >= 0.0)
            {
                double dSolorRadiation = m_pUIParam->m_pBuildingParam->dSolorRadiation;
                if (dSolorRadiation > 0.0)
                {   // kWh/m2 �����͕s�K
                    resultJudgment.m_strSuitable1_1_1 = Land.dSolorRadiation < dSolorRadiation ? "�~" : "��";
                }
                double dLowerPercent = m_pUIParam->m_pBuildingParam->dLowerPercent;
                if (dLowerPercent > 0.0)
                {   // ���˗ʂ������Ń\�[�g�������̏���
                    double dTargetOrder = targetLand.GetSolorRadiationOrder(Land.m_strAreaId);
                    // ���ʁ��͕s�K
                    resultJudgment.m_strSuitable1_1_2 = (dTargetOrder / (double)targetLand.GetLandSize() <= dLowerPercent / 100.0) ? "�~" : "��";
                }
            }
        }

        /////////
        // �ЊQ���ɑ��z���p�l�����j���A��������댯���̔���
        /////////
        if (Land.dLandHeight >= 0.0)
        {
            // �������z�肳���ő�Ôg�����������(����)�y�n�����O
            {
                // �ő�Ôg�������y�n�����ɂȂ��ꍇ�͔��肵�Ȃ�
                if (Land.dTsunamiHeight > 0.0)
                {
                    if (m_pUIParam->m_pHazardParam->bBelowTsunamiHeight)
                    {
                        resultJudgment.m_strSuitable2_1 = (Land.dLandHeight < Land.dTsunamiHeight) ? "�~" : "��";
                    }
                }
            }
            // �����������z�肳���͐�Z���z��Z���[������錚�������O
            {
                // �Z���[�����������ɂȂ��ꍇ�͔��肵�Ȃ�
                if (Land.dFloodDepth > 0.0)
                {
                    if (m_pUIParam->m_pHazardParam->bBelowFloodDepth)
                    {
                        resultJudgment.m_strSuitable2_2 = (Land.dLandHeight < Land.dFloodDepth) ? "�~" : "��";
                    }
                }
            }
        }
        // �y���ЊQ�x�������ɑ��݂��錚�������O
        {
            if (m_pUIParam->m_pHazardParam->bLandslideWarningArea)
            {
                resultJudgment.m_strSuitable2_3 = (Land.bLandslideArea) ? "�~" : "��";
            }
        }
        // �ϐ�̑����n��̌��������O
        {
            if (!m_pUIParam->m_pHazardParam->strWeatherDataPath.empty())
            {   // cm�ȏ�͏��O
                double dSnowDepth = m_pUIParam->m_pHazardParam->dSnowDepth;
                if (dSnowDepth > 0.0)
                {
                    // �ϐ�[���擾
                    double dTargetDepth = m_pWeatherData->GetSnowDepth(posCenter);
                    resultJudgment.m_strSuitable2_4 = (dTargetDepth >= dSnowDepth) ? "�~" : "��";
                }
                // �ϐ�׏d(kgf/m2)�ȏ�͏��O
                double dSnowLoad = m_pUIParam->m_pHazardParam->dS;
                if (dSnowLoad > 0.0)
                {
                    double dTargetSnowLoad = m_pWeatherData->CalSnowLoad(posCenter, m_pUIParam->m_pHazardParam->dp);
                    resultJudgment.m_strSuitable2_4 = (dTargetSnowLoad >= dSnowLoad) ? "�~" : "��";
                }
            }
        }

        /////////
        // ���z���p�l���̐ݒu�ɐ���������{�݂̔���
        /////////
        //if( bReadAzimuth )
        {
            // ������݂���͈͂̃V�F�[�v�t�@�C��_1
            if (!m_pUIParam->m_pRestrictParam->strRestrictAreaPath_1.empty())
            {
                // ����
                double dHeight = m_pUIParam->m_pRestrictParam->dHeight_1;
                if (dHeight != -1)
                {
                    bool bRet = true;
                    // ������悩�ǂ���
                    if (m_pRestrictAreaData[0]->IsLandInRestrictArea(*Land.m_pSurface))
                    {
                        if (Land.dLandHeight >= dHeight)
                        {
                            // ����
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
                    resultJudgment.m_strSuitable3_1 = bRet ? "��" : "�~";
                }
            }
            // ������݂���͈͂̃V�F�[�v�t�@�C��_2
            if (!m_pUIParam->m_pRestrictParam->strRestrictAreaPath_2.empty())
            {
                // ����
                double dHeight = m_pUIParam->m_pRestrictParam->dHeight_2;
                if (dHeight != -1)
                {
                    bool bRet = true;
                    // ������悩�ǂ���
                    if (m_pRestrictAreaData[0]->IsLandInRestrictArea(*Land.m_pSurface))
                    {
                        if (Land.dLandHeight >= dHeight)
                        {
                            // ����
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
                    resultJudgment.m_strSuitable3_2 = bRet ? "��" : "�~";
                }
            }
            // ������݂���͈͂̃V�F�[�v    _3
            if (!m_pUIParam->m_pRestrictParam->strRestrictAreaPath_3.empty())
            {
                // ����
                double dHeight = m_pUIParam->m_pRestrictParam->dHeight_3;
                if (dHeight != -1)
                {
                    bool bRet = true;
                    // ������悩�ǂ���
                    if (m_pRestrictAreaData[0]->IsLandInRestrictArea(*Land.m_pSurface))
                    {
                        if (Land.dLandHeight >= dHeight)
                        {
                            // ����
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
                    resultJudgment.m_strSuitable3_3 = bRet ? "��" : "�~";
                }
            }
        }
        // ���茋�ʂ��i�[
        result.Add(resultJudgment);
    }

    // ���茋�ʂ���D��x�����肷��
    result.Prioritization();

    // ���茋�ʂ�CSV�o�͂���
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

    // �L�����Z���`�F�b�N
    std::wstring strCancelFilePath = GetFUtil()->Combine(m_pUIParam->m_strOutputDirPath, CStringEx::ToWString(CANCELFILE));
    if (GetFUtil()->IsExistPath(strCancelFilePath))
    {
        return 2; // �L�����Z��
    }

    // GeoTiff�摜���o�͂���
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

// ���ʊp���擾����
eDirections CJudgeSuitablePlaceMng::GetDirection(double dAzimuth)
{
    // �k		0��(337.5���`22.5��)
    if (CEpsUtil::LessEqual( 0, dAzimuth ) && CEpsUtil::Less( dAzimuth, 22.5 ) )
    {
        return eDirections::NORTH;
    }
    // �k�k��	22.5��
    // �k��		45��(22.5���`67.5��)
    else if (CEpsUtil::Less(dAzimuth, 67.5))
    {
        return eDirections::NORTHEAST;
    }
    // ���k��	67.5��		    
    //  ��		90��(67.5���`112.5��)
    else if (CEpsUtil::Less(dAzimuth, 112.5))
    {
        return eDirections::EAST;
    }
    // ���쓌	112.5��
    // �쓌		135��(112.5���`157.5��)
    else if (CEpsUtil::Less(dAzimuth, 157.5))
    {
        return eDirections::SOUTHEAST;
    }
    // ��쓌	157.5��
    // ��		180��(157.5���`202.5��)
    else if (CEpsUtil::Less(dAzimuth, 202.5))
    {
        return eDirections::SOUTH;
    }
    // ��쐼	202.5��
    // �쐼	225��(202.5���`247.5��)
    else if (CEpsUtil::Less(dAzimuth, 247.5))
    {
        return eDirections::SOUTHWEST;
    }
    // ���쐼	247.5��
    // ��		270��(247.5���`292.5��)
    else if (CEpsUtil::Less(dAzimuth, 292.5))
    {
        return eDirections::WEST;
    }
    // ���k��	292.5��
    // �k��		315��(292.5���`337.5��)
    else if (CEpsUtil::Less(dAzimuth, 337.5))
    {
        return eDirections::NORTHWEST;
    }
    // �k�k��	337.5��
    // �k		0��(337.5���`22.5��)
    else if (CEpsUtil::Less(dAzimuth, 360))
    {
        return eDirections::NORTH;
    }
    return eDirections::NORTH;
}

// GeoTiff�摜���o�͂���
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

    // �������Ƃ̃f�[�^���쐬����
    for (int i = 0; i < buildings.GetBuildingSize(); i++)
    {
        CBuilding Building = *buildings.GetBuildingAt(i);

        // �����̗D��x�����N���擾����
        int iPriority = (int)result.GetPriority(Building.m_strBuildingId);

        // �����̃o�E���f�B���O�̎擾
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

                // �������Ƃɓ��O���肷��
                for (const auto& roofSurfaces : *Building.m_pRoofSurfaceList)
                {
                    for (const auto& surfaceMembers : roofSurfaces.roofSurfaceList)
                    {
                        // ���O����p
                        int iCountPoint = (int)surfaceMembers.posList.size();
                        CPoint2D* ppoint = new CPoint2D[iCountPoint];
                        for (int n = 0; n < iCountPoint; n++)
                        {
                            ppoint[n] = CPoint2D(surfaceMembers.posList[n].x, surfaceMembers.posList[n].y);
                        }
                        CPoint2D target2d(curtX, curtY);
                        bool bRet = CGeoUtil::IsPointInPolygon(target2d, iCountPoint, ppoint);

                        // Z�l�ɗD��x��ݒ肷��
                        if (bRet)
                        {
                            vecData->push_back(CPointBase(curtX, curtY, iPriority));
                        }
                        delete[] ppoint;
                    }
                }
            }
            // �L�����Z���`�F�b�N
            std::wstring strCancelFilePath = GetFUtil()->Combine(m_pUIParam->m_strOutputDirPath, CStringEx::ToWString(CANCELFILE));
            if (GetFUtil()->IsExistPath(strCancelFilePath))
            {
                return 2; // �L�����Z��
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

// GeoTiff�摜���o�͂���
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

    // �y�n���Ƃ̃f�[�^���쐬����
    for (int i = 0; i < lands.GetLandSize(); i++)
    {
        CLand Land = *lands.GetLandAt(i);

        // �D��x�����N���擾����
        int iPriority = (int)result.GetPriority(Land.m_strAreaId);

        // �o�E���f�B���O�̎擾
        double dMinX = 0.0, dMinY = 0.0, dMinZ = 0.0;
        double dMaxX = 0.0, dMaxY = 0.0, dMaxZ = 0.0;
        Land.CalcBounding(&dMinX, &dMinY, &dMaxX, &dMaxY, &dMinZ, &dMaxZ);

        int iH = (int)std::ceil((dMaxY - dMinY) / dMeshSize);
        int iW = (int)std::ceil((dMaxX - dMinX) / dMeshSize);

        // ���O����p
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

                // �������Ƃɓ��O���肷��
                CPoint2D target2d(curtX, curtY);
                bool bRet = CGeoUtil::IsPointInPolygon(target2d, iCountPoint, ppoint);

                // Z�l�ɗD��x��ݒ肷��
                if (bRet)
                {
                    vecData->push_back(CPointBase(curtX, curtY, iPriority));
                }
            }
            // �L�����Z���`�F�b�N
            std::wstring strCancelFilePath = GetFUtil()->Combine(m_pUIParam->m_strOutputDirPath, CStringEx::ToWString(CANCELFILE));
            if (GetFUtil()->IsExistPath(strCancelFilePath))
            {
                return 2; // �L�����Z��
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
    ret = CImageUtil::CreateLegendImage(strColorSettingPath, L"�D��x(Rank)");
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

