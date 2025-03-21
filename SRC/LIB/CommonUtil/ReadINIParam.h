#pragma once
#include "CINIFileIO.h"

/*! INI�t�@�C���ǂݍ��݃N���X
	�ǂݎ���p
*/
class CReadINIParam
{
public:
	static CReadINIParam* GetInstance() { return &m_instance; }
	bool Initialize();
	
	// [CoordinateSystem]
	int					GetJPZone();				// �n�ԍ�

	// [File]
	std::wstring		GetAzimuthCSVPath();		// ����ID���Ƃ̕��ʊp�f�[�^CSV�t�@�C���p�X

	// [SolarRadiation]
	double				GetTransmissivity(const int& month);	// ��C���ߗ�(P)
	double				GetReflectivity();						// ���˗�(R)
	double				GetReflectivitySnow();					// ���˗�(R)
	double				GetDemHeight_Build();					// ������͗p�̕W���������l[m]
	double				GetDemHeight_Land();					// �y�n��͗p�̕W���������l[m]
	double				GetDemDist();							// ����ΏۂƂ���DEM�̋���[m]
	double				GetNeighborBuildDist_SolarRad();		// �ߗ׌����̌����͈�[m]

	// [ReflectionSimulator]
	double				GetNeighborBuildDist_Reflection();		// �ߗ׌����̌����͈�[m]


private:
	CReadINIParam(void);
	virtual ~CReadINIParam(void);

	static CReadINIParam	m_instance;				//!< ���N���X�̗B��̃C���X�^���X

	// [CoordinateSystem]
	int				m_iJPZone;				// �n�ԍ�

	// [File]
	std::wstring	m_strAzimuthCSVPath;	// ����ID���Ƃ̕��ʊp�f�[�^CSV�t�@�C���p�X

	// [SolarRadiation]
	double			m_dTransmissivity[12];	// ��C���ߗ�(P)
	double			m_dReflectivity;		// ���˗�(R)
	double			m_dReflectivitySnow;	// �ϐ᎞�̔��˗�(R)
	double			m_dDemHeight_Build;		// ������͗p��DEM�W���������l[m]
	double			m_dDemHeight_Land;		// �y�n��͗p��DEM�W���������l[m]
	double			m_dDemDist;				// ����ΏۂƂ���DEM�̋���[m]
	double			m_dBuildDist_SolarRad;		// �ߗ׌����̌����͈�[m]
	double			m_dBuildDist_Reflection;	// �ߗ׌����̌����͈�[m]

};

#define GetINIParam() (CReadINIParam::GetInstance())

