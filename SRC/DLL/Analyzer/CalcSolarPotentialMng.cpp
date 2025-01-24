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


// 計算用データ等の初期化
void CCalcSolarPotentialMng::initialize()
{
	// データ取得
	m_pvecAllAreaList = reinterpret_cast<std::vector<AREADATA>*>(GetAllAreaList());

	m_pmapResultData = new CResultDataMap;
	m_pRadiationData = new CAnalysisRadiationCommon;

	// キャンセルファイル
	std::wstring strDir = GetFUtil()->GetParentDir(GetFUtil()->GetParentDir(m_pUIParam->strOutputDirPath));
	m_strCancelFilePath = GetFUtil()->Combine(strDir, CStringEx::ToWString(CANCELFILE));

	// 解析期間を設定
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

// 解放処理
void CCalcSolarPotentialMng::finalize()
{
	if (m_pRadiationData)
	{
		delete m_pRadiationData;
		m_pRadiationData = NULL;
	}

	if (m_pmapResultData)
	{
		m_pmapResultData->clear();
		delete m_pmapResultData;
		m_pmapResultData = NULL;
	}

	if (m_targetArea.m_pvecAllBuildList)
	{
		m_targetArea.m_pvecAllBuildList->clear();
		m_targetArea.m_pvecAllBuildList->shrink_to_fit();
#if _DEBUG
		std::cout << "BuildList Capacity: " << m_targetArea.m_pvecAllBuildList->capacity() << std::endl;
#endif
	}

	if (m_targetArea.m_pvecAllDemList)
	{
		m_targetArea.m_pvecAllDemList->clear();
		m_targetArea.m_pvecAllDemList->shrink_to_fit();
#if _DEBUG
		std::cout << "DemList Capacity: " << m_targetArea.m_pvecAllDemList->capacity() << std::endl;
#endif
	}
}


// 発電ポテンシャル推計(メイン処理)
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

	// 初期化
	initialize();
	assert(m_pRadiationData);
	assert(m_pvecAllAreaList);

	bool ret = false;

	// 月ごとの日照率を計算
	calcMonthlyRate();

	int dataCount = (int)m_pvecAllAreaList->size();
	for (const auto& areaData : *m_pvecAllAreaList)
	{
		if (IsCancel())	break;

		// 範囲ごとのデータを保持しておく
		m_targetArea.m_pvecAllBuildList = reinterpret_cast<std::vector<BLDGLIST>*>(GetAreaBuildList(areaData));
		m_targetArea.m_pvecAllDemList = reinterpret_cast<std::vector<DEMMEMBERS>*>(GetAreaDemList(areaData));

		CResultData dataMap;

		// 建物の解析
		if (areaData.analyzeBuild)
		{
			// 日射量・発電量を計算
			AnalyzeBuild(areaData, dataMap.pBuildMap);
			if (IsCancel())	break;
		}
		// 建物の結果を出力
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

		// 土地の解析
		if (areaData.analyzeLand)
		{
			// 日射量・発電量を計算
			AnalyzeLand(areaData, dataMap.pLandData);
			if (IsCancel())	break;
		}

		// 土地の結果を出力
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
		// 適地判定用の中間ファイル生成(CSV)
		ret &= outputAzimuthDataCSV();

		// 凡例出力
		ret &= outputLegendImage();

		// 予測日射量・発電量
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

	// 解放処理
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
/// 建物解析
/// </summary>
/// <param name="pvecAllBuildList">建物リスト</param>
/// <param name="resultDataMap">解析結果</param>
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
		// 傾斜角・方位角の算出
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

	// 発電量推計設定
	calcSolarPower.SetPperUnit(m_pUIParam->pSolarPotentialParam->dPanelMakerSolarPower);
	calcSolarPower.SetPanelRatio(m_pUIParam->pSolarPotentialParam->dPanelRatio);

	for (auto& [meshId, resultData] : *resultDataMap)
	{
		for (auto& [buildId, data] : resultData)
		{
#ifdef CHRONO
			m_ofs << "------ Target Build: " << buildId << std::endl;

			// 太陽軌道をもとにした日射量の算出
			m_ofs << "ExecBuild Start " << std::endl;
			start = std::chrono::system_clock::now();
			ret = calcSolarRad.ExecBuild(data, m_dateStart, m_dateEnd);
			end = std::chrono::system_clock::now();
			time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 0.001);
			m_ofs << "ExecBuild Time: " << time << " sec" << std::endl;
			if (!ret)	break;

			// 日照率による補正
			m_ofs << "ModifySunRate Start " << std::endl;
			start = std::chrono::system_clock::now();
			ret &= calcSolarRad.ModifySunRate(data);
			end = std::chrono::system_clock::now();
			time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 0.001);
			m_ofs << "ModifySunRate Time: " << time << " sec" << std::endl;
			if (!ret)	break;

			// 年間日射量
			m_ofs << "CalcAreaSolarRadiation Start " << std::endl;
			start = std::chrono::system_clock::now();
			ret &= calcSolarRad.CalcAreaSolarRadiation(data);
			end = std::chrono::system_clock::now();
			time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 0.001);
			m_ofs << "CalcAreaSolarRadiation Time: " << time << " sec" << std::endl;
			if (!ret)	break;

			// 発電量推計
			m_ofs << "CalcEPY Start " << std::endl;
			start = std::chrono::system_clock::now();
			ret = calcSolarPower.CalcEPY(data);
			end = std::chrono::system_clock::now();
			time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 0.001);
			m_ofs << "CalcEPY Time: " << time << " sec" << std::endl;
			if (!ret)	break;
#else
			// 太陽軌道をもとにした日射量の算出
			ret = calcSolarRad.ExecBuild(data, m_dateStart, m_dateEnd);
			if (!ret)	break;

			// 日照率による補正
			ret &= calcSolarRad.ModifySunRate(data);
			if (!ret)	break;

			// 年間日射量
			ret &= calcSolarRad.CalcAreaSolarRadiation(data);
			if (!ret)	break;

			// 発電量推計
			ret = calcSolarPower.CalcEPY(data);
			if (!ret)	break;
#endif
		}

		if (!ret)	break;
	}

	if (IsCancel())
	{	// キャンセル
		m_eExitCode = eExitCode::Cancel;
		return;
	}
	else if (!ret)
	{	// エラー
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
/// 土地解析
/// </summary>
/// <param name="pvecAllLandList"土地リスト</param>
/// <param name="resultDataMap">解析結果</param>
void CCalcSolarPotentialMng::AnalyzeLand(const AREADATA& areaData, CPotentialData*& resultData)
{
	#ifdef CHRONO
	m_ofs << "AnalyzeLand Start " << std::endl;
	auto astart = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point start, end;
	double time;
	#endif

	resultData = new CPotentialData();

	// 傾斜角・方位角の算出(対象データ取得)
	calcLandAspect(areaData, *resultData);
	if (resultData->mapSurface.empty())
	{
		resultData = NULL;
		return;
	}

	CCalcSolarRadiation calcSolarRad(this);
	CCalcSolarPower calcSolarPower;

#ifdef CHRONO
	// 太陽軌道をもとにした日射量の算出
	m_ofs << "ExecLand Start " << std::endl;
	start = std::chrono::system_clock::now();
	bool ret = calcSolarRad.ExecLand(*resultData, m_dateStart, m_dateEnd);
	end = std::chrono::system_clock::now();
	time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 0.001);
	m_ofs << "ExecLand Time: " << time << " sec" << std::endl;

	// 日照率による補正
	m_ofs << "ModifySunRate Start " << std::endl;
	start = std::chrono::system_clock::now();
	ret &= calcSolarRad.ModifySunRate(*resultData);
	end = std::chrono::system_clock::now();
	time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 0.001);
	m_ofs << "ModifySunRate Time: " << time << " sec" << std::endl;

	// 年間日射量
	m_ofs << "CalcAreaSolarRadiation Start " << std::endl;
	start = std::chrono::system_clock::now();
	ret &= calcSolarRad.CalcAreaSolarRadiation(*resultData);
	end = std::chrono::system_clock::now();
	time = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() * 0.001);
	m_ofs << "CalcAreaSolarRadiation Time: " << time << " sec" << std::endl;
#else
	// 太陽軌道をもとにした日射量の算出
	bool ret = calcSolarRad.ExecLand(*resultData, m_dateStart, m_dateEnd);

	// 日照率による補正
	ret &= calcSolarRad.ModifySunRate(*resultData);

	// 年間日射量
	ret &= calcSolarRad.CalcAreaSolarRadiation(*resultData);
#endif

	if (IsCancel())
	{	// キャンセル
		m_eExitCode = eExitCode::Cancel;
		return;
	}
	else if (!ret)
	{	// エラー
		m_eExitCode = eExitCode::Error;
		return;
	}

	// 発電量推計
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
	{	// キャンセル
		m_eExitCode = eExitCode::Cancel;
		return;
	}
	else if (!ret)
	{	// エラー
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

// 傾斜角と方位角を算出
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

		// 建物全体のBB
		double bbBldMinX = DBL_MAX, bbBldMinY = DBL_MAX;
		double bbBldMaxX = -DBL_MAX, bbBldMaxY = -DBL_MAX;

		std::vector<CPointBase> vecAllRoofPos;

		for (const auto& surface : build->roofSurfaceList)
		{	// 屋根面
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

			// 屋根面全体のBB
			double bbRoofMinX = DBL_MAX, bbRoofMinY = DBL_MAX;
			double bbRoofMaxX = -DBL_MAX, bbRoofMaxY = -DBL_MAX;

			std::vector<CPointBase> vecRoofPos;
			
			// 平均傾斜角計算用
			double slopSum = 0;

			// 平均方位角計算用
			double azSum = 0;

			// 面積和
			double areaSum = 0;

			std::vector<CMeshData> tempRoofMesh;

			for (const auto& member : surface.roofSurfaceList)
			{	// 屋根面詳細
				int polygonsize = (int)member.posList.size();
				if (polygonsize == 0)
				{
					continue;
				}
				const std::vector<CPointBase> vecAry(member.posList.begin(), member.posList.end() - 1);	// 構成点の始点と終点は同じ点なので除外する
				vecAllRoofPos.insert(vecAllRoofPos.end(), vecAry.begin(), vecAry.end());
				vecRoofPos.insert(vecRoofPos.end(), vecAry.begin(), vecAry.end());
				
				// 建物全体のBBを求める(除外屋根を含む)
				for (const auto& pos : vecAry)
				{
					if (pos.x < bbBldMinX)	bbBldMinX = pos.x;
					if (pos.y < bbBldMinY)	bbBldMinY = pos.y;
					if (pos.x > bbBldMaxX)	bbBldMaxX = pos.x;
					if (pos.y > bbBldMaxY)	bbBldMaxY = pos.y;
				}

				// 面積算出
				double area = calcArea(vecAry);

				// 法線算出
				CVector3D normal;
				if (!calcRansacPlane(vecAry, normal))
				{
					continue;
				}

				// 傾斜角
				double slopeDeg;
				CGeoUtil::CalcSlope(normal, slopeDeg);

				// 傾き除外判定
				if (slopeDeg > m_pUIParam->pSolarPotentialParam->pRoof->dSlopeDegree)
				{
					continue;
				}

				// 方位角
				double azDeg;
				int JPZONE = GetINIParam()->GetJPZone();
				CGeoUtil::CalcAzimuth(normal, azDeg, JPZONE);

				// 方位除外判定
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

				// 屋根面のBBを求める(除外屋根を含まない)
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

			// 面積が小さい場合除外
			if (areaSum < m_pUIParam->pSolarPotentialParam->pRoof->dArea2D)
			{
				continue;
			}

			// 屋根面BBに高さを付与
			std::vector<CVector3D> bbPos{
				{bbRoofMinX, bbRoofMinY, 0.0},
				{bbRoofMinX, bbRoofMaxY, 0.0},
				{bbRoofMaxX, bbRoofMinY, 0.0},
				{bbRoofMaxX, bbRoofMaxY, 0.0}
			};
			roofData.bbPos = bbPos;

			// 屋根面BBの法線
			CVector3D n;
			CGeoUtil::OuterProduct(
				CVector3D(bbPos[1], bbPos[0]),
				CVector3D(bbPos[2], bbPos[1]), n);
			if (n.z < 0) n *= -1;

			CVector3D p(vecRoofPos[0].x, vecRoofPos[0].y, vecRoofPos[0].z);
			double d = CGeoUtil::InnerProduct(p, n);
			CVector3D inVec(0.0, 0.0, 1.0);
			double dot = CGeoUtil::InnerProduct(n, inVec);

			CVector3D roofcenter;		// 中心の座標
			for (auto& pos : roofData.bbPos)
			{
				// 平面と垂線の交点
				CVector3D p0(pos.x, pos.y, 0.0);
				double t = (d - CGeoUtil::InnerProduct(p0, n)) / dot;
				// 交点
				CVector3D tempPoint = p0 + t * inVec;
				pos.z = tempPoint.z;
				// 中心
				roofcenter += pos;
			}
			roofcenter *= 0.25;

			double dMinDist = DBL_MAX;
			CMeshData neighborMesh;

			// メッシュごとの計算結果格納用データを追加
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

				CVector3D center;		// 中心の座標
				for (auto& meshPos : mesh.meshPos)
				{
					// 平面と垂線の交点
					CVector3D p0(meshPos.x, meshPos.y, 0.0);
					double t = (d - CGeoUtil::InnerProduct(p0, n)) / dot;
					// 交点
					CVector3D tempPoint = p0 + t * inVec;
					meshPos.z = tempPoint.z;
					// 中心
					center += meshPos;
				}
				center *= 0.25;

				mesh.meshId = CStringEx::Format("%s_%d", surface.roofSurfaceId.c_str(), i);
				mesh.center = center;
				mesh.centerMod = center;

				tempRoofMesh.emplace_back(mesh);

				// 面中心との距離が最も近いメッシュを保持
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

			// 対象メッシュがない
			if (tempRoofMesh.size() == 0) { continue; }

			// 屋根面ごとの計算結果格納用データを作成
			{
				roofData.area = areaSum;

				// 傾斜角と方位角の平均設定
				roofData.slopeDegreeAve = slopSum / areaSum;
				roofData.azDegreeAve = azSum / areaSum;

				double slopeMdDeg = roofData.slopeDegreeAve;
				double azMdDeg = roofData.azDegreeAve;

				// 補正
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

					// メッシュ座標を補正
					for (auto& mesh : tempRoofMesh)
					{
						CVector3D centerMd;

						for (auto& meshPos : mesh.meshPos)
						{
							CVector3D orgPos(meshPos - mesh.center);
							orgPos.z = 0.0;
							CVector3D rotPos;
							double theta = slopeMdDeg * _COEF_DEG_TO_RAD;

							// 補正方位に変換
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

							// 中心
							centerMd += meshPos;
						}
						centerMd *= 0.25;
						mesh.centerMod = centerMd;

						// 面中心との距離が最も近いメッシュを保持
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

				// 面の中心
				roofData.center = neighborMesh.centerMod;

				// メッシュリストを追加
				roofData.vecMeshData = tempRoofMesh;

				roofData.meshSize = 1.0;

				// 対象屋根面に追加
				bldData.mapSurface[surface.roofSurfaceId] = roofData;

			}
		}

		// 対象屋根面がない
		if (bldData.mapSurface.size() == 0) { continue; }

		// 建物BBに高さを付与
		std::vector<CVector3D> bbPos{
			{bbBldMinX, bbBldMinY, 0.0},
			{bbBldMinX, bbBldMaxY, 0.0},
			{bbBldMaxX, bbBldMinY, 0.0},
			{bbBldMaxX, bbBldMaxY, 0.0}
		};
		bldData.bbPos = bbPos;

		// 建物全体BBの法線
		CVector3D n;
		CGeoUtil::OuterProduct(
			CVector3D(bbPos[1], bbPos[0]),
			CVector3D(bbPos[2], bbPos[1]), n);
		if (n.z < 0) n *= -1;

		CVector3D p(vecAllRoofPos[0].x, vecAllRoofPos[0].y, vecAllRoofPos[0].z);
		double d = CGeoUtil::InnerProduct(p, n);
		CVector3D inVec(0.0, 0.0, 1.0);
		double dot = CGeoUtil::InnerProduct(n, inVec);

		CVector3D center;		// 中心の座標
		for (auto& pos : bldData.bbPos)
		{
			// 平面と垂線の交点
			CVector3D p0(pos.x, pos.y, 0.0);
			double t = (d - CGeoUtil::InnerProduct(p0, n)) / dot;
			// 交点
			CVector3D tempPoint = p0 + t * inVec;
			pos.z = tempPoint.z;
			// 中心
			center += pos;
		}
		center *= 0.25;
		bldData.center = center;

		bldDataMap[build->building] = bldData;

	}
}

// 傾斜角と方位角を算出
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

		// 面積算出
		double surfacearea = calcArea(vecAry);
		// 面積が小さい場合除外
		if (surfacearea < m_pUIParam->pSolarPotentialParam->pLand->dArea2D)
		{
			continue;
		}

		// 法線算出
		CVector3D normal;
		if (!calcRansacPlane(vecAry, normal))
		{
			continue;
		}

		// 傾斜角
		double slopeDeg;
		CGeoUtil::CalcSlope(normal, slopeDeg);
		// 傾き除外判定
		if (slopeDeg > m_pUIParam->pSolarPotentialParam->pLand->dSlopeAngle)
		{
			continue;
		}

		// 方位角
		double azDeg;
		int JPZONE = GetINIParam()->GetJPZone();
		CGeoUtil::CalcAzimuth(normal, azDeg, JPZONE);

		// 対応する土地面(メッシュ)の計算結果格納用データを追加
		CMeshData mesh;
		for (const auto& pos : vecAry)
		{
			mesh.meshPos.emplace_back(CVector3D(pos.x, pos.y, pos.z));
		}
		mesh.area = surfacearea;
		mesh.meshId = CStringEx::Format("%s_%d", area.areaID.c_str(), meshCount);

		// 土地面(メッシュ)の中心座標を算出
		CVector3D center;
		for (auto& pt : vecAry)
		{
			CVector3D pos(pt.x, pt.y, pt.z);
			center += pos;
		}
		center *= 0.25;
		mesh.center = center;
		mesh.centerMod = center;

		// 土地面ごとの計算結果格納用データを作成
		{
			surfaceData.area = surfacearea;

			// 傾斜角と方位角の平均設定
			surfaceData.slopeDegreeAve = slopeDeg;
			surfaceData.azDegreeAve = azDeg;

			double slopeMdDeg = surfaceData.slopeDegreeAve;
			double azMdDeg = surfaceData.azDegreeAve;

			// 補正
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

				// メッシュ座標を補正
				CVector3D centerMd;

				for (auto& meshPos : mesh.meshPos)
				{
					CVector3D orgPos(meshPos - mesh.center);
					orgPos.z = 0.0;
					CVector3D rotPos;
					double theta = slopeMdDeg * _COEF_DEG_TO_RAD;

					// 補正方位に変換
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

					// 中心
					centerMd += meshPos;
				}
				centerMd *= 0.25;
				mesh.centerMod = centerMd;
			}

			surfaceData.slopeModDegree = slopeMdDeg;
			surfaceData.azModDegree = azMdDeg;

			// 面中心を設定(メッシュと同様)
			surfaceData.center = center;

			// メッシュリストを追加
			surfaceData.vecMeshData.emplace_back(mesh);

			surfaceData.meshSize = surface.meshSize;

			// 対象屋根面に追加
			std::string id = CStringEx::Format("%10d", meshCount);
			landData.mapSurface[id] = surfaceData;

		}

		meshCount++;

	}

	// 範囲BBに高さを付与
	std::vector<CVector3D> bbPos{
		{area.bbMinX, area.bbMinY, 0.0},
		{area.bbMinX, area.bbMaxY, 0.0},
		{area.bbMaxX, area.bbMinY, 0.0},
		{area.bbMaxX, area.bbMaxY, 0.0}
	};
	landData.bbPos = bbPos;

	// 範囲全体BBの法線
	CVector3D n;
	CGeoUtil::OuterProduct(
		CVector3D(bbPos[1], bbPos[0]),
		CVector3D(bbPos[2], bbPos[1]), n);
	if (n.z < 0) n *= -1;

	CVector3D p(vecAllLandPos[0].x, vecAllLandPos[0].y, vecAllLandPos[0].z);
	double d = CGeoUtil::InnerProduct(p, n);
	CVector3D inVec(0.0, 0.0, 1.0);
	double dot = CGeoUtil::InnerProduct(n, inVec);

	CVector3D center;		// 中心の座標
	for (auto& pos : landData.bbPos)
	{
		// 平面と垂線の交点
		CVector3D p0(pos.x, pos.y, 0.0);
		double t = (d - CGeoUtil::InnerProduct(p0, n)) / dot;
		// 交点
		CVector3D tempPoint = p0 + t * inVec;
		pos.z = tempPoint.z;
		// 中心
		center += pos;
	}
	center *= 0.25;
	landData.center = center;

}

/*!	頂点群から近似平面算出
@note   屋根面の傾斜角・方位角
*/
bool CCalcSolarPotentialMng::calcRansacPlane(
	const std::vector<CPointBase>& vecAry,		//!< in		頂点配列
	CVector3D& vNormal							//!< out	法線
)
{
	bool	bRet = false;

	CVector3D vec1;
	CVector3D vec2;
	// [0]からの各点のベクトル
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
		// 同じ方向か逆方向のときは法線求まらない
		if (abs(CGeoUtil::InnerProduct(vec1, tempVec)) > 0.999)
			continue;
		vec2 = tempVec;
		break;
	}
	CGeoUtil::OuterProduct(vec1, vec2, vNormal);

	if (vNormal.z < 0.0)
	{
		// 上向きに反転
		vNormal.Inverse();
	}

	return	true;
}

// 面積を算出
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
	
		// 月ごとの可照時間
		double dPossibleSunshineDuration = m_pSunshineData->GetPossibleSunshineDuration(time);

		// 月ごとの日照時間
		double dSunshineTime = m_pPointData->GetAverageSunshineTime(time);

		// 晴天/曇天時の月ごとの日照率を算出
		// 晴天時の日照率(Ⅰ)＝[日照時間] / [可照時間]
		m_pRadiationData->sunnyRate[i] = dSunshineTime / dPossibleSunshineDuration;
		// 曇天時の日照率(Ⅱ)＝(1 - 晴天時の日照率(Ⅰ))
		m_pRadiationData->cloudRate[i] = 1 - m_pRadiationData->sunnyRate[i];
	}
}

/// <summary>
/// 発電ポテンシャル推計結果を出力
/// </summary>
/// <param name="target"></param>
bool CCalcSolarPotentialMng::outputAreaBuildResult(const AREADATA& areaData)
{
	bool ret = true;

	const auto& resultData = (*m_pmapResultData)[areaData.areaID];

	// 対象エリアのフォルダ名
	std::wstring strAreaDirName = CStringEx::Format(L"%s_%s", CStringEx::ToWString(areaData.areaID).c_str(), CStringEx::ToWString(areaData.areaName).c_str());
	if (areaData.areaName == "")	strAreaDirName.pop_back();	// 名称が無い場合は末尾の"_"を削除

	// 建物
	if (areaData.analyzeBuild && resultData.pBuildMap)
	{
		// 解析対象ごとの出力先フォルダ名
		std::wstring strAnalyzeTargetDir = GetDirName_AnalyzeTargetDir(eAnalyzeTarget::ROOF);
		// 対象エリアのフォルダを作成
		std::wstring strAreaDir = GetFUtil()->Combine(strAnalyzeTargetDir, strAreaDirName);
		// フォルダを作成
		if (!GetFUtil()->IsExistPath(strAreaDir))
		{
			if (CreateDirectory(strAreaDir.c_str(), NULL) == FALSE)
			{
				return false;
			}
		}

		// 3次メッシュごとに出力
		for (const auto& [Lv3meshId, dataMap] : *resultData.pBuildMap)
		{
			// 建物リストを取得
			BLDGLIST bldList{};  // 格納用

			for (const auto& bldli : *m_targetArea.m_pvecAllBuildList)
			{
				if (Lv3meshId == bldli.meshID)
				{
					bldList = bldli;
					break;
				}
			}

			// メッシュのフォルダを作成
			std::wstring strMeshDir = GetFUtil()->Combine(strAreaDir, CStringEx::ToWString(Lv3meshId));
			// フォルダを作成
			if (!GetFUtil()->IsExistPath(strMeshDir))
			{
				if (CreateDirectory(strMeshDir.c_str(), NULL) == FALSE)
				{
					return false;
				}
			}

			// CSV
			{
				// 面ごとの日射量算出結果
				ret &= outputSurfaceRadCSV(eAnalyzeTarget::ROOF, dataMap, strMeshDir);

				// 月別日射量CSV(年間のみ)
				if (m_pUIParam->eAnalyzeDate == eDateType::Year)
				{
					ret &= outputMonthlyRadCSV(dataMap, strMeshDir);
				}
			}

			// 日射量画像を出力
			{
				double outMeshsize = 1.0;

				// 出力用データを作成
				std::vector<CPointBase>* pvecPoint3d = new std::vector<CPointBase>();
				ret &= createPointData_Build(*pvecPoint3d, areaData, bldList, dataMap, outMeshsize, eOutputImageTarget::SOLAR_RAD);

				std::wstring strFileName = CStringEx::Format(L"日射量_%s_%s.tif", CStringEx::ToWString(areaData.areaID).c_str(), CStringEx::ToWString(Lv3meshId).c_str());
				std::wstring strTiffPath = GetFUtil()->Combine(strMeshDir, strFileName);

				if (!outputImage(strTiffPath, pvecPoint3d, outMeshsize, eOutputImageTarget::SOLAR_RAD))
				{
					return false;
				}
				else
				{
					// 出力TIFF画像をコピー
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

			// 発電量画像を出力
			{
				double outMeshsize = 1.0;

				// 出力用データを作成
				std::vector<CPointBase>* pvecPoint3d = new std::vector<CPointBase>();
				ret &= createPointData_Build(*pvecPoint3d, areaData, bldList, dataMap, outMeshsize, eOutputImageTarget::SOLAR_POWER);

				// 出力フォルダを作成
				std::wstring strOutDir = GetFUtil()->Combine(strAnalyzeTargetDir, GetDirName_SolarPotentialImage());
				if (!GetFUtil()->IsExistPath(strOutDir))
				{
					if (CreateDirectory(strOutDir.c_str(), NULL) == FALSE)
					{
						return false;
					}
				}
				// テクスチャ出力
				std::wstring strFileName = CStringEx::Format(L"発電量_%s_%s.tif", CStringEx::ToWString(areaData.areaID).c_str(), CStringEx::ToWString(Lv3meshId).c_str());
				std::wstring strTiffPath = GetFUtil()->Combine(strOutDir, strFileName);
				ret &= outputImage(strTiffPath, pvecPoint3d, outMeshsize, eOutputImageTarget::SOLAR_POWER);

			}
		}

	}

	return ret;
}


/// <summary>
/// 発電ポテンシャル推計結果を出力
/// </summary>
/// <param name="target"></param>
bool CCalcSolarPotentialMng::outputAreaLandResult(const AREADATA& areaData)
{
	bool ret = true;

	const auto& resultData = (*m_pmapResultData)[areaData.areaID];

	// 対象エリアのフォルダ名
	std::wstring strAreaDirName = CStringEx::Format(L"%s_%s", CStringEx::ToWString(areaData.areaID).c_str(), CStringEx::ToWString(areaData.areaName).c_str());
	if (areaData.areaName == "")	strAreaDirName.pop_back();	// 名称が無い場合は末尾の"_"を削除

	// 土地
	if (areaData.analyzeLand && resultData.pLandData)
	{
		// 解析対象ごとの出力先フォルダ名
		std::wstring strAnalyzeTargetDir = GetDirName_AnalyzeTargetDir(eAnalyzeTarget::LAND);
		// 対象エリアのフォルダを作成
		std::wstring strAreaDir = GetFUtil()->Combine(strAnalyzeTargetDir, strAreaDirName);
		// フォルダを作成
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
			// 面ごとの日射量算出結果
			ret &= outputSurfaceRadCSV(eAnalyzeTarget::LAND, dataMap, strAreaDir);

			// 月別日射量CSV(年間のみ)
			if (m_pUIParam->eAnalyzeDate == eDateType::Year)
			{
				ret &= outputMonthlyRadCSV(dataMap, strAreaDir);
			}
		}

		// 日射量画像を出力
		{
			double outMeshsize = 1.0;

			// 出力用データを作成
			std::vector<CPointBase>* pvecPoint3d = new std::vector<CPointBase>;
			ret &= createPointData_Land(*pvecPoint3d, areaData, *resultData.pLandData, eOutputImageTarget::SOLAR_RAD);

			std::wstring strFileName = CStringEx::Format(L"日射量_%s.tif", CStringEx::ToWString(areaData.areaID).c_str());
			std::wstring strTiffPath = GetFUtil()->Combine(strAreaDir, strFileName);

			if (ret && !outputImage(strTiffPath, pvecPoint3d, outMeshsize, eOutputImageTarget::SOLAR_RAD))
			{
				return false;
			}
			else
			{
				// 出力TIFF画像をコピー
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

		// 発電量画像を出力
		{
			double outMeshsize = 1.0;

			// 出力用データを作成
			std::vector<CPointBase>* pvecPoint3d = new std::vector<CPointBase>;
			ret &= createPointData_Land(*pvecPoint3d, areaData, *resultData.pLandData, eOutputImageTarget::SOLAR_POWER);

			if (ret)
			{
				// 出力フォルダを作成
				std::wstring strOutDir = GetFUtil()->Combine(strAnalyzeTargetDir, GetDirName_SolarPotentialImage());
				if (!GetFUtil()->IsExistPath(strOutDir))
				{
					if (CreateDirectory(strOutDir.c_str(), NULL) == FALSE)
					{
						return false;
					}
				}
				// テクスチャ出力
				std::wstring strFileName = CStringEx::Format(L"発電量_%s.tif", CStringEx::ToWString(areaData.areaID).c_str());
				std::wstring strTiffPath = GetFUtil()->Combine(strOutDir, strFileName);

				ret &= outputImage(strTiffPath, pvecPoint3d, outMeshsize, eOutputImageTarget::SOLAR_POWER);
			}
		}
	}

	return ret;
}

// 日射量算出結果のポイントデータを作成
bool CCalcSolarPotentialMng::createPointData_Build(

	std::vector<CPointBase>& vecPoint3d,
	const AREADATA& areaData,
	const BLDGLIST& bldList,
	const CPotentialDataMap& bldDataMap,
	double outMeshsize,
	const eOutputImageTarget& eTarget
)
{
	// 1mメッシュの中央座標(x,y)と出力したい日射量算出結果(z)
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

		int surfaceCount = build.roofSurfaceList.size();
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

					// 屋根ごとに内外判定する
					for (const auto& roofSurfaces : surface.roofSurfaceList)
					{
						// 内外判定用
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
							// interior面を除外する場合は内周を描画しない
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

						// Z値に日射量算出結果を設定する
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

	// 3次メッシュのサイズで画像出力したいので四隅の座標を追加
	bool addLB, addLT, addRB, addRT;
	addLB = ((tmpMinX > bldList.bbMinX && tmpMinX < (bldList.bbMinX + 1.0)) && (tmpMinY > bldList.bbMinY && tmpMinY < (bldList.bbMinY + 1.0))) ? false : true;
	addLT = ((tmpMinX > bldList.bbMinX && tmpMinX < (bldList.bbMinX + 1.0)) && (tmpMaxY < bldList.bbMaxY && tmpMaxY > (bldList.bbMaxY - 1.0))) ? false : true;
	addRB = ((tmpMaxX < bldList.bbMaxX && tmpMaxX > (bldList.bbMaxX - 1.0)) && (tmpMinY > bldList.bbMinY && tmpMinY < (bldList.bbMinY + 1.0))) ? false : true;
	addRT = ((tmpMaxX < bldList.bbMaxX && tmpMaxX > (bldList.bbMaxX - 1.0)) && (tmpMaxY < bldList.bbMaxY && tmpMaxY > (bldList.bbMaxY - 1.0))) ? false : true;
	// 中央
	if (addLB)	vecPoint3d.push_back(CPointBase(bldList.bbMinX + 0.5, bldList.bbMinY + 0.5, DEF_IMG_NODATA));
	if (addLT)	vecPoint3d.push_back(CPointBase(bldList.bbMinX + 0.5, bldList.bbMaxY - 0.5, DEF_IMG_NODATA));
	if (addRB)	vecPoint3d.push_back(CPointBase(bldList.bbMaxX - 0.5, bldList.bbMinY + 0.5, DEF_IMG_NODATA));
	if (addRT)	vecPoint3d.push_back(CPointBase(bldList.bbMaxX - 0.5, bldList.bbMaxY - 0.5, DEF_IMG_NODATA));

	if (vecPoint3d.size() > 0)	ret = true;

	return ret;
}


// 日射量算出結果のポイントデータを作成(土地)
bool CCalcSolarPotentialMng::createPointData_Land(
	std::vector<CPointBase>& vecPoint3d,
	const AREADATA& areaData,
	const CPotentialData& landData,
	const eOutputImageTarget& eTarget
)
{
	// 1mメッシュの中央座標(x,y)と出力したい日射量算出結果(z)
	bool ret = false;

	vecPoint3d.clear();

	if (landData.mapSurface.size() == 0)	return false;

	vecPoint3d.insert(vecPoint3d.end(), areaData.pointMemData.begin(), areaData.pointMemData.end());
	for (auto& point : vecPoint3d)
	{
		if (point.z == DEF_IMG_NODATA) continue;

		CPoint2D target2d(point.x + 0.5, point.y + 0.5);

		// メッシュ単位で作成
		bool inside = false;

		for (const auto& [surfaceId, surfaceData] : landData.mapSurface)
		{
			double dx = target2d.x - surfaceData.center.x;
			double dy = target2d.y - surfaceData.center.y;
			if (calcLength(dx, dy, 0.0) > surfaceData.meshSize)
			{
				// 1メッシュサイズより離れていたらスキップ
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

	// 選択したエリアのBBで画像出力したいので四隅の座標を追加
	bool addLB, addLT, addRB, addRT;
	addLB = ((tmpMinX > areaData.bbMinX && tmpMinX < (areaData.bbMinX + 1.0)) && (tmpMinY > areaData.bbMinY && tmpMinY < (areaData.bbMinY + 1.0))) ? false : true;
	addLT = ((tmpMinX > areaData.bbMinX && tmpMinX < (areaData.bbMinX + 1.0)) && (tmpMaxY < areaData.bbMaxY&& tmpMaxY >(areaData.bbMaxY - 1.0))) ? false : true;
	addRB = ((tmpMaxX < areaData.bbMaxX&& tmpMaxX >(areaData.bbMaxX - 1.0)) && (tmpMinY > areaData.bbMinY && tmpMinY < (areaData.bbMinY + 1.0))) ? false : true;
	addRT = ((tmpMaxX < areaData.bbMaxX&& tmpMaxX >(areaData.bbMaxX - 1.0)) && (tmpMaxY < areaData.bbMaxY&& tmpMaxY >(areaData.bbMaxY - 1.0))) ? false : true;
	// 中央
	if (addLB)	vecPoint3d.push_back(CPointBase(areaData.bbMinX + 0.5, areaData.bbMinY + 0.5, DEF_IMG_NODATA));
	if (addLT)	vecPoint3d.push_back(CPointBase(areaData.bbMinX + 0.5, areaData.bbMaxY - 0.5, DEF_IMG_NODATA));
	if (addRB)	vecPoint3d.push_back(CPointBase(areaData.bbMaxX - 0.5, areaData.bbMinY + 0.5, DEF_IMG_NODATA));
	if (addRT)	vecPoint3d.push_back(CPointBase(areaData.bbMaxX - 0.5, areaData.bbMaxY - 0.5, DEF_IMG_NODATA));

	if (vecPoint3d.size() > 0)	ret = true;

	return ret;
}

// 画像出力
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

	// TIFF画像作成
	if (ret)
	{
		ret &= tiffDataMng.Create();
	}

	// JPEG出力
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

// 解析対象ごとの出力フォルダ名を取得
const std::wstring CCalcSolarPotentialMng::GetDirName_AnalyzeTargetDir(eAnalyzeTarget target)
{
	std::wstring wstr = L"";

	switch (target)
	{
	case eAnalyzeTarget::ROOF:
		wstr = GetFUtil()->Combine(m_pUIParam->strOutputDirPath, L"建物");
		break;

	case eAnalyzeTarget::LAND:
		wstr = GetFUtil()->Combine(m_pUIParam->strOutputDirPath, L"土地");
		break;

	default:
		break;
	}

	// フォルダ作成
	if (!GetFUtil()->IsExistPath(wstr))
	{
		if (CreateDirectory(wstr.c_str(), NULL) == FALSE)
		{
			return L"";
		}
	}

	return wstr;
}

// 建物・エリアごと予測発電量CSV
const std::wstring CCalcSolarPotentialMng::GetFileName_SolarPotentialCsv(eAnalyzeTarget target)
{
	std::wstring wstr = L"";

	switch (target)
	{
	case eAnalyzeTarget::ROOF:
		wstr = L"建物ごと予測発電量.csv";
		break;

	case eAnalyzeTarget::LAND:
		wstr = L"土地ごと予測発電量.csv";
		break;

	default:
		break;
	}

	return wstr;
}

// (土地面のみ)メッシュごと予測発電量CSV
const std::wstring CCalcSolarPotentialMng::GetFileName_MeshPotentialCsv(eAnalyzeTarget target)
{
	return L"メッシュごと予測発電量.csv";
}

// (土地面のみ)エリアごと予測発電量SHP
const std::wstring CCalcSolarPotentialMng::GetFileName_SolarPotentialShp()
{
	return L"土地ごと予測発電量.shp";
}

// 屋根面別・土地面メッシュ別日射量CSV
const std::wstring CCalcSolarPotentialMng::GetFileName_MeshSolarRadCsv(eAnalyzeTarget target)
{
	std::wstring wstr = L"";

	switch (target)
	{
	case eAnalyzeTarget::ROOF:
		wstr = L"屋根面別日射量.csv";
		break;

	case eAnalyzeTarget::LAND:
		wstr = L"土地面メッシュ別日射量.csv";
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

	// 日射量
	strColorSettingPath = L"Assets\\ColorSettings\\Template\\" + getColorSettingFileName(eOutputImageTarget::SOLAR_RAD);
	ret = CImageUtil::CreateLegendImage(strColorSettingPath, L"日射量(kWh/m2)");
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

	// 発電量
	strColorSettingPath = L"Assets\\ColorSettings\\Template\\" + getColorSettingFileName(eOutputImageTarget::SOLAR_POWER);
	ret &= CImageUtil::CreateLegendImage(strColorSettingPath, L"発電量(kWh/m2)");
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


// 全範囲における日射量・発電量CSVを出力
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

			// 3次メッシュごとに出力
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

		// 建物結果があれば書き込み
		if (outputList.size() > 0)
		{
			// 解析対象ごとの出力先フォルダ名
			std::wstring wstrAnalyzeTargetDir = GetDirName_AnalyzeTargetDir(eAnalyzeTarget::ROOF);
			if (wstrAnalyzeTargetDir.empty())	return false;

			// 出力CSVファイル
			std::wstring wstrFilePath = GetFUtil()->Combine(wstrAnalyzeTargetDir, GetFileName_SolarPotentialCsv(eAnalyzeTarget::ROOF));

			// ヘッダ部
			std::string header = "解析エリアID,3次メッシュID,建物ID,予測日射量(kWh),予測日射量(kWh/m2),予測発電量(kWh),予測発電量(kWh/m2),屋根面面積(m2),PV設置面積(m2),X,Y,Z";

			CFileIO file;
			if (!file.Open(wstrFilePath, L"w"))
			{
				return false;
			}

			// ヘッダ部
			file.WriteLineA(header);

			// データ部
			for (const auto& str : outputList)
			{
				file.WriteLineA(str);
			}

			file.Close();
		}

	}

	if (m_pUIParam->bExecLand)
	{

		// 解析対象ごとの出力先フォルダ名
		std::wstring wstrAnalyzeTargetDir = GetDirName_AnalyzeTargetDir(eAnalyzeTarget::LAND);
		if (wstrAnalyzeTargetDir.empty())	return false;

		// エリアごとCSV
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

			// 土地結果があれば書き込み
			if (outputList.size() > 0)
			{
				// 出力CSVファイル
				std::wstring wstrFilePath = GetFUtil()->Combine(wstrAnalyzeTargetDir, GetFileName_SolarPotentialCsv(eAnalyzeTarget::LAND));

				// ヘッダ部
				std::string header = "解析エリアID,予測日射量(kWh),予測日射量(kWh/m2),予測発電量(kWh),予測発電量(kWh/m2),土地面面積(m2),PV設置面積(m2),X,Y,Z";

				CFileIO file;
				if (!file.Open(wstrFilePath, L"w"))
				{
					return false;
				}

				// ヘッダ部
				file.WriteLineA(header);

				// データ部
				for (const auto& str : outputList)
				{
					file.WriteLineA(str);
				}

				file.Close();
			}
		}

		// メッシュごとCSV
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

			// 土地結果があれば書き込み
			if (outputList.size() > 0)
			{
				// 出力CSVファイル
				std::wstring wstrFilePath = GetFUtil()->Combine(wstrAnalyzeTargetDir, GetFileName_MeshPotentialCsv(eAnalyzeTarget::LAND));

				// ヘッダ部
				std::string header = "エリアID,土地面メッシュID,予測日射量(kWh/m2),予測発電量(kWh/m2),PV設置面積(m2),X,Y,Z";

				CFileIO file;
				if (!file.Open(wstrFilePath, L"w"))
				{
					return false;
				}

				// ヘッダ部
				file.WriteLineA(header);

				// データ部
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

// 月別日射量CSV出力
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
	// デバッグ用出力
	std::wstring strPath = GetFUtil()->Combine(wstrOutDir, L"月別日射量_角度補正.csv");

	CFileIO file;
	if (!file.Open(strPath, L"w"))
	{
		return false;
	}

	// ヘッダ部
	if (!file.WriteLineA("ID,面ID,メッシュID,年,月,日射量(Wh/m2),日射量(MJ/m2)"))
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


// 面ごとの日射量CSV出力
bool CCalcSolarPotentialMng::outputSurfaceRadCSV(
	const eAnalyzeTarget target,
	const CPotentialDataMap& dataMap,
	const std::wstring& wstrOutDir			// 出力フォルダ
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
		// ヘッダ部
		if (!file.WriteLineA("建物ID,屋根面ID,年,単位面積あたりの日射量(kWh/m2),→(MJ/m2),対象面全体の日射量(kWh),→(MJ),方位角(平均),傾斜角(平均),方位角(補正),傾斜角(補正),x,y,z"))
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
		// ヘッダ部
		if (!file.WriteLineA("エリアID,メッシュID,年,1メッシュあたりの日射量(kWh/m2),→(MJ/m2),土地面全体の日射量(kWh),→(MJ),方位角(平均),傾斜角(平均),方位角(補正),傾斜角(補正),x,y,z"))
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
					// デバッグ用出力
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


// 建物ごとの方位角中間データ出力
bool CCalcSolarPotentialMng::outputAzimuthDataCSV()
{
	if (m_pmapResultData->empty())	return false;
	if (IsCancel())	return false;

	bool ret = false;

	// 出力パス
	std::wstring strCsvPath = GetINIParam()->GetAzimuthCSVPath();
	std::wstring strTempDir = GetFUtil()->GetParentDir(m_pUIParam->strOutputDirPath);
	strTempDir = GetFUtil()->GetParentDir(strTempDir) + L"system";
	std::wstring strPath = GetFUtil()->Combine(strTempDir, strCsvPath);

	// 出力用フォルダ作成
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

	// ヘッダ部
	if (!file.WriteLineA("エリアID,3次メッシュID,建物ID,同一屋根面数,方位角(平均値)"))
	{
		return false;
	}

	// 方位角データ書き込み
	for (const auto& [areaId, result] : *m_pmapResultData)
	{
		// 建物
		if (m_pUIParam->bExecBuild && result.pBuildMap)
		{
			// 3次メッシュごとに出力
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

					// エリアID, メッシュID, 建物ID, 同一屋根面数, 方位角(平均値)・・・"
					std::string strLine = CStringEx::Format("%s,%s,%s,%d,%s", areaId.c_str(), meshId.c_str(), buildId.c_str(), roofsize, strAzimuths.c_str());

					file.WriteLineA(strLine);
				}
			}
		}

		// 土地
		if (m_pUIParam->bExecLand && result.pLandData)
		{
			CPotentialData& landData = *result.pLandData;

			int size = (int)landData.mapSurface.size();
			if (size != 0)
			{
				// エリアごとに方位角は同じになるので先頭データだけ出力
				auto itr = landData.mapSurface.begin();
				auto val = *itr;
				std::string strAzimuths = CStringEx::Format("%f,", val.second.azModDegree);

				// エリアID, メッシュID, 建物ID, 同一屋根面数, 方位角(平均値)・・・"
				std::string strLine = CStringEx::Format("%s,%s,%s,%d,%s", areaId.c_str(), areaId.c_str(), areaId.c_str(), 1, strAzimuths.c_str());

				file.WriteLineA(strLine);
			}
		}

	}

	file.Close();

	return true;

}

// SHPに付与
bool CCalcSolarPotentialMng::outputLandShape()
{
	if (m_pmapResultData->empty()) return false;
	if (IsCancel())	return false;

	// 解析対象ごとの出力先フォルダ名
	std::wstring wstrAnalyzeTargetDir = GetDirName_AnalyzeTargetDir(eAnalyzeTarget::LAND);
	if (wstrAnalyzeTargetDir.empty())	return false;

	std::wstring wstrDir = GetFUtil()->Combine(wstrAnalyzeTargetDir, GetDirName_LandShape());
	// フォルダを作成
	if (!GetFUtil()->IsExistPath(wstrDir))
	{
		if (CreateDirectory(wstrDir.c_str(), NULL) == FALSE)
		{
			return false;
		}
	}

	// shp作成
	std::string strShpFilePath = CStringEx::ToString(GetFUtil()->Combine(wstrDir, GetFileName_SolarPotentialShp()));
	SHPHandle hSHP = SHPCreate(strShpFilePath.c_str(), SHPT_POLYGON);
	
	// dbf作成
	std::string strDbfFilePath = CFileUtil::ChangeFileNameExt(strShpFilePath, ".dbf");
	DBFHandle hDBF = DBFCreate(strDbfFilePath.c_str());

	// 属性
	DBFAddField(hDBF, "AreaID", FTString, 255, 0);
	DBFAddField(hDBF, "Name", FTString, 255, 0);
	DBFAddField(hDBF, "面積", FTDouble, 11, 3);
	DBFAddField(hDBF, "PV面積", FTInteger, 10, 0);
	DBFAddField(hDBF, "日射量1", FTDouble, 11, 3);
	DBFAddField(hDBF, "日射量2", FTDouble, 11, 3);
	DBFAddField(hDBF, "発電量1", FTDouble, 11, 3);
	DBFAddField(hDBF, "発電量2", FTDouble, 11, 3);

	for (const auto& areaData : *m_pvecAllAreaList)
	{
		if (!areaData.analyzeLand)	continue;

		CPotentialData* landData = (*m_pmapResultData)[areaData.areaID].pLandData;
		if (!landData)	continue;

		// ポリゴン作成
		int nVertices = areaData.pos2dList.size();
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

		// 属性書き込み
		int iRecord = DBFGetRecordCount(hDBF);
		DBFWriteStringAttribute(hDBF, iRecord, 0, areaData.areaID.c_str());
		DBFWriteStringAttribute(hDBF, iRecord, 1, areaData.areaName.c_str());
		DBFWriteDoubleAttribute(hDBF, iRecord, 2, landData->GetAllArea());
		DBFWriteIntegerAttribute(hDBF, iRecord, 3, landData->panelArea);
		DBFWriteDoubleAttribute(hDBF, iRecord, 4, landData->solarRadiationTotal);
		DBFWriteDoubleAttribute(hDBF, iRecord, 5, landData->solarRadiationUnit);
		DBFWriteDoubleAttribute(hDBF, iRecord, 6, landData->solarPower);
		DBFWriteDoubleAttribute(hDBF, iRecord, 7, landData->solarPowerUnit);
	}

	SHPClose(hSHP);
	DBFClose(hDBF);


	// cpgファイルを出力
	std::string strCpgFilePath = CFileUtil::ChangeFileNameExt(strShpFilePath, ".cpg");
	std::ofstream ofs(strCpgFilePath);
	ofs << "SJIS" << endl;

	return true;
}

// 対象面の中心に入射光があたっているか
bool CCalcSolarPotentialMng::IntersectSurfaceCenter(
	const CVector3D& inputVec,						// 入射光
	const std::vector<CVector3D>& surfaceBB,		// 対象面BB
	const CVector3D& center,						// 対象面中心
	const std::string& strId,						// 屋根面ID(建物のみ使用)
	const vector<BLDGLIST>& neighborBuildings,		// 周辺の建物リスト
	const vector<CTriangle>& neighborDems			// 周辺の地形TINリスト
)
{
	// 光線の有効距離
	const double LIGHT_LENGTH = GetINIParam()->GetNeighborBuildDist_SolarRad();

	// 屋根メッシュの法線
	CVector3D n;
	CGeoUtil::OuterProduct(
		CVector3D(surfaceBB[1], surfaceBB[0]),
		CVector3D(surfaceBB[2], surfaceBB[1]), n);
	if (n.z < 0) n *= -1;

	// 対象面の裏側から入射光が当たっているときは反射しないので解析終了
	if (CGeoUtil::InnerProduct(n, inputVec) >= 0.0)
		return false;

	// 入射光の光源を算出
	// 屋根メッシュ座標の延長線上に設定する
	CVector3D inputInverseVec = CGeoUtil::Normalize(inputVec) * ((-1) * LIGHT_LENGTH);
	CVector3D sunPos = center + inputInverseVec;
	CLightRay lightRay(sunPos, CGeoUtil::Normalize(inputVec) * LIGHT_LENGTH);

	// 入射光が周りの建物に邪魔されずに対象面に当たるかチェック
	if (intersectBuildings(lightRay, strId, neighborBuildings))
	{
		// 建物にあたっている場合は光線があたっていないので解析終了
		return false;
	}

	// 入射光が周りの地形に邪魔されずに対象面に当たるかチェック
	if (IsEnableDEMData() || m_pUIParam->bExecLand)
	{
		if (intersectLandDEM(lightRay, neighborDems, surfaceBB))
		{
			return false;
		}
	}

	return true;
}

// 建物群に光線があたっているか
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

// 建物に光線があたっているかどうか
bool CCalcSolarPotentialMng::intersectBuilding(
	const CLightRay& lightRay,
	const vector<WALLSURFACES>& wallSurfaceList
)
{
	// 光源が建物より遠すぎないか調べる
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

			// 光線とポリゴンの交点を探す
			if (lightRay.Intersect(posList, &tempTargetPos, &tempDist))
			{
				return true;
			}
		}
	}

	return false;
}

// 建物に光線があたっているかどうか
bool CCalcSolarPotentialMng::intersectBuilding(
	const CLightRay& lightRay,
	const std::string& strId,
	const BUILDINGS& buildiings
)
{
	// 光源が建物より遠すぎないか調べる
	if (!checkDistance(lightRay, buildiings.wallSurfaceList))
		return false;

	double tempDist;
	CVector3D tempTargetPos;

	// 屋根
	for (const auto& roof : buildiings.roofSurfaceList)
	{
		if (strId == roof.roofSurfaceId)	continue;	// 自身の屋根は除外

		for (const auto& polygon : roof.roofSurfaceList)
		{
			vector<CVector3D> posList(polygon.posList.size());
			int i = 0;
			for (const auto& pos : polygon.posList)
			{
				posList[i] = CVector3D(pos.x, pos.y, pos.z);
				++i;
			}

			// 光線とポリゴンの交点を探す
			if (lightRay.Intersect(posList, &tempTargetPos, &tempDist))
			{
				return true;
			}
		}
	}

	// 壁
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

			// 光線とポリゴンの交点を探す
			if (lightRay.Intersect(posList, &tempTargetPos, &tempDist))
			{
				return true;
			}
		}
	}

	return false;
}

// 建物が光線の範囲内か大まかにまずは判定する
bool CCalcSolarPotentialMng::checkDistance(const CLightRay& lightRay, const vector<WALLSURFACES>& wallSurfaceList)
{
	// 光線が範囲内か判定する距離範囲
	const double LIGHT_LENGTH = GetINIParam()->GetNeighborBuildDist_SolarRad() + 50;	//余裕を持たせる
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

				// 平面の頂点が逆方向にあるときは範囲外とする
				double dot = CGeoUtil::InnerProduct(lightRayVec, CVector3D(dx, dy, dz));
				if (dot < 0.0)	continue;

				// 距離が遠すぎないかチェック
				double len = calcLength(dx, dy, dz);
				if (len > SQUARE_LINGHT_LENGTH)	continue;

				return true;
			}
		}
	}

	return false;
}

// 隣接する建物を取得
void CCalcSolarPotentialMng::GetNeighborBuildings(
	const CVector3D& center,
	std::vector<BLDGLIST>& neighborBuildings
)
{
	const double DIST = GetINIParam()->GetNeighborBuildDist_SolarRad();	// 隣接するBBoxの範囲[m]

	// 建物中心XY
	CVector2D CenterXY(center.x, center.y);

	for (const auto& bldList : *m_targetArea.m_pvecAllBuildList)
	{
		BLDGLIST tmpBldList = bldList;
		tmpBldList.buildingList.clear();
		tmpBldList.buildingListLOD1.clear();

		// 範囲内にあるか
		// LOD2
		for (const auto& build : bldList.buildingList)
		{
			double bbBldMinX = DBL_MAX, bbBldMinY = DBL_MAX;
			double bbBldMaxX = -DBL_MAX, bbBldMaxY = -DBL_MAX;
			double dMaxH = -DBL_MAX;

			// 建物全体のBBを求める
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

			// 対象より低い建物は除外
			if (center.z > dMaxH)	continue;

			double buildCenterX = ((int64_t)bbBldMaxX + (int64_t)bbBldMinX) * 0.5;
			double buildCenterY = ((int64_t)bbBldMaxY + (int64_t)bbBldMinY) * 0.5;
			// 中心同士の距離
			double tmpdist = CenterXY.Distance(buildCenterX, buildCenterY);
			// DIST以内の距離のとき近隣とする
			if (tmpdist <= DIST)
			{
				tmpBldList.buildingList.emplace_back(build);
			}
		}

		// LOD1
		for (const auto& build : bldList.buildingListLOD1)
		{
			double bbBldMinX = DBL_MAX, bbBldMinY = DBL_MAX;
			double bbBldMaxX = -DBL_MAX, bbBldMaxY = -DBL_MAX;
			double dMaxH = -DBL_MAX;

			// 建物全体のBBを求める
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

			// 対象より低い建物は除外
			if (center.z > dMaxH)	continue;

			double buildCenterX = ((int64_t)bbBldMaxX + (int64_t)bbBldMinX) * 0.5;
			double buildCenterY = ((int64_t)bbBldMaxY + (int64_t)bbBldMinY) * 0.5;
			// 中心同士の距離
			double tmpdist = CenterXY.Distance(buildCenterX, buildCenterY);
			// DIST以内の距離のとき近隣とする
			if (tmpdist <= DIST)
			{
				tmpBldList.buildingListLOD1.emplace_back(build);
			}
		}

		if (tmpBldList.buildingList.empty() && tmpBldList.buildingListLOD1.empty())	continue;

		neighborBuildings.emplace_back(tmpBldList);
	}

}

// 隣接するDEMを取得
void CCalcSolarPotentialMng::GetNeighborDems(
	const CVector3D& center,
	std::vector<CTriangle>& neighborDems,
	eAnalyzeTarget target
)
{
	const double DIST = GetINIParam()->GetDemDist();	// 対象範囲[m]

	// 中心XY
	CVector2D CenterXY(center.x, center.y);

	// 除外するDEM高さ
	double demHeight = target == eAnalyzeTarget::ROOF ? GetINIParam()->GetDemHeight_Build() : GetINIParam()->GetDemHeight_Land();

	for (const auto& member : *m_targetArea.m_pvecAllDemList)
	{
		for (const auto& triangle : member.posTriangleList)
		{
			// XY平面の重心を求める
			double x = (triangle.posTriangle[0].x + triangle.posTriangle[1].x + triangle.posTriangle[2].x) / 3.0;
			double y = (triangle.posTriangle[0].y + triangle.posTriangle[1].y + triangle.posTriangle[2].y) / 3.0;
			double z = (triangle.posTriangle[0].z + triangle.posTriangle[1].z + triangle.posTriangle[2].z) / 3.0;
			if (z < demHeight)	continue;

			double tmpdist = CenterXY.Distance(x, y);

			bool bAddList = false;

			// 判定
			if (tmpdist <= DIST &&		// 中心との距離が対象範囲内
				center.z < z)			// 中心より高い位置にある
			{
				neighborDems.emplace_back(triangle);
			}
		}
	}
}


// 地形に光線があたっているかどうか
bool CCalcSolarPotentialMng::intersectLandDEM(
	const CLightRay& lightRay,					// 光線
	const vector<CTriangle>& tinList,			// 光線があたっているかチェックする地形のTIN
	const std::vector<CVector3D>& surfaceBB		// 対象面のBB
)
{
	double tempDist;
	CVector3D tempTargetPos;

	const CVector3D lightRayPos = lightRay.GetPos();
	const CVector3D lightRayVec = lightRay.GetVector();

	// 光線が範囲内か判定する距離範囲
	const double LIGHT_LENGTH = GetINIParam()->GetNeighborBuildDist_SolarRad() + 50;	//余裕を持たせる
	const double SQUARE_LINGHT_LENGTH = LIGHT_LENGTH * LIGHT_LENGTH;

	// 内外判定用に対象面のポリゴンを作成
	CPoint2DPolygon surfacePolygon;
	for (int i = 0; i < surfaceBB.size(); i++)
	{
		CPoint2D p2D = CPoint2D(surfaceBB[i].x, surfaceBB[i].y);
		surfacePolygon.Add(p2D);
	}

	for (const auto& triangle : tinList)
	{
		if (IsCancel())		return false;

		// 内外判定用にTINのポリゴンを作成
		CPoint2DPolygon tinPolygon;

		bool bDirect = false;
		for (const auto& pos : triangle.posTriangle)
		{
			double dx = pos.x - lightRayPos.x;
			double dy = pos.y - lightRayPos.y;
			double dz = pos.z - lightRayPos.z;

			CPoint2D p2D = CPoint2D(pos.x, pos.y);
			tinPolygon.Add(p2D);

			// 平面の頂点が逆方向にあるときは範囲外とする
			double dot = CGeoUtil::InnerProduct(lightRayVec, CVector3D(dx, dy, dz));
			if (dot < 0.0)	continue;

			// 距離が遠すぎないかチェック
			double len = calcLength(dx, dy, dz);
			if (len > SQUARE_LINGHT_LENGTH)	continue;

			bDirect = true;
		}
		if (!bDirect)	continue;

		// 対象面の範囲内にあるTINは除外
		CPoint2DPolygon tmpPolygon;
		if (surfacePolygon.GetCrossingPolygon(tinPolygon, tmpPolygon))
		{
			continue;
		}

		vector<CVector3D> posList;
		posList.emplace_back(CVector3D(triangle.posTriangle[0].x, triangle.posTriangle[0].y, triangle.posTriangle[0].z));
		posList.emplace_back(CVector3D(triangle.posTriangle[1].x, triangle.posTriangle[1].y, triangle.posTriangle[1].z));
		posList.emplace_back(CVector3D(triangle.posTriangle[2].x, triangle.posTriangle[2].y, triangle.posTriangle[2].z));

		// 光線とポリゴンの交点を探す
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
