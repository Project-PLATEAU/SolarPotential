#pragma once
#include <string>
#include <CommonUtil/CGeoUtil.h>

// 反射シミュレーション
// 1メッシュごとの反射シミュレーションデータ
class CAnalysisReflectionMesh
{
public:
	CVector3D inputVec;		// 入射光ベクトル

	// 屋根面メッシュとの法線ベクトルと入射ベクトルのなす角度より
	// 屋根面の座標系に置き換えて求める。
	CVector3D outputVec;	// 反射ベクトル


	// 衝突屋根
	struct TargetRoof
	{
		std::string buildingId{ "" };		// 建物ID
		int buildingIndex{ 0 };				// 建物IDの順番
		std::string roofSurfaceId{ "" };	// 屋根ID
		CPointBase targetPos;				// 衝突点

	};

	// 反射先
	TargetRoof reflectionTarget;
	// 反射先面座標
	std::vector<CVector3D> reflectionPosList;

	// 入射屋根メッシュ
	TargetRoof reflectionRoof;

};

//
//// 反射シミュレーション
//// エリアごとの反射シミュレーションデータ
//class CAnalysisReflectionArea
//{
//public:
//	std::string areaID;
//	CAnalysisReflection refList;
//
//};
