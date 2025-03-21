#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "CalcSolarPotentialMng.h"

// �������Ƃ̓��˗ʂ��v�Z����
class CCalcSolarRadiation
{
public:
	CCalcSolarRadiation(CCalcSolarPotentialMng* mng);
	~CCalcSolarRadiation(void);

	// �����̓��˗ʎZ�o
	bool ExecBuild(
		CPotentialData& result,					// �v�Z���ʊi�[�p�f�[�^�}�b�v
		const CTime& startDate,
		const CTime& endDate
	);

	// �y�n�̓��˗ʎZ�o
	bool ExecLand(
		CPotentialData& result,					// �v�Z���ʊi�[�p�f�[�^�}�b�v
		const CTime& startDate,
		const CTime& endDate
	);

	// ���Ɨ��ɂ��␳
	bool ModifySunRate(
		CPotentialData& result					// �v�Z���ʊi�[�p�f�[�^
	);

	// �P�ʖʐς�����̔N�ԓ��˗ʂ��Z�o
	bool CalcAreaSolarRadiation(CPotentialData& result);

private:
	// �����ʔN�ԓ��˗�
	// �����ʂ��Ƃ�1m^2������̓��˗ʂ��W�v
	void calcTotalSolarRadiation(CPotentialData& result);

	static double calcSlopeDif(
		const double& surfaceAngle,	// �X�Ίp
		const double& angIn,		// ���ˊp
		const double& sunAngle,		// ���z���x
		const double& refRate,		// ���˗�
		const bool& bDirect,		// ���B�����̗L��
		const double& pdif			// ���ߗ�
	);

	// ���˗ʂ̊e�v�Z
	// ���ˊp�Z�o
	static double calcAngleIn(double sunAngle, double surfaceAngle, double surfaceAz, double alpha);
	//// �@���ʒ��B���˗�(W/m2)
	//static double calcDirectNormal(double sunAngle, double pdif);
	//// �����ʓV����˗�(W/m2)
	//static double calcSkyHorizon(double sunAngle, double pdif);
	//// �Ζʒ��B���˗�
	//static double calcDirectSlope(double dDN, double angIn);
	//// �ΖʓV����˗�
	//static double calcSkySlope(double dSH, double surfaceAngle);
	//// �����ʑS�V���˗�
	//static double calcSolarHorizon(double dDN, double dSH, double sunAngle);
	//// �Ζʂɓ��˂��锽�˓��˗�
	//static double calcRefrectSlope(double dTH, double surfaceAngle, double refRate);


private:
	CCalcSolarPotentialMng*		m_pMng;

};
