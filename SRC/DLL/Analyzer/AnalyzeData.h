#pragma once

#include <string>
#include <vector>
#include <map>
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "../../LIB/CommonUtil/CPoint2DPolygon.h"

using namespace std;

#define CANCELFILE "cancel.txt"

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

} DEMMEMBERS;

// DEM
typedef struct demList
{
    std::string lv2meshID;                  // 2�����b�V��ID
    std::vector<DEMMEMBERS> demMemberList;  // DEM�ڍ׃��X�g

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

} INTERIOR;

// �ʏڍ�
typedef struct surfaceMembers
{
    std::string polygon;                                    // �|���S��ID
    std::string linearRing;                                 // ���C��ID
    std::vector<CPointBase> posList;                        // ���W���X�g(�O��)
    std::vector<INTERIOR> interiorList;                     // �����|���S�����X�g

} SURFACEMEMBERS;

// ����
typedef struct roofSurfaces
{
    std::string roofSurfaceId;		            // ����ID
    std::vector<SURFACEMEMBERS> roofSurfaceList; // �����ڍ׃��X�g
    std::vector<MESHPOSITION_XY> meshPosList;       // 1m���b�V�����W���X�g

} ROOFSURFACES;

// ��
typedef struct wallSurfaces
{
    std::string wallSurfaceId;		            // ��ID
    std::vector<SURFACEMEMBERS> wallSurfaceList; // �Ǐڍ׃��X�g

} WALLSURFACES;

// ����LOD2
typedef struct buildings
{
    std::string building;		                // ����ID
    std::vector<ROOFSURFACES> roofSurfaceList;  // �������X�g
    std::vector<WALLSURFACES> wallSurfaceList;  // �ǃ��X�g
    //bool analyze{ false };

} BUILDINGS;

// ����LOD1(LOD1�����Ȃ�����)
typedef struct buildingsLOD1
{
    std::string building;
    std::vector<WALLSURFACES> wallSurfaceList;   // �ǃ��X�g�i�S�Ă̖ʁj

}BUILDINGSLOD1;

// �����A���b�V�����W
typedef struct buildingsInfo
{
    std::vector<BUILDINGS> buildingList;        // �������X�g
    std::vector<BUILDINGSLOD1> buildingListLOD1; // �������X�g(LOD1)
    int bbMinX{ 0 }, bbMinY{ 0 };
    int bbMaxX{ 0 }, bbMaxY{ 0 };
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
} BLDGLIST;


// �y�n�ʏڍ�
typedef struct landSurfaceMembers
{
    std::vector<CPointBase> posList;        // ���W���X�g(���b�V��)
    //std::vector<CTriangle> triangleList;    // �O�p�`���X�g

} LANDSURFACEMEMBERS;

// �y�n��
typedef struct landSurfaces
{
    std::string surfaceID;
    std::vector<LANDSURFACEMEMBERS> landSurfaceList;    // �y�n��(���b�V��)�̏ڍ׃��X�g
    std::vector<MESHPOSITION_XY> meshPosList;           // ���b�V�����W���X�g
    int meshSize{ 5 };                                  // 5m
    int area;                                           // �y�n�ʐ�(���o�����y�n�ʂ̃s�N�Z����)

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

} AREADATA;

// ���H
typedef struct tranSurfaces
{
    //std::string tranSurfaceId;                      // ���H��ID
    std::vector<SURFACEMEMBERS> tranSurfaceList;    // ���H�ڍ׃��X�g

} TRANSURFACES;

// ���HLOD1
typedef struct transLOD1
{
    //std::string road;                           // ���HID
    std::vector<TRANSURFACES> tranSurfaceList;  // ���H���X�g

} TRANSLOD1;

typedef struct transInfo
{
    std::vector<TRANSLOD1> tranListLOD1;        // ���H���X�g(LOD1)
    int bbMinX{ 0 }, bbMinY{ 0 };
    int bbMaxX{ 0 }, bbMaxY{ 0 };
} TRANSINFO;

// ���H���X�g
typedef struct tranList
{
    std::string meshID;		                    // 3�����b�V��ID
    std::vector<TRANSLOD1> tranListLOD1;        // ���H���X�g(LOD1)
    // ���b�V�����W��MIN,MAX(���ʒ��p���W)
    int bbMinX{ 0 }, bbMinY{ 0 };
    int bbMaxX{ 0 }, bbMaxY{ 0 };
} TRANLIST;


// LOD2�o�͍���
typedef struct lod2outList
{
    std::string meshID;		                    // ���b�V��ID
    std::string building;		                // ����ID
    std::string solarInsolation;                // �N�ԓ��˗�
    std::string solarPowerGeneration;           // �N�Ԕ��d��
    std::string lightPollutionTimeSpring;		// ���Q�������Ԑ�_�t��
    std::string lightPollutionTimeSummer;		// ���Q�������Ԑ�_�Ď�
    std::string lightPollutionTimeWinter;		// ���Q�������Ԑ�_�~��

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
    meshID = 0,                 // ���b�V��ID
    building = 1,               // ����ID
    summer = 2,                 // �Ď�
    spling = 3,                 // �t��
    winter = 4,                 // �~��
};

// vector<AREADATA>���擾����
extern "C" __declspec(dllimport) void* __cdecl GetAllAreaList();

