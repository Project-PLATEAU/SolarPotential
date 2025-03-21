#pragma once
#include <string>
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "../../LIB/CommonUtil/ReadINIParam.h"

// ���z�\���̎��
enum class eBuildingStructure
{
	WOOD						= 0,	// �ؑ��E�y����
	STEEL_REINFORCED_CONCRETE,			// �S���S�؃R���N���[�g��
	REINFORCED_CONCRETE,				// �S�؃R���N���[�g��
	STEEL,								// �S����
	LIGHT_GAUGE_STEEL,					// �y�ʓS����
	MASONRY_CONSTRUCTION,				// �����K���E�R���N���[�g�u���b�N���E�Α�
	NON_WOOD,							// ��ؑ�
	UNKNOWN,							// �s��
};

// ����
enum class eDirections
{
	UNKNOWN = 0,			// �s��
	NORTH = 1,				// �k		0��
	NORTH_NORTHEAST,		// �k�k��	22.5��
	NORTHEAST,				// �k��		45��
	EAST_NORTHEAST,			// ���k��	67.5��
	EAST,					// ��		90��
	EAST_SOUTHEAST,			// ���쓌	112.5��
	SOUTHEAST,				// �쓌		135��
	SOUTH_SOUTHEAST,		// ��쓌	157.5��
	SOUTH,					// ��		180��
	SOUTH_SOUTHWEST,		// ��쐼	202.5��
	SOUTHWEST,				// �쐼		225��
	WEST_SOUTHWEST,			// ���쐼	247.5��
	WEST,					// ��		270��
	WEST_NORTHWEST,			// ���k��	292.5��
	NORTHWEST,				// �k��		315��
	NORTH_NORTHWEST,		// �k�k��	337.5��
};

// ���W�n
enum class eDatum
{
	UNKNOWN = 0,			// �s��
	LATLON = 1,				// �ܓx�o�x
	XY,						// ���ʒ��p���W

};

// �W�v�͈�
class CAggregationRange
{
public:
	double dMaxLat;		// �ő�ܓx
	double dMinLon;		// �ŏ��o�x
	double dMaxLon;		// �ő�o�x
	double dMinLat;		// �ŏ��ܓx
	double dMaxY;		// ���ʒ��p���W�ő�Y���W
	double dMinX;		// ���ʒ��p���W�ŏ�X���W
	double dMaxX;		// ���ʒ��p���W�ő�X���W
	double dMinY;		// ���ʒ��p���W�ŏ�Y���W
	CAggregationRange(const double& d1, const double& d2, const double& d3, const double& d4)
		: dMaxLat(-DBL_MAX)
		, dMinLon(DBL_MAX)
		, dMaxLon(-DBL_MAX)
		, dMinLat(DBL_MAX)
		, dMaxY(-DBL_MAX)
		, dMinX(DBL_MAX)
		, dMaxX(-DBL_MAX)
		, dMinY(DBL_MAX)
	{
		dMaxLat = d1; dMinLon = d2; dMaxLon = d3; dMinLat = d4;
		// �ܓx�o�x�@���@���ʒ��p���W�n�ɕϊ�
		int iJPZone = GetINIParam()->GetJPZone(); // ���W�n
		double dEast, dNorth;
		CGeoUtil::LonLatToXY(dMinLon, dMinLat, iJPZone, dEast, dNorth);
		dMinX = dEast; dMinY = dNorth;
		CGeoUtil::LonLatToXY(dMaxLon, dMaxLat, iJPZone, dEast, dNorth);
		dMaxX = dEast; dMaxY = dNorth;
	}

	CAggregationRange& operator = (const CAggregationRange& v) {
		if (&v != this) { dMaxLat = v.dMaxLat; dMinLon = v.dMinLon; dMaxLon = v.dMaxLon; dMinLat = v.dMinLat;
		}
		return *this;
	}
};

// ���z���p�l���̐ݒu�Ɋւ��ėD��x���Ⴂ�{�݂̔���p�����[�^
class CBuildingParam
{
public:
	// ���˗ʂ����Ȃ��{�݂����O
	double dSolorRadiation;				// kWh/m2 ����
	double dLowerPercent;				// ���ʁ�
	// �����\���ɂ�鏜�O
	int    iBuildingStructure;			// ���z�\���̎��
	// ����̊K�w�̎{�݂����O
	int iFloorsBelow;					// �Z�K�ȉ�
	int iUpperFloors;					// �Z�K�ȏ�

	CBuildingParam(const double& d1, const double& d2, const int& d3, const int& d4, const int& d5)
		: dSolorRadiation(0.0)
		, dLowerPercent(0.0)
		, iBuildingStructure(0)
		, iFloorsBelow(-1)
		, iUpperFloors(-1)
	{
		dSolorRadiation = d1; dLowerPercent = d2; iBuildingStructure = d3; iFloorsBelow = d4; iUpperFloors = d5;
	}

	CBuildingParam& operator = (const CBuildingParam& v) {
		if (&v != this) {
			dSolorRadiation = v.dSolorRadiation; dLowerPercent = v.dLowerPercent;
			iBuildingStructure = v.iBuildingStructure;
			iFloorsBelow = v.iUpperFloors; iFloorsBelow = v.iFloorsBelow;
		}
		return *this;
	}
};

// �ЊQ���ɑ��z���p�l�����j���A��������댯���̔���
class CHazardParam
{
public:
	bool bBelowTsunamiHeight;			// �������z�肳���ő�Ôg����������錚�������O
	bool bBelowFloodDepth;				// �����������z�肳���͐�Z���z��Z���[������錚�������O
	bool bLandslideWarningArea;			// �y���ЊQ�x�������ɑ��݂��錚�������O
	std::string strWeatherDataPath;		// �C�ۃf�[�^(�ϐ�)�t�H���_�p�X
	double dSnowDepth;					// �C�ۃf�[�^(�ϐ�)_cm�ȏ�
	double dS;							// �ϐ�׏d(kgf / m3)
	double dp;							// �P�ʉ׏d(N / m3)

	CHazardParam(const bool& d1, const bool& d2, const bool& d3, const std::string& d4, const double& d5, const double& d6, const double& d7)
		: bBelowTsunamiHeight(false)
		, bBelowFloodDepth(false)
		, bLandslideWarningArea(false)
		, strWeatherDataPath("")
		, dSnowDepth(-1.0)
		, dS(0.0)
		, dp(0.0)
	{
		bBelowTsunamiHeight = d1; bBelowFloodDepth = d2; bLandslideWarningArea = d3;
		strWeatherDataPath = d4; dSnowDepth = d5; dS = d6; dp = d7;
	}

	CHazardParam& operator = (const CHazardParam& v) {
		if (&v != this) {
			bBelowTsunamiHeight = v.bBelowTsunamiHeight; bBelowFloodDepth = v.bBelowFloodDepth; bLandslideWarningArea = v.bLandslideWarningArea;
			strWeatherDataPath = v.strWeatherDataPath; dSnowDepth = v.dSnowDepth; dS = v.dS; dp = v.dp;
		}
		return *this;
	}
};


// ���z���p�l���̐ݒu�ɐ���������{�݂̔���p�����[�^
class CRestrictParam
{
public:
	// ������݂���͈͂̃V�F�[�v�t�@�C��_1
	std::string strRestrictAreaPath_1;		// �t�H���_�p�X�P
	double		dHeight_1;					// �����P
	eDirections eDirections_1;				// ���ʂP
	eDatum		eDatum_1;					// ���W�n�P
	// ������݂���͈͂̃V�F�[�v�t�@�C��_2
	std::string strRestrictAreaPath_2;		// �t�H���_�p�X�Q
	double		dHeight_2;					// �����Q
	eDirections eDirections_2;				// ���ʂQ
	eDatum		eDatum_2;					// ���W�n�Q
	// ������݂���͈͂̃V�F�[�v�t�@�C��_3
	std::string strRestrictAreaPath_3;		// �t�H���_�p�X�R
	double		dHeight_3;					// �����R
	eDirections eDirections_3;				// ���ʂR
	eDatum		eDatum_3;					// ���W�n�R

	CRestrictParam(const std::string& d1, const double& d2, const eDirections& d3, const eDatum& e1, const std::string& d4, const double& d5, const eDirections& d6, const eDatum& e2, const std::string& d7, const double& d8, const eDirections& d9, const eDatum& e3)
		: strRestrictAreaPath_1("")
		, dHeight_1(0.0)
		, eDirections_1(eDirections::UNKNOWN)
		, eDatum_1(eDatum::UNKNOWN)
		, strRestrictAreaPath_2("")
		, dHeight_2(0.0)
		, eDirections_2(eDirections::UNKNOWN)
		, eDatum_2(eDatum::UNKNOWN)
		, strRestrictAreaPath_3("")
		, dHeight_3(0.0)
		, eDirections_3(eDirections::UNKNOWN)
		, eDatum_3(eDatum::UNKNOWN)
	{
		strRestrictAreaPath_1 = d1; dHeight_1 = d2; eDirections_1 = d3; eDatum_1 = e1;
		strRestrictAreaPath_2 = d4; dHeight_2 = d5; eDirections_2 = d6; eDatum_2 = e2;
		strRestrictAreaPath_3 = d7; dHeight_3 = d8; eDirections_3 = d9; eDatum_3 = e3;
	}

	CRestrictParam& operator = (const CRestrictParam& v) {
		if (&v != this) {
			strRestrictAreaPath_1 = v.strRestrictAreaPath_1; dHeight_1 = v.dHeight_1; eDirections_1 = v.eDirections_1; eDatum_1 = v.eDatum_1;
			strRestrictAreaPath_2 = v.strRestrictAreaPath_2; dHeight_2 = v.dHeight_2; eDirections_2 = v.eDirections_2; eDatum_2 = v.eDatum_2;
			strRestrictAreaPath_3 = v.strRestrictAreaPath_3; dHeight_3 = v.dHeight_3; eDirections_3 = v.eDirections_3; eDatum_3 = v.eDatum_3;
		}
		return *this;
	}
};

class CUIParam
{
public:
	CUIParam()
		: m_pAggregationRange(NULL)
		, m_pBuildingParam(NULL)
		, m_pHazardParam(NULL)
		, m_pRestrictParam(NULL)
		, m_bExecBuild(false), m_bExecLand(false)
		, m_strOutputDirPath(L"")
		, m_strAnalyzeResultPath(L"")
	{
	
	};
	~CUIParam()
	{
		delete m_pAggregationRange;
		delete m_pBuildingParam;
		delete m_pHazardParam;
		delete m_pRestrictParam;
	};

	CAggregationRange*	m_pAggregationRange;
	CBuildingParam*		m_pBuildingParam;
	CHazardParam*		m_pHazardParam;
	CRestrictParam*		m_pRestrictParam;

	bool				m_bExecBuild;
	bool				m_bExecLand;

	std::wstring		m_strOutputDirPath;
	std::wstring		m_strAnalyzeResultPath;
}; 