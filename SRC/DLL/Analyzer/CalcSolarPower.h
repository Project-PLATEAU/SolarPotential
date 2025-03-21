#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "CalcSolarPotentialMng.h"


// 1���b�V�����Ƃ̔��d�|�e���V�������v�f�[�^
class CCalcSolarPower
{
public:
	CCalcSolarPower();
	~CCalcSolarPower(void);

public:
	// �N�ԗ\�����d��(EPY)�̎Z�o [kWh/�N]
	bool CalcEPY(CPotentialData& result);
	void SetPperUnit(double d) { m_dPperUnit = d; };
	void SetPanelRatio(double d) { m_dPanelRatio = d; };

private:
	double m_dPperUnit;
	double m_dPanelRatio;
};

