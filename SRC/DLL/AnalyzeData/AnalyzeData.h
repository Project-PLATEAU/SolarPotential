#pragma once
#include "stdafx.h"
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "../../LIB/CommonUtil/CPoint2DPolygon.h"


#include <Windows.h>
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
#include <fstream>
#include <iomanip>

#include <assert.h>
#include <cmath>

#include <map>

#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")

// �g�p���閼�O��Ԃ�Key�i�K�v�ɂȂ�����ǉ��j
std::wstring namespaceKeys[] = {
     L"xmlns:core",
     L"xmlns:bldg",
     L"xmlns:gen",
     L"xmlns:gml",
     L"xmlns:app",
     L"xmlns:uro",
     L"xmlns:dem",
     L"xmlns:tran",

};

// CityGML�̃o�[�W����
enum class eCityGMLVersion
{
    VERSION_1,          // v1
    VERSION_2,          // v2
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

#define XPATH1 _T("bldg:Building/bldg:lod2Solid")
#define XPATH2 _T("core:CityModel/core:cityObjectMember")
#define XPATH3 _T("bldg:Building")
#define XPATH4 _T("bldg:Building/bldg:boundedBy")
#define XPATH5 _T("bldg:RoofSurface")
#define XPATH6 _T("bldg:RoofSurface/bldg:lod2MultiSurface/gml:MultiSurface/gml:surfaceMember")
#define XPATH7 _T("gml:Polygon")
#define XPATH8 _T("gml:Polygon/gml:exterior/gml:LinearRing")
#define XPATH9 _T("gml:Polygon/gml:exterior/gml:LinearRing/gml:posList")

#define XPATH10 _T("bldg:WallSurface")
#define XPATH11 _T("bldg:WallSurface/bldg:lod2MultiSurface/gml:MultiSurface/gml:surfaceMember")

#define XPATH12 _T("bldg:Building/bldg:lod1Solid/gml:Solid")
#define XPATH13 _T("gml:exterior/gml:CompositeSurface/gml:surfaceMember")

#define XPATH14 _T("gml:Polygon/gml:interior")
#define XPATH15 _T("gml:LinearRing")
#define XPATH16 _T("gml:LinearRing/gml:posList")

// ����ID
#define XPATH_stringAttribute1 _T("bldg:Building/gen:stringAttribute[@name='����ID']")
#define XPATH_stringAttribute2 _T("gen:value")
#define XPATH_stringAttribute1_2 _T("bldg:Building/uro:buildingIDAttribute/uro:BuildingIDAttribute")
#define XPATH_stringAttribute2_2 _T("uro:buildingID")

#define BOUND_XPATH1 _T("core:CityModel/gml:boundedBy/gml:Envelope/gml:lowerCorner")
#define BOUND_XPATH2 _T("core:CityModel/gml:boundedBy/gml:Envelope/gml:upperCorner")


#define DEM_XPATH1 _T("dem:ReliefFeature/dem:reliefComponent/dem:TINRelief/dem:tin/gml:TriangulatedSurface/gml:trianglePatches/gml:Triangle")
#define DEM_XPATH2 _T("gml:exterior/gml:LinearRing/gml:posList")
#define DEM_XPATH3 _T("dem:ReliefFeature/gml:name")

#define TRAN_XPATH1 _T("tran:Road/tran:lod1MultiSurface/gml:MultiSurface")
#define TRAN_XPATH2 _T("gml:surfaceMember")

#define TEX_XPATH1 _T("core:CityModel")
#define TEX_XPATH2 _T("core:CityModel/app:appearanceMember/app:Appearance")
#define TEX_XPATH3 _T("core:CityModel/app:appearanceMember")
#define OUTPUTFILE _T("initFile_Coordinates.txt")
#define CANCELFILE _T("cancel.txt")

#define INPUTFILE1 _T("�������ƌ��Q��������.csv")         // �\�����Q��������
#define INPUTFILE2 _T("�������Ɨ\�����d��.csv")           // �\�����˗ʁE�\�����d�ʃt�@�C����

#define IMG_NODATA -9999

// �n�ԍ�
int JPZONE;

// �ő�ܓx�o�x
CPoint2D maxPosition;
// �ŏ��ܓx�o�x
CPoint2D minPosition;

// �S��jpg�T�C�Y
int jpgWidth = 0;
int jpgHeight = 0;

// �؂�o��jpg�T�C�Y
int trimWidth = 0;
int trimHeight = 0;

// jpg�������W
double tfw_x = 0.0;;
double tfw_y = 0.0;;
double tfw_meshSize = 0.0;

// 1m���b�V�����W
typedef struct meshPositionXY
{
    double leftTopX;                        // ����X
    double leftTopY;                        // ����Y
    double leftDownX;                       // ����X
    double leftDownY;                       // ����Y
    double rightTopX;                       // �E��X
    double rightTopY;                       // �E��Y
    double rightDownX;                      // �E��X
    double rightDownY;                      // �E��Y

} MESHPOSITION_XY;

// DEM���W
typedef struct demPosition
{
    double lon;		                        // �o�x
    double lat;		                        // �ܓx
    double ht;		                        // ����

} DEMPOSITION;

// DEM�ڍ�
typedef struct demMembers
{
    std::string lv3meshID;                      // 3�����b�V��ID
    std::vector<CTriangle> posTriangleList;     // �O�p�`�̍��W���X�g
    // 3�����b�V���͈�(�O�p�`�����݂���͈�)�̃o�E���f�B���O
    double bbMinX{ 0.0 }, bbMinY{ 0.0 };
    double bbMaxX{ 0.0 }, bbMaxY{ 0.0 };

    ~demMembers()
    {
        posTriangleList.clear();
    }
} DEMMEMBERS;

// DEM
typedef struct demList
{
    std::string lv2meshID;                  // 2�����b�V��ID
    std::vector<DEMMEMBERS> demMemberList;  // DEM�ڍ׃��X�g

    ~demList()
    {
        demMemberList.clear();
    }
} DEMLIST;

// ���W
typedef struct position
{
    double lon;		                        // �o�x
    double lat;		                        // �ܓx
    double ht;		                        // ����


} POSITION;

// interior
typedef struct interior
{
    std::string linearRing;                 // ���C��ID
    std::vector<CPointBase> posList;        // ���W���X�g

    ~interior()
    {
        posList.clear();
    }
} INTERIOR;

// �ʏڍ�
typedef struct surfaceMembers
{
    std::string polygon;                                    // �|���S��ID
    std::string linearRing;                                 // ���C��ID
    std::vector<CPointBase> posList;                        // ���W���X�g(�O��)
    std::vector<INTERIOR> interiorList;                     // �����|���S�����X�g

    ~surfaceMembers()
    {
        posList.clear();
        interiorList.clear();
    }
} SURFACEMEMBERS;

// ����
typedef struct roofSurfaces
{
    std::string roofSurfaceId;		            // ����ID
    std::vector<SURFACEMEMBERS> roofSurfaceList; // �����ڍ׃��X�g
    std::vector<MESHPOSITION_XY> meshPosList;       // 1m���b�V�����W���X�g

    ~roofSurfaces()
    {
        roofSurfaceList.clear();
        meshPosList.clear();
    }
} ROOFSURFACES;

// ��
typedef struct wallSurfaces
{
    std::string wallSurfaceId;		            // ��ID
    std::vector<SURFACEMEMBERS> wallSurfaceList; // �Ǐڍ׃��X�g

    ~wallSurfaces()
    {
        wallSurfaceList.clear();
    }
} WALLSURFACES;

// ����LOD2
typedef struct buildings
{
    std::string building;		                // ����ID
    std::vector<ROOFSURFACES> roofSurfaceList;  // �������X�g
    std::vector<WALLSURFACES> wallSurfaceList;  // �ǃ��X�g

    ~buildings()
    {
        roofSurfaceList.clear();
        wallSurfaceList.clear();
    }
} BUILDINGS;

// ����LOD1(LOD1�����Ȃ�����)
typedef struct buildingsLOD1
{
    std::string building;
    std::vector<WALLSURFACES> wallSurfaceList;   // �ǃ��X�g�i�S�Ă̖ʁj

    ~buildingsLOD1()
    {
        wallSurfaceList.clear();
    }
}BUILDINGSLOD1;

// �����A���b�V�����W
typedef struct buildingsInfo
{
    std::vector<BUILDINGS> buildingList;        // �������X�g
    std::vector<BUILDINGSLOD1> buildingListLOD1; // �������X�g(LOD1)
    int bbMinX{ 0 }, bbMinY{ 0 };
    int bbMaxX{ 0 }, bbMaxY{ 0 };

    ~buildingsInfo()
    {
        buildingList.clear();
        buildingListLOD1.clear();
    }
} BUILDINGSINFO;

// 3�����b�V��
typedef struct bldgList
{
    std::string meshID;		                    // 3�����b�V��ID
    std::vector<BUILDINGS> buildingList;        // �������X�g
    std::vector<BUILDINGSLOD1> buildingListLOD1; // �������X�g(LOD1)
    // ���b�V�����W��MIN,MAX(���ʒ��p���W)
    int bbMinX{ 0 }, bbMinY{ 0 };
    int bbMaxX{ 0 }, bbMaxY{ 0 };

    ~bldgList()
    {
        buildingList.clear();
        buildingListLOD1.clear();
    }

} BLDGLIST;


// �y�n�ʏڍ�
typedef struct landSurfaceMembers
{
    std::vector<CPointBase> posList;        // ���W���X�g(�֊s)
    //std::vector<CTriangle> triangleList;    // �O�p�`���X�g

    ~landSurfaceMembers()
    {
        posList.clear();
    }
} LANDSURFACEMEMBERS;

// �y�n��
typedef struct landSurfaces
{
    std::string surfaceID;
    std::vector<LANDSURFACEMEMBERS> landSurfaceList;    // �y�n�ʂ̏ڍ׃��X�g(���b�V��)
    std::vector<MESHPOSITION_XY> meshPosList;           // ���b�V�����W���X�g
    int meshSize{ 5 };                                  // 5m or 10m
    int area;                                           // �y�n�ʐ�(���o�����y�n�ʂ̃s�N�Z����)

    ~landSurfaces()
    {
        landSurfaceList.clear();
        meshPosList.clear();
    }
} LANDSURFACES;

// 3�����b�V��ID�ƌ������X�g�̃}�b�v
typedef std::map<std::string, std::vector<BUILDINGS*>> BuildingsMap;

// �G���A
typedef struct areaData
{
    std::string areaID;                         // �G���AID
    std::string areaName;                       // �G���A����
    std::vector<CPoint2D> pos2dList;            // �G���A�\���_���X�g
    int direction;                              // �G���A���̕␳����
    double degree;                              // �G���A���̕␳�X�Ίp
    std::string areaExplanation;                // �G���A����
    bool isWater;                               // �����t���O

    // �G���A���̊e�f�[�^
    std::vector<BLDGLIST*> neighborBldgList;    // ��̓G���A���ӂ�3�����b�V���������X�g
    std::vector<DEMMEMBERS*> neighborDemList;   // ��̓G���A���ӂ�3�����b�V��DEM���X�g
    BuildingsMap targetBuildings;               // ��͑Ώۂ̌������X�g(key:3�����b�V��ID)
    LANDSURFACES landSurface;                   // �y�n�f�[�^
    std::vector<CPointBase> pointMemData;       // ���o�����y�n�ʂ̃|�C���g�f�[�^
    double bbMinX{ DBL_MAX }, bbMinY{ DBL_MAX };
    double bbMaxX{ -DBL_MAX }, bbMaxY{ -DBL_MAX };

    bool analyzeBuild{ false };                 // �͈͓������̉�͑Ώۃt���O
    bool analyzeLand{ false };                  // �͈͓��y�n�̉�͑Ώۃt���O

    std::vector<std::string> gmlFileList;       // �͈͓���CityGML�t�@�C�����X�g

    std::vector<CPoint2DPolygon> areaPolygons;  // �G���A�̓ʃ|���S�����X�g

    ~areaData()
    {
        neighborBldgList.clear();
        neighborDemList.clear();
        targetBuildings.clear();
        pointMemData.clear();
    }

    bool operator == (const areaData& a) const {
        return areaID == a.areaID;
    }

} AREADATA;


// ���H
typedef struct tranSurfaces
{
    //std::string tranSurfaceId;                      // ���H��ID
    std::vector<SURFACEMEMBERS> tranSurfaceList;    // ���H�ڍ׃��X�g

    ~tranSurfaces()
    {
        tranSurfaceList.clear();
    }
} TRANSURFACES;

// ���HLOD1
typedef struct transLOD1
{
    //std::string road;                           // ���HID
    std::vector<TRANSURFACES> tranSurfaceList;  // ���H���X�g

    ~transLOD1()
    {
        tranSurfaceList.clear();
    }
} TRANSLOD1;

typedef struct transInfo
{
    std::vector<TRANSLOD1> tranListLOD1;        // ���H���X�g(LOD1)
    int bbMinX{ 0 }, bbMinY{ 0 };
    int bbMaxX{ 0 }, bbMaxY{ 0 };

    ~transInfo()
    {
        tranListLOD1.clear();
    }
} TRANSINFO;

// ���H���X�g
typedef struct tranList
{
    std::string meshID;		                    // 3�����b�V��ID
    std::vector<TRANSLOD1> tranListLOD1;        // ���H���X�g(LOD1)
    // ���b�V�����W��MIN,MAX(���ʒ��p���W)
    int bbMinX{ 0 }, bbMinY{ 0 };
    int bbMaxX{ 0 }, bbMaxY{ 0 };

    ~tranList()
    {
        tranListLOD1.clear();
    }
} TRANLIST;


// LOD2�o�͍���
typedef struct lod2outList
{
    std::string areaID;		                    // �G���AID
    std::string meshID;		                    // ���b�V��ID
    std::string building;		                // ����ID
    std::string solarInsolation;                // �N�ԓ��˗�
    std::string solarPowerGeneration;           // �N�Ԕ��d��
    std::string lightPollutionTimeSpring;		// ���Q�������Ԑ�_�t��
    std::string lightPollutionTimeSummer;		// ���Q�������Ԑ�_�Ď�
    std::string lightPollutionTimeWinter;		// ���Q�������Ԑ�_�~��
    std::string lightPollutionTimeOneDay;		// ���Q�������Ԑ�_�w���(or �w�茎�̑�\��)


    bool operator<(const lod2outList& right) const {
        return meshID == right.meshID ? building < right.building : meshID < right.meshID;
    }

} LOD2OUTLIST;

// �iCSV�ǂݎ��p�j�N�ԗ\�����˗ʁA���d��
enum struct yearPrediction
{
    areaID = 0,                 // �G���AID
    meshID = 1,                 // ���b�V��ID
    building = 2,               // ����ID
    solarInsolation = 4,        // ���˗�
    solarPowerGeneration = 5,   // ���d��
};

// �iCSV�ǂݎ��p�j���Q�������ԑ���
enum struct lightPollution
{
    areaID = 0,                 // �G���AID
    meshID = 1,                 // ���b�V��ID
    building = 2,               // ����ID
    summer = 3,                 // �Ď�
    spling = 4,                 // �t��
    winter = 5,                 // �~��
    oneday = 6,                 // �w���(or �w�茎�̑�\��)
};

// ��͔͈̓��X�g���擾
std::vector<AREADATA> allAreaList{};
void* __cdecl GetAllAreaList()
{
    return (void*)(&allAreaList);
}

// ���H���X�g���擾
std::vector<TRANLIST> allTranList{};
void* __cdecl GetAllTranList()
{
    return (void*)(&allTranList);
}

// �G���R�[�_�[�̎擾
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

// src�Ŏw�肷��jpeg�t�@�C���̋�`�̈�(����sx,sy �� width ���� height)�Ńg���~���O��dtc�Ŏw�肷��jpeg�t�@�C���ɕۑ�
int imgcut(TCHAR* dtc, TCHAR* src, int sx, int sy, int wdith, int height);

class AnalyzeData
{
public:
	AnalyzeData(void);
	~AnalyzeData(void);

};
