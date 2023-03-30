#pragma once

#include <cstdio>

//=============================================================================
// class CGeoidData
//=============================================================================
class CGeoidData
{
public:
	double Value;
	short Flag;

	CGeoidData() { Value = 0.0; Flag = 0; }
};

//=============================================================================
// class CGeoid
//=============================================================================
class CGeoid
{
	int GeoidType;
	wchar_t GeoidPath[1024];

	double OriginB;
	double OriginL;

	int Width;    // 読み込んだメッシュ横数
	int Height;   // 読み込んだメッシュ縦数

	CGeoidData** Data;

	double MinB;  // 南端の緯度[deg]
	double MinL;  // 西端の経度[deg]
	double MaxB;  // 北端の緯度[deg]
	double MaxL;  // 東端の経度[deg]
	double DB;    // 緯度幅[deg]
	double DL;    // 経度幅[deg]

	double Exception;

	bool CrossDateLine;  // EGMのとき、日付変更線(E180 = W180)をまたいで読み込んでいるかどうか

private:
	int LoadGeoid2000();
	int LoadEGM96();

	int ExtractByBilinear
	(
		const double& inB,
		const double& inL,
		double* outHg
	);

	int ExtractForEGM96
	(
		const double& inB,
		const double& inL,
		double* outHg
	);

	int CalcValueByWeightedAveMethod
	(
		CGeoidData* inZ11,
		CGeoidData* inZ12,
		CGeoidData* inZ21,
		CGeoidData* inZ22,
		double& inPx,
		double& inPy,
		double* outHg
	);

public:
	static const int GEOID_GEOID2000 = 1;
	static const int GEOID_EGM96 = 2;

	static const int NOERROR_ = 0;
	static const int TRANS_ERROR = 1;
	static const int FILE_NONEXIST = 10;
	static const int FILE_UNOPEN = 11;
	static const int FILE_ERROR = 12;

	CGeoid(int inGeoidType, wchar_t* inGeoidPath, double inOrgB, double inOrgL);
	virtual ~CGeoid();

	int Load();

	int Extract
	(
		const double& inB,
		const double& inL,
		double* outHg
	);

	static void nSpline(double* x, double* y, int N, double* a, double* b, double* c);//スプライン関数係数算出
	static double calSpline(double xp, double* x, double* y, double* a, double* b, double* c, int N);//スプライン補間処理
	static void f_tri(double* a, double* b, double* c, double* x, double* y, int N);//スプライン関数係数算出のための一次方程式解法処理

	static void nSpline4(double* x, double* y, double* a, double* b, double* c);//スプライン関数係数算出
	static double calSpline4(double xp, double* x, double* y, double* a, double* b, double* c);//スプライン補間処理
	static void f_tri2(double* a, double* b, double* c, double* x, double* y);//スプライン関数係数算出のための一次方程式解法処理
};
