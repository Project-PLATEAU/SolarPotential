#include "pch.h"
#include <math.h>
#include <random>
#include <iostream>
#include <unordered_map>
#include "CalcSolarRadiation.h"
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "../../LIB/CommonUtil/CSunVector.h"
#include "../../LIB/CommonUtil/ReadINIParam.h"

#include <fstream>
#include <filesystem>

#ifdef CHRONO
#include <chrono>
#endif

// ���˗ʌv�Z�p�����[�^
// ���z�萔
const double def_Sconst = 1367.0;

CCalcSolarRadiation::CCalcSolarRadiation
(
	CCalcSolarPotentialMng* pMng
)
	: m_pMng(pMng)
{

}

CCalcSolarRadiation::~CCalcSolarRadiation(void)
{

}

// �����̓��˗ʎZ�o
bool CCalcSolarRadiation::ExecBuild(
	CPotentialData& result,					// �v�Z���ʊi�[�p�f�[�^
	const CTime& startDate,					// ��͂�����Ԃ̊J�n����
	const CTime& endDate					// ��͂�����Ԃ̏I������
)
{
	if (!m_pMng)	return false;

#ifdef CHRONO
	std::filesystem::path p = std::filesystem::path(m_pMng->GetUIParam()->strOutputDirPath) / "time2.log";
	ofstream ofs(p, ios::ios_base::app);

	ofs << "SolarRad::ExecBuild Start " << std::endl;
	chrono::system_clock::time_point start, end;
	start = std::chrono::system_clock::now();
	double time;
#endif

	int JPZONE = GetINIParam()->GetJPZone();

	int iYear = m_pMng->GetYear();

	const CVector3D center = result.center;

	// �אڂ��錚�����擾
#ifdef CHRONO
	ofs << "GetNeighborBuildings Start " << std::endl;
	start = std::chrono::system_clock::now();
#endif
	std::vector<BLDGLIST> neighborBuildings;
	m_pMng->GetNeighborBuildings(center, neighborBuildings);
#ifdef CHRONO
	end = std::chrono::system_clock::now();
	time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 0.001);
	ofs << "GetNeighborBuildings Time: " << time << " sec" << std::endl;
#endif

	// �אڂ���TIN���擾
	std::vector<CTriangle> neighborDems;
	if (m_pMng->IsEnableDEMData())
	{
		m_pMng->GetNeighborDems(center, neighborDems, CCalcSolarPotentialMng::eAnalyzeTarget::ROOF);
	}

#ifdef CHRONO
	ofs << "Calc Start " << std::endl;
	auto cstart = std::chrono::system_clock::now();
#endif

	for (auto& surface : result.mapSurface)
	{
		std::string surfaceId = surface.first;
		CSurfaceData& surfaceData = surface.second;

#ifdef CHRONO
		ofs << "--- Target Surface: " << surfaceId << std::endl;
		auto tstart = std::chrono::system_clock::now();
#endif
		// ���B�����̗L���𔻒�
		std::unordered_map<int, bool> mapSunDir;
		double dLat = 0.0, dLon = 0.0;
		CGeoUtil::XYToLatLon(JPZONE, surfaceData.center.y, surfaceData.center.x, dLat, dLon);

		double slopeDegree = surfaceData.slopeModDegree;	// �X�Ίp
		double azDegree = surfaceData.azModDegree;			// ���ʊp

		double surfaceAngle = slopeDegree * _COEF_DEG_TO_RAD;	// �Ζʂ̌X�Ίp
		double surfaceAz = azDegree * _COEF_DEG_TO_RAD;			// �Ζʂ̕��ʊp

		CTime date = startDate;
		do {

#ifdef CHRONO
			start = std::chrono::system_clock::now();
#endif
			if (m_pMng->IsCancel())	return false;

			double dSunnyVal = 0.0; double dCloudVal = 0.0;
			int month = date.GetMonth();

			double p = GetINIParam()->GetTransmissivity(month);		// ��C���ߗ�
			double pdifCloud = 1.0 - p;								// ���ߗ�(�ܓV��)

			const CSunVector outSun(dLat, dLon, date);

			for (int hour = 0; hour < 24; hour++)
			{
				HorizontalCoordinate pos;
				outSun.GetPos(hour, pos);
				double sunAngle = pos.altitude;	// ���z�V���p(���z���x)	
				if (sunAngle < 0)
				{
					//���z���x��0����
					continue;
				}
				double alpha = pos.azimuth + _PI;		// ���z����

				int idx = date.iYDayCnt * 24 + hour;

				// ���˗�
				double refRate = 0.0;	// ���˗�
				date.iHour = hour;
				double depth = this->m_pMng->GetMetpvData()->GetSnowDepth(date);
				if (depth > 10.0)		// 10cm�ȏ�
				{
					refRate = GetINIParam()->GetReflectivitySnow();
				}
				else
				{
					refRate = GetINIParam()->GetReflectivity();
				}

				// ���ˊp
				double angIn = calcAngleIn(sunAngle, surfaceAngle, surfaceAz, alpha);
				if (angIn < 0.0)	angIn = 0.0;

				CVector3D sunVector;
				outSun.GetVector(hour, sunVector);

				// ���B�����̗L���𔻒�
				if (mapSunDir.find(idx) == mapSunDir.end())
				{
					mapSunDir[idx] = m_pMng->IntersectSurfaceCenter(sunVector, surfaceData.bbPos, surfaceData.center, surfaceId, neighborBuildings, neighborDems);
				}
				bool bDirect = mapSunDir.at(idx);

				dSunnyVal += calcSlopeDif(surfaceAngle, angIn, sunAngle, refRate, bDirect, p);
				dCloudVal += calcSlopeDif(surfaceAngle, angIn, sunAngle, refRate, bDirect, pdifCloud);

			}

			int mIdx = month - 1;
			for (auto& mesh : surfaceData.vecMeshData)
			{
				// �Ђƌ����Ƃɒl��K�p
				mesh.solarRadiationSunny[mIdx] += dSunnyVal;
				mesh.solarRadiationCloud[mIdx] += dCloudVal;
			}

#ifdef CHRONO
			end = std::chrono::system_clock::now();
			time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 0.001);
			ofs << month << "/" << date.iDay << " Time : " << time << " sec" << std::endl;
#endif
			++date;

		} while (date <= endDate);

		mapSunDir.clear();

#ifdef CHRONO
		auto tend = std::chrono::system_clock::now();
		time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(tend - tstart).count() * 0.001);
		ofs << "Surface Time: " << time << " sec" << std::endl;
#endif
	}

#ifdef CHRONO
	auto cend = std::chrono::system_clock::now();
	time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(cend - cstart).count() * 0.001);
	ofs << "Calc Time: " << time << " sec" << std::endl;
#endif

	return true;
}

// �y�n�̓��˗ʎZ�o
bool CCalcSolarRadiation::ExecLand(
	CPotentialData& result,					// �v�Z���ʊi�[�p�f�[�^�}�b�v
	const CTime& startDate,					// ��͂�����Ԃ̊J�n����
	const CTime& endDate					// ��͂�����Ԃ̏I������
)
{
	if (!m_pMng)	return false;

	int JPZONE = GetINIParam()->GetJPZone();

	int iYear = m_pMng->GetYear();

	for (auto& [surfaceId, surfaceData] : result.mapSurface)
	{
		// �X�Ίp�A���ʊp
		double slopeDegree = surfaceData.slopeModDegree;
		double azDegree = surfaceData.azModDegree;

		double surfaceAngle = slopeDegree * _COEF_DEG_TO_RAD;	// �Ζʂ̌X�Ίp
		double surfaceAz = azDegree * _COEF_DEG_TO_RAD;			// �Ζʂ̕��ʊp

		for (auto& mesh : surfaceData.vecMeshData)
		{
			// �אڂ��錚�����擾
			std::vector<BLDGLIST> neighborBuildings;
			m_pMng->GetNeighborBuildings(mesh.center, neighborBuildings);

			// �אڂ���DEM���擾
			std::vector<CTriangle> neighborDems;
			m_pMng->GetNeighborDems(mesh.center, neighborDems, CCalcSolarPotentialMng::eAnalyzeTarget::LAND);

			double dLat = 0.0, dLon = 0.0;
			CGeoUtil::XYToLatLon(JPZONE, mesh.center.y, mesh.center.x, dLat, dLon);

			CTime date = startDate;
			do {
				if (m_pMng->IsCancel())	return false;

				double dSunnyVal = 0.0; double dCloudVal = 0.0;
				int month = date.GetMonth();

				double p = GetINIParam()->GetTransmissivity(month);		// ��C���ߗ�
				double pdifCloud = 1.0 - p;								// ���ߗ�(�ܓV��)

				const CSunVector outSun(dLat, dLon, date);

				for (int hour = 0; hour < 24; hour++)
				{
					HorizontalCoordinate pos;
					outSun.GetPos(hour, pos);
					double sunAngle = pos.altitude;	// ���z�V���p(���z���x)	
					if (sunAngle < 0)
					{
						//���z���x��0����
						continue;
					}
					double alpha = pos.azimuth + _PI;		// ���z����

					int idx = date.iYDayCnt * 24 + hour;

					// ���˗�
					double refRate = 0.0;	// ���˗�
					date.iHour = hour;
					double depth = this->m_pMng->GetMetpvData()->GetSnowDepth(date);
					if (depth > 10.0)		// 10cm�ȏ�
					{
						refRate = GetINIParam()->GetReflectivitySnow();
					}
					else
					{
						refRate = GetINIParam()->GetReflectivity();
					}

					// ���ˊp
					double angIn = calcAngleIn(sunAngle, surfaceAngle, surfaceAz, alpha);
					if (angIn < 0.0)	angIn = 0.0;

					CVector3D sunVector;
					outSun.GetVector(hour, sunVector);

					// ���B�����̗L���𔻒�
					bool bDirect = m_pMng->IntersectSurfaceCenter(sunVector, mesh.meshPos, mesh.centerMod, surfaceId, neighborBuildings, neighborDems);

					dSunnyVal += calcSlopeDif(surfaceAngle, angIn, sunAngle, refRate, bDirect, p);
					dCloudVal += calcSlopeDif(surfaceAngle, angIn, sunAngle, refRate, bDirect, pdifCloud);
				}

				// �Ђƌ����Ƃɒl��K�p
				mesh.solarRadiationSunny[month - 1] += dSunnyVal;
				mesh.solarRadiationCloud[month - 1] += dCloudVal;

				++date;

			} while (date <= endDate);

		}
	}

	return true;
}

bool CCalcSolarRadiation::ModifySunRate(CPotentialData& result)
{
	if (!m_pMng)	return false;

	for (auto& surface : result.mapSurface)
	{
		CSurfaceData& surfaceData = surface.second;

		for (auto& mesh : surfaceData.vecMeshData)
		{
			for (int month = 1; month <= 12; month++)
			{
				// ���V���̓��˗� �~ ���V���̓��Ɨ�(�T)
				double sunnyVal = mesh.solarRadiationSunny[month - 1] * m_pMng->GetRadiationData()->sunnyRate[month - 1];
				// �ܓV���̓��˗� �~ �ܓV���̓��Ɨ�(�U)
				double cloudVal = mesh.solarRadiationCloud[month - 1] * m_pMng->GetRadiationData()->cloudRate[month - 1];
				double val = sunnyVal + cloudVal;
				mesh.solarRadiation[month - 1] = val;	// �␳�������˗ʂ�K�p
			}
		}
	}

	return true;
}

// �Ζʓ��˗ʂ̎Z�o
inline double CCalcSolarRadiation::calcSlopeDif(
	const double& surfaceAngle,	// �X�Ίp
	const double& angIn,		// ���ˊp
	const double& sunAngle,		// ���z���x
	const double& refRate,		// ���˗�
	const bool& bDirect,		// ���B�����̗L��
	const double& pdif			// ���ߗ�
)
{
	double sinA = sin(sunAngle);
	double m = 1 / sinA;
	double betaM = pow(pdif, m);

	// �@���ʒ��B���˗�(W/m2)
	double dDN = 0.0;
	if (bDirect)
	{
		dDN = def_Sconst * betaM;
	}

	// �����ʓV����˗�(W/m2)
	double dN = 1.0 - betaM;
	double dD = 1.0 - 1.4 * log(pdif);
	double dSH = def_Sconst * sinA * (dN / dD) * 0.5;

	// �Ζʒ��B���˗�
	double dDT = 0.0;
	if (bDirect)
	{
		dDT = dDN * angIn;
	}

	// �ΖʓV����˗�
	double dST = dSH * (1 + cos(surfaceAngle)) * 0.5;

	// �����ʑS�V���˗�
	double dTH = dDN * sinA + dSH;

	// �Ζʂɓ��˂��锽�˓��˗�
	double dRT = dTH * (1 - cos(surfaceAngle)) * 0.5 * refRate;

	// �ΖʑS�V���˗�
	double dTT = dDT + dST + dRT;

	return dTT;
}

// ���ˊp�Z�o
inline double CCalcSolarRadiation::calcAngleIn(double sunAngle, double surfaceAngle, double surfaceAz, double alpha)
{
	return sin(sunAngle) * cos(surfaceAngle) + cos(sunAngle) * sin(surfaceAngle) * cos(alpha - surfaceAz);
}

// �N�ԓ��˗�
bool CCalcSolarRadiation::CalcAreaSolarRadiation(CPotentialData& result)
{
	if (!m_pMng)	return false;

	// �W�v
	calcTotalSolarRadiation(result);

	return true;
}


// �W�v
void CCalcSolarRadiation::calcTotalSolarRadiation(CPotentialData& result)
{
	for ( auto& surface : result.mapSurface)
	{
		// �L�����Z�����m
		if (m_pMng->IsCancel())
		{
			return;
		};

		std::string surfaceId = surface.first;
		CSurfaceData& surfaceData = surface.second;

		double sumVal = 0.0;

		for (auto& mesh : surfaceData.vecMeshData)
		{
			for (int month = 0; month < 12; month++)
			{
				sumVal += mesh.solarRadiation[month];
				mesh.solarRadiationUnit += mesh.solarRadiation[month];
			}

			mesh.solarRadiationUnit *= 0.001;	// kWh�ɕϊ�
		}

		double areaunit = (double)surfaceData.vecMeshData.size();

		// �P�ʖʐϓ�����̓��˗�
		surfaceData.solarRadiationUnit = sumVal / areaunit;		// ����(1m2������)
		surfaceData.solarRadiationUnit *= 0.001;				// kWh�ɕϊ�

		// �ΏۖʑS�̂̓��˗�(kWh)
		double area = surfaceData.GetArea() * m_pMng->GetUIParam()->pSolarPotentialParam->dPanelRatio;	// PV�ݒu������K�p�����ʐ�
		surfaceData.solarRadiation = surfaceData.solarRadiationUnit * area;

		// ���v(kWh)
		result.solarRadiationTotal += surfaceData.solarRadiation;
	}

	// 1m2������̔N�ԓ��˗ʂ��Z�o
	result.panelArea = result.GetAllArea() * m_pMng->GetUIParam()->pSolarPotentialParam->dPanelRatio;	// PV�ݒu������K�p�������ʐ�
	if (result.panelArea > 0.0)
	{
		result.solarRadiationUnit = result.solarRadiationTotal / result.panelArea;
	}

}

