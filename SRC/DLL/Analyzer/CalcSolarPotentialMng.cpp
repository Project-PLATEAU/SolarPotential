#include "pch.h"
#include <math.h>
#include <random>
#include <iostream>
#include "CalcSolarPotentialMng.h"
#include "CalcSolarRadiation.h"
#include "CalcSolarPower.h"
#include "../../LIB/CommonUtil/CFileIO.h"
#include "../../LIB/CommonUtil/CFileUtil.h"
#include "../../LIB/CommonUtil/ReadINIParam.h"
#include "../../LIB/CommonUtil/CImageUtil.h"
#include "../../LIB/CommonUtil/CEpsUtil.h"

#define DEF_IMG_NODATA -9999
#define DEF_EPSGCODE 6675

CCalcSolarPotentialMng::CCalcSolarPotentialMng
(
	CImportPossibleSunshineData* pSunshineData,
	CImportAverageSunshineData* pPointData,
	CImportMetpvData* pMetpvData,
	UIParam* pParam,
	const int& iYear
)
	: m_pSunshineData(pSunshineData)
	, m_pPointData(pPointData)
	, m_pMetpvData(pMetpvData)
	, m_pUIParam(pParam)
	, m_pRadiationData(NULL)
	, m_pvecAllBuildList(NULL)
	, m_pmapResultData(NULL)
	, m_iYear(iYear)
	, m_strCancelFilePath(L"")
	, m_pvecAllDemList(NULL)
{

}

CCalcSolarPotentialMng::~CCalcSolarPotentialMng(void)
{
}


// 計算用データ等の初期化
void CCalcSolarPotentialMng::initialize()
{
	// データ取得
	m_pvecAllBuildList = reinterpret_cast<std::vector<BLDGLIST>*>(GetAllList());
	m_pvecAllDemList = reinterpret_cast<std::vector<DEMLIST>*>(GetAllDemList());

	m_pmapResultData = new CResultDataMap;
	m_pRadiationData = new CAnalysisRadiationCommon;

	// キャンセル
	std::wstring strDir = GetFUtil()->GetParentDir(m_pUIParam->strOutputDirPath);
	m_strCancelFilePath = GetFUtil()->Combine(strDir, CStringEx::ToWString(CANCELFILE));

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
}


// 発電ポテンシャル推計(メイン処理)
bool CCalcSolarPotentialMng::AnalyzeSolarPotential()
{
	setlocale(LC_ALL, "");

	if (!m_pSunshineData) return false;
	if (!m_pPointData) return false;
	if (!m_pMetpvData) return false;
	if (!m_pUIParam) return false;

	// 初期化
	initialize();
	if (!m_pRadiationData) return false;
	if (!m_pvecAllBuildList) return false;
	if (m_pUIParam->bEnableDEMData && !m_pvecAllDemList) return false;

	bool ret = false;

	int dataCount = (int)m_pvecAllBuildList->size();
	for (int ic = 0; ic < dataCount; ic++)
	{
		if (IsCancel())
		{
			finalize();
			return false;
		}

		const BLDGLIST& bldList = m_pvecAllBuildList->at(ic);

		// 建物ごとの結果データ
		CBuildingDataMap buildDataMap;

		// 傾斜角・方位角の算出(対象データ取得)
		calcRoofAspect(bldList, buildDataMap);
		if (buildDataMap.empty())	continue;

		// 日射量推計
		ret = calcSolarRadiation(bldList.meshID, bldList.bbMinX, bldList.bbMinY, bldList.bbMaxX, bldList.bbMaxY, buildDataMap);
		if (!ret)
		{
			finalize();
			return false;
		}

		// 発電量推計
		ret &= calcSolarPower(bldList.meshID, bldList.bbMinX, bldList.bbMinY, bldList.bbMaxX, bldList.bbMaxY, buildDataMap);
		if (!ret)
		{
			finalize();
			return false;
		}

		(*m_pmapResultData)[bldList.meshID] = buildDataMap;

	}

	if (m_pmapResultData->empty())	ret = false;	// 対象データが無い

	if (ret && !IsCancel())
	{
		// 出力処理(全データ)
		ret &= outputAzimuthDataCSV();		// 適地判定用の中間ファイル生成(CSV)
		ret &= outputResultCSV();			// 年間予測日射量・発電量を出力

		// 凡例出力
		ret &= outputLegendImage();
	}

	// 解放処理
	finalize();

	return ret;
}

// 日射量推計(3次メッシュごと)
bool CCalcSolarPotentialMng::calcSolarRadiation
(
	const std::string&	Lv3meshId,			// 3次メッシュID
	double bbMinX,
	double bbMinY,
	double bbMaxX,
	double bbMaxY,
	CBuildingDataMap&	bldDataMap			// 対象の建物データリスト
)
{
	if (bldDataMap.empty())		return false;

	if (IsCancel())	return false;

	bool ret = false;

	CCalcSolarRadiation calcSolarRad(this);

	// 3次メッシュID
	std::wstring meshId = CStringEx::ToWString(Lv3meshId);

	// 中間CSVの出力フォルダを作成
	std::wstring strOutDir = m_pUIParam->strOutputDirPath;	// 出力フォルダ
	if (!GetFUtil()->IsExistPath(strOutDir))
	{
		if (CreateDirectory(strOutDir.c_str(), NULL) == FALSE)
		{
			return false;
		}
	}
	// 3次メッシュIDごとにサブフォルダを作成
	std::wstring strOutSubDir = GetFUtil()->Combine(strOutDir, CStringEx::Format(L"%s", meshId.c_str()));
	if (!GetFUtil()->IsExistPath(strOutSubDir))
	{
		if (CreateDirectory(strOutSubDir.c_str(), NULL) == FALSE)
		{
			return false;
		}
	}

	// 中間CSVファイルパス
	std::wstring strCsvPath;

	// 月ごとの日照率を計算
	calcMonthlyRate();

	// 日射量の算出
	{
		// 太陽軌道をもとにした日射量の算出
		strCsvPath = GetFUtil()->Combine(strOutSubDir, L"晴天曇天時日射量_日別_角度補正.csv");
		ret = calcSolarRad.Exec(bldDataMap, true, strCsvPath, Lv3meshId);
		if (!ret)	return false;

		// 日照率による補正	
		ret &= calcSolarRad.ModifySunRate(bldDataMap, strCsvPath);
		ret &= outputMonthlyRadCSV(Lv3meshId, bldDataMap, strOutSubDir);
		if (!ret)	return false;
	}

	// 屋根面別年間日射量
	ret &= calcSolarRad.CalcBuildSolarRadiation(bldDataMap, strOutSubDir);
	ret &= outputRoofRadCSV(Lv3meshId, bldDataMap, strOutSubDir);

	// 日射量テクスチャ出力
	std::wstring strFileName = CStringEx::Format(L"日射量%s.tif", meshId.c_str());
	std::wstring strTiffPath = GetFUtil()->Combine(strOutSubDir, strFileName);
	ret &= outputImage(strTiffPath, Lv3meshId, bbMinX, bbMinY, bbMaxX, bbMaxY, bldDataMap, eOutputImageTarget::SOLAR_RAD);

	if (!ret)
	{
		return false;
	}
	else
	{
		// 出力TIFF画像をコピー
		std::wstring strCopyDir = GetFUtil()->Combine(strOutDir, L"日射量画像");
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

	return ret;

}

// 発電量推計
bool CCalcSolarPotentialMng::calcSolarPower
(
	const std::string& Lv3meshId,			// 3次メッシュID
	double bbMinX,
	double bbMinY,
	double bbMaxX,
	double bbMaxY,
	CBuildingDataMap& bldDataMap			// 対象の建物データリスト
)
{
	if (IsCancel())	return false;

	CCalcSolarPower calcSolarPower;
	calcSolarPower.SetPperUnit(m_pUIParam->_dAreaSolarPower);
	bool ret = calcSolarPower.CalcEPY(bldDataMap);

	// テクスチャ出力
	// 出力フォルダを作成
	std::wstring strOutDir = GetFUtil()->Combine(m_pUIParam->strOutputDirPath, L"発電ポテンシャル画像");
	if (!GetFUtil()->IsExistPath(strOutDir))
	{
		if (CreateDirectory(strOutDir.c_str(), NULL) == FALSE)
		{
			return false;
		}
	}
	// 3次メッシュID
	std::wstring meshId = CStringEx::ToWString(Lv3meshId);
	// テクスチャ出力
	std::wstring strFileName = CStringEx::Format(L"%s.tif", meshId.c_str());
	std::wstring strTiffPath = GetFUtil()->Combine(strOutDir, strFileName);
	ret &= outputImage(strTiffPath, Lv3meshId, bbMinX, bbMinY, bbMaxX, bbMaxY, bldDataMap, eOutputImageTarget::SOLAR_POWER);

	return ret;
}


// 傾斜角と方位角を算出
// 
void CCalcSolarPotentialMng::calcRoofAspect(const BLDGLIST& bldList, CBuildingDataMap& bldDataMap)
{
	int bldsize = (int)bldList.buildingList.size();

	for (int ic = 0; ic < bldsize; ic++)
	{	// 建物
		BUILDINGS build = bldList.buildingList[ic];
		int surfacesize = (int)build.roofSurfaceList.size();
		if (surfacesize == 0)
		{
			continue;
		}

		CBuildingData bldData;

		// 建物全体のBB
		double bbBldMinX = DBL_MAX, bbBldMinY = DBL_MAX;
		double bbBldMaxX = -DBL_MAX, bbBldMaxY = -DBL_MAX;

		std::vector<CPointBase> vecAllRoofPos;

		for (int jc = 0; jc < surfacesize; jc++)
		{	// 屋根面
			ROOFSURFACES surface = build.roofSurfaceList[jc];
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

			CRoofSurfaceData roofData;

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

			for (int kc = 0; kc < roofsize; kc++)
			{	// 屋根面詳細
				SURFACEMEMBERS member = surface.roofSurfaceList[kc];
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
				if (slopeDeg > m_pUIParam->_elecPotential.dSlopeAngle)
				{
					continue;
				}

				// 方位角
				double azDeg;
				int JPZONE = GetINIParam()->GetJPZone();
				CGeoUtil::CalcAzimuth(normal, azDeg, JPZONE);

				// 方位除外判定
				bool bExclusion = false;
				switch (m_pUIParam->_elecPotential.eAzimuth)
				{
					case eDirections::EAST:
					{
						bExclusion = (abs(azDeg) < 90.0 + AZ_RANGE_JUDGE_DEGREE || abs(azDeg) > 90.0 - AZ_RANGE_JUDGE_DEGREE) ? true : false;
						bExclusion &= (slopeDeg > m_pUIParam->_elecPotential.dAzimuthAngle) ? true : false;
						break;
					}
					case eDirections::WEST:
					{
						bExclusion = (abs(azDeg) < 270.0 + AZ_RANGE_JUDGE_DEGREE || abs(azDeg) > 270.0 - AZ_RANGE_JUDGE_DEGREE) ? true : false;
						bExclusion &= (slopeDeg > m_pUIParam->_elecPotential.dAzimuthAngle) ? true : false;
						break;
					}
					case eDirections::SOUTH:
					{
						bExclusion = (abs(azDeg) < 180.0 + AZ_RANGE_JUDGE_DEGREE || abs(azDeg) > 180.0 - AZ_RANGE_JUDGE_DEGREE) ? true : false;
						bExclusion &= (slopeDeg > m_pUIParam->_elecPotential.dAzimuthAngle) ? true : false;
						break;
					}
					case eDirections::NORTH:
					{
						bExclusion = (abs(azDeg) < 0.0 + AZ_RANGE_JUDGE_DEGREE || abs(azDeg) > 360.0 - AZ_RANGE_JUDGE_DEGREE) ? true : false;
						bExclusion &= (slopeDeg > m_pUIParam->_elecPotential.dAzimuthAngle) ? true : false;
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
			if (areaSum < m_pUIParam->_elecPotential.dArea2D)
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

			CVector3D center;		// 中心の座標
			for (auto& pos : roofData.bbPos)
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
			roofData.center = center;


			// メッシュごとの計算結果格納用データを追加
			for (int lc = 0; lc < meshsize; lc++)
			{
				MESHPOSITION_XY meshXY = surface.meshPosList[lc];

				CMeshData mesh;

				std::vector<CVector3D> meshXYZ{
					{meshXY.leftDownX, meshXY.leftDownY, 0.0},
					{meshXY.leftTopX, meshXY.leftTopY, 0.0},
					{meshXY.rightDownX, meshXY.rightDownY, 0.0},
					{meshXY.rightTopX, meshXY.rightTopY, 0.0}
				};
				mesh.meshPos = meshXYZ;

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

				mesh.meshId = CStringEx::Format("%s_%d", surface.roofSurfaceId.c_str(), jc);
				mesh.center = center;
				mesh.centerMod = center;

				tempRoofMesh.emplace_back(mesh);

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
				if (slopeMdDeg < m_pUIParam->_roofSurfaceCorrect.dLowerAngle)
				{
					slopeMdDeg = m_pUIParam->_roofSurfaceCorrect.dTargetAngle;
					azMdDeg = 180.0;	// 南向き

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

							// 南向きに変換
							rotPos.x = orgPos.x;
							rotPos.y = orgPos.y * cos(theta);
							rotPos.z = orgPos.y * sin(theta);

							rotPos += mesh.center;
							meshPos = rotPos;

							// 中心
							centerMd += meshPos;
						}
						centerMd *= 0.25;
						mesh.centerMod = centerMd;
					}
				}

				roofData.slopeModDegree = slopeMdDeg;
				roofData.azModDegree = azMdDeg;

				// メッシュリストを追加
				roofData.vecRoofMesh = tempRoofMesh;

				// 対象屋根面に追加
				bldData.mapRoofSurface[surface.roofSurfaceId] = roofData;

			}
		}

		// 対象屋根面がない
		if (bldData.mapRoofSurface.size() == 0) { continue; }

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

		bldDataMap[build.building] = bldData;

	}
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

// 日射量算出結果のポイントデータを作成
bool CCalcSolarPotentialMng::createPointData(
	std::vector<CPointBase>& vecPoint3d,
	const std::string& Lv3meshId,
	double bbMinX,
	double bbMinY,
	double bbMaxX,
	double bbMaxY,
	double outMeshsize,		// 出力メッシュサイズ(1.0m以下)
	const CBuildingDataMap& bldDataMap,
	const eOutputImageTarget& eTarget
)
{
	// 1mメッシュの中央座標(x,y)と出力したい日射量算出結果(z)
	bool ret = false;

	vecPoint3d.clear();

	int dataCount = (int)m_pvecAllBuildList->size();
	for (int ic = 0; ic < dataCount; ic++)
	{
		const BLDGLIST& bldList = m_pvecAllBuildList->at(ic);
		if (bldList.meshID != Lv3meshId)	continue;

		int bldCount = (int)bldList.buildingList.size();
		for (int jc = 0; jc < bldCount; jc++)
		{
			BUILDINGS build = bldList.buildingList[jc];
			std::string buildId = build.building;
			if (bldDataMap.count(buildId) == 0)		continue;

			const CBuildingData& bldData = bldDataMap.at(buildId);

			int surfaceCount = (int)build.roofSurfaceList.size();
			for (int kc = 0; kc < surfaceCount; kc++)
			{
				ROOFSURFACES surface = build.roofSurfaceList[kc];
				std::string surfaceId = surface.roofSurfaceId;
				if (bldData.mapRoofSurface.count(surfaceId) == 0)	continue;

				const CRoofSurfaceData& surfaceData = bldData.mapRoofSurface.at(surfaceId);
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
							bool bRet = CGeoUtil::IsPointInPolygon(target2d, iCountPoint, ppoint);

							// Z値に日射量算出結果を設定する
							if (bRet)
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
							delete[] ppoint;
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
	addLB = ((tmpMinX > bbMinX && tmpMinX < (bbMinX + 1.0)) && (tmpMinY > bbMinY && tmpMinY < (bbMinY + 1.0))) ? false : true;
	addLT = ((tmpMinX > bbMinX && tmpMinX < (bbMinX + 1.0)) && (tmpMaxY < bbMaxY && tmpMaxY > (bbMaxY - 1.0))) ? false : true;
	addRB = ((tmpMaxX < bbMaxX && tmpMaxX > (bbMaxX - 1.0)) && (tmpMinY > bbMinY && tmpMinY < (bbMinY + 1.0))) ? false : true;
	addRT = ((tmpMaxX < bbMaxX && tmpMaxX > (bbMaxX - 1.0)) && (tmpMaxY < bbMaxY && tmpMaxY > (bbMaxY - 1.0))) ? false : true;
	// 中央
	if (addLB)	vecPoint3d.push_back(CPointBase(bbMinX + 0.5, bbMinY + 0.5, DEF_IMG_NODATA));
	if (addLT)	vecPoint3d.push_back(CPointBase(bbMinX + 0.5, bbMaxY - 0.5, DEF_IMG_NODATA));
	if (addRB)	vecPoint3d.push_back(CPointBase(bbMaxX - 0.5, bbMinY + 0.5, DEF_IMG_NODATA));
	if (addRT)	vecPoint3d.push_back(CPointBase(bbMaxX - 0.5, bbMaxY - 0.5, DEF_IMG_NODATA));

	if (vecPoint3d.size() > 0)	ret = true;

	return ret;
}

// 画像出力
bool CCalcSolarPotentialMng::outputImage(
	const std::wstring strFilePath,
	const std::string& Lv3meshId,
	double bbMinX,
	double bbMinY,
	double bbMaxX,
	double bbMaxY,
	const CBuildingDataMap& bldDataMap,
	const eOutputImageTarget& eTarget
)
{
	if (IsCancel())		return false;

	bool ret = false;

	double outMeshsize = 1.0;

	// 出力用データを作成
	std::vector<CPointBase>* pvecPoint3d = new std::vector<CPointBase>;
	ret = createPointData(*pvecPoint3d, Lv3meshId, bbMinX, bbMinY, bbMaxX, bbMaxY, outMeshsize, bldDataMap, eTarget);

	std::wstring strColorSettingPath = L"";
	if (eTarget == eOutputImageTarget::SOLAR_RAD)
	{
		strColorSettingPath = L"colorSetting_SolarRad.txt";
	}
	else if (eTarget == eOutputImageTarget::SOLAR_POWER)
	{
		strColorSettingPath = L"colorSetting_SolarPower.txt";
	}

	CTiffDataManager tiffDataMng;
	tiffDataMng.SetColorSetting(strColorSettingPath);
	tiffDataMng.SetMeshSize((float)outMeshsize);
	tiffDataMng.SetNoDataVal(DEF_IMG_NODATA);
	tiffDataMng.SetEPSGCode(DEF_EPSGCODE);
	tiffDataMng.SetFilePath(strFilePath);
	ret &= tiffDataMng.AddTiffData(pvecPoint3d);

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

bool CCalcSolarPotentialMng::outputLegendImage()
{
	bool ret = false;

	std::wstring strColorSettingPath = L"";
	std::wstring strMdlPath = CFileUtil::GetModulePathW();
	std::wstring strOutDir = m_pUIParam->strOutputDirPath;

	// 日射量
	strColorSettingPath = L"colorSetting_SolarRad.txt";
	ret = CImageUtil::CreateLegendImage(strColorSettingPath, L"日射量(kWh/m2)");
	if (ret)
	{
		std::wstring colorSetting = CFileUtil::Combine(strMdlPath, strColorSettingPath);
		std::wstring srcPath = GetFUtil()->ChangeFileNameExt(colorSetting, L".jpg");

		std::wstring tmpPath = CFileUtil::Combine(strOutDir, strColorSettingPath);
		std::wstring dstPath = GetFUtil()->ChangeFileNameExt(tmpPath, L".jpg");

		if (MoveFile(srcPath.c_str(), dstPath.c_str()) == FALSE)
		{
			return false;
		}
	}

	// 発電量
	strColorSettingPath = L"colorSetting_SolarPower.txt";
	ret &= CImageUtil::CreateLegendImage(strColorSettingPath, L"発電量(kWh/m2)");
	if (ret)
	{
		std::wstring colorSetting = CFileUtil::Combine(strMdlPath, strColorSettingPath);
		std::wstring srcPath = GetFUtil()->ChangeFileNameExt(colorSetting, L".jpg");

		std::wstring tmpPath = CFileUtil::Combine(strOutDir, strColorSettingPath);
		std::wstring dstPath = GetFUtil()->ChangeFileNameExt(tmpPath, L".jpg");

		if (MoveFile(srcPath.c_str(), dstPath.c_str()) == FALSE)
		{
			return false;
		}
	}

	return ret;
}


// 年間日射量・発電量を出力
bool CCalcSolarPotentialMng::outputResultCSV()
{
	if (m_pmapResultData->empty()) return false;

	std::wstring strOutDir = m_pUIParam->strOutputDirPath;
	if (!GetFUtil()->IsExistPath(strOutDir))
	{
		if (CreateDirectory(strOutDir.c_str(), NULL) == FALSE)
		{
			return false;
		}
	}

	CFileIO file;
	std::wstring strPath = GetFUtil()->Combine(strOutDir, L"建物毎年間予測発電量.csv");
	if (!file.Open(strPath, L"w"))
	{
		return false;
	}

	// ヘッダ部
	if (!file.WriteLineA("3次メッシュID,建物ID,年間予測日射量(kWh/m2),年間予測発電量(kWh),パネル面積,年間予測発電量(kWh/m2),X,Y"))
	{
		return false;
	}

	int dataCount = (int)m_pvecAllBuildList->size();
	for (int ic = 0; ic < dataCount; ic++)
	{
		const BLDGLIST& bldList = m_pvecAllBuildList->at(ic);
		std::string meshId = bldList.meshID;
		if (m_pmapResultData->count(meshId) == 0)	continue;
		const CBuildingDataMap& bldDataMap = m_pmapResultData->at(meshId);

		int bldCount = (int)bldList.buildingList.size();
		for (int jc = 0; jc < bldCount; jc++)
		{
			BUILDINGS build = bldList.buildingList[jc];
			std::string buildId = build.building;
			if (bldDataMap.count(buildId) == 0)	continue;

			const CBuildingData& bldData = bldDataMap.at(buildId);
			std::string strLine = CStringEx::Format("%s,%s,%f,%f,%f,%f,%f,%f",
				meshId.c_str(),
				buildId.c_str(),
				bldData.solarRadiationUnit,
				bldData.solarPower,
				bldData.panelArea,
				bldData.solarPowerUnit,
				bldData.center.x,
				bldData.center.y
			);
			file.WriteLineA(strLine);
		}
	}

	file.Close();

	return true;
}

// 月別日射量CSV出力
bool CCalcSolarPotentialMng::outputMonthlyRadCSV(
	const std::string& Lv3meshId,
	const CBuildingDataMap& dataMap, 
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
	if (!file.WriteLineA("建物ID,屋根面ID,MeshId,年,月,日射量(Wh/m2),日射量(MJ/m2)"))
	{
		return false;
	}

	int dataCount = (int)m_pvecAllBuildList->size();
	for (int ic = 0; ic < dataCount; ic++)
	{
		const BLDGLIST& bldList = m_pvecAllBuildList->at(ic);
		if (bldList.meshID != Lv3meshId)	continue;

		int bldCount = (int)bldList.buildingList.size();
		for (int jc = 0; jc < bldCount; jc++)
		{
			BUILDINGS build = bldList.buildingList[jc];
			std::string buildId = build.building;
			if (dataMap.count(buildId) == 0)	continue;

			const CBuildingData& bldData = dataMap.at(buildId);

			int surfaceCount = (int)build.roofSurfaceList.size();
			for (int kc = 0; kc < surfaceCount; kc++)
			{
				ROOFSURFACES surface = build.roofSurfaceList[kc];
				std::string surfaceId = surface.roofSurfaceId;
				if (bldData.mapRoofSurface.count(surfaceId) == 0)	continue;

				const CRoofSurfaceData& surfaceData = bldData.mapRoofSurface.at(surfaceId);

				for (auto& mesh : surfaceData.vecRoofMesh)
				{
					for (int month = 1; month <= 12; month++)
					{
						double val = mesh.solarRadiation[month - 1];

						std::string strLine = CStringEx::Format("%s,%s,%s,%d,%d,%f,%f",
							buildId.c_str(),
							surfaceId.c_str(),
							mesh.meshId.c_str(),
							GetYear(),
							month,
							val,
							val / 1000 * 3.6
						);
						file.WriteLineA(strLine);
					}
				}

			}
		}
	}
	file.Close();

	return true;
}


// 月別日射量CSV出力
bool CCalcSolarPotentialMng::outputRoofRadCSV(
	const std::string& Lv3meshId,			// 3次メッシュID
	const CBuildingDataMap& dataMap,
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
	// デバッグ用出力
	std::wstring strPath = GetFUtil()->Combine(wstrOutDir, L"屋根面別年間日射量.csv");

	CFileIO file;
	if (!file.Open(strPath, L"w"))
	{
		return false;
	}
	// ヘッダ部
	if (!file.WriteLineA("建物ID,屋根面ID,年,1m2当たりの日射量(kWh/m2),→(MJ/m2),屋根面全体の日射量(kWh),→(MJ)"))
	{
		return false;
	}

	int dataCount = (int)m_pvecAllBuildList->size();
	for (int ic = 0; ic < dataCount; ic++)
	{
		const BLDGLIST& bldList = m_pvecAllBuildList->at(ic);
		if (bldList.meshID != Lv3meshId)	continue;

		int bldCount = (int)bldList.buildingList.size();
		for (int jc = 0; jc < bldCount; jc++)
		{
			BUILDINGS build = bldList.buildingList[jc];
			std::string buildId = build.building;
			if (dataMap.count(buildId) == 0)	continue;

			const CBuildingData& bldData = dataMap.at(buildId);

			int surfaceCount = (int)build.roofSurfaceList.size();
			for (int kc = 0; kc < surfaceCount; kc++)
			{
				ROOFSURFACES surface = build.roofSurfaceList[kc];
				std::string surfaceId = surface.roofSurfaceId;
				if (bldData.mapRoofSurface.count(surfaceId) == 0)	continue;

				const CRoofSurfaceData& surfaceData = bldData.mapRoofSurface.at(surfaceId);

				// デバッグ用出力
				std::string strLine = CStringEx::Format("%s,%s,%d,%f,%f,%f,%f",
					buildId.c_str(),
					surfaceId.c_str(),
					GetYear(),
					surfaceData.solarRadiationUnit,
					surfaceData.solarRadiationUnit * 3.6,
					surfaceData.solarRadiation,
					surfaceData.solarRadiation * 3.6
				);
				file.WriteLineA(strLine);
			}
		}
	}
	file.Close();

	return true;
}


// 建物ごとの方位角中間データ出力
bool CCalcSolarPotentialMng::outputAzimuthDataCSV()
{
	if (m_pmapResultData->empty())	return false;

	bool ret = false;

	// 出力パス
	std::wstring strCsvPath = GetINIParam()->GetAzimuthCSVPath();
	std::wstring strTempDir = GetFUtil()->GetParentDir(m_pUIParam->strOutputDirPath) + L"output";
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
	if (!file.WriteLineA("3次メッシュID,建物ID,同一屋根面数,方位角(平均値)"))
	{
		return false;
	}

	// 建物ごとの方位角データ書き込み
	for (const auto& result : *m_pmapResultData)
	{
		std::string meshId = result.first;
		const CBuildingDataMap& bldDataMap = result.second;

		for (const auto& bldMap : bldDataMap)
		{
			std::string buildId = bldMap.first;
			const CBuildingData& bldData = bldMap.second;

			int roofsize = (int)bldData.mapRoofSurface.size();
			if (roofsize == 0)	continue;

			std::string strAzimuths = "";
			for (auto val : bldData.mapRoofSurface)
			{
				CRoofSurfaceData roofData = val.second;
				std::string strTemp = CStringEx::Format("%f,", roofData.azModDegree);
				strAzimuths += strTemp;
			}

			// 建物ID, 同一屋根面数, 方位角(平均値)・・・"
			std::string strLine = CStringEx::Format("%s,%s,%d,%s", meshId.c_str(), buildId.c_str(), roofsize, strAzimuths.c_str());

			file.WriteLineA(strLine);
		}
	}

	file.Close();

	return true;

}


// SHPに付与
bool CCalcSolarPotentialMng::setTotalSolarRadiationToSHP()
{
	return true;

}

// 建物の中心に入射光があたっているか
bool CCalcSolarPotentialMng::IntersectRoofSurfaceCenter(
	const CVector3D& inputVec,						// 入射光
	const std::vector<CVector3D>& roofMesh,			// 対象の屋根BB
	const std::string& strId,						// 対象の屋根ID
	const vector<BLDGLIST>& neighborBuildings,		// 周辺の建物リスト
	const vector<DEMLIST>& neighborDems				// 周辺の地形DEMリスト
)
{
	// 光線の有効距離
	constexpr double LIGHT_LENGTH = 500.0;

	// 屋根メッシュの座標
	CVector3D roofMeshPos;
	for (const auto& mesh : roofMesh)
		roofMeshPos += mesh;
	roofMeshPos *= 0.25;	// 4点の平均
	// 屋根メッシュの法線
	CVector3D n;
	CGeoUtil::OuterProduct(
		CVector3D(roofMesh[1], roofMesh[0]),
		CVector3D(roofMesh[2], roofMesh[1]), n);
	if (n.z < 0) n *= -1;

	// 屋根面メッシュの裏側から入射光が当たっているときは反射しないので解析終了
	if (CGeoUtil::InnerProduct(n, inputVec) >= 0.0)
		return false;

	// 入射光の光源を算出
	// 屋根メッシュ座標の延長線上500mに設定する
	CVector3D inputInverseVec = CGeoUtil::Normalize(inputVec) * ((-1) * LIGHT_LENGTH);
	CVector3D sunPos = roofMeshPos + inputInverseVec;
	CLightRay lightRay(sunPos, CGeoUtil::Normalize(inputVec) * LIGHT_LENGTH);

	// 入射光が周りの建物に邪魔されずに屋根面に当たるかチェック
	if (intersectBuildings(lightRay, strId, neighborBuildings))
	{
		// 建物にあたっている場合は屋根メッシュに光線があたっていないので解析終了
		return false;
	}

	// 入射光が周りの地形に邪魔されずに建物に当たるかチェック
	if (IsEnableDEMData() && roofMeshPos.z > GetINIParam()->GetDemHeight())
	{
		if (intersectLandDEM(lightRay, neighborDems))
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
	constexpr double LIGHT_LENGTH = 550.0; //余裕を持たせる
	constexpr double SQUARE_LINGHT_LENGTH = LIGHT_LENGTH * LIGHT_LENGTH;

	const CVector3D lightRayPos = lightRay.GetPos();
	const CVector3D lightRayVec = lightRay.GetVector();

	bool bDist = false;
	bool bDirect = false;
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

// 対象建物に隣接するメッシュを取得
void CCalcSolarPotentialMng::GetNeighborBuildings(
	const std::string& targetMeshId,
	const CVector3D& bldCenter,
	std::vector<BLDGLIST>& neighborBuildings
)
{
	const double DIST = GetINIParam()->GetNeighborBuildDist_SolarRad();	// 隣接するBBoxの範囲[m]

	// 建物中心XY
	CVector2D bldCenterXY(bldCenter.x, bldCenter.y);

	for (const auto& building : *m_pvecAllBuildList)
	{
		if (!isNeighborMesh(targetMeshId, building.meshID))	continue;

		BLDGLIST tmpBldList = building;
		tmpBldList.buildingList.clear();
		tmpBldList.buildingListLOD1.clear();

		// 範囲内にあるか
		// LOD2
		for (const auto& build : building.buildingList)
		{
			double bbBldMinX = DBL_MAX, bbBldMinY = DBL_MAX;
			double bbBldMaxX = -DBL_MAX, bbBldMaxY = -DBL_MAX;

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
					}
				}
			}

			double buildCenterX = ((int64_t)bbBldMaxX + (int64_t)bbBldMinX) * 0.5;
			double buildCenterY = ((int64_t)bbBldMaxY + (int64_t)bbBldMinY) * 0.5;
			// 中心同士の距離
			double tmpdist = bldCenterXY.Distance(buildCenterX, buildCenterY);
			// DIST以内の距離のとき近隣とする
			if (tmpdist <= DIST)
			{
				tmpBldList.buildingList.emplace_back(build);
				continue;
			}
		}

		// LOD1
		for (const auto& build : building.buildingListLOD1)
		{
			double bbBldMinX = DBL_MAX, bbBldMinY = DBL_MAX;
			double bbBldMaxX = -DBL_MAX, bbBldMaxY = -DBL_MAX;

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
					}
				}
			}

			double buildCenterX = ((int64_t)bbBldMaxX + (int64_t)bbBldMinX) * 0.5;
			double buildCenterY = ((int64_t)bbBldMaxY + (int64_t)bbBldMinY) * 0.5;
			// 中心同士の距離
			double tmpdist = bldCenterXY.Distance(buildCenterX, buildCenterY);
			// DIST以内の距離のとき近隣とする
			if (tmpdist <= DIST)
			{
				tmpBldList.buildingListLOD1.emplace_back(build);
				continue;
			}
		}

		if (tmpBldList.buildingList.empty() && tmpBldList.buildingListLOD1.empty())	continue;

		neighborBuildings.emplace_back(tmpBldList);
	}

}

// 対象建物に隣接するDEMを取得
void CCalcSolarPotentialMng::GetNeighborDems(
	const std::string& targetMeshId,
	const CVector3D& bldCenter,
	std::vector<DEMLIST>& neighborDems
)
{
	// 除外判定
	if (bldCenter.z < GetINIParam()->GetDemHeight())	return;

	const double DIST = GetINIParam()->GetDemDist();	// 対象範囲[m]

	// 建物中心XY
	CVector2D bldCenterXY(bldCenter.x, bldCenter.y);

	for (const auto& dem : *m_pvecAllDemList)
	{
		if (isNeighborMesh(targetMeshId, dem.meshID))
		{
			DEMLIST targetDem;
			targetDem.meshID = dem.meshID;
			targetDem.posTriangleList.clear();

			for (const auto& triangle : dem.posTriangleList)
			{
				// XY平面の重心を求める
				double x = (triangle.posTriangle[0].x + triangle.posTriangle[1].x + triangle.posTriangle[2].x) / 3.0;
				double y = (triangle.posTriangle[0].y + triangle.posTriangle[1].y + triangle.posTriangle[2].y) / 3.0;
				double z = (triangle.posTriangle[0].z + triangle.posTriangle[1].z + triangle.posTriangle[2].z) / 3.0;

				double tmpdist = bldCenterXY.Distance(x, y);

				bool bAddList = false;

				// 判定
				if (tmpdist <= DIST &&	// 対象の建物(中心)との距離が対象範囲内
					bldCenter.z < z)	// 対象の建物(中心)より高い位置にある
				{
					targetDem.posTriangleList.emplace_back(triangle);
				}
			}

			// 対象範囲で間引いたDEMを追加する
			neighborDems.emplace_back(targetDem);
		}

	}
}


// 地形に光線があたっているかどうか
bool CCalcSolarPotentialMng::intersectLandDEM(
	const CLightRay& lightRay,					// 光線
	//const double& height,						// 屋根標高
	const vector<DEMLIST>& demList				// 光線があたっているかチェックする地形のDEM
)
{
	double tempDist;
	CVector3D tempTargetPos;

	const CVector3D lightRayPos = lightRay.GetPos();
	const CVector3D lightRayVec = lightRay.GetVector();

	for (const auto& dem : demList)
	{
		for (const auto& triangle : dem.posTriangleList)
		{
			if (IsCancel())		return false;

			vector<CVector3D> posList;

			posList.emplace_back(CVector3D(triangle.posTriangle[0].x, triangle.posTriangle[0].y, triangle.posTriangle[0].z));
			posList.emplace_back(CVector3D(triangle.posTriangle[1].x, triangle.posTriangle[1].y, triangle.posTriangle[1].z));
			posList.emplace_back(CVector3D(triangle.posTriangle[2].x, triangle.posTriangle[2].y, triangle.posTriangle[2].z));

			bool bDirect = false;
			for (const auto& pos : triangle.posTriangle)
			{
				double dx = pos.x - lightRayPos.x;
				double dy = pos.y - lightRayPos.y;
				double dz = pos.z - lightRayPos.z;

				// 平面の頂点が逆方向にあるときは範囲外とする
				double dot = CGeoUtil::InnerProduct(lightRayVec, CVector3D(dx, dy, dz));
				if (dot > 0.0)
				{
					bDirect = true;
					break;
				}
			}
			if (!bDirect)	continue;

			// 光線とポリゴンの交点を探す
			if (lightRay.Intersect(posList, &tempTargetPos, &tempDist))
			{
				return true;
			}

		}
	}

	return false;
}

bool CCalcSolarPotentialMng::IsCancel()
{
	return GetFUtil()->IsExistPath(m_strCancelFilePath);
}

bool CCalcSolarPotentialMng::IsEnableDEMData()
{
	return m_pUIParam->bEnableDEMData;
}

// 3次メッシュが隣接(周囲8方向)しているか
bool CCalcSolarPotentialMng::isNeighborMesh(const std::string& meshId1, const std::string& meshId2)
{
	bool bret = false;

	// 同じメッシュ
	if (meshId1 == meshId2)
	{
		return true;
	}

	// 下2桁の数字からメッシュの位置パターンを判定
	bool bTop = (meshId1.substr(6, 1) == "9") ? true : false;		// 7桁目が9なら北側
	bool bBottom = (meshId1.substr(6, 1) == "0") ? true : false;	// 7桁目が0なら南側
	bool bLeft = (meshId1.substr(7, 1) == "0") ? true : false;		// 8桁目が9なら西側
	bool bRight = (meshId1.substr(7, 1) == "9") ? true : false;		// 8桁目が9なら東側

	int nRet = 0;

	//北
	if (bTop) nRet = (stoi(meshId1) + 1000 - 90);
	else      nRet = (stoi(meshId1) + 10);
	std::string strId_N = CStringEx::Format("%d", nRet);

	//南
	if (bBottom) nRet = (stoi(meshId1) - 1000 + 90);
	else      nRet = (stoi(meshId1) - 10);
	std::string strId_S = CStringEx::Format("%d", nRet);

	//東
	if (bRight) nRet = (stoi(meshId1) + 100 - 9);
	else      nRet = (stoi(meshId1) + 1);
	std::string strId_E = CStringEx::Format("%d", nRet);

	//西
	if (bLeft) nRet = (stoi(meshId1) - 100 + 9);
	else      nRet = (stoi(meshId1) - 1);
	std::string strId_W = CStringEx::Format("%d", nRet);

	//北東
	if (bTop && bRight) nRet = (stoi(meshId1) + 1000 + 1);
	else if (bRight) nRet = (stoi(strId_E) + 10);
	else      nRet = (stoi(strId_N) + 1);
	std::string strId_NE = CStringEx::Format("%d", nRet);

	//南東
	if (bBottom && bRight) nRet = (stoi(strId_S) + 100 - 9);
	else if (bRight) nRet = (stoi(strId_E) - 10);
	else      nRet = (stoi(strId_S) + 1);
	std::string strId_SE = CStringEx::Format("%d", nRet);

	//南西
	if (bBottom && bLeft) nRet = (stoi(meshId1) - 1000 - 1);
	else if (bLeft) nRet = (stoi(strId_W) - 10);
	else      nRet = (stoi(strId_S) - 1);
	std::string strId_SW = CStringEx::Format("%d", nRet);

	//北西
	if (bTop && bLeft) nRet = (stoi(strId_N) - 100 + 9);
	else if (bLeft) nRet = (stoi(strId_W) + 10);
	else      nRet = (stoi(strId_N) - 1);
	std::string strId_NW = CStringEx::Format("%d", nRet);

	if (strId_N == meshId2)			return true;
	else if (strId_S == meshId2)	return true;
	else if (strId_E == meshId2)	return true;
	else if (strId_W == meshId2)	return true;
	else if (strId_NE == meshId2)	return true;
	else if (strId_SE == meshId2)	return true;
	else if (strId_SW == meshId2)	return true;
	else if (strId_NW == meshId2)	return true;

	return false;
}
