#pragma once
#include "../../LIB/CommonUtil/StringEx.h"
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "../../LIB/CommonUtil/ReadINIParam.h"

// ���p
enum class eDirections
{
	NONE = -1,
	NORTH = 0,		// �k����
	EAST = 1,		// ������
	SOUTH = 2,		// �����
	WEST = 3		// ������
};

enum class eDateType
{
	OneMonth = 0,	// �w�茎
	OneDay,			// �w���
	Summer,			// �Ď�
	Winter,			// �~��
	Year			// �N��
};

// ���̓f�[�^
class CInputData
{
public:
	std::string	strKashoData;
	std::string	strNisshoData;
	std::string	strSnowDepthData;
	std::string	strLandData;
	bool		bUseDemData;

	CInputData(const std::string& s1, const std::string& s2, const std::string& s3, const std::string& s4, const bool& b)
		: strKashoData(""), strNisshoData(""), strSnowDepthData(""), strLandData("")
	{
		strKashoData = s1; strNisshoData = s2; strSnowDepthData= s3; strLandData = s4;
		bUseDemData = b;
	}

	CInputData& operator = (const CInputData& v) {
		if (&v != this) { strKashoData = v.strKashoData; strNisshoData = v.strNisshoData; strSnowDepthData = v.strSnowDepthData; strLandData = v.strLandData; }
		return *this;
	}
};

// ���d�|�e���V�������v����(����)
class CSolarPotential_Roof
{
public:
	// ���O����
	double dArea2D;				// ���O����ʐ�
	eDirections eDirection;		// ���O�������
	double dDirectionDegree;	// ���O������ʁ{�X��
	double dSlopeDegree;		// ���O���鉮���ʂ̌X��
	bool bExclusionInterior;	// interior�ʂ����O���邩(���O:true)

	// �␳
	double dCorrectionCaseDeg;				// �␳�ΏۂƂȂ�X��(�)
	eDirections eCorrectionDirection;		// �␳�������
	double dCorrectionDirectionDegree;		// �␳������ʁ{�X��

	CSolarPotential_Roof(const double& d1, const eDirections& e1, const double& d2, const double& d3, const double& d4, const eDirections& e2, const double& d5, bool b1)
		: dArea2D(0.0)
		, eDirection(eDirections::NONE), dDirectionDegree(0.0)
		, dSlopeDegree(0.0)
		, dCorrectionCaseDeg(0.0)
		, eCorrectionDirection(eDirections::NONE), dCorrectionDirectionDegree(0.0)
		, bExclusionInterior(true)
	{
		dArea2D = d1; eDirection = e1; dDirectionDegree = d2; dSlopeDegree = d3;
		dCorrectionCaseDeg = d4; eCorrectionDirection = e2; dCorrectionDirectionDegree = d5;
		bExclusionInterior = b1;
	}

	CSolarPotential_Roof& operator = (const CSolarPotential_Roof& v) {
		if (&v != this) 
		{
			dArea2D = v.dArea2D;
			eDirection = v.eDirection; dDirectionDegree = v.dDirectionDegree;
			dSlopeDegree = v.dSlopeDegree;
			dCorrectionCaseDeg = v.dCorrectionCaseDeg;
			eCorrectionDirection = v.eCorrectionDirection; dCorrectionDirectionDegree = v.dCorrectionDirectionDegree;
			bExclusionInterior = v.bExclusionInterior;
		}
		return *this;
	}
};

// ���d�|�e���V�������v����(�y�n)
class CSolarPotential_Land
{
public:
	// ���O����
	double dArea2D;			// ���O����ʐ�
	double dSlopeAngle;		// ���O����y�n�ʂ̌X��

	// �␳
	eDirections eCorrectionDirection;		// �␳�������
	double dCorrectionDirectionDegree;		// �␳������ʁ{�X��

	CSolarPotential_Land(const double& d1, const double& d2, const eDirections& e, const double& d3)
		: dArea2D(0.0), dSlopeAngle(0.0)
		, eCorrectionDirection(eDirections::NONE), dCorrectionDirectionDegree(0.0)
	{
		dArea2D = d1; dSlopeAngle = d2;
		eCorrectionDirection = e; dCorrectionDirectionDegree = d3;
	}

	CSolarPotential_Land& operator = (const CSolarPotential_Land& v) {
		if (&v != this) { dArea2D = v.dArea2D; dSlopeAngle = v.dSlopeAngle; eCorrectionDirection = v.eCorrectionDirection; dCorrectionDirectionDegree = v.dCorrectionDirectionDegree;}
		return *this;
	}
};

// ���˃V�~�����[�V��������
class CReflectionCorrect
{
public:
	bool bCustom;
	eDirections eAzimuth;
	double dSlopeAngle;

	CReflectionCorrect() { }
	CReflectionCorrect(const bool& b, const eDirections& e, const double& d)
		: bCustom(false), eAzimuth(eDirections::NONE), dSlopeAngle(0.0)
	{
		bCustom = b; eAzimuth = e; dSlopeAngle = d;
	}

	CReflectionCorrect& operator = (const CReflectionCorrect& v) {
		if (&v != this) { bCustom = v.bCustom; eAzimuth = v.eAzimuth; dSlopeAngle = v.dSlopeAngle; }
		return *this;
	}
};

class CSolarPotentialParam
{
public:
	CSolarPotential_Roof*	pRoof;			// �����ʏ���
	CSolarPotential_Land*	pLand;			// �y�n�ʏ���
	double		dPanelMakerSolarPower;		// ���d�e��
	double		dPanelRatio;				// �p�l���ݒu����

	CSolarPotentialParam(CSolarPotential_Roof* p1, CSolarPotential_Land* p2, const double& d1, const double& d2)
		: pRoof(NULL), pLand(NULL), dPanelMakerSolarPower(0.0), dPanelRatio(0.0)
	{
		pRoof = p1; pLand = p2;
		dPanelMakerSolarPower = d1; dPanelRatio = d2;
	}
	~CSolarPotentialParam()
	{
		delete pRoof;
		delete pLand;
	}

	CSolarPotentialParam& operator = (const CSolarPotentialParam& v) {
		if (&v != this) { pRoof = v.pRoof; pLand = v.pLand; dPanelMakerSolarPower = v.dPanelMakerSolarPower; dPanelRatio = v.dPanelRatio; }
		return *this;
	}
};

class CReflectionParam
{
public:
	CReflectionCorrect* pRoof_Lower;
	CReflectionCorrect* pRoof_Upper;
	CReflectionCorrect* pLand_Lower;
	CReflectionCorrect* pLand_Upper;

	double dReflectionRange;

	CReflectionParam(CReflectionCorrect* p1, CReflectionCorrect* p2, CReflectionCorrect* p3, CReflectionCorrect* p4, const double& d)
		: pRoof_Lower(NULL), pRoof_Upper(NULL)
		, pLand_Lower(NULL), pLand_Upper(NULL)
		, dReflectionRange(0.0)
	{
		pRoof_Lower = p1; pRoof_Upper = p2;
		pLand_Lower = p3; pLand_Upper = p4;
		dReflectionRange = d;
	}
	~CReflectionParam()
	{
		delete pRoof_Lower;
		delete pRoof_Upper;
		delete pLand_Lower;
		delete pLand_Upper;
	}

	CReflectionParam& operator = (const CReflectionParam& v) {
		if (&v != this) { pRoof_Lower = v.pRoof_Lower; pRoof_Upper = v.pRoof_Upper; pLand_Lower = v.pLand_Lower; pLand_Upper = v.pLand_Upper; dReflectionRange = v.dReflectionRange;}
		return *this;
	}
};

class CUIParam
{
public:
	bool					bExecSolarPotantial;		// ���d�|�e���V�������v���s�t���O
	bool					bExecReflection;			// ���˃V�~�����[�V�������s�t���O
	bool					bExecBuild;					// �����V�~�����[�V�������s�t���O
	bool					bExecLand;					// �y�n�V�~�����[�V�������s�t���O

	eDateType				eAnalyzeDate;				// ��͊���
	int						nMonth;						// �w�茎
	int						nDay;						// �w���

	CInputData*				pInputData;					// ���̓f�[�^

	CSolarPotentialParam*	pSolarPotentialParam;		// ���d�|�e���V�������v����
	CReflectionParam*		pReflectionParam;			// ���˃V�~�����[�V��������

	std::wstring			strOutputDirPath;			// ��͌��ʏo�̓t�H���_(data)

	CUIParam()
		: bExecSolarPotantial(false), bExecReflection(false), bExecBuild(false), bExecLand(false)
		, eAnalyzeDate(eDateType::OneMonth), nMonth(0), nDay(0)
		, pInputData(NULL)
		, strOutputDirPath(L"")
	{

	};
	~CUIParam()
	{
		delete pInputData;
		delete pSolarPotentialParam;
		delete pReflectionParam;
	};

};