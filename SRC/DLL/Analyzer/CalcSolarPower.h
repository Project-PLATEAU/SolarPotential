#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "CalcSolarPotentialMng.h"


// 1メッシュごとの発電ポテンシャル推計データ
class CCalcSolarPower
{
public:
	CCalcSolarPower();
	~CCalcSolarPower(void);

public:
	// 年間予測発電量(EPY)の算出 [kWh/年]
	bool CalcEPY(CPotentialData& result);
	void SetPperUnit(double d) { m_dPperUnit = d; };
	void SetPanelRatio(double d) { m_dPanelRatio = d; };

private:
	double m_dPperUnit;
	double m_dPanelRatio;
};

