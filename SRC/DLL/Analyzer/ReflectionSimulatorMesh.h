#pragma once

#include <vector>

#include <CommonUtil/CGeoUtil.h>
#include <CommonUtil/CLightRay.h>
#include "AnalysisReflectionMesh.h"
#include "AnalyzeData.h"

class CReflectionSimulatorMesh
{
public:
	// 反射光が建物にあたるか解析する。
	// あたっているときはtrueを返す。
	// あたっている解析結果はGetResult()で取得できる型で確認できる。
	// 入射光の反射した建物IDは持っていないので
	// GetResult()で取得できる値には含まれないので注意。
	bool Exec(
		const CVector3D& inputVec,					// 入射光
		const vector<CVector3D>& roofMesh, 			// 入射屋根メッシュ
		const BUILDINGS& building,					// 屋根メッシュがある建物
		const std::vector<BLDGLIST>& buildingsList	// 反射光があたるか調べる建物
	);

	// 結果取得
	const CAnalysisReflectionMesh& GetResult() const;


private:
	CAnalysisReflectionMesh m_reflectionMesh;	// 解析結果


	// 光線が建物群にあたっているかどうか
	bool IntersectBuildings(
		const CLightRay& lightRay,				// 光線
		const std::vector<BLDGLIST>& buildingsList,// 光線があたっているかチェックする建物群
		CVector3D* targetPos,					// [out]光線があたっている座標
		double* dist,							// [out]光源からあたっている位置までの距離
		std::string* strTargetBuilding,			// [out]光線があたっている建物ID
		std::vector<CVector3D>* wallPosList		// [out]壁面
	);

	// 平面に光線があたっているかどうか
	bool IntersectSurface(
		const CLightRay& lightRay,				// 光線
		const std::vector<SURFACEMEMBERS>& surfaceList,	// 光線があたっているかチェックする平面リスト
		CVector3D* targetPos,					// [out]光線があたっている座標
		double* dist,							// [out]光源からあたっている位置までの距離
		vector<CVector3D>* posList				// [out]当たっている面
	);

	// 面が光源から光線方向の大まかな範囲内にあるかチェック
	bool CheckDistance(const CLightRay& lightRay, const vector<SURFACEMEMBERS>& roofSurfaceList);

	// 指定点が光源から光線方向の範囲内にあるかチェック
	bool CheckDistance(const CLightRay& lightRay, const CPointBase& pos, const double& dist);
};
