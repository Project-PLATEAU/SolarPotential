#include "pch.h"
#include "CalcSolarPower.h"
#include "AnalysisRadiationData.h"



// ���d�ʎZ�o�p�����[�^
// ��{�݌v�W��
double def_KPY = 0.88;

// �W�����������ɂ�������ˋ��x
double def_GS = 1;


CCalcSolarPower::CCalcSolarPower
()
	: m_dPperUnit(0.0)
	, m_dPanelRatio(0.0)
{

}

CCalcSolarPower::~CCalcSolarPower(void)
{

}


// �N�ԗ\�����d��(EPY)�̎Z�o [kWh/�N]
bool CCalcSolarPower::CalcEPY(CPotentialData& result)
{
	double EPY = 0.0;

	// ���b�V�����Ƃ̔��d��
	for (auto& surface : result.mapSurface)
	{
		CSurfaceData& surfaceData = surface.second;

		for (auto& mesh : surfaceData.vecMeshData)
		{
			// �ݒu�\�V�X�e���e��(P)
			// �p�l���ʐρ��P�ʖʐϓ�����e��
			double P0 = 1.0 * m_dPperUnit;	// 1m2������

			// �N�ԗ\�����˗�(HAY)
			double HAY0 = mesh.solarRadiationUnit;

			// �ݒu�\�V�X�e���e��(P) �� �N�ԗ\�����˗�(HAY) �� ��{�݌v�W��(KPY) �� �P �^ �W�����������ɂ�������ˋ��x(GS)
			EPY = P0 * HAY0 * def_KPY * 1 / def_GS;

			// ���d��
			mesh.solarPowerUnit = EPY;
		}
	}

	//// �����E�y�n���Ƃ̔��d��
	//result.panelArea = result.GetAllArea() * m_dPanelRatio;

	// �ݒu�\�V�X�e���e��(P)
	// �p�l���ʐρ��P�ʖʐϓ�����e��
	double P = result.panelArea * m_dPperUnit;

	// �N�ԗ\�����˗�(HAY)
	double HAY = result.solarRadiationUnit;

	// �ݒu�\�V�X�e���e��(P) �� �N�ԗ\�����˗�(HAY) �� ��{�݌v�W��(KPY) �� �P �^ �W�����������ɂ�������ˋ��x(GS)
	EPY = P * HAY * def_KPY * 1 / def_GS;

	// �������ƂɎZ�o�������d�ʂ�K�p
	result.solarPower = EPY;

	// 1m2������̔��d��
	result.solarPowerUnit = EPY / result.panelArea;

	return true;

}

