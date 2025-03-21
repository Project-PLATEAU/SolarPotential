#pragma once

#define WIN32_LEAN_AND_MEAN             // Windows �w�b�_�[����قƂ�ǎg�p����Ă��Ȃ����������O����

// �v���O�����ɕK�v�Ȓǉ��w�b�_�[�������ŎQ�Ƃ��Ă�������
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "../../LIB/CommonUtil/CPoint2DPolygon.h"

#include <Windows.h>
#include <string.h>

#include <stdexcept>
#include <iostream>
#include <vector>
#include <filesystem>
#include <string>
#include <tchar.h>


#include <algorithm>
#include <iterator> 

#import "msxml6.dll" named_guids raw_interfaces_only
#include <atlbase.h>	// CComVariant, CComBSTR
#include <conio.h>
#include <iomanip>
#include <fstream>
#include <sstream>


// �g�p���閼�O��Ԃ�Key�i�K�v�ɂȂ�����ǉ��j
std::wstring namespaceKeys[] = {
     L"xmlns:core",
     L"xmlns:bldg",
     L"xmlns:gen",
     L"xmlns:gml",
     L"xmlns:uro",
     L"xmlns:wtr",
     L"xmlns:urf",

};

// CityGML�̃o�[�W����
enum class eCityGMLVersion
{
    VERSION_1,          // 1: 2021�N�x��
    VERSION_2,          // 2: 2022�N�x��
    End
};
// operator ++
eCityGMLVersion& operator ++ (eCityGMLVersion& ver)
{
    if (ver == eCityGMLVersion::End) {
        throw std::out_of_range("for eCityGMLVersion& operator ++ (eCityGMLVersion&)");
    }
    ver = eCityGMLVersion(static_cast<std::underlying_type<eCityGMLVersion>::type>(ver) + 1);
    return ver;
}

#define XPATH1 _T("core:CityModel/core:cityObjectMember/bldg:Building/bldg:boundedBy/bldg:RoofSurface")
#define XPATH2 _T("core:CityModel/core:cityObjectMember")
#define XPATH3 _T("bldg:Building")
#define XPATH4 _T("bldg:Building/bldg:boundedBy")
#define XPATH5 _T("bldg:RoofSurface")
#define XPATH6 _T("bldg:RoofSurface/bldg:lod2MultiSurface/gml:MultiSurface/gml:surfaceMember")
#define XPATH7 _T("gml:Polygon/gml:exterior/gml:LinearRing/gml:posList")

#define XPATH_measureAttribute1 _T("bldg:Building/gen:measureAttribute[@name='�\�����˗�']")
#define XPATH_measureAttribute2 _T("gen:value")
#define XPATH_measuredHeight1 _T("bldg:Building/bldg:measuredHeight")

// 2021
#define XPATH_genericAttributeSet1 _T("bldg:Building/gen:genericAttributeSet[contains(@name,'�^���Z��')]")
#define XPATH_genericAttributeSet2 _T("gen:measureAttribute[@name='�Z���[']")
#define XPATH_genericAttributeSet3 _T("gen:value")
#define XPATH_genericAttributeSet4 _T("bldg:Building/gen:genericAttributeSet[contains(@name,'�Ôg�Z��')]")
#define XPATH_genericAttributeSet5 _T("bldg:Building/gen:genericAttributeSet[@name='�y���ЊQ�x�����']")
// 2022
#define XPATH_genericAttributeSet1_2 _T("bldg:Building/uro:buildingDisasterRiskAttribute/uro:BuildingRiverFloodingRiskAttribute") // �^���Z��
#define XPATH_genericAttributeSet2_2 _T("uro:depth")
#define XPATH_genericAttributeSet3_2 _T("bldg:Building/uro:buildingDisasterRiskAttribute/uro:BuildingTsunamiRiskAttribute") // �Ôg�Z��
#define XPATH_genericAttributeSet4_2 _T("bldg:Building/uro:buildingDisasterRiskAttribute/uro:BuildingLandSlideRiskAttribute") // �y���ЊQ
#define XPATH_genericAttributeSet5_2 _T("uro:description")

// 2021
#define XPATH_buildingStructureType _T("bldg:Building/uro:buildingDetails/uro:BuildingDetails/uro:buildingStructureType")
// 2022
#define XPATH_buildingStructureType_2 _T("bldg:Building/uro:buildingDetailAttribute/uro:BuildingDetailAttribute/uro:buildingStructureType")

#define XPATH_extendedAttribute1 _T("bldg:Building/uro:extendedAttribute")
#define XPATH_extendedAttribute2 _T("uro:KeyValuePair/uro:key")
#define XPATH_extendedAttribute3 _T("uro:KeyValuePair/uro:codeValue")

#define XPATH_aggregateData1 _T("gen:measureAttribute[@name='�\�����˗�']")
#define XPATH_aggregateData2 _T("gen:value")
#define XPATH_aggregateData3 _T("gen:measureAttribute[@name='�\�����d��']")
#define XPATH_aggregateData4 _T("gen:measureAttribute[@name='���Q�������ԁi�Ď��j']")
#define XPATH_aggregateData5 _T("gen:measureAttribute[@name='���Q�������ԁi�t���j']")
#define XPATH_aggregateData6 _T("gen:measureAttribute[@name='���Q�������ԁi�~���j']")
#define XPATH_aggregateData7 _T("gen:measureAttribute[@name='���Q�������ԁi�w����j']")

// ����ID
#define XPATH_stringAttribute1 _T("bldg:Building/gen:stringAttribute[@name='����ID']")
#define XPATH_stringAttribute2 _T("gen:value")
#define XPATH_stringAttribute1_2 _T("bldg:Building/uro:buildingIDAttribute/uro:BuildingIDAttribute")
#define XPATH_stringAttribute2_2 _T("uro:buildingID")

#define BOUND_XPATH1 _T("core:CityModel/gml:boundedBy/gml:Envelope/gml:lowerCorner")
#define BOUND_XPATH2 _T("core:CityModel/gml:boundedBy/gml:Envelope/gml:upperCorner")

// �ЊQ���X�N(�Z��)
#define WTR_XPATH1 _T("wtr:WaterBody")
#define WTR_XPATH2 _T("wtr:lod1MultiSurface/gml:MultiSurface/gml:surfaceMember")
// �ЊQ���X�N(�y���ЊQ)
#define LSLD_XPATH1 _T("urf:SedimentDisasterProneArea")
#define LSLD_XPATH2 _T("urf:lod1MultiSurface/gml:MultiSurface/gml:surfaceMember")

#define CANCELFILE _T("cancel.txt")

const std::string landAnalyzeFile = "�y�n���Ɨ\�����d��.csv";
const std::string judgeFile = "�����ʓK�n���茋��.csv";
const std::string outputFile = "�W�v����.csv";
const std::string extension_csv = ".csv";
const std::string extension_gml = ".gml";
const std::string outputHeader = "�G���AID,�͈͓�������,�\�����˗ʑ��v(kWh/m2),�\�����d�ʑ��v(kWh),���Q�𔭐������錚����,���Q�������ԑ��v�i�Ď��j,���Q�������ԑ��v�i�t���j,���Q�������ԑ��v�i�~���j,���Q�������ԑ��v�i�w����j,�͈͓��D��x5������,�͈͓��D��x4������,�͈͓��D��x3������,�͈͓��D��x2������,�͈͓��D��x1������";
const int priorityLevel1 = 1;
const int priorityLevel2 = 2;
const int priorityLevel3 = 3;
const int priorityLevel4 = 4;
const int priorityLevel5 = 5;

// �n�ԍ�
int JPZONE;


// �G�N�X�|�[�g�ƃC���|�[�g�̐؂�ւ�
#ifdef AGGREGATEDATA_EXPORTS
#define VC_DLL_EXPORTS extern "C" __declspec(dllexport)
#else
#define VC_DLL_EXPORTS extern "C" __declspec(dllimport)
#endif


// ��̓G���A(C#�󂯓n���p)
typedef struct AnalyzeAreaData
{
    char strAreaId[10];
    char strAreaName[100];
    int nPointCount;
    double pPointArray[256];
};

VC_DLL_EXPORTS int __cdecl AggregateBldgFiles(const char* str, const char* strOut);
VC_DLL_EXPORTS int __cdecl AggregateLandFiles(const char* str, const char* strOut);
VC_DLL_EXPORTS int __cdecl AggregateAllData(const char* str, const char* strOut);
VC_DLL_EXPORTS void __cdecl Initialize();
VC_DLL_EXPORTS void __cdecl DllDispose();
VC_DLL_EXPORTS int __cdecl AnalyzeHazardRisk(const char* str, const char* strOut, bool fldrisk, bool tnmrisk, bool lsldrisk);
VC_DLL_EXPORTS void __cdecl AddAnalyzeAreaData(AnalyzeAreaData* p);

VC_DLL_EXPORTS void* __cdecl GetAllList();
VC_DLL_EXPORTS void* __cdecl GetAllAreaList();

// �K�n���茋��
typedef struct judgeSuitablePlace
{
    std::string strBuildingId;           // ����ID
    int priority;						 // �D��x
    std::string condition_1_1_1;		 // �������_1_1_1
    std::string condition_1_1_2;		 // �������_1_1_2
    std::string condition_1_2;			 // �������_1_2
    std::string condition_1_3;			 // �������_1_3
    std::string condition_2_1;			 // �������_2_1
    std::string condition_2_2;			 // �������_2_2
    std::string condition_2_3;			 // �������_2_3
    std::string condition_2_4;			 // �������_2_4
    std::string condition_3_1;			 // �������_3_1
    std::string condition_3_2;			 // �������_3_2
    std::string condition_3_3;			 // �������_3_3

} JUDGESUITABLEPLACE;

// �K�n���胊�X�g
typedef struct judgeList
{
    int meshID;                          // ���b�V��ID
    std::vector<JUDGESUITABLEPLACE> judgeSuitablePlaceList;  // �K�n���茋�ʃ��X�g

} JUDGELIST;

// �����ڍ�
typedef struct surfaceMembers
{
    std::vector<CPointBase> posList;               // ���W���X�g
} SURFACEMEMBERS;

// ����
typedef struct roofSurfaces
{
    std::string roofSurfaceId;                     // ����ID
    std::vector<SURFACEMEMBERS> roofSurfaceList;   // �����ڍ׃��X�g
} ROOFSURFACES;

// ����
typedef struct building
{

    std::string strBuildingId;           // ����ID
    double dBuildingHeight;		         // ����
    std::vector<ROOFSURFACES> roofSurfaceList;   // �������X�g
    double dSolorRadiation;              // �N�ԗ\�����˗�(kWh/m2)
    int iBldStructureType;               // �\�����
    double dFloodDepth;                  // �^���Z���z��̐Z���[(���[�g��)
    double dTsunamiHeight;               // �Ôg�Z���z��(���[�g��)
    bool bLandslideArea;                 // �y���ЊQ�x�����(�^�O�������true)
    int iBldStructureType2;              // �s�s���Ƃ̓Ǝ��敪�Ɋ�Â����z�\���̎��
    int iFloorType;                      // �s�s���Ƃ̓Ǝ��敪�Ɋ�Â��n��K���͈̔�

} BUILDING;


// �������X�g
typedef struct bldgList
{
    int meshID;                          // ���b�V��ID
    std::vector<BUILDING> buildingList;  // �������X�g

} BLDGLIST;

// �W�v���������
typedef struct agtBuilding
{
    std::string strBuildingId;           // ����ID
    double dSolorRadiation;              // �\�����˗�(kWh/m2)
    double dElectricGeneration;          // �\�����d��(kWh)
    double dLightPollutionSummer;        // ���Q��������(�Ď�)
    double dLightPollutionSpling;        // ���Q��������(�t��)
    double dLightPollutionWinter;        // ���Q��������(�~��)
    double dLightPollutionOneDay;        // ���Q��������(�w���)

} AGTBUILDING;

// �y�n
typedef struct landsurface
{
    double dLandHeight;                  // ����
    double dSolorRadiation;              // �\�����˗�(kWh/m2)
    double dFloodDepth;                  // �^���Z���z��̐Z���[(���[�g��)
    double dTsunamiHeight;               // �Ôg�Z���z��(���[�g��)
    bool bLandslideArea;                 // �y���ЊQ���X�N���(�͈͓�true)

} LANDSURFACE;

// �G���A
typedef struct areaData
{
    std::string areaID;                         // �G���AID
    std::string areaName;                       // �G���A����
    std::vector<CPoint2D> pos2dList;            // �G���A�\���_���X�g
    std::vector<CPoint2DPolygon> polygons;      // ���������ʃ|���S�����X�g

    // �G���A���̊e�f�[�^
    std::vector<BLDGLIST> buildList;            // ��̓G���A���ӂ̌������X�g
    LANDSURFACE landData;                       // �y�n�f�[�^

} AREADATA;

// �W�v���������X�g
typedef struct agtBldgList
{
    int meshID;                           // ���b�V��ID
    std::vector<AGTBUILDING> buildingList;// �������X�g

} AGTBLDGLIST;


// �W�v���y�n���
typedef struct agtLand
{
    std::string strLandId;               // �G���AID
    double dSolorRadiation;              // �\�����˗�(kWh/m2)
    double dElectricGeneration;          // �\�����d��(kWh)

} AGTLAND;


// ���W
typedef struct position
{
    double lon;                         // �o�x
    double lat;                         // �ܓx
    double ht;                          // ����

} POSITION;

// �ЊQ���X�N(�^���Z���E�Ôg)���f������ LOD1�f�[�^
typedef struct hazardRiskLOD1
{
    std::vector<SURFACEMEMBERS> wtrSurfaceList;    // �ʏڍ׃��X�g

} HAZARDRISKLOD1;

typedef struct fldRisk
{
    std::string meshID;                         // ���b�V��ID
    std::string scale;                          // �K��
    std::vector<HAZARDRISKLOD1> fldListLOD1;    // �Z���ʃ��X�g(LOD1)
    // �ő�ܓx�o�x
    POSITION upperCorner;
    // �ŏ��ܓx�o�x
    POSITION lowerCorner;
} FLDRISK;

typedef struct fldRisks
{
    std::string type;                           // �^���Z���z����t�H���_��
    std::string description;                    // �^���Z���z����}�t�H���_��(���n���A�w��͐얼)
    std::vector<FLDRISK> vecFldRisk;            // �^���Z���z��f�[�^���X�g

} FLDRISKLIST;

// �ЊQ���X�N(�Ôg)
typedef struct tnmRisk
{
    std::string meshID;                         // ���b�V��ID
    std::vector<HAZARDRISKLOD1> tnmRiskLOD1;    // �Z���ʃ��X�g(LOD1)
    // �ő�ܓx�o�x
    POSITION upperCorner;
    // �ŏ��ܓx�o�x
    POSITION lowerCorner;
} TNMRISK;

typedef struct tnmRisks
{
    std::string description;                    // �Ôg�Z���z��t�H���_��
    std::vector<TNMRISK> vecTnmRisk;            // �Ôg�Z���z��f�[�^���X�g

} TNMRISKLIST;

// �ЊQ���X�N(�y���ЊQ)
typedef struct lsldRisk
{
    std::string meshID;		                    // 3�����b�V��ID
    std::vector<HAZARDRISKLOD1> lsldRiskLOD1;   // �ЊQ���f�����X�g(LOD1)
    // �ő�ܓx�o�x
    POSITION upperCorner;
    // �ŏ��ܓx�o�x
    POSITION lowerCorner;
} LSLDRISK;

// �ЊQ���X�N
typedef struct hazardRisk
{
    std::vector<FLDRISKLIST> fldRisks;          // �^���Z���z��f�[�^���X�g
    std::vector<TNMRISKLIST> tnmRisks;          // �Ôg�Z���z��f�[�^���X�g
    std::vector<LSLDRISK> lsldRisks;            // �y���ЊQ�f�[�^���X�g

} HAZARDRISK;

// �ЊQ���X�N�f�[�^
HAZARDRISK hazardRiskData{};

// �S�̃��X�g
std::vector<BLDGLIST> allList{};
void* GetAllList()
{
    return (void*)(&allList);
}

// �͈̓��X�g���擾
std::vector<AREADATA> allAreaList{};
void* __cdecl GetAllAreaList()
{
    return (void*)(&allAreaList);
}

class AggregateData
{
public:
    AggregateData(void);
    ~AggregateData(void);

};

