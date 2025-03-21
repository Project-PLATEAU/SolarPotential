#include "pch.h"
#include "ReadINIParam.h"
#include "CFileUtil.h"

CReadINIParam	CReadINIParam::m_instance;		// �C���X�^���X

/*! �R���X�g���N�^
*/
CReadINIParam::CReadINIParam(void)
	: m_iJPZone(0)
	, m_strAzimuthCSVPath(L"")
	, m_dReflectivity(0.0)
	, m_dReflectivitySnow(0.0)
	, m_dDemHeight_Build(0.0)
	, m_dDemHeight_Land(0.0)
	, m_dDemDist(0.0)
{
	for (int i = 0; i < 12; i++)
	{
		m_dTransmissivity[i] = 0.0;
	}
}

/*! �f�X�g���N�^
*/
CReadINIParam::~CReadINIParam(void)
{
}

/*!	������
@retval	true	����
@retval	false	���s
@note	Ini�t�@�C���̓ǂݍ��݂��s���܂��B
*/
bool CReadINIParam::Initialize()
{
	bool		bRet = false;
	CINIFileIO	inifile;

	// �t�@�C��OPEN
	std::string strFilePath = GetFUtil()->Combine(GetFUtil()->GetModulePath(), "SolarPotential.ini");
	bRet = inifile.Open(strFilePath);

	// [CoordinateSystem]
	// �n�ԍ� �f�t�H���g�l: 7
	m_iJPZone = inifile.GetInt("CoordinateSystem", "JPZone", 7);

	// [File]
	std::string str = inifile.GetString("File", "AzimuthCSVPath", "");
	m_strAzimuthCSVPath = CStringEx::ToWString(str);
	
	// [SolarRadiation]
	// ��C���ߗ�(P) �f�t�H���g�l: 0.6
	for (int i = 0; i < 12; i++)
	{
		std::string strKey;
		strKey = CStringEx::Format("Transmissivity%d", i+1);
		m_dTransmissivity[i] = inifile.GetDouble("SolarRadiation", strKey, 0.6);
	};

	// ���˗�(R) �f�t�H���g�l: 0.1
	m_dReflectivity = inifile.GetDouble("SolarRadiation", "Reflectivity", 0.1);

	// �ϐ᎞�̔��˗�(R)
	m_dReflectivitySnow = inifile.GetDouble("SolarRadiation", "ReflectivitySnow", 0.7);

	// �W���������l[m]
	m_dDemHeight_Build = inifile.GetDouble("SolarRadiation", "DemHeight_Build", 10.0);

	// �W���������l[m]
	m_dDemHeight_Land = inifile.GetDouble("SolarRadiation", "DemHeight_Land", 1.0);

	// ����ΏۂƂ���DEM�̋���[m]
	m_dDemDist = inifile.GetDouble("SolarRadiation", "DemDist", 100.0);

	// �ߗ׌����̌����͈�[m]
	m_dBuildDist_SolarRad = inifile.GetDouble("SolarRadiation", "NeighborBuildDist", 500.0);

	// [ReflectionSimulator]
	// �ߗ׌����̌����͈�[m]
	m_dBuildDist_Reflection = inifile.GetDouble("ReflectionSimulator", "NeighborBuildDist", 500.0);

	return bRet;
}

#pragma region "���ʒ��p���W�n�ݒ�"

int CReadINIParam::GetJPZone()
{
	return	m_iJPZone;
}

#pragma endregion

#pragma region "���o�̓t�@�C���p�X�ݒ�(���ԃt�@�C����)"

std::wstring CReadINIParam::GetAzimuthCSVPath()
{
	return	m_strAzimuthCSVPath;
}

#pragma endregion

#pragma region "���˗ʐ��v"

// ��C���ߗ�(P)�̎擾
double CReadINIParam::GetTransmissivity(const int& month)
{
	if (month >= 1 && month <= 12)
	{
		return m_dTransmissivity[month-1];

	}
	return 0.0;
}

// ���˗�(R)�̎擾
double CReadINIParam::GetReflectivity()
{
	return	m_dReflectivity;
}

// ���˗�(R)�̎擾
double CReadINIParam::GetReflectivitySnow()
{
	return	m_dReflectivitySnow;
}

// ������͗p��DEM�W���������l[m]
double CReadINIParam::GetDemHeight_Build()
{
	return	m_dDemHeight_Build;
}

// ������͗p��DEM�W���������l[m]
double CReadINIParam::GetDemHeight_Land()
{
	return	m_dDemHeight_Land;
}

// ����ΏۂƂ���DEM�̋���[m]
double CReadINIParam::GetDemDist()
{
	return	m_dDemDist;
}

// �ߗ׌����̌����͈�[m]
double CReadINIParam::GetNeighborBuildDist_SolarRad()
{
	return	m_dBuildDist_SolarRad;

}

#pragma endregion

#pragma region "���˃V�~�����[�V����"


// [ReflectionSimulator]
// �ߗ׌����̌����͈�[m]
double CReadINIParam::GetNeighborBuildDist_Reflection()
{
	return	m_dBuildDist_Reflection;
}

#pragma endregion
