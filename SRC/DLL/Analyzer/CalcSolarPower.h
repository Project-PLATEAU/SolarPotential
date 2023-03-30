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
	bool CalcEPY(CBuildingDataMap& dataMap);
	void SetPperUnit(double d) { m_dPperUnit = d; };

private:
	double m_dPperUnit;
};

