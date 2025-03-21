#pragma once

#include "../../../../LIB/CommonUtil/CGeoUtil.h"
#include "../../LIB/CommonUtil/CPoint2DPolygon.h"

#include <string>
#include <vector>
using namespace std;

#define CANCELFILE "cancel.txt"

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


// vector<BLDGLIST>���擾����
extern "C" __declspec(dllimport) void* __cdecl GetAllList();
// vector<AREADATA>���擾����
extern "C" __declspec(dllimport) void* __cdecl GetAllAreaList();

