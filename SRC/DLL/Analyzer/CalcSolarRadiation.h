#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "CalcSolarPotentialMng.h"

// 建物ごとの日射量を計算する
class CCalcSolarRadiation
{
public:
	CCalcSolarRadiation(CCalcSolarPotentialMng* mng);
	~CCalcSolarRadiation(void);

	// 建物の日射量算出
	bool ExecBuild(
		CPotentialData& result,					// 計算結果格納用データマップ
		const CTime& startDate,
		const CTime& endDate
	);

	// 土地の日射量算出
	bool ExecLand(
		CPotentialData& result,					// 計算結果格納用データマップ
		const CTime& startDate,
		const CTime& endDate
	);

	// 日照率による補正
	bool ModifySunRate(
		CPotentialData& result					// 計算結果格納用データ
	);

	// 単位面積あたりの年間日射量を算出
	bool CalcAreaSolarRadiation(CPotentialData& result);

private:
	// 建物別年間日射量
	// 屋根面ごとの1m^2当たりの日射量を集計
	void calcTotalSolarRadiation(CPotentialData& result);

	static double calcSlopeDif(
		const double& surfaceAngle,	// 傾斜角
		const double& angIn,		// 入射角
		const double& sunAngle,		// 太陽高度
		const double& refRate,		// 反射率
		const bool& bDirect,		// 直達成分の有無
		const double& pdif			// 透過率
	);

	// 日射量の各計算
	// 入射角算出
	static double calcAngleIn(double sunAngle, double surfaceAngle, double surfaceAz, double alpha);
	//// 法線面直達日射量(W/m2)
	//static double calcDirectNormal(double sunAngle, double pdif);
	//// 水平面天空日射量(W/m2)
	//static double calcSkyHorizon(double sunAngle, double pdif);
	//// 斜面直達日射量
	//static double calcDirectSlope(double dDN, double angIn);
	//// 斜面天空日射量
	//static double calcSkySlope(double dSH, double surfaceAngle);
	//// 水平面全天日射量
	//static double calcSolarHorizon(double dDN, double dSH, double sunAngle);
	//// 斜面に入射する反射日射量
	//static double calcRefrectSlope(double dTH, double surfaceAngle, double refRate);


private:
	CCalcSolarPotentialMng*		m_pMng;

};
