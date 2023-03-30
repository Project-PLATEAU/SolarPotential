#include "pch.h"
#include "ReflectionSimulatorMesh.h"
#include <CommonUtil/ReadINIParam.h>

using namespace std;

bool CReflectionSimulatorMesh::Exec(
	const CVector3D& inputVec,
	const vector<CVector3D>& roofMesh,
	const BUILDINGS& building,
	const std::vector<BLDGLIST>& buildingsList
)
{
	// 光線の有効距離
	const double LIGHT_LENGTH = GetINIParam()->GetNeighborBuildDist_Reflection();

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
	double dist = 0.0;
	if (IntersectBuildings(lightRay, buildingsList, nullptr, &dist, nullptr, nullptr))
	{
		return false;	// 屋根メッシュに光線があたっていないので解析終了
	}

	// 反射光
	CLightRay reflectedLightRay = lightRay.Reflect(roofMeshPos, n);

	// 反射光が建物にあたるときどこにあたるのか
	CVector3D targetPos;		// あたった座標
	string strTargetBuilding;	// あたった建物ID
	vector<CVector3D> targetPosList;	// あたった面座標
	if (IntersectBuildings(reflectedLightRay, buildingsList, &targetPos, &dist, &strTargetBuilding, &targetPosList))
	{
		// どこか建物にあたるとき、結果に記録する
		m_reflectionMesh.inputVec = inputVec;
		m_reflectionMesh.outputVec = reflectedLightRay.GetVector();
		m_reflectionMesh.reflectionRoof.targetPos = roofMeshPos;
		m_reflectionMesh.reflectionTarget.buildingId = strTargetBuilding;
		m_reflectionMesh.reflectionTarget.targetPos = targetPos;
		m_reflectionMesh.reflectionPosList = targetPosList;

		return true;
	}

	return false;
}

const CAnalysisReflectionMesh& CReflectionSimulatorMesh::GetResult() const
{
	return m_reflectionMesh;
}

// 建物群に光線があたっているか
// あたっているときの座標と光源からの距離を取得
bool CReflectionSimulatorMesh::IntersectBuildings(
	const CLightRay& lightRay,
	const std::vector<BLDGLIST>& buildingsList,
	CVector3D* targetPos,
	double* dist,
	string* strTargetBuilding,
	vector<CVector3D>* posList
)
{
	bool result = false;
	double minDist = DBL_MAX;
	double tempDist;
	CVector3D tempTargetPos;
	BUILDINGS tempTargetBuilding;
	vector<CVector3D> tempPosList;
	for (const auto& bldglist : buildingsList)
	{
		// LOD2
		const vector<BUILDINGS>& buildings = bldglist.buildingList;
		for (const auto& building : buildings)
		{
			// 壁面と屋根面のリスト
			vector<SURFACEMEMBERS> surfaceList;
			// 壁面
			for (const auto& wall : building.wallSurfaceList)
				surfaceList.insert(surfaceList.end(), wall.wallSurfaceList.begin(), wall.wallSurfaceList.end());
			// 屋根面
			for (const auto& roof : building.roofSurfaceList)
				surfaceList.insert(surfaceList.end(), roof.roofSurfaceList.begin(), roof.roofSurfaceList.end());

			// 光線と面の当たり判定
			if (IntersectSurface(lightRay, surfaceList, &tempTargetPos, &tempDist, &tempPosList))
			{
				// 一番近い位置のデータを採用する
				if (tempDist < minDist)
				{
					if (targetPos)		*targetPos = tempTargetPos;
					if (dist)			*dist = tempDist;
					if (strTargetBuilding)	*strTargetBuilding = building.building;
					if (posList)		*posList = tempPosList;
					minDist = tempDist;
				}
				result = true;
			}
		}

		// LOD1
		const vector<BUILDINGSLOD1>& buildingsLOD1 = bldglist.buildingListLOD1;
		for (const auto& building : buildingsLOD1)
		{
			// 壁面のリスト
			vector<SURFACEMEMBERS> surfaceList;
			for (const auto& wall : building.wallSurfaceList)
				surfaceList.insert(surfaceList.end(), wall.wallSurfaceList.begin(), wall.wallSurfaceList.end());

			// 光線と面の当たり判定
			if (IntersectSurface(lightRay, surfaceList, &tempTargetPos, &tempDist, &tempPosList))
			{
				// 一番近い位置のデータを採用する
				if (tempDist < minDist)
				{
					if (targetPos)		*targetPos = tempTargetPos;
					if (dist)			*dist = tempDist;
					if (strTargetBuilding)	*strTargetBuilding = building.building;
					if (posList)		*posList = tempPosList;
					minDist = tempDist;
				}
				result = true;
			}
		}
	}

	return result;
}

// 平面に光線があたっているかどうか
bool CReflectionSimulatorMesh::IntersectSurface(
	const CLightRay& lightRay,
	const vector<SURFACEMEMBERS>& surfaceList,
	CVector3D* targetPos,
	double* dist,
	vector<CVector3D>* surfacePosList
)
{
	// 距離が近すぎるときは誤差内の光源自身として判定除外するための距離
	constexpr double MINIMUM_DIST = 0.001;

	// 光源が建物より遠すぎないか調べる
	if (!CheckDistance(lightRay, surfaceList))
		return false;

	bool result = false;
	double minDist = DBL_MAX;
	double tempDist;
	CVector3D tempTargetPos;
	for (const auto& polygon : surfaceList)
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
			if ((tempDist > MINIMUM_DIST) &&
				(abs(lightRay.GetVector().Length() - tempDist) > MINIMUM_DIST))
			{
				if (tempDist < minDist)
				{
					if (targetPos)		*targetPos = tempTargetPos;
					if (dist)			*dist = tempDist;
					if (surfacePosList)	*surfacePosList = posList;
					minDist = tempDist;
				}
				result = true;
			}
		}
	}

	return result;
}

// 平面が光源から光線方向の範囲内か大まかにチェック
bool CReflectionSimulatorMesh::CheckDistance(
	const CLightRay& lightRay,
	const vector<SURFACEMEMBERS>& surfaceList
)
{
	// 光線が範囲内か判定する距離範囲
	const double LIGHT_LENGTH = GetINIParam()->GetNeighborBuildDist_Reflection() + 50;//余裕を持たせる

	for (const auto& polygon : surfaceList)
	{
		for (const auto& pos : polygon.posList)
		{
			if (CheckDistance(lightRay, pos, LIGHT_LENGTH))
				return true;	// 範囲内があればOK
		}
	}

	return false;
}

// 指定点が光源から光線方向の範囲内かチェック
bool CReflectionSimulatorMesh::CheckDistance(
	const CLightRay& lightRay,
	const CPointBase& pos,
	const double& dist
)
{
	// 光線が範囲内か判定する距離範囲
	const double SQUARE_LINGHT_LENGTH = dist * dist;

	bool bDist = false;
	bool bDirect = false;

	double dx = pos.x - lightRay.GetPos().x;
	double dy = pos.y - lightRay.GetPos().y;
	double dz = pos.z - lightRay.GetPos().z;

	// 平面の頂点が逆方向にあるときは範囲外とする
	double dot = CGeoUtil::InnerProduct(lightRay.GetVector(), CVector3D(dx, dy, dz));
	if (dot > 0.0)
		bDirect = true;

	// 距離が遠すぎないかチェック
	double len = dx * dx + dy * dy + dz * dz;
	if (len <= SQUARE_LINGHT_LENGTH)
		bDist = true;

	if (bDirect && bDist)
		return true;

	return false;
}