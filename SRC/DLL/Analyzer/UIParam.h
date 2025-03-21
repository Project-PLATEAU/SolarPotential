#pragma once
#include "../../LIB/CommonUtil/StringEx.h"
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "../../LIB/CommonUtil/ReadINIParam.h"

// 方角
enum class eDirections
{
	NONE = -1,
	NORTH = 0,		// 北向き
	EAST = 1,		// 東向き
	SOUTH = 2,		// 南向き
	WEST = 3		// 西向き
};

enum class eDateType
{
	OneMonth = 0,	// 指定月
	OneDay,			// 指定日
	Summer,			// 夏至
	Winter,			// 冬至
	Year			// 年間
};

// 入力データ
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

// 発電ポテンシャル推計条件(屋根)
class CSolarPotential_Roof
{
public:
	// 除外条件
	double dArea2D;				// 除外する面積
	eDirections eDirection;		// 除外する方位
	double dDirectionDegree;	// 除外する方位＋傾き
	double dSlopeDegree;		// 除外する屋根面の傾き
	bool bExclusionInterior;	// interior面を除外するか(除外:true)

	// 補正
	double dCorrectionCaseDeg;				// 補正対象となる傾き(基準)
	eDirections eCorrectionDirection;		// 補正する方位
	double dCorrectionDirectionDegree;		// 補正する方位＋傾き

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

// 発電ポテンシャル推計条件(土地)
class CSolarPotential_Land
{
public:
	// 除外条件
	double dArea2D;			// 除外する面積
	double dSlopeAngle;		// 除外する土地面の傾き

	// 補正
	eDirections eCorrectionDirection;		// 補正する方位
	double dCorrectionDirectionDegree;		// 補正する方位＋傾き

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

// 反射シミュレーション条件
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
	CSolarPotential_Roof*	pRoof;			// 屋根面条件
	CSolarPotential_Land*	pLand;			// 土地面条件
	double		dPanelMakerSolarPower;		// 発電容量
	double		dPanelRatio;				// パネル設置割合

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
	bool					bExecSolarPotantial;		// 発電ポテンシャル推計実行フラグ
	bool					bExecReflection;			// 反射シミュレーション実行フラグ
	bool					bExecBuild;					// 建物シミュレーション実行フラグ
	bool					bExecLand;					// 土地シミュレーション実行フラグ

	eDateType				eAnalyzeDate;				// 解析期間
	int						nMonth;						// 指定月
	int						nDay;						// 指定日

	CInputData*				pInputData;					// 入力データ

	CSolarPotentialParam*	pSolarPotentialParam;		// 発電ポテンシャル推計条件
	CReflectionParam*		pReflectionParam;			// 反射シミュレーション条件

	std::wstring			strOutputDirPath;			// 解析結果出力フォルダ(data)

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