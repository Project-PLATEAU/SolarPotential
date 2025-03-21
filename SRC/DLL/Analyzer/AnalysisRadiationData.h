#pragma once
#include <map>
#include "..\..\LIB\CommonUtil\CSunVector.h"

// ���d�|�e���V�������v
// ���˗ʋ���
class CAnalysisRadiationCommon
{
public:
	// ���V���̓��Ɨ��i�s�s���Ɓj
	double sunnyRate[12]; // �����Ƃ̓��Ǝ���/�Ǝ���
	// �ܓV���̓��Ɨ��i�s�s���Ɓj
	double cloudRate[12]; // ������ 1 - ���V���̓��Ɨ�


};

// 1���b�V�����Ƃ̔��d�|�e���V�������v�f�[�^
class CMeshData
{
public:
	std::string meshId;		// ID

	std::vector<CVector3D> meshPos;		// ���b�V�����
	CVector3D center;		// ���b�V�����S
	CVector3D centerMod;	// ���b�V�����S(�p�x�␳��)

	// ���ˌv�Z����
	double solarRadiationSunny[12];		// ���˗�(WH/m2) �����Ɓ@���V
	double solarRadiationCloud[12];		// ���˗�(WH/m2) �����Ɓ@�ܓV
	double solarRadiation[12];			// ���Ɨ��ɂ��␳�������˗�(WH/m2)�@������

	double solarRadiationUnit;			// �\�����˗�(kWh/m2)
	double solarPowerUnit;				// �\�����d��(kWh/m2)

	double area;						// ���b�V���̖ʐ�(m2)

	CMeshData()
	{
		meshId = "";
		for (int n = 0; n < 12; n++) solarRadiationSunny[n] = 0;
		for (int n = 0; n < 12; n++) solarRadiationCloud[n] = 0;
		for (int n = 0; n < 12; n++) solarRadiation[n] = 0;
		solarRadiationUnit = 0.0;
		area = 0.0;

	};

};

// �ʂ��Ƃ̔��d�|�e���V�������v�f�[�^
class CSurfaceData
{
public:
	std::vector<CMeshData> vecMeshData;		// ���b�V�����Ƃ̃f�[�^
	double meshSize;						// ���b�V���T�C�Y(m)

	std::vector<CVector3D> bbPos;			// �Ώۖʂ�BB
	CVector3D center;						// BB���S

	double slopeDegreeAve;					// �X�Ίp(���ϒl)(�x)
	double azDegreeAve;						// ���ʊp(���ϒl)(�x)
	double slopeModDegree;					// �␳�����X�Ίp(�x)
	double azModDegree;						// �␳�������ʊp(�x)

	double solarRadiation;					// �\�����˗�(kWh)
	double solarRadiationUnit;				// 1m2������̗\�����˗�(kWh/m2)

	double area;							// �Ώۖʂ̖ʐ�(m2)


	// �ʐς��擾����
	double GetArea() { return area; };

	CSurfaceData()
	{
		slopeDegreeAve = 0.0; azDegreeAve = 0.0;
		slopeModDegree = 0.0; azModDegree = 0.0;
		solarRadiation = 0.0; solarRadiationUnit = 0.0;
		area = 0.0;
		meshSize = 0.0;
	};

};

typedef std::map<std::string, CSurfaceData> CSurfaceDataMap;	// ��ID, �f�[�^�}�b�v

// 
class CPotentialData
{
public:

	CSurfaceDataMap	mapSurface;			// �Ώۖʂ��Ƃ̃f�[�^�}�b�v

	std::vector<CVector3D> bbPos;		// �ΏۖʑS�̂�BB
	CVector3D center;					// BB���S

	// ��͌���(CityGML�ɑ����t�^)
	double solarRadiationTotal;			// �\�����˗� ���v
	double solarRadiationUnit;			// �\�����˗� 1m2������̓��˗�(���ϒl)
	double solarPower;					// �\�����d��
	double solarPowerUnit;				// �\�����d�� 1m2������̔��d��(���ϒl)

	double panelArea;					// �p�l���ʐ�(�ݒu������K�p�����ʐ�)

	// ���ʐ�
	double GetAllArea()
	{
		double area = 0.0;
		for (auto val : mapSurface)
		{
			area += val.second.GetArea();
		}
		return area;
	};

	CPotentialData()
	{
		solarRadiationTotal = 0.0; solarRadiationUnit = 0.0; solarPower = 0.0;
		solarPowerUnit = 0.0; panelArea = 0.0;
	};
};

typedef std::map<std::string, CPotentialData> CPotentialDataMap;	// ����ID, �f�[�^
typedef std::map<std::string, CPotentialDataMap> CBuildListDataMap;	// 3�����b�V��ID, �f�[�^

class CResultData
{
public:

	CBuildListDataMap*	pBuildMap;			// �������Ƃ̃f�[�^�}�b�v
	CPotentialData*		pLandData;			// �y�n���Ƃ̃f�[�^

	CResultData()
	: pBuildMap(NULL), pLandData(NULL)
	{}
};

