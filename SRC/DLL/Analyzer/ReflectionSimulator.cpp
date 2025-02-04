#include "pch.h"
#include "ReflectionSimulator.h"
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <string>

#include <CommonUtil/CGeoUtil.h>
#include <CommonUtil/CLightRay.h>
#include <CommonUtil/ReadINIParam.h>
#include "ReflectionSimulatorMesh.h"
#include "AnalysisReflectionMesh.h"
#include "AnalyzeData.h"

using namespace std;


CReflectionSimulator::CReflectionSimulator(const CTime& date, CUIParam* pUIParam)
	: m_date(date)
	, m_pParam(pUIParam)
	, m_bExec(eExitCode::Normal)
	, m_buildingIndex(0)
{
	Init();

	if (pUIParam)
	{
		m_roofCorrectLower = *pUIParam->pReflectionParam->pRoof_Lower;
		m_roofCorrectUpper = *pUIParam->pReflectionParam->pRoof_Upper;
	}
}

CReflectionSimulator::~CReflectionSimulator()
{
	if (m_pSunVector)
	{
		delete m_pSunVector;
		m_pSunVector = nullptr;
	}
}

// 太陽位置の緯度経度を設定する
void CReflectionSimulator::SetSunVector(double lat, double lon)
{
	// 指定日の太陽光を生成
	if (m_pSunVector)
		delete m_pSunVector;
	m_pSunVector = new CSunVector(lat, lon, m_date);
}

// 反射シミュレーション解析を実行する
eExitCode CReflectionSimulator::Exec(
	const vector<AREADATA>& analyzeAreas
)
{
	assert(m_pParam);

	InitResult();

	// 24時間の解析を行う
	for (uint8_t hour = 0; hour < 24; ++hour)
	{
		CAnalysisReflection result;
		bool ret = true;

		for (const auto& area : analyzeAreas)
		{
			if (area.analyzeBuild)
			{
				vector<BLDGLIST> targetBuildings{};	// 反射解析する建物

				// 解析対象の建物リストをメッシュごとに作成
				for (const auto& bldList : area.neighborBldgList)
				{
					if (area.targetBuildings.find(bldList->meshID) == area.targetBuildings.end())	continue;

					BLDGLIST tmpBldList = *bldList;
					tmpBldList.buildingList.clear();

					const auto tmpBuildings = area.targetBuildings.at(bldList->meshID);
					for (auto bld : tmpBuildings)
					{
						tmpBldList.buildingList.emplace_back(*bld);
					}

					targetBuildings.emplace_back(tmpBldList);
				}

				ret &= AnalyzeBuildings(targetBuildings, area.neighborBldgList, hour, result);
			}

			// キャンセル
			if (IsCancel())
			{
				m_bExec = eExitCode::Cancel;
				return m_bExec;
			}

			if (area.analyzeLand)
			{
				ret &= AnalyzeLand(area, area.neighborBldgList, hour, result);
			}

			// キャンセル
			if (IsCancel())
			{
				m_bExec = eExitCode::Cancel;
				return m_bExec;
			}

		}

		// 解析したIDの順番を結果に設定する
		SetBuildingIndex(result);

		// 結果を時間ごとの配列に保持
		SetResult(hour, result);

		// キャンセル
		if (IsCancel())
		{
			m_bExec = eExitCode::Cancel;
			return m_bExec;
		}
	}

	m_bExec = eExitCode::Normal;
	return m_bExec;
}

// 反射シミュレーション結果をCSVファイルに出力する
bool CReflectionSimulator::OutResult(const std::string& csvfile) const
{
	ofstream ofs;
	ofs.open(csvfile);
	if (!ofs.is_open())
		return false;

	// ヘッダー部
	ofs << "建物/土地ID,屋根面ID,ｼﾐｭﾚｰｼｮﾝ日時,\
反射点座標.X(ｍ),反射点座標.Y(ｍ),反射点座標.Z(ｍ),\
反射先座標.X(ｍ),反射先座標.Y(ｍ),反射先座標.Z(ｍ),\
反射先" << endl;

	// データ部
	// m_resultには配列順に0～23時の結果データが入っている
	int hour = 0;
	for (const auto& resultVector : m_result)
	{
		for (const auto& result : resultVector)
		{
			// 同一建物は出力しない
			if (result.reflectionTarget.buildingId == result.reflectionRoof.buildingId)
				continue;

			string strDate = CStringEx::Format("%d/%02d/%02d %02d:%02d",
				m_date.iYear, m_date.iMonth, m_date.iDay, hour, 0);

			ofs << fixed << setprecision(3)
				<< result.reflectionRoof.buildingId << ","
				<< result.reflectionRoof.roofSurfaceId << ","
				<< strDate << ","
				<< result.reflectionRoof.targetPos.x << ","
				<< result.reflectionRoof.targetPos.y << ","
				<< result.reflectionRoof.targetPos.z << ","
				<< result.reflectionTarget.targetPos.x << ","
				<< result.reflectionTarget.targetPos.y << ","
				<< result.reflectionTarget.targetPos.z << ",";
			if (result.reflectionTarget.buildingId == result.reflectionRoof.buildingId)
				ofs << "同一建物" << endl;
			else
				ofs << result.reflectionTarget.buildingId << endl;
		}
		hour++;
	}

	ofs.close();

	return true;
}

// 反射シミュレーション結果をCZMLファイルに出力する
bool CReflectionSimulator::OutResultCZML(const std::string& czmlfile) const
{
	return OutResultCZML(czmlfile, m_result);
}

// CZMLファイルに出力する
bool CReflectionSimulator::OutResultCZML(
	const std::string& czmlfile,			// czmlファイルパス
	const CAnalysisReflectionOneDay& result	// czmlに出力するデータ
)
{
	ofstream ofs;
	ofs.open(czmlfile);
	if (!ofs.is_open())
		return false;

	ofs << "[{\"id\":\"document\",\"name\":\"Light Trail\",\"version\":\"1.0\"}";

	// 拡張子なしファイル名
	filesystem::path filepath(czmlfile);
	string czmlfilename = filepath.stem().string();

	// 色をczmlファイル名で決める
	string rgba;
	if (czmlfilename == "summer")
		rgba = "255,80,80,204";
	else if (czmlfilename == "spring")
		rgba = "169,208,142,204";
	else if (czmlfilename == "winter")
		rgba = "91,155,213,204";
	else // 指定日
		rgba = "128,128,128,204";

	// 座標系番号を取得
	const int JPZONE = GetINIParam()->GetJPZone();

	// ジオイドファイルを読む
	CGeoid* pGeoid = nullptr;
	double dOriginLat, dOriginLon;
	CGeoUtil::XYToLatLon(JPZONE, 0.0, 0.0, dOriginLat, dOriginLon);
	wstring strFilePath = GetFUtil()->Combine(GetFUtil()->GetModulePathW(), L"data/Geoid/gsigeo2011_ver1.asc");
	pGeoid = new CGeoid(CGeoid::GEOID_GEOID2000, strFilePath.data(), dOriginLat, dOriginLon);
	pGeoid->Load();

	// CZMLにLine出力
	OutResultCZMLLine(ofs, result, JPZONE, pGeoid, rgba);
	
	// CZMLにPoint出力
	OutResultCZMLPoint(ofs, result, JPZONE, pGeoid, rgba, czmlfilename);

	delete pGeoid;

	ofs << "]";

	ofs.close();

	return true;
}

// 結果CSVをCZMLに変換する
bool CReflectionSimulator::ConvertResultCSVtoCZML(
	const std::string& csvfile,		// 変換するCSVファイルパス
	const std::string& czmlfile		// 出力するCZMLファイルパス
)
{
	// 結果CSVファイルの全行をリストに格納
	vector<string> csvlines;
	if (!ReadResultCSVLines(csvfile, csvlines))
		return false;

	// 結果CSVのデータを結果に格納する
	CAnalysisReflectionOneDay result;
	CAnalysisReflection resultReflection;
	// CZMLに出力するときは、時間や屋根面によってリストを分ける必要ないため、すべての一つのリストに格納しておく
	vector<string> datas;
	for (const auto& csvline : csvlines)
	{
		CFileUtil::SplitCSVData(csvline, &datas);
		if (datas.size() < 9)
			return false;
		// 建物ID
		string buildingId = datas[0];
		// 反射点座標
		double x1 = stod(datas[3]);
		double y1 = stod(datas[4]);
		double z1 = stod(datas[5]);
		// 反射先座標
		double x2 = stod(datas[6]);
		double y2 = stod(datas[7]);
		double z2 = stod(datas[8]);

		// 結果リストに格納
		CAnalysisReflectionMesh resultReflectionMesh;
		resultReflectionMesh.reflectionRoof.buildingId = buildingId;
		resultReflectionMesh.reflectionRoof.targetPos.x = x1;
		resultReflectionMesh.reflectionRoof.targetPos.y = y1;
		resultReflectionMesh.reflectionRoof.targetPos.z = z1;
		resultReflectionMesh.reflectionTarget.targetPos.x = x2;
		resultReflectionMesh.reflectionTarget.targetPos.y = y2;
		resultReflectionMesh.reflectionTarget.targetPos.z = z2;
		resultReflection.emplace_back(resultReflectionMesh);
	}
	result.emplace_back(resultReflection);

	// CZMLファイル出力
	return OutResultCZML(czmlfile, result);
}

// CZMLのLineを出力
void CReflectionSimulator::OutResultCZMLLine(
	ofstream& ofs,
	const CAnalysisReflectionOneDay& result,
	int JPZONE,
	CGeoid* pGeoid,
	const string& rgba
)
{
	int id_counter = 0;
	for (const auto& resultVector : result)
	{
		for (const auto& result : resultVector)
		{
			// 同一建物は出力しない
			if (result.reflectionTarget.buildingId == result.reflectionRoof.buildingId)
				continue;

			// 反射点座標を緯度経度に変換
			double roof_lat, roof_lon, roof_z;
			ConvertXYZToLatLonZ(
				result.reflectionRoof.targetPos.x,
				result.reflectionRoof.targetPos.y,
				result.reflectionRoof.targetPos.z,
				JPZONE, pGeoid,
				roof_lat, roof_lon, roof_z
			);

			// 反射先座標を緯度経度に変換
			double target_lat, target_lon, target_z;
			ConvertXYZToLatLonZ(
				result.reflectionTarget.targetPos.x,
				result.reflectionTarget.targetPos.y,
				result.reflectionTarget.targetPos.z,
				JPZONE, pGeoid,
				target_lat, target_lon, target_z
			);

			ofs << fixed << setprecision(6)
				<< ",{"
				<< "\"id\":\"line_" << id_counter << "\","
				<< "\"name\":\"" << result.reflectionRoof.buildingId << "\","
				<< "\"description\":null,"
				<< "\"polyline\":{"
				<< "\"positions\":{"
				<< "\"cartographicDegrees\":["
				<< roof_lon << ","
				<< roof_lat << ",";
			ofs << fixed << setprecision(3)
				<< roof_z << ",";
			ofs << fixed << setprecision(6)
				<< target_lon << ","
				<< target_lat << ",";
			ofs << fixed << setprecision(3)
				<< target_z << "]"
				<< "},"
				<< "\"width\":2,"
				<< "\"material\":{"
				<< "\"solidColor\":{"
				<< "\"color\":{\"rgba\":[" << rgba << "]}"
				<< "}"
				<< "}"
				<< "}"
				<< "}";

			id_counter++;
		}
	}
}

// CZMLのPointを出力
void CReflectionSimulator::OutResultCZMLPoint(
	ofstream& ofs,
	const CAnalysisReflectionOneDay& result,
	int JPZONE,
	CGeoid* pGeoid,
	const string& rgba,
	const string& czmlfilename
)
{
	// gltf
	string gltf = "model/" + czmlfilename + ".glb";

	int id_counter = 0;
	for (const auto& resultVector : result)
	{
		for (const auto& result : resultVector)
		{
			// 同一建物は出力しない
			if (result.reflectionTarget.buildingId == result.reflectionRoof.buildingId)
				continue;

			// 反射先座標を緯度経度に変換
			double target_lat, target_lon, target_z;
			ConvertXYZToLatLonZ(
				result.reflectionTarget.targetPos.x,
				result.reflectionTarget.targetPos.y,
				result.reflectionTarget.targetPos.z,
				JPZONE, pGeoid,
				target_lat, target_lon, target_z
			);

			ofs << fixed << setprecision(6)
				<< ",{"
				<< "\"id\":\"point_" << id_counter << "\","
				<< "\"name\":\"" << result.reflectionRoof.buildingId << "\","
				<< "\"description\":null,"
				<< "\"position\":{"
				<< "\"cartographicDegrees\":["
				<< target_lon << ","
				<< target_lat << ",";
			ofs << fixed << setprecision(3)
				<< target_z << "]"
				<< "},"
				<< "\"model\":{"
				<< "\"gltf\":\"" << gltf << "\","
				<< "\"scale\":0.25,"
				<< "\"color\":{"
				<< "\"rgba\":[" << rgba << "]},"
				<< "\"colorBlendMode\":{"
				<< "\"colorBlendMode\":\"REPLACE\"},"
				<< "\"shadows\":{\"shadowMode\":\"DISABLED\"}"
				<< "}"
				<< "}";

			id_counter++;
		}
	}
}

// XYZをCZMLの緯度経度、高さに変換する
void CReflectionSimulator::ConvertXYZToLatLonZ(
	double x, double y, double z,
	int JPZONE, CGeoid* pGeoid,
	double& lat, double& lon, double& h
)
{
	// 緯度経度に変換
	CGeoUtil::XYToLatLon(
		JPZONE,	// 座標系
		y,		// 北
		x,		// 東
		lat,	// 緯度
		lon		// 経度
	);
	
	// 高さ
	h = z;
	if (pGeoid)
	{
		// ジオイド高があるときは標高を楕円体高に変換
		double dGeoidHeight;
		if (pGeoid->Extract(lat, lon, &dGeoidHeight) == CGeoid::NOERROR_)
		{
			h = z + dGeoidHeight;
		}
	}
}

// 結果ファイルの全行を読み込む
bool CReflectionSimulator::ReadResultCSVLines(
	const string& csvfile,		// 結果CSVファイル
	std::vector<string>& lines	// 読み込み行のリスト
)
{
	lines.clear();

	ifstream ifs;
	ifs.open(csvfile);
	if (!ifs.is_open())
		return false;

	string line;
	// 1行目はヘッダー部なので読み飛ばす
	getline(ifs, line);
	// 2行目以降のデータ部
	while (getline(ifs, line))
	{
		lines.emplace_back(line);
	}

	ifs.close();

	return true;
}

// 解析結果を取得する
const CAnalysisReflectionOneDay& CReflectionSimulator::GetResult() const
{
	return m_result;
}

// Exec()実行結果
eExitCode CReflectionSimulator::GetExecResult() const
{
	return m_bExec;
}

void CReflectionSimulator::Init()
{
	InitResult();

	if (m_pSunVector)
	{
		delete m_pSunVector;
		m_pSunVector = nullptr;
	}
}

void CReflectionSimulator::InitResult()
{
	m_result.clear();
	m_result.resize(24);	// 24時間の解析結果
}

void CReflectionSimulator::SetResult(uint8_t hour, const CAnalysisReflection& result)
{
	m_result[hour] = result;
}

// 全建物での解析
bool CReflectionSimulator::AnalyzeBuildings(
	const std::vector<BLDGLIST>& targetBuildings,
	const std::vector<BLDGLIST*>& buildings,
	uint8_t hour,
	CAnalysisReflection& result
)
{
	bool ret = false;	// 解析結果がないときfalse

	for (const auto& targetBldgList : targetBuildings)
	{
		double lat, lon;
		CGeoUtil::MeshIDToLatLon(targetBldgList.meshID, lat, lon);

		HorizontalCoordinate sunPos;
		SetSunVector(lat, lon);
		m_pSunVector->GetPos(hour, sunPos);

		// 高度が0以上の場合のみ反射シミュレーション解析を行う
		if (sunPos.altitude >= 0)
		{
			// 入射光のベクトル
			CVector3D sunVector;
			m_pSunVector->GetVector(hour, sunVector);

			// 対象メッシュの隣接するメッシュを取得
			vector<BLDGLIST> neighborBuildings;
			GetNeighborBuildings(targetBldgList, buildings, neighborBuildings);

			for (const auto& building : targetBldgList.buildingList)
			{
				// 建物IDの処理順を記録する
				m_buildingIndex++;
				if (m_mapBuildingIndex.find(building.building) == m_mapBuildingIndex.end())
					m_mapBuildingIndex[building.building] = m_buildingIndex;

				// 1建物の反射解析
				if (AnalyzeBuilding(building, neighborBuildings, sunVector, result))
					ret = true;

				// キャンセル
				if (IsCancel())
				{
					return false;
				}
			}
		}
	}

	return ret;
}

// 1建物での解析
bool CReflectionSimulator::AnalyzeBuilding(
	const BUILDINGS& building,
	const std::vector<BLDGLIST>& buildings,
	const CVector3D& sunVector,
	CAnalysisReflection& result
)
{
	bool ret = false;
	for (const auto& roof : building.roofSurfaceList)
	{
		if (AnalyzeRoof(roof, building, buildings, sunVector, result))
			ret = true;

		// キャンセル
		if (IsCancel())
		{
			return false;
		}
	}

	return ret;
}

// 1屋根での解析
bool CReflectionSimulator::AnalyzeRoof(
	const ROOFSURFACES& roof,
	const BUILDINGS& building,
	const std::vector<BLDGLIST>& buildings,
	const CVector3D& sunVector,
	CAnalysisReflection& result
)
{
	bool ret = false;

	// 屋根面全体の座標リストを作成
	vector<CPointBase> posList = {};
	for (const auto& polygon : roof.roofSurfaceList)
	{
		posList.insert(posList.end(), polygon.posList.begin(), polygon.posList.end() - 1);	// 構成点の始点と終点は同じ点なので除外する
	}

	for (const auto& mesh : roof.meshPosList)
	{
		CAnalysisReflectionMesh resultMesh;
		bool res = AnalyzeMesh(mesh, posList, /*building,*/ buildings, sunVector, resultMesh);
		if (res)
		{
			resultMesh.reflectionRoof.buildingId = building.building;
			resultMesh.reflectionRoof.roofSurfaceId = roof.roofSurfaceId;
			result.emplace_back(resultMesh);
			ret = true;
		}

		// キャンセル
		if (IsCancel())
		{
			return false;
		}
	}

	return ret;
}

// 1屋根メッシュでの解析
bool CReflectionSimulator::AnalyzeMesh(
	const MESHPOSITION_XY& mesh,
	const vector<CPointBase>& posList,
	//const BUILDINGS& building,
	const vector<BLDGLIST>& buildings,
	const CVector3D& sunVector,
	CAnalysisReflectionMesh& result
)
{
	// メッシュのz座標を算出(CLightRay::Intersect()の平面と線分の交点計算を参考)
	vector<CVector3D> meshXYZ {
		{mesh.leftDownX, mesh.leftDownY, 0.0},
		{mesh.leftTopX, mesh.leftTopY, 0.0},
		{mesh.rightDownX, mesh.rightDownY, 0.0},
		{mesh.rightTopX, mesh.rightTopY, 0.0}
	};
	// 屋根面の法線
	CVector3D n;
	{
		CVector3D vec1;
		CVector3D vec2;
		// [0]からの各点のベクトル
		vector<CVector3D> vecPolyList;
		for (int i = 1; i < posList.size(); ++i)
		{
			CVector3D vec(posList[i], posList[0]);
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
		CGeoUtil::OuterProduct(vec1, vec2, n);
		if (n.z < 0.0) n *= -1;
	}

	// 平面の式
	CVector3D p(posList[0].x, posList[0].y, posList[0].z);
	double d = CGeoUtil::InnerProduct(p, n);
	// 垂線と法線
	CVector3D inVec(0.0, 0.0, 1.0);
	double dot = CGeoUtil::InnerProduct(n, inVec);
	CVector3D center;		// 中心の座標
	for (auto& meshPos : meshXYZ)
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

	// 屋根傾斜の補正
	{
		// 傾斜角を求める
		double degree = acos(CGeoUtil::InnerProduct(CGeoUtil::Normalize(n), CVector3D(0.0, 0.0, 1.0))) * _COEF_RAD_TO_DEG;
		CReflectionCorrect* pRoofCorrect = (degree < 3.0) ? &m_roofCorrectLower : &m_roofCorrectUpper;
		// meshを傾斜させる
		if (pRoofCorrect->bCustom)
		{
			for (auto& meshPos : meshXYZ)
			{
				CVector3D orgPos(meshPos - center);
				orgPos.z = 0.0;
				CVector3D rotPos;
				double theta = pRoofCorrect->dSlopeAngle * _COEF_DEG_TO_RAD;
				switch (pRoofCorrect->eAzimuth)
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
				rotPos += center;
				meshPos = rotPos;
			}
		}
		
	}

	CReflectionSimulatorMesh reflectionSimMesh(m_pParam);
	bool bResult = reflectionSimMesh.Exec(sunVector, meshXYZ, /*building,*/ buildings);
	if (bResult)
		result = reflectionSimMesh.GetResult();

	return bResult;
}

// 土地面での解析
bool CReflectionSimulator::AnalyzeLand(
	const AREADATA& targetArea,
	const std::vector<BLDGLIST*>& buildings,
	uint8_t hour,
	CAnalysisReflection& result
)
{
	bool ret = false;	// 解析結果がないときfalse

	double lat, lon;
	int JPZONE = GetINIParam()->GetJPZone();
	CGeoUtil().XYToLatLon(JPZONE, targetArea.bbMinY, targetArea.bbMinX, lat, lon);

	HorizontalCoordinate sunPos;
	SetSunVector(lat, lon);
	m_pSunVector->GetPos(hour, sunPos);

	// 高度が0以上の場合のみ反射シミュレーション解析を行う
	if (sunPos.altitude >= 0)
	{
		// 入射光のベクトル
		CVector3D sunVector;
		m_pSunVector->GetVector(hour, sunVector);

		// 対象メッシュの隣接するメッシュを取得
		vector<BLDGLIST> neighborBuildings;
		GetNeighborBuildings(targetArea, buildings, neighborBuildings);

		// エリアIDの処理を記録する
		m_buildingIndex++;		// 暫定：建物と一緒に保持する
		if (m_mapBuildingIndex.find(targetArea.areaID) == m_mapBuildingIndex.end())
			m_mapBuildingIndex[targetArea.areaID] = m_buildingIndex;

		// 有効な土地面全体の座標リストを作成
		vector<CPointBase> posList = {};
		for (const auto& polygon : targetArea.landSurface.landSurfaceList)
		{
			posList.insert(posList.end(), polygon.posList.begin(), polygon.posList.end());
		}

		// 1土地メッシュの反射解析
		for (const auto& mesh : targetArea.landSurface.meshPosList)
		{
			CAnalysisReflectionMesh resultMesh;
			bool res = AnalyzeMesh(mesh, posList, neighborBuildings, sunVector, resultMesh);
			if (res)
			{
				resultMesh.reflectionRoof.buildingId = targetArea.areaID;		// エリアID
				resultMesh.reflectionRoof.roofSurfaceId = "";					// 空欄
				result.emplace_back(resultMesh);
				ret = true;
			}

			// キャンセル
			if (IsCancel())
			{
				return false;
			}
		}

	}

	return ret;
}

// 対象メッシュの隣接するメッシュを取得
void CReflectionSimulator::GetNeighborBuildings(
	const BLDGLIST& targetBuildings,
	const std::vector<BLDGLIST*>& buildings,
	std::vector<BLDGLIST>& neighborBuildings
)
{
	const double DIST = GetINIParam()->GetNeighborBuildDist_Reflection();	// 隣接するBBoxの範囲[m]

	// targetBuildingsの中心
	double targetCenterX = ((int64_t)targetBuildings.bbMaxX + targetBuildings.bbMinX) * 0.5;
	double targetCenterY = ((int64_t)targetBuildings.bbMaxY + targetBuildings.bbMinY) * 0.5;
	// 外接円半径
	double targetR = sqrt((targetCenterX - targetBuildings.bbMaxX) * (targetCenterX - targetBuildings.bbMaxX) +
		(targetCenterY - targetBuildings.bbMaxY) * (targetCenterY - targetBuildings.bbMaxY));

	for (const auto& building : buildings)
	{
		// 範囲内にあるか
		// buldingの中心
		double buildCenterX = ((int64_t)building->bbMaxX + building->bbMinX) * 0.5;
		double buildCenterY = ((int64_t)building->bbMaxY + building->bbMinY) * 0.5;
		// 外接円半径
		double buildR = sqrt((buildCenterX - building->bbMaxX) * (buildCenterX - building->bbMaxX) +
			(buildCenterY - building->bbMaxY) * (buildCenterY - building->bbMaxY));
		// 中心同士の距離
		double dist = sqrt((buildCenterX - targetCenterX) * (buildCenterX - targetCenterX) + (buildCenterY - targetCenterY) * (buildCenterY - targetCenterY));
		// DIST以内の距離のとき近隣とする
		if ((dist - targetR - buildR) <= DIST)
		{
			neighborBuildings.emplace_back(*building);
			continue;
		}
	}
}

// 対象メッシュの隣接するメッシュを取得
void CReflectionSimulator::GetNeighborBuildings(
	const AREADATA& targetArea,
	const std::vector<BLDGLIST*>& buildings,
	std::vector<BLDGLIST>& neighborBuildings
)
{
	const double DIST = GetINIParam()->GetNeighborBuildDist_Reflection();	// 隣接するBBoxの範囲[m]

	// targetLandの中心
	double targetCenterX = ((int64_t)targetArea.bbMaxX + targetArea.bbMinX) * 0.5;
	double targetCenterY = ((int64_t)targetArea.bbMaxY + targetArea.bbMinY) * 0.5;
	// 外接円半径
	double targetR = sqrt((targetCenterX - targetArea.bbMaxX) * (targetCenterX - targetArea.bbMaxX) +
		(targetCenterY - targetArea.bbMaxY) * (targetCenterY - targetArea.bbMaxY));

	for (const auto& building : buildings)
	{
		// 範囲内にあるか
		// buldingの中心
		double buildCenterX = ((int64_t)building->bbMaxX + building->bbMinX) * 0.5;
		double buildCenterY = ((int64_t)building->bbMaxY + building->bbMinY) * 0.5;
		// 外接円半径
		double buildR = sqrt((buildCenterX - building->bbMaxX) * (buildCenterX - building->bbMaxX) +
			(buildCenterY - building->bbMaxY) * (buildCenterY - building->bbMaxY));
		// 中心同士の距離
		double dist = sqrt((buildCenterX - targetCenterX) * (buildCenterX - targetCenterX) + (buildCenterY - targetCenterY) * (buildCenterY - targetCenterY));
		// DIST以内の距離のとき近隣とする
		if ((dist - targetR - buildR) <= DIST)
		{
			neighborBuildings.emplace_back(*building);
			continue;
		}
	}
}

// キャンセルチェック
bool CReflectionSimulator::IsCancel()
{
	assert(m_pParam);

	// キャンセルファイルのディレクトリパス
	filesystem::path filepath = m_pParam->strOutputDirPath;
	filepath = filepath.parent_path().parent_path();

	// キャンセルファイルのパス
	filepath /= CANCELFILE;
	string cancelPath = filepath.string();

	if (std::filesystem::exists(cancelPath))
		return true;

	return false;
}

// 建物IDの順番を結果に設定する
// 処理中に順番を保存したm_mapBuildingIndexを結果に設定する
void CReflectionSimulator::SetBuildingIndex(CAnalysisReflection& result)
{
	for (auto& roofMesh : result)
	{
		// 入射屋根
		roofMesh.reflectionRoof.buildingIndex = m_mapBuildingIndex[roofMesh.reflectionRoof.buildingId];

		// 反射先
		roofMesh.reflectionTarget.buildingIndex = m_mapBuildingIndex[roofMesh.reflectionTarget.buildingId];
	}
}
