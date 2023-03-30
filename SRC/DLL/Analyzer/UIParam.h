#pragma once
#include "../../LIB/CommonUtil/StringEx.h"

// 方角
enum class eDirections
{
	NONE = 0,
	NORTH = 1,		// 北向き
	EAST = 2,		// 東向き
	SOUTH = 3,		// 南向き
	WEST = 4		// 西向き
};

// 発電ポテンシャル推計
class CElecPotential
{
public:
	double dArea2D;			// 面積
	eDirections eAzimuth;	// 方位
	double dAzimuthAngle;	// 方位傾き
	double dSlopeAngle;		// 傾き

	CElecPotential(const double& d1, const eDirections& e, const double& d2, const double& d3)
	{
		dArea2D = d1; eAzimuth = e; dAzimuthAngle = d2; dSlopeAngle = d3;
	}
	CElecPotential()
	{
		dArea2D = 0.0; eAzimuth = eDirections::NONE; dAzimuthAngle = 0.0; dSlopeAngle = 0.0;
	}

	CElecPotential& operator = (const CElecPotential& v) {
		if (&v != this) { dArea2D = v.dArea2D; eAzimuth = v.eAzimuth; dAzimuthAngle = v.dAzimuthAngle; dSlopeAngle = v.dSlopeAngle;	}
		return *this;
	}
};

// 屋根面補正
class CRoofSurfaceCorrect
{
public:
	double dLowerAngle;			// 傾き(基準)
	double dTargetAngle;		// 傾き(補正値)

	CRoofSurfaceCorrect(const double& d1, const double& d2)
	{
		dLowerAngle = d1; dTargetAngle = d2;
	}
	CRoofSurfaceCorrect()
	{
		dLowerAngle = 0.0; dTargetAngle = 0.0;
	}

	CRoofSurfaceCorrect& operator = (const CRoofSurfaceCorrect& v) {
		if (&v != this) { dLowerAngle = v.dLowerAngle; dTargetAngle = v.dTargetAngle; }
		return *this;
	}
};

// 反射シミュレーション時の屋根面の向き・傾き補正
class CReflectionRoofCorrect
{
public:
	bool bRoofSurface;
	bool bSpecify;
	eDirections eAzimuth;
	double dSlopeAngle;

	CReflectionRoofCorrect(const bool& b1, const bool& b2, const eDirections& e, const double& d)
	{
		bRoofSurface = b1; bSpecify = b2; eAzimuth = e; dSlopeAngle = d;
	}
	CReflectionRoofCorrect()
	{
		bRoofSurface = false; bSpecify = false; eAzimuth = eDirections::NONE; dSlopeAngle = 0.0;
	}

	CReflectionRoofCorrect& operator = (const CReflectionRoofCorrect& v) {
		if (&v != this) { bRoofSurface = v.bRoofSurface; bSpecify = v.bSpecify; eAzimuth = v.eAzimuth; dSlopeAngle = v.dSlopeAngle; }
		return *this;
	}
};


class UIParam
{
public:
	CElecPotential			_elecPotential;
	CRoofSurfaceCorrect		_roofSurfaceCorrect;
	double					_dAreaSolarPower;			// 太陽光パネル単位面積当たりの発電容量
	CReflectionRoofCorrect	_reflectRoofCorrect_Lower;	// 3度未満
	CReflectionRoofCorrect	_reflectRoofCorrect_Upper;	// 3度以上

	std::wstring			strOutputDirPath;

	bool					bEnableDEMData;				// DEMデータを使用するか

	bool					bExecSolarPotantial;		// 発電ポテンシャル推計実行フラグ
	bool					bExecReflection;			// 反射シミュレーション実行フラグ

	UIParam()
	{
		_dAreaSolarPower = 0.0;	strOutputDirPath = L"";
		bEnableDEMData = false;
		bExecSolarPotantial = true; bExecReflection = true;
	}

public:
	void SetElecPotential(const double& d1, const eDirections& e, const double& d2, const double& d3)
	{
		_elecPotential = CElecPotential(d1, e, d2, d3);
	};

	void SetRoofSurfaceCorrect(const double& d1, const double& d2)
	{
		_roofSurfaceCorrect = CRoofSurfaceCorrect(d1, d2);
	};

	void SetAreaSolarPower(const double& d){ _dAreaSolarPower = d; };

	void SetReflectionRoofCorrect_Lower(const bool& b1, const bool& b2, const eDirections& e, const double& d3)
	{
		_reflectRoofCorrect_Lower = CReflectionRoofCorrect(b1, b2, e, d3);
	};

	void SetReflectionRoofCorrect_Upper(const bool& b1, const bool& b2, const eDirections& e, const double& d3)
	{
		_reflectRoofCorrect_Upper = CReflectionRoofCorrect(b1, b2, e, d3);
	};

	void SetOutputDirPath(std::string str) { strOutputDirPath = CStringEx::ToWString(str); };

	void SetEnableDEMData(bool b) { bEnableDEMData = b; };

	void SetExecSolarPotantial(bool b) { bExecSolarPotantial = b; };
	void SetExecReflection(bool b) { bExecReflection = b; };

};