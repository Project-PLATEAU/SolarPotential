#include "pch.h"
#include <math.h>
#include <random>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "CalcSolarPotentialMng.h"
#include "CalcSolarRadiation.h"
#include "CalcSolarPower.h"
#include "../../LIB/CommonUtil/CFileIO.h"
#include "../../LIB/CommonUtil/CFileUtil.h"
#include "../../LIB/CommonUtil/ReadINIParam.h"
#include "../../LIB/CommonUtil/CImageUtil.h"
#include "../../LIB/CommonUtil/CEpsUtil.h"
#include "../../LIB/CommonUtil/ExitCode.h"
#include "../../LIB/CommonUtil/CPoint2DPolygon.h"
#include "shapefil.h"
#include "AnalyzeData.h"

#ifdef CHRONO
#include <chrono>
#endif

#ifdef _DEBUG
#pragma comment(lib,"shapelib_i.lib")
#else
#pragma comment(lib,"shapelib_i.lib")
#endif

#define DEF_IMG_NODATA -9999
#define DEF_EPSGCODE 6675

CCalcSolarPotentialMng::CCalcSolarPotentialMng
(
	CImportPossibleSunshineData* pSunshineData,
	CImportAverageSunshineData* pPointData,
	CImportMetpvData* pMetpvData,
	CUIParam* pParam,
	const int& iYear
)
	: m_pSunshineData(pSunshineData)
	, m_pPointData(pPointData)
	, m_pMetpvData(pMetpvData)
	, m_pUIParam(pParam)
	, m_pRadiationData(NULL)
	, m_pmapResultData(NULL)
	, m_iYear(iYear)
	, m_strCancelFilePath(L"")
	, m_pvecAllAreaList(NULL)
	, m_isCancel(false)
{

}

CCalcSolarPotentialMng::~CCalcSolarPotentialMng(void)
{
}


// �v�Z�p�f�[�^���̏�����
void CCalcSolarPotentialMng::initialize()
{
	// �f�[�^�擾
	m_pvecAllAreaList = reinterpret_cast<std::vector<AREADATA>*>(GetAllAreaList());

	m_pmapResultData = new CResultDataMap;
	m_pRadiationData = new CAnalysisRadiationCommon;

	// �L�����Z���t�@�C��
	std::wstring strDir = GetFUtil()->GetParentDir(GetFUtil()->GetParentDir(m_pUIParam->strOutputDirPath));
	m_strCancelFilePath = GetFUtil()->Combine(strDir, CStringEx::ToWString(CANCELFILE));

	// ��͊��Ԃ�ݒ�
	switch (m_pUIParam->eAnalyzeDate)
	{
	case eDateType::OneDay:
		m_dateStart = m_dateEnd = CTime(GetYear(), m_pUIParam->nMonth, m_pUIParam->nDay, 0, 0, 0);
		break;

	case eDateType::OneMonth:
		m_dateStart = CTime(GetYear(), m_pUIParam->nMonth, 1, 0, 0, 0);
		m_dateEnd = CTime(GetYear(), m_pUIParam->nMonth, CTime::GetDayNum(m_pUIParam->nMonth), 23, 59, 59);
		break;

	case eDateType::Summer:
		m_dateStart = m_dateEnd = CTime::GetSummerSolstice(GetYear());
		break;

	case eDateType::Winter:
		m_dateStart = m_dateEnd = CTime::GetWinterSolstice(GetYear());
		break;

	case eDateType::Year:
		m_dateStart = CTime(GetYear(), 1, 1, 0, 0, 0);
		m_dateEnd = CTime(GetYear(), 12, 31, 23, 59, 59);
		break;

	default:
		break;
	}
}

// �������
void CCalcSolarPotentialMng::finalize()
{
	if (m_pRadiationData)
	{
		delete m_pRadiationData;
		m_pRadiationData = NULL;
	}

	if (m_pmapResultData)
	{
		for (auto& [areaId, resultdata] : *m_pmapResultData)
		{
			delete resultdata.pBuildMap;
			delete resultdata.pLandData;
		}
		m_pmapResultData->clear();
		delete m_pmapResultData;
		m_pmapResultData = NULL;
	}
}


// ���d�|�e���V�������v(���C������)
bool CCalcSolarPotentialMng::AnalyzeSolarPotential()
{
	setlocale(LC_ALL, "");

#ifdef CHRONO
	std::filesystem::path p = std::filesystem::path(m_pUIParam->strOutputDirPath) / "time.log";
	m_ofs.open(p);

	m_ofs << "AnalyzeSolarPotential Start " << std::endl;
	auto astart = std::chrono::system_clock::now();
	chrono::system_clock::time_point start, end;
	double time;
#endif

	assert(m_pUIParam);
	assert(m_pSunshineData);
	assert(m_pPointData);
	if (m_pUIParam->pInputData->strSnowDepthData != "")
	{
		assert(m_pMetpvData);
	}

	// ������
	initialize();
	assert(m_pRadiationData);
	assert(m_pvecAllAreaList);

	bool ret = false;

	// �����Ƃ̓��Ɨ����v�Z
	calcMonthlyRate();

	int dataCount = (int)m_pvecAllAreaList->size();
	for (auto& areaData : *m_pvecAllAreaList)
	{
		if (IsCancel())	break;

		// �������G���A
		m_targetArea = &areaData;

		CResultData dataMap;

		// �����̉��
		if (areaData.analyzeBuild)
		{
			// ���˗ʁE���d�ʂ��v�Z
			AnalyzeBuild(areaData, dataMap.pBuildMap);
			if (IsCancel())	break;
		}
		// �����̌��ʂ��o��
#ifdef CHRONO
		m_ofs << "outputAreaBuildResult Start " << std::endl;
		start = std::chrono::system_clock::now();
#endif
		if (dataMap.pBuildMap)
		{
			(*m_pmapResultData)[areaData.areaID].pBuildMap = *&dataMap.pBuildMap;
			ret = outputAreaBuildResult(areaData);
		}
#ifdef CHRONO
		end = std::chrono::system_clock::now();
		time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 0.001);
		m_ofs << "outputAreaBuildResult Time: " << time << " sec" << std::endl;
#endif

		// �y�n�̉��
		if (areaData.analyzeLand)
		{
			// ���˗ʁE���d�ʂ��v�Z
			AnalyzeLand(areaData, dataMap.pLandData);
			if (IsCancel())	break;
		}

		// �y�n�̌��ʂ��o��
#ifdef CHRONO
		m_ofs << "outputAreaLandResult Start " << std::endl;
		start = std::chrono::system_clock::now();
#endif
		if (dataMap.pLandData)
		{
			(*m_pmapResultData)[areaData.areaID].pLandData = *&dataMap.pLandData;
			ret = outputAreaLandResult(areaData);
		}
#ifdef CHRONO
		end = std::chrono::system_clock::now();
		time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 0.001);
		m_ofs << "outputAreaLandResult Time: " << time << " sec" << std::endl;
#endif

		if (!ret)	break;

	}

	if (ret)
	{
		// �K�n����p�̒��ԃt�@�C������(CSV)
		ret &= outputAzimuthDataCSV();

		// �}��o��
		ret &= outputLegendImage();

		// �\�����˗ʁE���d��
#ifdef CHRONO
		m_ofs << "outputAllAreaResultCSV Start " << std::endl;
		start = std::chrono::system_clock::now();
#endif
		ret &= outputAllAreaResultCSV();
#ifdef CHRONO
		end = std::chrono::system_clock::now();
		time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 0.001);
		m_ofs << "outputAllAreaResultCSV Time: " << time << " sec" << std::endl;
#endif
	}

	if (IsCancel())
	{
		m_eExitCode = eExitCode::Cancel;
		ret = false;
	}
	else if (!ret)
	{
		m_eExitCode = eExitCode::Error;
	}

	// �������
	finalize();

#ifdef CHRONO
	auto aend = std::chrono::system_clock::now();
	time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(aend - astart).count() * 0.001);
	m_ofs << "AnalyzeSolarPotential Time: " << time << " sec" << std::endl;
	m_ofs.close();
#endif

	return ret;
}

/// <summary>
/// �������
/// </summary>
/// <param name="pvecAllBuildList">�������X�g</param>
/// <param name="resultDataMap">��͌���</param>
void CCalcSolarPotentialMng::AnalyzeBuild(const AREADATA& areaData, CBuildListDataMap*& resultDataMap)
{
#ifdef CHRONO
	m_ofs << "AnalyzeBuild Start " << std::endl;
	auto astart = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point start, end;
	double time;
#endif

	if (IsCancel())
	{
		m_eExitCode = eExitCode::Cancel;
		return;
	}

	if (areaData.targetBuildings.empty())	return;

	bool ret = false;

	resultDataMap = new CBuildListDataMap();

	for (const auto& [meshId, targetBuildings] : areaData.targetBuildings)
	{
		// �X�Ίp�E���ʊp�̎Z�o
		CPotentialDataMap dataMap;
		calcRoofAspect(targetBuildings, dataMap);
		if (!dataMap.empty())
		{
			(*resultDataMap)[meshId] = *&dataMap;
		}
	}
	if (resultDataMap->empty())
	{
		delete resultDataMap;
		resultDataMap = NULL;
		return;
	}

	CCalcSolarRadiation calcSolarRad(this);
	CCalcSolarPower calcSolarPower;

	// ���d�ʐ��v�ݒ�
	calcSolarPower.SetPperUnit(m_pUIParam->pSolarPotentialParam->dPanelMakerSolarPower);
	calcSolarPower.SetPanelRatio(m_pUIParam->pSolarPotentialParam->dPanelRatio);

	for (auto& [meshId, resultData] : *resultDataMap)
	{
		for (auto& [buildId, data] : resultData)
		{
#ifdef CHRONO
			m_ofs << "------ Target Build: " << buildId << std::endl;

			// ���z�O�������Ƃɂ������˗ʂ̎Z�o
			m_ofs << "ExecBuild Start " << std::endl;
			start = std::chrono::system_clock::now();
			ret = calcSolarRad.ExecBuild(data, m_dateStart, m_dateEnd);
			end = std::chrono::system_clock::now();
			time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 0.001);
			m_ofs << "ExecBuild Time: " << time << " sec" << std::endl;
			if (!ret)	break;

			// ���Ɨ��ɂ��␳
			m_ofs << "ModifySunRate Start " << std::endl;
			start = std::chrono::system_clock::now();
			ret &= calcSolarRad.ModifySunRate(data);
			end = std::chrono::system_clock::now();
			time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 0.001);
			m_ofs << "ModifySunRate Time: " << time << " sec" << std::endl;
			if (!ret)	break;

			// �N�ԓ��˗�
			m_ofs << "CalcAreaSolarRadiation Start " << std::endl;
			start = std::chrono::system_clock::now();
			ret &= calcSolarRad.CalcAreaSolarRadiation(data);
			end = std::chrono::system_clock::now();
			time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 0.001);
			m_ofs << "CalcAreaSolarRadiation Time: " << time << " sec" << std::endl;
			if (!ret)	break;

			// ���d�ʐ��v
			m_ofs << "CalcEPY Start " << std::endl;
			start = std::chrono::system_clock::now();
			ret = calcSolarPower.CalcEPY(data);
			end = std::chrono::system_clock::now();
			time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 0.001);
			m_ofs << "CalcEPY Time: " << time << " sec" << std::endl;
			if (!ret)	break;
#else
			// ���z�O�������Ƃɂ������˗ʂ̎Z�o
			ret = calcSolarRad.ExecBuild(data, m_dateStart, m_dateEnd);
			if (!ret)	break;

			// ���Ɨ��ɂ��␳
			ret &= calcSolarRad.ModifySunRate(data);
			if (!ret)	break;

			// �N�ԓ��˗�
			ret &= calcSolarRad.CalcAreaSolarRadiation(data);
			if (!ret)	break;

			// ���d�ʐ��v
			ret = calcSolarPower.CalcEPY(data);
			if (!ret)	break;
#endif
		}

		if (!ret)	break;
	}

	if (IsCancel())
	{	// �L�����Z��
		m_eExitCode = eExitCode::Cancel;
		return;
	}
	else if (!ret)
	{	// �G���[
		m_eExitCode = eExitCode::Error;
		return;
	}

	m_eExitCode = eExitCode::Normal;

	#ifdef CHRONO
	auto aend = std::chrono::system_clock::now();
	time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(aend - astart).count() * 0.001);
	m_ofs << "AnalyzeBuild Time: " << time << " sec" << std::endl;
	#endif
}

/// <summary>
/// �y�n���
/// </summary>
/// <param name="pvecAllLandList"�y�n���X�g</param>
/// <param name="resultDataMap">��͌���</param>
void CCalcSolarPotentialMng::AnalyzeLand(const AREADATA& areaData, CPotentialData*& resultData)
{
	#ifdef CHRONO
	m_ofs << "AnalyzeLand Start " << std::endl;
	auto astart = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point start, end;
	double time;
	#endif

	resultData = new CPotentialData();

	// �X�Ίp�E���ʊp�̎Z�o(�Ώۃf�[�^�擾)
	calcLandAspect(areaData, *resultData);
	if (resultData->mapSurface.empty())
	{
		resultData = NULL;
		return;
	}

	CCalcSolarRadiation calcSolarRad(this);
	CCalcSolarPower calcSolarPower;

#ifdef CHRONO
	// ���z�O�������Ƃɂ������˗ʂ̎Z�o
	m_ofs << "ExecLand Start " << std::endl;
	start = std::chrono::system_clock::now();
	bool ret = calcSolarRad.ExecLand(*resultData, m_dateStart, m_dateEnd);
	end = std::chrono::system_clock::now();
	time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 0.001);
	m_ofs << "ExecLand Time: " << time << " sec" << std::endl;

	// ���Ɨ��ɂ��␳
	m_ofs << "ModifySunRate Start " << std::endl;
	start = std::chrono::system_clock::now();
	ret &= calcSolarRad.ModifySunRate(*resultData);
	end = std::chrono::system_clock::now();
	time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 0.001);
	m_ofs << "ModifySunRate Time: " << time << " sec" << std::endl;

	// �N�ԓ��˗�
	m_ofs << "CalcAreaSolarRadiation Start " << std::endl;
	start = std::chrono::system_clock::now();
	ret &= calcSolarRad.CalcAreaSolarRadiation(*resultData);
	end = std::chrono::system_clock::now();
	time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 0.001);
	m_ofs << "CalcAreaSolarRadiation Time: " << time << " sec" << std::endl;
#else
	// ���z�O�������Ƃɂ������˗ʂ̎Z�o
	bool ret = calcSolarRad.ExecLand(*resultData, m_dateStart, m_dateEnd);

	// ���Ɨ��ɂ��␳
	ret &= calcSolarRad.ModifySunRate(*resultData);

	// �N�ԓ��˗�
	ret &= calcSolarRad.CalcAreaSolarRadiation(*resultData);
#endif

	if (IsCancel())
	{	// �L�����Z��
		m_eExitCode = eExitCode::Cancel;
		return;
	}
	else if (!ret)
	{	// �G���[
		m_eExitCode = eExitCode::Error;
		return;
	}

	// ���d�ʐ��v
#ifdef CHRONO
	m_ofs << "CalcEPY Start " << std::endl;
	start = std::chrono::system_clock::now();
#endif
	calcSolarPower.SetPperUnit(m_pUIParam->pSolarPotentialParam->dPanelMakerSolarPower);
	calcSolarPower.SetPanelRatio(m_pUIParam->pSolarPotentialParam->dPanelRatio);
	ret = calcSolarPower.CalcEPY(*resultData);
#ifdef CHRONO
	end = std::chrono::system_clock::now();
	time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 0.001);
	m_ofs << "CalcEPY Time: " << time << " sec" << std::endl;
#endif

	if (IsCancel())
	{	// �L�����Z��
		m_eExitCode = eExitCode::Cancel;
		return;
	}
	else if (!ret)
	{	// �G���[
		m_eExitCode = eExitCode::Error;
		return;
	}

	m_eExitCode = eExitCode::Normal;

#ifdef CHRONO
	auto aend = std::chrono::system_clock::now();
	time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(aend - astart).count() * 0.001);
	m_ofs << "AnalyzeLand Time: " << time << " sec" << std::endl;
#endif
}

// �X�Ίp�ƕ��ʊp���Z�o
void CCalcSolarPotentialMng::calcRoofAspect(const vector<BUILDINGS*>& targetBuildings, CPotentialDataMap& bldDataMap)
{

	for (const auto& build : targetBuildings)
	{
		int surfacesize = (int)build->roofSurfaceList.size();
		if (surfacesize == 0)
		{
			continue;
		}

		CPotentialData bldData;

		// �����S�̂�BB
		double bbBldMinX = DBL_MAX, bbBldMinY = DBL_MAX;
		double bbBldMaxX = -DBL_MAX, bbBldMaxY = -DBL_MAX;

		std::vector<CPointBase> vecAllRoofPos;

		for (const auto& surface : build->roofSurfaceList)
		{	// ������
			int roofsize = (int)surface.roofSurfaceList.size();
			if (roofsize == 0)
			{
				continue;
			}

			int meshsize = (int)surface.meshPosList.size();
			if (meshsize == 0)
			{
				continue;
			}

			CSurfaceData roofData;

			// �����ʑS�̂�BB
			double bbRoofMinX = DBL_MAX, bbRoofMinY = DBL_MAX;
			double bbRoofMaxX = -DBL_MAX, bbRoofMaxY = -DBL_MAX;

			std::vector<CPointBase> vecRoofPos;
			
			// ���όX�Ίp�v�Z�p
			double slopSum = 0;

			// ���ϕ��ʊp�v�Z�p
			double azSum = 0;

			// �ʐϘa
			double areaSum = 0;

			std::vector<CMeshData> tempRoofMesh;

			for (const auto& member : surface.roofSurfaceList)
			{	// �����ʏڍ�
				int polygonsize = (int)member.posList.size();
				if (polygonsize == 0)
				{
					continue;
				}
				const std::vector<CPointBase> vecAry(member.posList.begin(), member.posList.end() - 1);	// �\���_�̎n�_�ƏI�_�͓����_�Ȃ̂ŏ��O����
				vecAllRoofPos.insert(vecAllRoofPos.end(), vecAry.begin(), vecAry.end());
				vecRoofPos.insert(vecRoofPos.end(), vecAry.begin(), vecAry.end());
				
				// �����S�̂�BB�����߂�(���O�������܂�)
				for (const auto& pos : vecAry)
				{
					if (pos.x < bbBldMinX)	bbBldMinX = pos.x;
					if (pos.y < bbBldMinY)	bbBldMinY = pos.y;
					if (pos.x > bbBldMaxX)	bbBldMaxX = pos.x;
					if (pos.y > bbBldMaxY)	bbBldMaxY = pos.y;
				}

				// �ʐώZ�o
				double area = calcArea(vecAry);

				// �@���Z�o
				CVector3D normal;
				if (!calcRansacPlane(vecAry, normal))
				{
					continue;
				}

				// �X�Ίp
				double slopeDeg;
				CGeoUtil::CalcSlope(normal, slopeDeg);

				// �X�����O����
				if (slopeDeg > m_pUIParam->pSolarPotentialParam->pRoof->dSlopeDegree)
				{
					continue;
				}

				// ���ʊp
				double azDeg;
				int JPZONE = GetINIParam()->GetJPZone();
				CGeoUtil::CalcAzimuth(normal, azDeg, JPZONE);

				// ���ʏ��O����
				bool bExclusion = false;
				switch (m_pUIParam->pSolarPotentialParam->pRoof->eDirection)
				{
					case eDirections::EAST:
					{
						bExclusion = (abs(azDeg) < 90.0 + AZ_RANGE_JUDGE_DEGREE || abs(azDeg) > 90.0 - AZ_RANGE_JUDGE_DEGREE) ? true : false;
						bExclusion &= (slopeDeg > m_pUIParam->pSolarPotentialParam->pRoof->dDirectionDegree) ? true : false;
						break;
					}
					case eDirections::WEST:
					{
						bExclusion = (abs(azDeg) < 270.0 + AZ_RANGE_JUDGE_DEGREE || abs(azDeg) > 270.0 - AZ_RANGE_JUDGE_DEGREE) ? true : false;
						bExclusion &= (slopeDeg > m_pUIParam->pSolarPotentialParam->pRoof->dDirectionDegree) ? true : false;
						break;
					}
					case eDirections::SOUTH:
					{
						bExclusion = (abs(azDeg) < 180.0 + AZ_RANGE_JUDGE_DEGREE || abs(azDeg) > 180.0 - AZ_RANGE_JUDGE_DEGREE) ? true : false;
						bExclusion &= (slopeDeg > m_pUIParam->pSolarPotentialParam->pRoof->dDirectionDegree) ? true : false;
						break;
					}
					case eDirections::NORTH:
					{
						bExclusion = (abs(azDeg) < 0.0 + AZ_RANGE_JUDGE_DEGREE || abs(azDeg) > 360.0 - AZ_RANGE_JUDGE_DEGREE) ? true : false;
						bExclusion &= (slopeDeg > m_pUIParam->pSolarPotentialParam->pRoof->dDirectionDegree) ? true : false;
						break;
					}
				}
				if (bExclusion)
				{
					continue;
				}

				// �����ʂ�BB�����߂�(���O�������܂܂Ȃ�)
				for (const auto& pos : vecAry)
				{
					if (pos.x < bbRoofMinX)	bbRoofMinX = pos.x;
					if (pos.y < bbRoofMinY)	bbRoofMinY = pos.y;
					if (pos.x > bbRoofMaxX)	bbRoofMaxX = pos.x;
					if (pos.y > bbRoofMaxY)	bbRoofMaxY = pos.y;
				}

				areaSum += area;
				slopSum += slopeDeg * area;
				azSum += azDeg * area;

			}
			
			if (areaSum < _MAX_TOL) { continue; }

			// �ʐς��������ꍇ���O
			if (areaSum < m_pUIParam->pSolarPotentialParam->pRoof->dArea2D)
			{
				continue;
			}

			// ������BB�ɍ�����t�^
			std::vector<CVector3D> bbPos{
				{bbRoofMinX, bbRoofMinY, 0.0},
				{bbRoofMinX, bbRoofMaxY, 0.0},
				{bbRoofMaxX, bbRoofMinY, 0.0},
				{bbRoofMaxX, bbRoofMaxY, 0.0}
			};
			roofData.bbPos = bbPos;

			// ������BB�̖@��
			CVector3D n;
			CGeoUtil::OuterProduct(
				CVector3D(bbPos[1], bbPos[0]),
				CVector3D(bbPos[2], bbPos[1]), n);
			if (n.z < 0) n *= -1;

			CVector3D p(vecRoofPos[0].x, vecRoofPos[0].y, vecRoofPos[0].z);
			double d = CGeoUtil::InnerProduct(p, n);
			CVector3D inVec(0.0, 0.0, 1.0);
			double dot = CGeoUtil::InnerProduct(n, inVec);

			CVector3D roofcenter;		// ���S�̍��W
			for (auto& pos : roofData.bbPos)
			{
				// ���ʂƐ����̌�_
				CVector3D p0(pos.x, pos.y, 0.0);
				double t = (d - CGeoUtil::InnerProduct(p0, n)) / dot;
				// ��_
				CVector3D tempPoint = p0 + t * inVec;
				pos.z = tempPoint.z;
				// ���S
				roofcenter += pos;
			}
			roofcenter *= 0.25;

			double dMinDist = DBL_MAX;
			CMeshData neighborMesh;

			// ���b�V�����Ƃ̌v�Z���ʊi�[�p�f�[�^��ǉ�
			for (int i = 0; i < meshsize; i++)
			{
				MESHPOSITION_XY meshXY = surface.meshPosList[i];

				CMeshData mesh;

				std::vector<CVector3D> meshXYZ{
					{meshXY.leftDownX, meshXY.leftDownY, 0.0},
					{meshXY.leftTopX, meshXY.leftTopY, 0.0},
					{meshXY.rightTopX, meshXY.rightTopY, 0.0},
					{meshXY.rightDownX, meshXY.rightDownY, 0.0}
				};
				mesh.meshPos = meshXYZ;

				mesh.area = abs(meshXY.rightTopX - meshXY.leftTopX) * abs(meshXY.leftTopY - meshXY.leftDownY);

				CVector3D center;		// ���S�̍��W
				for (auto& meshPos : mesh.meshPos)
				{
					// ���ʂƐ����̌�_
					CVector3D p0(meshPos.x, meshPos.y, 0.0);
					double t = (d - CGeoUtil::InnerProduct(p0, n)) / dot;
					// ��_
					CVector3D tempPoint = p0 + t * inVec;
					meshPos.z = tempPoint.z;
					// ���S
					center += meshPos;
				}
				center *= 0.25;

				mesh.meshId = CStringEx::Format("%s_%d", surface.roofSurfaceId.c_str(), i);
				mesh.center = center;
				mesh.centerMod = center;

				tempRoofMesh.emplace_back(mesh);

				// �ʒ��S�Ƃ̋������ł��߂����b�V����ێ�
				double dx = roofcenter.x - center.x;
				double dy = roofcenter.y - center.y;
				double dz = roofcenter.z - center.z;
				double dist = calcLength(dx, dy, dz);
				if (dist < dMinDist)
				{
					neighborMesh = mesh;
					dMinDist = dist;
				}
			}

			// �Ώۃ��b�V�����Ȃ�
			if (tempRoofMesh.size() == 0) { continue; }

			// �����ʂ��Ƃ̌v�Z���ʊi�[�p�f�[�^���쐬
			{
				roofData.area = areaSum;

				// �X�Ίp�ƕ��ʊp�̕��ϐݒ�
				roofData.slopeDegreeAve = slopSum / areaSum;
				roofData.azDegreeAve = azSum / areaSum;

				double slopeMdDeg = roofData.slopeDegreeAve;
				double azMdDeg = roofData.azDegreeAve;

				// �␳
				if (slopeMdDeg < m_pUIParam->pSolarPotentialParam->pRoof->dCorrectionCaseDeg)
				{
					slopeMdDeg = m_pUIParam->pSolarPotentialParam->pRoof->dCorrectionDirectionDegree;
					switch ((eDirections)m_pUIParam->pSolarPotentialParam->pRoof->eCorrectionDirection)
					{
					case eDirections::EAST:
						azMdDeg = 90.0;
						break;
					case eDirections::WEST:
						azMdDeg = 270.0;
						break;
					case eDirections::SOUTH:
						azMdDeg = 180.0;
						break;
					case eDirections::NORTH:
						azMdDeg = 0.0;
						break;
					}

					dMinDist = DBL_MAX;

					// ���b�V�����W��␳
					for (auto& mesh : tempRoofMesh)
					{
						CVector3D centerMd;

						for (auto& meshPos : mesh.meshPos)
						{
							CVector3D orgPos(meshPos - mesh.center);
							orgPos.z = 0.0;
							CVector3D rotPos;
							double theta = slopeMdDeg * _COEF_DEG_TO_RAD;

							// �␳���ʂɕϊ�
							switch ((eDirections)m_pUIParam->pSolarPotentialParam->pRoof->eCorrectionDirection)
							{
								case eDirections::EAST:
									rotPos.x = orgPos.x * cos(-theta);
									rotPos.y = orgPos.y;
									rotPos.z = orgPos.x * sin(-theta);
									break;
								case eDirections::WEST:
									rotPos.x = orgPos.x * cos(theta);
									rotPos.y = orgPos.y;
									rotPos.z = orgPos.x * sin(theta);
									break;
								case eDirections::SOUTH:
									rotPos.x = orgPos.x;
									rotPos.y = orgPos.y * cos(theta);
									rotPos.z = orgPos.y * sin(theta);
									break;
								case eDirections::NORTH:
									rotPos.x = orgPos.x;
									rotPos.y = orgPos.y * cos(-theta);
									rotPos.z = orgPos.y * sin(-theta);
									break;
								default:
									rotPos = orgPos;
									break;
							}

							rotPos += mesh.center;
							meshPos = rotPos;

							// ���S
							centerMd += meshPos;
						}
						centerMd *= 0.25;
						mesh.centerMod = centerMd;

						// �ʒ��S�Ƃ̋������ł��߂����b�V����ێ�
						double dx = roofcenter.x - centerMd.x;
						double dy = roofcenter.y - centerMd.y;
						double dz = roofcenter.z - centerMd.z;
						double dist = calcLength(dx, dy, dz);
						if (dist < dMinDist)
						{
							neighborMesh = mesh;
							dMinDist = dist;
						}
					}
				}

				roofData.slopeModDegree = slopeMdDeg;
				roofData.azModDegree = azMdDeg;

				// �ʂ̒��S
				roofData.center = neighborMesh.centerMod;

				// ���b�V�����X�g��ǉ�
				roofData.vecMeshData = tempRoofMesh;

				roofData.meshSize = 1.0;

				// �Ώۉ����ʂɒǉ�
				bldData.mapSurface[surface.roofSurfaceId] = roofData;

			}
		}

		// �Ώۉ����ʂ��Ȃ�
		if (bldData.mapSurface.size() == 0) { continue; }

		// ����BB�ɍ�����t�^
		std::vector<CVector3D> bbPos{
			{bbBldMinX, bbBldMinY, 0.0},
			{bbBldMinX, bbBldMaxY, 0.0},
			{bbBldMaxX, bbBldMinY, 0.0},
			{bbBldMaxX, bbBldMaxY, 0.0}
		};
		bldData.bbPos = bbPos;

		// �����S��BB�̖@��
		CVector3D n;
		CGeoUtil::OuterProduct(
			CVector3D(bbPos[1], bbPos[0]),
			CVector3D(bbPos[2], bbPos[1]), n);
		if (n.z < 0) n *= -1;

		CVector3D p(vecAllRoofPos[0].x, vecAllRoofPos[0].y, vecAllRoofPos[0].z);
		double d = CGeoUtil::InnerProduct(p, n);
		CVector3D inVec(0.0, 0.0, 1.0);
		double dot = CGeoUtil::InnerProduct(n, inVec);

		CVector3D center;		// ���S�̍��W
		for (auto& pos : bldData.bbPos)
		{
			// ���ʂƐ����̌�_
			CVector3D p0(pos.x, pos.y, 0.0);
			double t = (d - CGeoUtil::InnerProduct(p0, n)) / dot;
			// ��_
			CVector3D tempPoint = p0 + t * inVec;
			pos.z = tempPoint.z;
			// ���S
			center += pos;
		}
		center *= 0.25;
		bldData.center = center;

		bldDataMap[build->building] = bldData;

	}
}

// �X�Ίp�ƕ��ʊp���Z�o
// 
void CCalcSolarPotentialMng::calcLandAspect(const AREADATA& area, CPotentialData& landData)
{
	LANDSURFACES surface = area.landSurface;

	int surfacesize = (int)surface.landSurfaceList.size();
	if (surfacesize == 0)	return;

	std::vector<CPointBase> vecAllLandPos;

	int meshCount = 0;

	for (const auto& member : surface.landSurfaceList)
	{
		CSurfaceData surfaceData;

		int polygonsize = (int)member.posList.size();
		if (polygonsize != 4)
		{
			continue;
		}

		const std::vector<CPointBase> vecAry(member.posList.begin(), member.posList.end());
		vecAllLandPos.insert(vecAllLandPos.end(), vecAry.begin(), vecAry.end());

		// �ʐώZ�o
		double surfacearea = calcArea(vecAry);
		// �ʐς��������ꍇ���O
		if (surfacearea < m_pUIParam->pSolarPotentialParam->pLand->dArea2D)
		{
			continue;
		}

		// �@���Z�o
		CVector3D normal;
		if (!calcRansacPlane(vecAry, normal))
		{
			continue;
		}

		// �X�Ίp
		double slopeDeg;
		CGeoUtil::CalcSlope(normal, slopeDeg);
		// �X�����O����
		if (slopeDeg > m_pUIParam->pSolarPotentialParam->pLand->dSlopeAngle)
		{
			continue;
		}

		// ���ʊp
		double azDeg;
		int JPZONE = GetINIParam()->GetJPZone();
		CGeoUtil::CalcAzimuth(normal, azDeg, JPZONE);

		// �Ή�����y�n��(���b�V��)�̌v�Z���ʊi�[�p�f�[�^��ǉ�
		CMeshData mesh;
		for (const auto& pos : vecAry)
		{
			mesh.meshPos.emplace_back(CVector3D(pos.x, pos.y, pos.z));
		}
		mesh.area = surfacearea;
		mesh.meshId = CStringEx::Format("%s_%d", area.areaID.c_str(), meshCount);

		// �y�n��(���b�V��)�̒��S���W���Z�o
		CVector3D center;
		for (auto& pt : vecAry)
		{
			CVector3D pos(pt.x, pt.y, pt.z);
			center += pos;
		}
		center *= 0.25;
		mesh.center = center;
		mesh.centerMod = center;

		// �y�n�ʂ��Ƃ̌v�Z���ʊi�[�p�f�[�^���쐬
		{
			surfaceData.area = surfacearea;

			// �X�Ίp�ƕ��ʊp�̕��ϐݒ�
			surfaceData.slopeDegreeAve = slopeDeg;
			surfaceData.azDegreeAve = azDeg;

			double slopeMdDeg = surfaceData.slopeDegreeAve;
			double azMdDeg = surfaceData.azDegreeAve;

			// �␳
			{
				slopeMdDeg = area.degree;
				switch ((eDirections)area.direction)
				{
				case eDirections::EAST:
					azMdDeg = 90.0;
					break;
				case eDirections::WEST:
					azMdDeg = 270.0;
					break;
				case eDirections::SOUTH:
					azMdDeg = 180.0;
					break;
				case eDirections::NORTH:
					azMdDeg = 0.0;
					break;
				}

				// ���b�V�����W��␳
				CVector3D centerMd;

				for (auto& meshPos : mesh.meshPos)
				{
					CVector3D orgPos(meshPos - mesh.center);
					orgPos.z = 0.0;
					CVector3D rotPos;
					double theta = slopeMdDeg * _COEF_DEG_TO_RAD;

					// �␳���ʂɕϊ�
					switch ((eDirections)area.direction)
					{
					case eDirections::EAST:
						rotPos.x = orgPos.x * cos(-theta);
						rotPos.y = orgPos.y;
						rotPos.z = orgPos.x * sin(-theta);
						break;
					case eDirections::WEST:
						rotPos.x = orgPos.x * cos(theta);
						rotPos.y = orgPos.y;
						rotPos.z = orgPos.x * sin(theta);
						break;
					case eDirections::SOUTH:
						rotPos.x = orgPos.x;
						rotPos.y = orgPos.y * cos(theta);
						rotPos.z = orgPos.y * sin(theta);
						break;
					case eDirections::NORTH:
						rotPos.x = orgPos.x;
						rotPos.y = orgPos.y * cos(-theta);
						rotPos.z = orgPos.y * sin(-theta);
						break;
					default:
						rotPos = orgPos;
						break;
					}

					rotPos += mesh.center;
					meshPos = rotPos;

					// ���S
					centerMd += meshPos;
				}
				centerMd *= 0.25;
				mesh.centerMod = centerMd;
			}

			surfaceData.slopeModDegree = slopeMdDeg;
			surfaceData.azModDegree = azMdDeg;

			// �ʒ��S��ݒ�(���b�V���Ɠ��l)
			surfaceData.center = center;

			// ���b�V�����X�g��ǉ�
			surfaceData.vecMeshData.emplace_back(mesh);

			surfaceData.meshSize = surface.meshSize;

			// �Ώۉ����ʂɒǉ�
			std::string id = CStringEx::Format("%10d", meshCount);
			landData.mapSurface[id] = surfaceData;

		}

		meshCount++;

	}

	// �͈�BB�ɍ�����t�^
	std::vector<CVector3D> bbPos{
		{area.bbMinX, area.bbMinY, 0.0},
		{area.bbMinX, area.bbMaxY, 0.0},
		{area.bbMaxX, area.bbMinY, 0.0},
		{area.bbMaxX, area.bbMaxY, 0.0}
	};
	landData.bbPos = bbPos;

	// �͈͑S��BB�̖@��
	CVector3D n;
	CGeoUtil::OuterProduct(
		CVector3D(bbPos[1], bbPos[0]),
		CVector3D(bbPos[2], bbPos[1]), n);
	if (n.z < 0) n *= -1;

	CVector3D p(vecAllLandPos[0].x, vecAllLandPos[0].y, vecAllLandPos[0].z);
	double d = CGeoUtil::InnerProduct(p, n);
	CVector3D inVec(0.0, 0.0, 1.0);
	double dot = CGeoUtil::InnerProduct(n, inVec);

	CVector3D center;		// ���S�̍��W
	for (auto& pos : landData.bbPos)
	{
		// ���ʂƐ����̌�_
		CVector3D p0(pos.x, pos.y, 0.0);
		double t = (d - CGeoUtil::InnerProduct(p0, n)) / dot;
		// ��_
		CVector3D tempPoint = p0 + t * inVec;
		pos.z = tempPoint.z;
		// ���S
		center += pos;
	}
	center *= 0.25;
	landData.center = center;

}

/*!	���_�Q����ߎ����ʎZ�o
@note   �����ʂ̌X�Ίp�E���ʊp
*/
bool CCalcSolarPotentialMng::calcRansacPlane(
	const std::vector<CPointBase>& vecAry,		//!< in		���_�z��
	CVector3D& vNormal							//!< out	�@��
)
{
	bool	bRet = false;

	CVector3D vec1;
	CVector3D vec2;
	// [0]����̊e�_�̃x�N�g��
	vector<CVector3D> vecPolyList;
	for (int i = 1; i < vecAry.size(); ++i)
	{
		CVector3D vec(vecAry[i], vecAry[0]);
		vecPolyList.emplace_back(vec);
	}
	sort(
		vecPolyList.begin(),
		vecPolyList.end(),
		[](const CVector3D& x, const CVector3D& y) { return x.Length() > y.Length(); }
	);
	vec1 = vecPolyList[0];
	vec1.Normalize();
	for (const auto& pos : vecPolyList)
	{
		CVector3D tempVec = pos;
		tempVec.Normalize();
		// �����������t�����̂Ƃ��͖@�����܂�Ȃ�
		if (abs(CGeoUtil::InnerProduct(vec1, tempVec)) > 0.999)
			continue;
		vec2 = tempVec;
		break;
	}
	CGeoUtil::OuterProduct(vec1, vec2, vNormal);

	if (vNormal.z < 0.0)
	{
		// ������ɔ��]
		vNormal.Inverse();
	}

	return	true;
}

// �ʐς��Z�o
double CCalcSolarPotentialMng::calcArea(const std::vector<CPointBase>& vecPos)
{
	double area = 0.0;

	CVector3D v1, v2, v3;

	if (vecPos.size() < 3)
		return area;

	for (int i = 0; i < vecPos.size() - 2; i++)
	{
		v1 = CVector3D(vecPos.at((int64_t)i + 1).x, vecPos.at((int64_t)i + 1).y, 0.0) - CVector3D(vecPos.at(0).x, vecPos.at(0).y, 0.0);
		v2 = CVector3D(vecPos.at((int64_t)i + 2).x, vecPos.at((int64_t)i + 2).y, 0.0) - CVector3D(vecPos.at(0).x, vecPos.at(0).y, 0.0);
		CGeoUtil::OuterProduct(v1, v2, v3);

		area += v3.Length() / 2.0;
	}

	return area;
}

void CCalcSolarPotentialMng::calcMonthlyRate()
{
	for (int i = 0; i < 12; i++)
	{
		int month = i + 1;
		CTime time(m_iYear, month, 0, 0, 0, 0);
	
		// �����Ƃ̉Ǝ���
		double dPossibleSunshineDuration = m_pSunshineData->GetPossibleSunshineDuration(time);

		// �����Ƃ̓��Ǝ���
		double dSunshineTime = m_pPointData->GetAverageSunshineTime(time);

		// ���V/�ܓV���̌����Ƃ̓��Ɨ����Z�o
		// ���V���̓��Ɨ�(�T)��[���Ǝ���] / [�Ǝ���]
		m_pRadiationData->sunnyRate[i] = dSunshineTime / dPossibleSunshineDuration;
		// �ܓV���̓��Ɨ�(�U)��(1 - ���V���̓��Ɨ�(�T))
		m_pRadiationData->cloudRate[i] = 1 - m_pRadiationData->sunnyRate[i];
	}
}

/// <summary>
/// ���d�|�e���V�������v���ʂ��o��
/// </summary>
/// <param name="target"></param>
bool CCalcSolarPotentialMng::outputAreaBuildResult(const AREADATA& areaData)
{
	bool ret = true;

	const auto& resultData = (*m_pmapResultData)[areaData.areaID];

	// �ΏۃG���A�̃t�H���_��
	std::wstring strAreaDirName = CStringEx::Format(L"%s_%s", CStringEx::ToWString(areaData.areaID).c_str(), CStringEx::ToWString(areaData.areaName).c_str());
	if (areaData.areaName == "")	strAreaDirName.pop_back();	// ���̂������ꍇ�͖�����"_"���폜

	// ����
	if (areaData.analyzeBuild && resultData.pBuildMap)
	{
		// ��͑Ώۂ��Ƃ̏o�͐�t�H���_��
		std::wstring strAnalyzeTargetDir = GetDirName_AnalyzeTargetDir(eAnalyzeTarget::ROOF);
		// �ΏۃG���A�̃t�H���_���쐬
		std::wstring strAreaDir = GetFUtil()->Combine(strAnalyzeTargetDir, strAreaDirName);
		// �t�H���_���쐬
		if (!GetFUtil()->IsExistPath(strAreaDir))
		{
			if (CreateDirectory(strAreaDir.c_str(), NULL) == FALSE)
			{
				return false;
			}
		}

		// 3�����b�V�����Ƃɏo��
		for (const auto& [Lv3meshId, dataMap] : *resultData.pBuildMap)
		{
			// �������X�g���擾
			BLDGLIST bldList{};  // �i�[�p

			for (const auto& bldli : m_targetArea->neighborBldgList)
			{
				if (Lv3meshId == bldli->meshID)
				{
					bldList = *bldli;
					break;
				}
			}

			// ���b�V���̃t�H���_���쐬
			std::wstring strMeshDir = GetFUtil()->Combine(strAreaDir, CStringEx::ToWString(Lv3meshId));
			// �t�H���_���쐬
			if (!GetFUtil()->IsExistPath(strMeshDir))
			{
				if (CreateDirectory(strMeshDir.c_str(), NULL) == FALSE)
				{
					return false;
				}
			}

			// CSV
			{
				// �ʂ��Ƃ̓��˗ʎZ�o����
				ret &= outputSurfaceRadCSV(eAnalyzeTarget::ROOF, dataMap, strMeshDir);

				// ���ʓ��˗�CSV(�N�Ԃ̂�)
				if (m_pUIParam->eAnalyzeDate == eDateType::Year)
				{
					ret &= outputMonthlyRadCSV(dataMap, strMeshDir);
				}
			}

			// ���˗ʉ摜���o��
			{
				double outMeshsize = 1.0;

				// �o�͗p�f�[�^���쐬
				std::vector<CPointBase>* pvecPoint3d = new std::vector<CPointBase>();
				ret &= createPointData_Build(*pvecPoint3d, areaData, bldList, dataMap, outMeshsize, eOutputImageTarget::SOLAR_RAD);

				std::wstring strFileName = CStringEx::Format(L"���˗�_%s_%s.tif", CStringEx::ToWString(areaData.areaID).c_str(), CStringEx::ToWString(Lv3meshId).c_str());
				std::wstring strTiffPath = GetFUtil()->Combine(strMeshDir, strFileName);

				if (!outputImage(strTiffPath, pvecPoint3d, outMeshsize, eOutputImageTarget::SOLAR_RAD))
				{
					return false;
				}
				else
				{
					// �o��TIFF�摜���R�s�[
					std::wstring strCopyDir = GetFUtil()->Combine(strAnalyzeTargetDir, GetDirName_SolarRadImage());
					if (!GetFUtil()->IsExistPath(strCopyDir))
					{
						if (CreateDirectory(strCopyDir.c_str(), NULL) == FALSE)
						{
							return false;
						}
					}
					std::wstring dstPath = GetFUtil()->Combine(strCopyDir, strFileName);
					CopyFile(strTiffPath.c_str(), dstPath.c_str(), FALSE);
					std::wstring strWldPath = GetFUtil()->ChangeFileNameExt(strTiffPath, L".tfw");
					std::wstring dstPath2 = GetFUtil()->ChangeFileNameExt(dstPath, L".tfw");
					CopyFile(strWldPath.c_str(), dstPath2.c_str(), FALSE);

				}

			}

			// ���d�ʉ摜���o��
			{
				double outMeshsize = 1.0;

				// �o�͗p�f�[�^���쐬
				std::vector<CPointBase>* pvecPoint3d = new std::vector<CPointBase>();
				ret &= createPointData_Build(*pvecPoint3d, areaData, bldList, dataMap, outMeshsize, eOutputImageTarget::SOLAR_POWER);

				// �o�̓t�H���_���쐬
				std::wstring strOutDir = GetFUtil()->Combine(strAnalyzeTargetDir, GetDirName_SolarPotentialImage());
				if (!GetFUtil()->IsExistPath(strOutDir))
				{
					if (CreateDirectory(strOutDir.c_str(), NULL) == FALSE)
					{
						return false;
					}
				}
				// �e�N�X�`���o��
				std::wstring strFileName = CStringEx::Format(L"���d��_%s_%s.tif", CStringEx::ToWString(areaData.areaID).c_str(), CStringEx::ToWString(Lv3meshId).c_str());
				std::wstring strTiffPath = GetFUtil()->Combine(strOutDir, strFileName);
				ret &= outputImage(strTiffPath, pvecPoint3d, outMeshsize, eOutputImageTarget::SOLAR_POWER);

			}
		}

	}

	return ret;
}


/// <summary>
/// ���d�|�e���V�������v���ʂ��o��
/// </summary>
/// <param name="target"></param>
bool CCalcSolarPotentialMng::outputAreaLandResult(const AREADATA& areaData)
{
	bool ret = true;

	const auto& resultData = (*m_pmapResultData)[areaData.areaID];

	// �ΏۃG���A�̃t�H���_��
	std::wstring strAreaDirName = CStringEx::Format(L"%s_%s", CStringEx::ToWString(areaData.areaID).c_str(), CStringEx::ToWString(areaData.areaName).c_str());
	if (areaData.areaName == "")	strAreaDirName.pop_back();	// ���̂������ꍇ�͖�����"_"���폜

	// �y�n
	if (areaData.analyzeLand && resultData.pLandData)
	{
		// ��͑Ώۂ��Ƃ̏o�͐�t�H���_��
		std::wstring strAnalyzeTargetDir = GetDirName_AnalyzeTargetDir(eAnalyzeTarget::LAND);
		// �ΏۃG���A�̃t�H���_���쐬
		std::wstring strAreaDir = GetFUtil()->Combine(strAnalyzeTargetDir, strAreaDirName);
		// �t�H���_���쐬
		if (!GetFUtil()->IsExistPath(strAreaDir))
		{
			if (CreateDirectory(strAreaDir.c_str(), NULL) == FALSE)
			{
				return false;
			}
		}

		CPotentialDataMap dataMap;
		dataMap[areaData.areaID] = *resultData.pLandData;

		// CSV
		{
			// �ʂ��Ƃ̓��˗ʎZ�o����
			ret &= outputSurfaceRadCSV(eAnalyzeTarget::LAND, dataMap, strAreaDir);

			// ���ʓ��˗�CSV(�N�Ԃ̂�)
			if (m_pUIParam->eAnalyzeDate == eDateType::Year)
			{
				ret &= outputMonthlyRadCSV(dataMap, strAreaDir);
			}
		}

		// ���˗ʉ摜���o��
		{
			double outMeshsize = 1.0;

			// �o�͗p�f�[�^���쐬
			std::vector<CPointBase>* pvecPoint3d = new std::vector<CPointBase>;
			ret &= createPointData_Land(*pvecPoint3d, areaData, *resultData.pLandData, eOutputImageTarget::SOLAR_RAD);

			std::wstring strFileName = CStringEx::Format(L"���˗�_%s.tif", CStringEx::ToWString(areaData.areaID).c_str());
			std::wstring strTiffPath = GetFUtil()->Combine(strAreaDir, strFileName);

			if (ret && !outputImage(strTiffPath, pvecPoint3d, outMeshsize, eOutputImageTarget::SOLAR_RAD))
			{
				return false;
			}
			else
			{
				// �o��TIFF�摜���R�s�[
				std::wstring strCopyDir = GetFUtil()->Combine(strAnalyzeTargetDir, GetDirName_SolarRadImage());
				if (!GetFUtil()->IsExistPath(strCopyDir))
				{
					if (CreateDirectory(strCopyDir.c_str(), NULL) == FALSE)
					{
						return false;
					}
				}
				std::wstring dstPath = GetFUtil()->Combine(strCopyDir, strFileName);
				CopyFile(strTiffPath.c_str(), dstPath.c_str(), FALSE);
				std::wstring strWldPath = GetFUtil()->ChangeFileNameExt(strTiffPath, L".tfw");
				std::wstring dstPath2 = GetFUtil()->ChangeFileNameExt(dstPath, L".tfw");
				CopyFile(strWldPath.c_str(), dstPath2.c_str(), FALSE);
			}
		}

		// ���d�ʉ摜���o��
		{
			double outMeshsize = 1.0;

			// �o�͗p�f�[�^���쐬
			std::vector<CPointBase>* pvecPoint3d = new std::vector<CPointBase>;
			ret &= createPointData_Land(*pvecPoint3d, areaData, *resultData.pLandData, eOutputImageTarget::SOLAR_POWER);

			if (ret)
			{
				// �o�̓t�H���_���쐬
				std::wstring strOutDir = GetFUtil()->Combine(strAnalyzeTargetDir, GetDirName_SolarPotentialImage());
				if (!GetFUtil()->IsExistPath(strOutDir))
				{
					if (CreateDirectory(strOutDir.c_str(), NULL) == FALSE)
					{
						return false;
					}
				}
				// �e�N�X�`���o��
				std::wstring strFileName = CStringEx::Format(L"���d��_%s.tif", CStringEx::ToWString(areaData.areaID).c_str());
				std::wstring strTiffPath = GetFUtil()->Combine(strOutDir, strFileName);

				ret &= outputImage(strTiffPath, pvecPoint3d, outMeshsize, eOutputImageTarget::SOLAR_POWER);
			}
		}
	}

	return ret;
}

// ���˗ʎZ�o���ʂ̃|�C���g�f�[�^���쐬
bool CCalcSolarPotentialMng::createPointData_Build(

	std::vector<CPointBase>& vecPoint3d,
	const AREADATA& areaData,
	const BLDGLIST& bldList,
	const CPotentialDataMap& bldDataMap,
	double outMeshsize,
	const eOutputImageTarget& eTarget
)
{
	// 1m���b�V���̒������W(x,y)�Əo�͂��������˗ʎZ�o����(z)
	bool ret = false;

	vecPoint3d.clear();

	for (const auto& [buildId, bldData] : bldDataMap)
	{
		if (areaData.targetBuildings.find(bldList.meshID) == areaData.targetBuildings.end())	continue;

		BUILDINGS build;
		const auto& tmpBuildings = areaData.targetBuildings.at(bldList.meshID);
		for (const auto& bldg : tmpBuildings)
		{
			if (buildId == bldg->building)
			{
				build = *bldg;
				break;
			}
		}

		int surfaceCount = (int)build.roofSurfaceList.size();
		for (int kc = 0; kc < surfaceCount; kc++)
		{
			ROOFSURFACES surface = build.roofSurfaceList[kc];
			std::string surfaceId = surface.roofSurfaceId;
			if (bldData.mapSurface.count(surfaceId) == 0)	continue;

			const CSurfaceData& surfaceData = bldData.mapSurface.at(surfaceId);
			double dMinX = surfaceData.bbPos[0].x, dMaxX = surfaceData.bbPos[3].x;
			double dMinY = surfaceData.bbPos[0].y, dMaxY = surfaceData.bbPos[3].y;
			int iH = (int)std::ceil((dMaxY - dMinY) / outMeshsize);
			int iW = (int)std::ceil((dMaxX - dMinX) / outMeshsize);

			for (int h = 0; h < iH; h++)
			{
				double curtY = dMinY + h * (double)outMeshsize;
				if (CEpsUtil::Less(dMaxY, curtY)) curtY = dMaxY;

				for (int w = 0; w < iW; w++)
				{
					double curtX = dMinX + w * (double)outMeshsize;
					if (CEpsUtil::Less(dMaxX, curtX)) curtX = dMaxX;

					// �������Ƃɓ��O���肷��
					for (const auto& roofSurfaces : surface.roofSurfaceList)
					{
						// ���O����p
						int iCountPoint = (int)roofSurfaces.posList.size();
						CPoint2D* ppoint = new CPoint2D[iCountPoint];
						for (int n = 0; n < iCountPoint; n++)
						{
							ppoint[n] = CPoint2D(roofSurfaces.posList[n].x, roofSurfaces.posList[n].y);
						}
						CPoint2D target2d(curtX, curtY);
						bool bInside = CGeoUtil::IsPointInPolygon(target2d, iCountPoint, ppoint);
						delete[] ppoint;

						if (!bInside)	continue;

						if (m_pUIParam->pSolarPotentialParam->pRoof->bExclusionInterior)
						{
							// interior�ʂ����O����ꍇ�͓�����`�悵�Ȃ�
							for (const auto& interior : roofSurfaces.interiorList)
							{
								iCountPoint = (int)interior.posList.size();
								CPoint2D* pinpoint = new CPoint2D[iCountPoint];
								for (int n = 0; n < iCountPoint; n++)
								{
									pinpoint[n] = CPoint2D(interior.posList[n].x, interior.posList[n].y);
								}

								bInside = !CGeoUtil::IsPointInPolygon(target2d, iCountPoint, pinpoint);
								delete[] pinpoint;

								if (!bInside)	break;
							}
						}

						// Z�l�ɓ��˗ʎZ�o���ʂ�ݒ肷��
						if (bInside)
						{
							switch (eTarget)
							{
							case eOutputImageTarget::SOLAR_RAD:
								vecPoint3d.emplace_back(CPointBase(curtX, curtY, surfaceData.solarRadiationUnit));
								break;

							case eOutputImageTarget::SOLAR_POWER:
								vecPoint3d.emplace_back(CPointBase(curtX, curtY, bldData.solarPowerUnit));
								break;

							default:
								return false;
							}
						}
					}
				}
			}
		}

	}

	double tmpMinX = DBL_MAX, tmpMaxX = -DBL_MAX, tmpMinY = DBL_MAX, tmpMaxY = -DBL_MAX;
	for (const auto& point : vecPoint3d)
	{
		tmpMinX = (tmpMinX > point.x) ? point.x : tmpMinX;
		tmpMaxX = (tmpMaxX < point.x) ? point.x : tmpMaxX;
		tmpMinY = (tmpMinY > point.y) ? point.y : tmpMinY;
		tmpMaxY = (tmpMaxY < point.y) ? point.y : tmpMaxY;
	}

	// 3�����b�V���̃T�C�Y�ŉ摜�o�͂������̂Ŏl���̍��W��ǉ�
	bool addLB, addLT, addRB, addRT;
	addLB = ((tmpMinX > bldList.bbMinX && tmpMinX < (bldList.bbMinX + 1.0)) && (tmpMinY > bldList.bbMinY && tmpMinY < (bldList.bbMinY + 1.0))) ? false : true;
	addLT = ((tmpMinX > bldList.bbMinX && tmpMinX < (bldList.bbMinX + 1.0)) && (tmpMaxY < bldList.bbMaxY && tmpMaxY > (bldList.bbMaxY - 1.0))) ? false : true;
	addRB = ((tmpMaxX < bldList.bbMaxX && tmpMaxX > (bldList.bbMaxX - 1.0)) && (tmpMinY > bldList.bbMinY && tmpMinY < (bldList.bbMinY + 1.0))) ? false : true;
	addRT = ((tmpMaxX < bldList.bbMaxX && tmpMaxX > (bldList.bbMaxX - 1.0)) && (tmpMaxY < bldList.bbMaxY && tmpMaxY > (bldList.bbMaxY - 1.0))) ? false : true;
	// ����
	if (addLB)	vecPoint3d.push_back(CPointBase(bldList.bbMinX + 0.5, bldList.bbMinY + 0.5, DEF_IMG_NODATA));
	if (addLT)	vecPoint3d.push_back(CPointBase(bldList.bbMinX + 0.5, bldList.bbMaxY - 0.5, DEF_IMG_NODATA));
	if (addRB)	vecPoint3d.push_back(CPointBase(bldList.bbMaxX - 0.5, bldList.bbMinY + 0.5, DEF_IMG_NODATA));
	if (addRT)	vecPoint3d.push_back(CPointBase(bldList.bbMaxX - 0.5, bldList.bbMaxY - 0.5, DEF_IMG_NODATA));

	if (vecPoint3d.size() > 0)	ret = true;

	return ret;
}


// ���˗ʎZ�o���ʂ̃|�C���g�f�[�^���쐬(�y�n)
bool CCalcSolarPotentialMng::createPointData_Land(
	std::vector<CPointBase>& vecPoint3d,
	const AREADATA& areaData,
	const CPotentialData& landData,
	const eOutputImageTarget& eTarget
)
{
	// 1m���b�V���̒������W(x,y)�Əo�͂��������˗ʎZ�o����(z)
	bool ret = false;

	vecPoint3d.clear();

	if (landData.mapSurface.size() == 0)	return false;

	vecPoint3d.insert(vecPoint3d.end(), areaData.pointMemData.begin(), areaData.pointMemData.end());
	for (auto& point : vecPoint3d)
	{
		if (point.z == DEF_IMG_NODATA) continue;

		CPoint2D target2d(point.x + 0.5, point.y + 0.5);

		// ���b�V���P�ʂō쐬
		bool inside = false;

		for (const auto& [surfaceId, surfaceData] : landData.mapSurface)
		{
			double dx = target2d.x - surfaceData.center.x;
			double dy = target2d.y - surfaceData.center.y;
			if (calcLength(dx, dy, 0.0) > surfaceData.meshSize)
			{
				// 1���b�V���T�C�Y��藣��Ă�����X�L�b�v
				continue;
			}

			for (auto& mesh : surfaceData.vecMeshData)
			{
				CPoint2D* ppoint = new CPoint2D[4];
				int n = 0;
				for (auto& pos : mesh.meshPos)
				{
					ppoint[n] = CPoint2D(pos.x, pos.y);
					n++;
				}
				inside = CGeoUtil::IsPointInPolygon(target2d, 4, ppoint);
				delete[] ppoint;

				if (!inside)
				{
					continue;
				}

				switch (eTarget)
				{
				case eOutputImageTarget::SOLAR_RAD:
					point.z = mesh.solarRadiationUnit;
					break;

				case eOutputImageTarget::SOLAR_POWER:
					point.z = mesh.solarPowerUnit;
					break;

				default:
					return false;
				}

				break;
			}

			if (inside)
			{
				break;
			}
		}

		if (!inside)
		{
			point.z = DEF_IMG_NODATA;
		}
	}

	double tmpMinX = DBL_MAX, tmpMaxX = -DBL_MAX, tmpMinY = DBL_MAX, tmpMaxY = -DBL_MAX;
	for (const auto& point : vecPoint3d)
	{
		tmpMinX = (tmpMinX > point.x) ? point.x : tmpMinX;
		tmpMaxX = (tmpMaxX < point.x) ? point.x : tmpMaxX;
		tmpMinY = (tmpMinY > point.y) ? point.y : tmpMinY;
		tmpMaxY = (tmpMaxY < point.y) ? point.y : tmpMaxY;
	}

	// �I�������G���A��BB�ŉ摜�o�͂������̂Ŏl���̍��W��ǉ�
	bool addLB, addLT, addRB, addRT;
	addLB = ((tmpMinX > areaData.bbMinX && tmpMinX < (areaData.bbMinX + 1.0)) && (tmpMinY > areaData.bbMinY && tmpMinY < (areaData.bbMinY + 1.0))) ? false : true;
	addLT = ((tmpMinX > areaData.bbMinX && tmpMinX < (areaData.bbMinX + 1.0)) && (tmpMaxY < areaData.bbMaxY&& tmpMaxY >(areaData.bbMaxY - 1.0))) ? false : true;
	addRB = ((tmpMaxX < areaData.bbMaxX&& tmpMaxX >(areaData.bbMaxX - 1.0)) && (tmpMinY > areaData.bbMinY && tmpMinY < (areaData.bbMinY + 1.0))) ? false : true;
	addRT = ((tmpMaxX < areaData.bbMaxX&& tmpMaxX >(areaData.bbMaxX - 1.0)) && (tmpMaxY < areaData.bbMaxY&& tmpMaxY >(areaData.bbMaxY - 1.0))) ? false : true;
	// ����
	if (addLB)	vecPoint3d.push_back(CPointBase(areaData.bbMinX + 0.5, areaData.bbMinY + 0.5, DEF_IMG_NODATA));
	if (addLT)	vecPoint3d.push_back(CPointBase(areaData.bbMinX + 0.5, areaData.bbMaxY - 0.5, DEF_IMG_NODATA));
	if (addRB)	vecPoint3d.push_back(CPointBase(areaData.bbMaxX - 0.5, areaData.bbMinY + 0.5, DEF_IMG_NODATA));
	if (addRT)	vecPoint3d.push_back(CPointBase(areaData.bbMaxX - 0.5, areaData.bbMaxY - 0.5, DEF_IMG_NODATA));

	if (vecPoint3d.size() > 0)	ret = true;

	return ret;
}

// �摜�o��
bool CCalcSolarPotentialMng::outputImage(
	const std::wstring strFilePath,
	std::vector<CPointBase>* pvecPoint3d,
	double outMeshsize,
	const eOutputImageTarget& eTarget
)
{
	if (IsCancel())		return false;

	bool ret = false;

	std::wstring strColorSettingPath = L"Assets\\ColorSettings\\Template\\" + getColorSettingFileName(eTarget);

	CTiffDataManager tiffDataMng;
	tiffDataMng.SetColorSetting(strColorSettingPath);
	tiffDataMng.SetMeshSize((float)outMeshsize);
	tiffDataMng.SetNoDataVal(DEF_IMG_NODATA);
	tiffDataMng.SetEPSGCode(DEF_EPSGCODE);
	tiffDataMng.SetFilePath(strFilePath);
	ret = tiffDataMng.AddTiffData(pvecPoint3d);

	// TIFF�摜�쐬
	if (ret)
	{
		ret &= tiffDataMng.Create();
	}

	// JPEG�o��
	if (ret && eTarget == eOutputImageTarget::SOLAR_RAD)
	{
		ret &= CImageUtil::ConvertTiffToJpeg(strFilePath);
	}

	return ret;
}

std::wstring CCalcSolarPotentialMng::getColorSettingFileName(const eOutputImageTarget& eTarget)
{
	std::wstring strColorSettingPath = L"";

	if (eTarget == eOutputImageTarget::SOLAR_POWER)
	{
		switch (m_pUIParam->eAnalyzeDate)
		{
		case eDateType::OneMonth:
			strColorSettingPath = L"colorSetting_SolarPower_month.txt";
			break;
		case eDateType::OneDay:
			strColorSettingPath = L"colorSetting_SolarPower_day.txt";
			break;
		case eDateType::Summer:
			strColorSettingPath = L"colorSetting_SolarPower_summer.txt";
			break;
		case eDateType::Winter:
			strColorSettingPath = L"colorSetting_SolarPower_winter.txt";
			break;
		case eDateType::Year:
		default:
			strColorSettingPath = L"colorSetting_SolarPower.txt";
			break;
		}
	}
	else if (eTarget == eOutputImageTarget::SOLAR_RAD)
	{
		switch (m_pUIParam->eAnalyzeDate)
		{
		case eDateType::OneMonth:
			strColorSettingPath = L"colorSetting_SolarRad_month.txt";
			break;
		case eDateType::OneDay:
			strColorSettingPath = L"colorSetting_SolarRad_day.txt";
			break;
		case eDateType::Summer:
			strColorSettingPath = L"colorSetting_SolarRad_summer.txt";
			break;
		case eDateType::Winter:
			strColorSettingPath = L"colorSetting_SolarRad_winter.txt";
			break;
		case eDateType::Year:
		default:
			strColorSettingPath = L"colorSetting_SolarRad.txt";
			break;
		}
	}

	return strColorSettingPath;
}

// ��͑Ώۂ��Ƃ̏o�̓t�H���_�����擾
const std::wstring CCalcSolarPotentialMng::GetDirName_AnalyzeTargetDir(eAnalyzeTarget target)
{
	std::wstring wstr = L"";

	switch (target)
	{
	case eAnalyzeTarget::ROOF:
		wstr = GetFUtil()->Combine(m_pUIParam->strOutputDirPath, L"����");
		break;

	case eAnalyzeTarget::LAND:
		wstr = GetFUtil()->Combine(m_pUIParam->strOutputDirPath, L"�y�n");
		break;

	default:
		break;
	}

	// �t�H���_�쐬
	if (!GetFUtil()->IsExistPath(wstr))
	{
		if (CreateDirectory(wstr.c_str(), NULL) == FALSE)
		{
			return L"";
		}
	}

	return wstr;
}

// �����E�G���A���Ɨ\�����d��CSV
const std::wstring CCalcSolarPotentialMng::GetFileName_SolarPotentialCsv(eAnalyzeTarget target)
{
	std::wstring wstr = L"";

	switch (target)
	{
	case eAnalyzeTarget::ROOF:
		wstr = L"�������Ɨ\�����d��.csv";
		break;

	case eAnalyzeTarget::LAND:
		wstr = L"�y�n���Ɨ\�����d��.csv";
		break;

	default:
		break;
	}

	return wstr;
}

// (�y�n�ʂ̂�)���b�V�����Ɨ\�����d��CSV
const std::wstring CCalcSolarPotentialMng::GetFileName_MeshPotentialCsv(eAnalyzeTarget target)
{
	return L"���b�V�����Ɨ\�����d��.csv";
}

// (�y�n�ʂ̂�)�G���A���Ɨ\�����d��SHP
const std::wstring CCalcSolarPotentialMng::GetFileName_SolarPotentialShp()
{
	return L"�y�n���Ɨ\�����d��.shp";
}

// �����ʕʁE�y�n�ʃ��b�V���ʓ��˗�CSV
const std::wstring CCalcSolarPotentialMng::GetFileName_MeshSolarRadCsv(eAnalyzeTarget target)
{
	std::wstring wstr = L"";

	switch (target)
	{
	case eAnalyzeTarget::ROOF:
		wstr = L"�����ʕʓ��˗�.csv";
		break;

	case eAnalyzeTarget::LAND:
		wstr = L"�y�n�ʃ��b�V���ʓ��˗�.csv";
		break;

	default:
		break;
	}

	return wstr;
}

bool CCalcSolarPotentialMng::outputLegendImage()
{
	bool ret = false;

	std::wstring strColorSettingPath = L"";
	std::wstring strMdlPath = CFileUtil::GetModulePathW();

	// ���˗�
	strColorSettingPath = L"Assets\\ColorSettings\\Template\\" + getColorSettingFileName(eOutputImageTarget::SOLAR_RAD);
	ret = CImageUtil::CreateLegendImage(strColorSettingPath, L"���˗�(kWh/m2)");
	if (ret)
	{
		std::wstring colorSetting = CFileUtil::Combine(strMdlPath, strColorSettingPath);
		std::wstring srcPath = GetFUtil()->ChangeFileNameExt(colorSetting, L".jpg");

		std::wstring tmpPath = CFileUtil::Combine(m_pUIParam->strOutputDirPath, getColorSettingFileName(eOutputImageTarget::SOLAR_RAD));
		std::wstring dstPath = GetFUtil()->ChangeFileNameExt(tmpPath, L".jpg");

		if (MoveFile(srcPath.c_str(), dstPath.c_str()) == FALSE)
		{
			return false;
		}
	}

	// ���d��
	strColorSettingPath = L"Assets\\ColorSettings\\Template\\" + getColorSettingFileName(eOutputImageTarget::SOLAR_POWER);
	ret &= CImageUtil::CreateLegendImage(strColorSettingPath, L"���d��(kWh/m2)");
	if (ret)
	{
		std::wstring colorSetting = CFileUtil::Combine(strMdlPath, strColorSettingPath);
		std::wstring srcPath = GetFUtil()->ChangeFileNameExt(colorSetting, L".jpg");

		std::wstring tmpPath = CFileUtil::Combine(m_pUIParam->strOutputDirPath, getColorSettingFileName(eOutputImageTarget::SOLAR_POWER));
		std::wstring dstPath = GetFUtil()->ChangeFileNameExt(tmpPath, L".jpg");

		if (MoveFile(srcPath.c_str(), dstPath.c_str()) == FALSE)
		{
			return false;
		}
	}

	return ret;
}


// �S�͈͂ɂ�������˗ʁE���d��CSV���o��
bool CCalcSolarPotentialMng::outputAllAreaResultCSV()
{
	if (m_pmapResultData->empty()) return false;
	if (IsCancel())	return false;

	if (m_pUIParam->bExecBuild)
	{
		std::vector<std::string> outputList{};
		for (auto& [areaId, resultdata]: *m_pmapResultData)
		{
			if (!resultdata.pBuildMap)	continue;

			// 3�����b�V�����Ƃɏo��
			for (auto& [meshId, dataMap] : *resultdata.pBuildMap)
			{
				for (auto& [buildId, bldData] : dataMap)
				{
					std::string strLine = CStringEx::Format("%s,%s,%s,%f,%f,%f,%f,%f,%f,%f,%f,%f",
						areaId.c_str(),
						meshId.c_str(),
						buildId.c_str(),
						bldData.solarRadiationTotal,
						bldData.solarRadiationUnit,
						bldData.solarPower,
						bldData.solarPowerUnit,
						bldData.GetAllArea(),
						bldData.panelArea,
						bldData.center.x,
						bldData.center.y,
						bldData.center.z
					);
					outputList.push_back(strLine);
				}
			}
		}

		// �������ʂ�����Ώ�������
		if (outputList.size() > 0)
		{
			// ��͑Ώۂ��Ƃ̏o�͐�t�H���_��
			std::wstring wstrAnalyzeTargetDir = GetDirName_AnalyzeTargetDir(eAnalyzeTarget::ROOF);
			if (wstrAnalyzeTargetDir.empty())	return false;

			// �o��CSV�t�@�C��
			std::wstring wstrFilePath = GetFUtil()->Combine(wstrAnalyzeTargetDir, GetFileName_SolarPotentialCsv(eAnalyzeTarget::ROOF));

			// �w�b�_��
			std::string header = "��̓G���AID,3�����b�V��ID,����ID,�\�����˗�(kWh),�\�����˗�(kWh/m2),�\�����d��(kWh),�\�����d��(kWh/m2),�����ʖʐ�(m2),PV�ݒu�ʐ�(m2),X,Y,Z";

			CFileIO file;
			if (!file.Open(wstrFilePath, L"w"))
			{
				return false;
			}

			// �w�b�_��
			file.WriteLineA(header);

			// �f�[�^��
			for (const auto& str : outputList)
			{
				file.WriteLineA(str);
			}

			file.Close();
		}

	}

	if (m_pUIParam->bExecLand)
	{

		// ��͑Ώۂ��Ƃ̏o�͐�t�H���_��
		std::wstring wstrAnalyzeTargetDir = GetDirName_AnalyzeTargetDir(eAnalyzeTarget::LAND);
		if (wstrAnalyzeTargetDir.empty())	return false;

		// �G���A����CSV
		{
			std::vector<std::string> outputList{};
			for (auto& resultMap : *m_pmapResultData)
			{
				std::string areaId = resultMap.first;
				if (!resultMap.second.pLandData)	continue;
				CPotentialData landData = *resultMap.second.pLandData;

				std::string strLine = CStringEx::Format("%s,%f,%f,%f,%f,%f,%f,%f,%f,%f",
					areaId.c_str(),
					landData.solarRadiationTotal,
					landData.solarRadiationUnit,
					landData.solarPower,
					landData.solarPowerUnit,
					landData.GetAllArea(),
					landData.panelArea,
					landData.center.x,
					landData.center.y,
					landData.center.z
				);
				outputList.push_back(strLine);
			}

			// �y�n���ʂ�����Ώ�������
			if (outputList.size() > 0)
			{
				// �o��CSV�t�@�C��
				std::wstring wstrFilePath = GetFUtil()->Combine(wstrAnalyzeTargetDir, GetFileName_SolarPotentialCsv(eAnalyzeTarget::LAND));

				// �w�b�_��
				std::string header = "��̓G���AID,�\�����˗�(kWh),�\�����˗�(kWh/m2),�\�����d��(kWh),�\�����d��(kWh/m2),�y�n�ʖʐ�(m2),PV�ݒu�ʐ�(m2),X,Y,Z";

				CFileIO file;
				if (!file.Open(wstrFilePath, L"w"))
				{
					return false;
				}

				// �w�b�_��
				file.WriteLineA(header);

				// �f�[�^��
				for (const auto& str : outputList)
				{
					file.WriteLineA(str);
				}

				file.Close();
			}
		}

		// ���b�V������CSV
		{
			std::vector<std::string> outputList{};
			for (auto& resultMap : *m_pmapResultData)
			{
				std::string areaId = resultMap.first;
				if (!resultMap.second.pLandData)	continue;
				CPotentialData& landData = *resultMap.second.pLandData;

				auto& surfaceMap = landData.mapSurface;

				for (auto& surfaceData : surfaceMap)
				{
					auto& vecMesh = surfaceData.second.vecMeshData;

					for (auto& mesh : vecMesh)
					{
						std::string strLine = CStringEx::Format("%s,%s,%f,%f,%f,%f,%f,%f",
							areaId.c_str(),
							mesh.meshId.c_str(),
							mesh.solarRadiationUnit,
							mesh.solarPowerUnit,
							mesh.area,
							mesh.centerMod.x,
							mesh.centerMod.y,
							mesh.centerMod.z
						);
						outputList.push_back(strLine);
					}
				}
			}

			// �y�n���ʂ�����Ώ�������
			if (outputList.size() > 0)
			{
				// �o��CSV�t�@�C��
				std::wstring wstrFilePath = GetFUtil()->Combine(wstrAnalyzeTargetDir, GetFileName_MeshPotentialCsv(eAnalyzeTarget::LAND));

				// �w�b�_��
				std::string header = "�G���AID,�y�n�ʃ��b�V��ID,�\�����˗�(kWh/m2),�\�����d��(kWh/m2),PV�ݒu�ʐ�(m2),X,Y,Z";

				CFileIO file;
				if (!file.Open(wstrFilePath, L"w"))
				{
					return false;
				}

				// �w�b�_��
				file.WriteLineA(header);

				// �f�[�^��
				for (const auto& str : outputList)
				{
					file.WriteLineA(str);
				}

				file.Close();
			}
		}

		// SHP
		if (!outputLandShape())
		{
			return false;
		}

	}

	return true;
}

// ���ʓ��˗�CSV�o��
bool CCalcSolarPotentialMng::outputMonthlyRadCSV(
	const CPotentialDataMap& dataMap, 
	const std::wstring& wstrOutDir
)
{
	if (IsCancel())	return false;

	if (!GetFUtil()->IsExistPath(wstrOutDir))
	{
		if (CreateDirectory(wstrOutDir.c_str(), NULL) == FALSE)
		{
			return false;
		}
	}
	// �f�o�b�O�p�o��
	std::wstring strPath = GetFUtil()->Combine(wstrOutDir, L"���ʓ��˗�_�p�x�␳.csv");

	CFileIO file;
	if (!file.Open(strPath, L"w"))
	{
		return false;
	}

	// �w�b�_��
	if (!file.WriteLineA("ID,��ID,���b�V��ID,�N,��,���˗�(Wh/m2),���˗�(MJ/m2)"))
	{
		return false;
	}

	for (auto& [id, bldData] : dataMap)
	{
		for (auto& [surfaceId, surfaceData] : bldData.mapSurface)
		{
			for (auto& mesh : surfaceData.vecMeshData)
			{
				for (int month = 1; month <= 12; month++)
				{
					double val = mesh.solarRadiation[month - 1];

					std::string strLine = CStringEx::Format("%s,%s,%s,%d,%d,%f,%f",
						id.c_str(),
						surfaceId.c_str(),
						mesh.meshId.c_str(),
						GetYear(),
						month,
						val,
						val / 1000.0 * 3.6
					);
					file.WriteLineA(strLine);
				}
			}
		}
	}

	file.Close();

	return true;
}


// �ʂ��Ƃ̓��˗�CSV�o��
bool CCalcSolarPotentialMng::outputSurfaceRadCSV(
	const eAnalyzeTarget target,
	const CPotentialDataMap& dataMap,
	const std::wstring& wstrOutDir			// �o�̓t�H���_
)
{
	if (IsCancel())	return false;

	if (!GetFUtil()->IsExistPath(wstrOutDir))
	{
		if (CreateDirectory(wstrOutDir.c_str(), NULL) == FALSE)
		{
			return false;
		}
	}

	std::wstring strPath = GetFUtil()->Combine(wstrOutDir, GetFileName_MeshSolarRadCsv(target));

	CFileIO file;
	if (!file.Open(strPath, L"w"))
	{
		return false;
	}

	switch (target)
	{
	case eAnalyzeTarget::ROOF:
	{
		// �w�b�_��
		if (!file.WriteLineA("����ID,������ID,�N,�P�ʖʐς�����̓��˗�(kWh/m2),��(MJ/m2),�ΏۖʑS�̂̓��˗�(kWh),��(MJ),���ʊp(����),�X�Ίp(����),���ʊp(�␳),�X�Ίp(�␳),x,y,z"))
		{
			file.Close();
			return false;
		}

		for (const auto& [id, bldData]: dataMap)
		{
			for (auto& [surfaceId, surfaceData] : bldData.mapSurface)
			{
				std::string strLine = CStringEx::Format("%s,%s,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f",
					id.c_str(),
					surfaceId.c_str(),
					GetYear(),
					surfaceData.solarRadiationUnit,
					surfaceData.solarRadiationUnit * 3.6,
					surfaceData.solarRadiation,
					surfaceData.solarRadiation * 3.6,
					surfaceData.azDegreeAve,
					surfaceData.slopeDegreeAve,
					surfaceData.azModDegree,
					surfaceData.slopeModDegree,
					surfaceData.center.x,
					surfaceData.center.y,
					surfaceData.center.z
				);
				file.WriteLineA(strLine);
			}
		}

		break;
	}

	case eAnalyzeTarget::LAND:
	{
		// �w�b�_��
		if (!file.WriteLineA("�G���AID,���b�V��ID,�N,1���b�V��������̓��˗�(kWh/m2),��(MJ/m2),�y�n�ʑS�̂̓��˗�(kWh),��(MJ),���ʊp(����),�X�Ίp(����),���ʊp(�␳),�X�Ίp(�␳),x,y,z"))
		{
			file.Close();
			return false;
		}

		for (auto& [id, landData] : dataMap)
		{
			for (auto& [surfaceId, surfaceData] : landData.mapSurface)
			{
				for (auto& mesh : surfaceData.vecMeshData)
				{
					// �f�o�b�O�p�o��
					std::string strLine = CStringEx::Format("%s,%s,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f",
						id.c_str(),
						mesh.meshId.c_str(),
						GetYear(),
						mesh.solarRadiationUnit,
						mesh.solarRadiationUnit * 3.6,
						surfaceData.solarRadiation,
						surfaceData.solarRadiation * 3.6,
						surfaceData.azDegreeAve,
						surfaceData.slopeDegreeAve,
						surfaceData.azModDegree,
						surfaceData.slopeModDegree,
						mesh.centerMod.x,
						mesh.centerMod.y,
						mesh.centerMod.z
					);
					file.WriteLineA(strLine);
				}
			}
		}

		break;
	}

	default:
		break;
	}


	file.Close();

	return true;
}


// �������Ƃ̕��ʊp���ԃf�[�^�o��
bool CCalcSolarPotentialMng::outputAzimuthDataCSV()
{
	if (m_pmapResultData->empty())	return false;
	if (IsCancel())	return false;

	bool ret = false;

	// �o�̓p�X
	std::wstring strCsvPath = GetINIParam()->GetAzimuthCSVPath();
	std::wstring strTempDir = GetFUtil()->GetParentDir(m_pUIParam->strOutputDirPath);
	strTempDir = GetFUtil()->GetParentDir(strTempDir) + L"system";
	std::wstring strPath = GetFUtil()->Combine(strTempDir, strCsvPath);

	// �o�͗p�t�H���_�쐬
	std::wstring strParentDir = GetFUtil()->GetParentDir(strPath);
	if (!GetFUtil()->IsExistPath(strParentDir))
	{
		if (CreateDirectory(strParentDir.c_str(), NULL) == FALSE)
		{
			return false;
		}
	}

	CFileIO file;
	if (!file.Open(strPath, L"w"))
	{
		return false;
	}

	// �w�b�_��
	if (!file.WriteLineA("�G���AID,3�����b�V��ID,����ID,���ꉮ���ʐ�,���ʊp(���ϒl)"))
	{
		return false;
	}

	// ���ʊp�f�[�^��������
	for (const auto& [areaId, result] : *m_pmapResultData)
	{
		// ����
		if (m_pUIParam->bExecBuild && result.pBuildMap)
		{
			// 3�����b�V�����Ƃɏo��
			for (auto& [meshId, dataMap] : *result.pBuildMap)
			{

				for (auto& [buildId, bldData] : dataMap)
				{
					int roofsize = (int)bldData.mapSurface.size();
					if (roofsize == 0)	continue;

					std::string strAzimuths = "";
					for (auto val : bldData.mapSurface)
					{
						CSurfaceData roofData = val.second;
						std::string strTemp = CStringEx::Format("%f,", roofData.azModDegree);
						strAzimuths += strTemp;
					}

					// �G���AID, ���b�V��ID, ����ID, ���ꉮ���ʐ�, ���ʊp(���ϒl)�E�E�E"
					std::string strLine = CStringEx::Format("%s,%s,%s,%d,%s", areaId.c_str(), meshId.c_str(), buildId.c_str(), roofsize, strAzimuths.c_str());

					file.WriteLineA(strLine);
				}
			}
		}

		// �y�n
		if (m_pUIParam->bExecLand && result.pLandData)
		{
			CPotentialData& landData = *result.pLandData;

			int size = (int)landData.mapSurface.size();
			if (size != 0)
			{
				// �G���A���Ƃɕ��ʊp�͓����ɂȂ�̂Ő擪�f�[�^�����o��
				auto itr = landData.mapSurface.begin();
				auto val = *itr;
				std::string strAzimuths = CStringEx::Format("%f,", val.second.azModDegree);

				// �G���AID, ���b�V��ID, ����ID, ���ꉮ���ʐ�, ���ʊp(���ϒl)�E�E�E"
				std::string strLine = CStringEx::Format("%s,%s,%s,%d,%s", areaId.c_str(), areaId.c_str(), areaId.c_str(), 1, strAzimuths.c_str());

				file.WriteLineA(strLine);
			}
		}

	}

	file.Close();

	return true;

}

// SHP�ɕt�^
bool CCalcSolarPotentialMng::outputLandShape()
{
	if (m_pmapResultData->empty()) return false;
	if (IsCancel())	return false;

	// ��͑Ώۂ��Ƃ̏o�͐�t�H���_��
	std::wstring wstrAnalyzeTargetDir = GetDirName_AnalyzeTargetDir(eAnalyzeTarget::LAND);
	if (wstrAnalyzeTargetDir.empty())	return false;

	std::wstring wstrDir = GetFUtil()->Combine(wstrAnalyzeTargetDir, GetDirName_LandShape());
	// �t�H���_���쐬
	if (!GetFUtil()->IsExistPath(wstrDir))
	{
		if (CreateDirectory(wstrDir.c_str(), NULL) == FALSE)
		{
			return false;
		}
	}

	// shp�쐬
	std::string strShpFilePath = CStringEx::ToString(GetFUtil()->Combine(wstrDir, GetFileName_SolarPotentialShp()));
	SHPHandle hSHP = SHPCreate(strShpFilePath.c_str(), SHPT_POLYGON);
	
	// dbf�쐬
	std::string strDbfFilePath = CFileUtil::ChangeFileNameExt(strShpFilePath, ".dbf");
	DBFHandle hDBF = DBFCreate(strDbfFilePath.c_str());

	// ����
	DBFAddField(hDBF, "AreaID", FTString, 255, 0);
	DBFAddField(hDBF, "Name", FTString, 255, 0);
	DBFAddField(hDBF, "�ʐ�", FTDouble, 11, 3);
	DBFAddField(hDBF, "PV�ʐ�", FTInteger, 10, 0);
	DBFAddField(hDBF, "���˗�1", FTDouble, 11, 3);
	DBFAddField(hDBF, "���˗�2", FTDouble, 11, 3);
	DBFAddField(hDBF, "���d��1", FTDouble, 11, 3);
	DBFAddField(hDBF, "���d��2", FTDouble, 11, 3);

	for (const auto& areaData : *m_pvecAllAreaList)
	{
		if (!areaData.analyzeLand)	continue;

		CPotentialData* landData = (*m_pmapResultData)[areaData.areaID].pLandData;
		if (!landData)	continue;

		// �|���S���쐬
		int nVertices = (int)areaData.pos2dList.size();
		double* pX = new double[nVertices];
		double* pY = new double[nVertices];
		double* pZ = new double[nVertices];

		for (int n = 0; n < nVertices; n++)
		{
			pX[n] = areaData.pos2dList[n].x;
			pY[n] = areaData.pos2dList[n].y;
			pZ[n] = 0.0;
		}
		
		SHPObject* pSHPObj = SHPCreateSimpleObject(SHPT_POLYGON, nVertices, pX, pY, pZ);
		SHPWriteObject(hSHP, -1, pSHPObj);

		SHPDestroyObject(pSHPObj);
		delete[] pX;
		delete[] pY;
		delete[] pZ;

		// ������������
		int iRecord = DBFGetRecordCount(hDBF);
		DBFWriteStringAttribute(hDBF, iRecord, 0, areaData.areaID.c_str());
		DBFWriteStringAttribute(hDBF, iRecord, 1, areaData.areaName.c_str());
		DBFWriteDoubleAttribute(hDBF, iRecord, 2, landData->GetAllArea());
		DBFWriteIntegerAttribute(hDBF, iRecord, 3, (int)landData->panelArea);
		DBFWriteDoubleAttribute(hDBF, iRecord, 4, landData->solarRadiationTotal);
		DBFWriteDoubleAttribute(hDBF, iRecord, 5, landData->solarRadiationUnit);
		DBFWriteDoubleAttribute(hDBF, iRecord, 6, landData->solarPower);
		DBFWriteDoubleAttribute(hDBF, iRecord, 7, landData->solarPowerUnit);
	}

	SHPClose(hSHP);
	DBFClose(hDBF);


	// cpg�t�@�C�����o��
	std::string strCpgFilePath = CFileUtil::ChangeFileNameExt(strShpFilePath, ".cpg");
	std::ofstream ofs(strCpgFilePath);
	ofs << "SJIS" << endl;

	return true;
}

// �Ώۖʂ̒��S�ɓ��ˌ����������Ă��邩
bool CCalcSolarPotentialMng::IntersectSurfaceCenter(
	const CVector3D& inputVec,						// ���ˌ�
	const std::vector<CVector3D>& surfaceBB,		// �Ώۖ�BB
	const CVector3D& center,						// �Ώۖʒ��S
	const std::string& strId,						// ������ID(�����̂ݎg�p)
	const vector<BLDGLIST>& neighborBuildings,		// ���ӂ̌������X�g
	const vector<CTriangle>& neighborDems			// ���ӂ̒n�`TIN���X�g
)
{
	// �����̗L������
	const double LIGHT_LENGTH = GetINIParam()->GetNeighborBuildDist_SolarRad();

	// �������b�V���̖@��
	CVector3D n;
	CGeoUtil::OuterProduct(
		CVector3D(surfaceBB[1], surfaceBB[0]),
		CVector3D(surfaceBB[2], surfaceBB[1]), n);
	if (n.z < 0) n *= -1;

	// �Ώۖʂ̗���������ˌ����������Ă���Ƃ��͔��˂��Ȃ��̂ŉ�͏I��
	if (CGeoUtil::InnerProduct(n, inputVec) >= 0.0)
		return false;

	// ���ˌ��̌������Z�o
	// �������b�V�����W�̉�������ɐݒ肷��
	CVector3D inputInverseVec = CGeoUtil::Normalize(inputVec) * ((-1) * LIGHT_LENGTH);
	CVector3D sunPos = center + inputInverseVec;
	CLightRay lightRay(sunPos, CGeoUtil::Normalize(inputVec) * LIGHT_LENGTH);

	// ���ˌ�������̌����Ɏז����ꂸ�ɑΏۖʂɓ����邩�`�F�b�N
	if (intersectBuildings(lightRay, strId, neighborBuildings))
	{
		// �����ɂ������Ă���ꍇ�͌������������Ă��Ȃ��̂ŉ�͏I��
		return false;
	}

	// ���ˌ�������̒n�`�Ɏז����ꂸ�ɑΏۖʂɓ����邩�`�F�b�N
	if (IsEnableDEMData() || m_pUIParam->bExecLand)
	{
		if (intersectLandDEM(lightRay, neighborDems, surfaceBB))
		{
			return false;
		}
	}

	return true;
}

// �����Q�Ɍ������������Ă��邩
bool CCalcSolarPotentialMng::intersectBuildings(
	const CLightRay& lightRay,
	const std::string& strId,
	const std::vector<BLDGLIST>& buildingsList
)
{
	for (const auto& bldglist : buildingsList)
	{
		if (IsCancel())	return false;

		// LOD2
		const vector<BUILDINGS>& buildings = bldglist.buildingList;
		for (const auto& building : buildings)
		{
			if (intersectBuilding(lightRay, strId, building))
			{
				return true;
			}
		}


		// LOD1
		const vector<BUILDINGSLOD1>& buildingsLOD1 = bldglist.buildingListLOD1;
		for (const auto& building : buildingsLOD1)
		{
			if (intersectBuilding(lightRay, building.wallSurfaceList))
			{
				return true;
			}
		}

	}

	return false;
}

// �����Ɍ������������Ă��邩�ǂ���
bool CCalcSolarPotentialMng::intersectBuilding(
	const CLightRay& lightRay,
	const vector<WALLSURFACES>& wallSurfaceList
)
{
	// ������������艓�����Ȃ������ׂ�
	if (!checkDistance(lightRay, wallSurfaceList))
		return false;

	double tempDist;
	CVector3D tempTargetPos;
	for (const auto& wall : wallSurfaceList)
	{
		for (const auto& polygon : wall.wallSurfaceList)
		{
			vector<CVector3D> posList(polygon.posList.size());
			int i = 0;
			for (const auto& pos : polygon.posList)
			{
				posList[i] = CVector3D(pos.x, pos.y, pos.z);
				++i;
			}

			// �����ƃ|���S���̌�_��T��
			if (lightRay.Intersect(posList, &tempTargetPos, &tempDist))
			{
				return true;
			}
		}
	}

	return false;
}

// �����Ɍ������������Ă��邩�ǂ���
bool CCalcSolarPotentialMng::intersectBuilding(
	const CLightRay& lightRay,
	const std::string& strId,
	const BUILDINGS& buildiings
)
{
	// ������������艓�����Ȃ������ׂ�
	if (!checkDistance(lightRay, buildiings.wallSurfaceList))
		return false;

	double tempDist;
	CVector3D tempTargetPos;

	// ����
	for (const auto& roof : buildiings.roofSurfaceList)
	{
		if (strId == roof.roofSurfaceId)	continue;	// ���g�̉����͏��O

		for (const auto& polygon : roof.roofSurfaceList)
		{
			vector<CVector3D> posList(polygon.posList.size());
			int i = 0;
			for (const auto& pos : polygon.posList)
			{
				posList[i] = CVector3D(pos.x, pos.y, pos.z);
				++i;
			}

			// �����ƃ|���S���̌�_��T��
			if (lightRay.Intersect(posList, &tempTargetPos, &tempDist))
			{
				return true;
			}
		}
	}

	// ��
	for (const auto& wall : buildiings.wallSurfaceList)
	{
		for (const auto& polygon : wall.wallSurfaceList)
		{
			vector<CVector3D> posList(polygon.posList.size());
			int i = 0;
			for (const auto& pos : polygon.posList)
			{
				posList[i] = CVector3D(pos.x, pos.y, pos.z);
				++i;
			}

			// �����ƃ|���S���̌�_��T��
			if (lightRay.Intersect(posList, &tempTargetPos, &tempDist))
			{
				return true;
			}
		}
	}

	return false;
}

// �����������͈͓̔�����܂��ɂ܂��͔��肷��
bool CCalcSolarPotentialMng::checkDistance(const CLightRay& lightRay, const vector<WALLSURFACES>& wallSurfaceList)
{
	// �������͈͓������肷�鋗���͈�
	const double LIGHT_LENGTH = GetINIParam()->GetNeighborBuildDist_SolarRad() + 50;	//�]�T����������
	const double SQUARE_LINGHT_LENGTH = LIGHT_LENGTH * LIGHT_LENGTH;

	const CVector3D lightRayPos = lightRay.GetPos();
	const CVector3D lightRayVec = lightRay.GetVector();

	for (const auto& wall : wallSurfaceList)
	{
		for (const auto& polygon : wall.wallSurfaceList)
		{
			for (const auto& pos : polygon.posList)
			{
				double dx = pos.x - lightRayPos.x;
				double dy = pos.y - lightRayPos.y;
				double dz = pos.z - lightRayPos.z;

				// ���ʂ̒��_���t�����ɂ���Ƃ��͔͈͊O�Ƃ���
				double dot = CGeoUtil::InnerProduct(lightRayVec, CVector3D(dx, dy, dz));
				if (dot < 0.0)	continue;

				// �������������Ȃ����`�F�b�N
				double len = calcLength(dx, dy, dz);
				if (len > SQUARE_LINGHT_LENGTH)	continue;

				return true;
			}
		}
	}

	return false;
}

// �אڂ��錚�����擾
void CCalcSolarPotentialMng::GetNeighborBuildings(
	const CVector3D& center,
	std::vector<BLDGLIST>& neighborBuildings
)
{
	const double DIST = GetINIParam()->GetNeighborBuildDist_SolarRad();	// �אڂ���BBox�͈̔�[m]

	// �������SXY
	CVector2D CenterXY(center.x, center.y);

	for (const auto& bldList : m_targetArea->neighborBldgList)
	{
		BLDGLIST tmpBldList = *bldList;
		tmpBldList.buildingList.clear();
		tmpBldList.buildingListLOD1.clear();

		// �͈͓��ɂ��邩
		// LOD2
		for (const auto& build : bldList->buildingList)
		{
			double bbBldMinX = DBL_MAX, bbBldMinY = DBL_MAX;
			double bbBldMaxX = -DBL_MAX, bbBldMaxY = -DBL_MAX;
			double dMaxH = -DBL_MAX;

			// �����S�̂�BB�����߂�
			for (const auto& surface : build.roofSurfaceList)
			{
				for (const auto& member : surface.roofSurfaceList)
				{
					for (const auto& pos : member.posList)
					{
						if (pos.x < bbBldMinX)	bbBldMinX = pos.x;
						if (pos.y < bbBldMinY)	bbBldMinY = pos.y;
						if (pos.x > bbBldMaxX)	bbBldMaxX = pos.x;
						if (pos.y > bbBldMaxY)	bbBldMaxY = pos.y;
						dMaxH = max(pos.z, dMaxH);
					}
				}
			}

			// �Ώۂ��Ⴂ�����͏��O
			if (center.z > dMaxH)	continue;

			double buildCenterX = ((int64_t)bbBldMaxX + (int64_t)bbBldMinX) * 0.5;
			double buildCenterY = ((int64_t)bbBldMaxY + (int64_t)bbBldMinY) * 0.5;
			// ���S���m�̋���
			double tmpdist = CenterXY.Distance(buildCenterX, buildCenterY);
			// DIST�ȓ��̋����̂Ƃ��ߗׂƂ���
			if (tmpdist <= DIST)
			{
				tmpBldList.buildingList.emplace_back(build);
			}
		}

		// LOD1
		for (const auto& build : bldList->buildingListLOD1)
		{
			double bbBldMinX = DBL_MAX, bbBldMinY = DBL_MAX;
			double bbBldMaxX = -DBL_MAX, bbBldMaxY = -DBL_MAX;
			double dMaxH = -DBL_MAX;

			// �����S�̂�BB�����߂�
			for (const auto& wall : build.wallSurfaceList)
			{
				for (const auto& member : wall.wallSurfaceList)
				{
					for (const auto& pos : member.posList)
					{
						if (pos.x < bbBldMinX)	bbBldMinX = pos.x;
						if (pos.y < bbBldMinY)	bbBldMinY = pos.y;
						if (pos.x > bbBldMaxX)	bbBldMaxX = pos.x;
						if (pos.y > bbBldMaxY)	bbBldMaxY = pos.y;
						dMaxH = max(pos.z, dMaxH);
					}
				}
			}

			// �Ώۂ��Ⴂ�����͏��O
			if (center.z > dMaxH)	continue;

			double buildCenterX = ((int64_t)bbBldMaxX + (int64_t)bbBldMinX) * 0.5;
			double buildCenterY = ((int64_t)bbBldMaxY + (int64_t)bbBldMinY) * 0.5;
			// ���S���m�̋���
			double tmpdist = CenterXY.Distance(buildCenterX, buildCenterY);
			// DIST�ȓ��̋����̂Ƃ��ߗׂƂ���
			if (tmpdist <= DIST)
			{
				tmpBldList.buildingListLOD1.emplace_back(build);
			}
		}

		if (tmpBldList.buildingList.empty() && tmpBldList.buildingListLOD1.empty())	continue;

		neighborBuildings.emplace_back(tmpBldList);
	}

}

// �אڂ���DEM���擾
void CCalcSolarPotentialMng::GetNeighborDems(
	const CVector3D& center,
	std::vector<CTriangle>& neighborDems,
	eAnalyzeTarget target
)
{
	const double DIST = GetINIParam()->GetDemDist();	// �Ώ۔͈�[m]

	// ���SXY
	CVector2D CenterXY(center.x, center.y);

	// ���O����DEM����
	double demHeight = target == eAnalyzeTarget::ROOF ? GetINIParam()->GetDemHeight_Build() : GetINIParam()->GetDemHeight_Land();

	for (const auto& member : m_targetArea->neighborDemList)
	{
		for (const auto& triangle : member->posTriangleList)
		{
			// XY���ʂ̏d�S�����߂�
			double x = (triangle.posTriangle[0].x + triangle.posTriangle[1].x + triangle.posTriangle[2].x) / 3.0;
			double y = (triangle.posTriangle[0].y + triangle.posTriangle[1].y + triangle.posTriangle[2].y) / 3.0;
			double z = (triangle.posTriangle[0].z + triangle.posTriangle[1].z + triangle.posTriangle[2].z) / 3.0;
			if (z < demHeight)	continue;

			double tmpdist = CenterXY.Distance(x, y);

			bool bAddList = false;

			// ����
			if (tmpdist <= DIST &&		// ���S�Ƃ̋������Ώ۔͈͓�
				center.z < z)			// ���S��荂���ʒu�ɂ���
			{
				neighborDems.emplace_back(triangle);
			}
		}
	}
}


// �n�`�Ɍ������������Ă��邩�ǂ���
bool CCalcSolarPotentialMng::intersectLandDEM(
	const CLightRay& lightRay,					// ����
	const vector<CTriangle>& tinList,			// �������������Ă��邩�`�F�b�N����n�`��TIN
	const std::vector<CVector3D>& surfaceBB		// �Ώۖʂ�BB
)
{
	double tempDist;
	CVector3D tempTargetPos;

	const CVector3D lightRayPos = lightRay.GetPos();
	const CVector3D lightRayVec = lightRay.GetVector();

	// �������͈͓������肷�鋗���͈�
	const double LIGHT_LENGTH = GetINIParam()->GetNeighborBuildDist_SolarRad() + 50;	//�]�T����������
	const double SQUARE_LINGHT_LENGTH = LIGHT_LENGTH * LIGHT_LENGTH;

	// ���O����p�ɑΏۖʂ̃|���S�����쐬
	CPoint2DPolygon surfacePolygon;
	for (int i = 0; i < surfaceBB.size(); i++)
	{
		CPoint2D p2D = CPoint2D(surfaceBB[i].x, surfaceBB[i].y);
		surfacePolygon.Add(p2D);
	}

	for (const auto& triangle : tinList)
	{
		if (IsCancel())		return false;

		// ���O����p��TIN�̃|���S�����쐬
		CPoint2DPolygon tinPolygon;

		bool bDirect = false;
		for (const auto& pos : triangle.posTriangle)
		{
			double dx = pos.x - lightRayPos.x;
			double dy = pos.y - lightRayPos.y;
			double dz = pos.z - lightRayPos.z;

			CPoint2D p2D = CPoint2D(pos.x, pos.y);
			tinPolygon.Add(p2D);

			// ���ʂ̒��_���t�����ɂ���Ƃ��͔͈͊O�Ƃ���
			double dot = CGeoUtil::InnerProduct(lightRayVec, CVector3D(dx, dy, dz));
			if (dot < 0.0)	continue;

			// �������������Ȃ����`�F�b�N
			double len = calcLength(dx, dy, dz);
			if (len > SQUARE_LINGHT_LENGTH)	continue;

			bDirect = true;
		}
		if (!bDirect)	continue;

		// �Ώۖʂ͈͓̔��ɂ���TIN�͏��O
		CPoint2DPolygon tmpPolygon;
		if (surfacePolygon.GetCrossingPolygon(tinPolygon, tmpPolygon))
		{
			continue;
		}

		vector<CVector3D> posList;
		posList.emplace_back(CVector3D(triangle.posTriangle[0].x, triangle.posTriangle[0].y, triangle.posTriangle[0].z));
		posList.emplace_back(CVector3D(triangle.posTriangle[1].x, triangle.posTriangle[1].y, triangle.posTriangle[1].z));
		posList.emplace_back(CVector3D(triangle.posTriangle[2].x, triangle.posTriangle[2].y, triangle.posTriangle[2].z));

		// �����ƃ|���S���̌�_��T��
		if (lightRay.Intersect(posList, &tempTargetPos, &tempDist))
		{
			return true;
		}

	}

	return false;
}

bool CCalcSolarPotentialMng::IsCancel()
{
	if (!m_isCancel)
	{
		m_isCancel = GetFUtil()->IsExistPath(m_strCancelFilePath);
	}
	return m_isCancel;
}

bool CCalcSolarPotentialMng::IsEnableDEMData()
{
	return m_pUIParam->pInputData->bUseDemData;
}
