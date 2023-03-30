#pragma once

#include <vector>
#include "CGeoUtil.h"

// 光線に関するクラス
// 反射光の計算や物体の当たり判定など行う
class CLightRay
{
public:
	CLightRay() = delete;
	CLightRay(const CVector3D& pos, const CVector3D& vec);

	// 反射後の光線を算出
	CLightRay Reflect(const CVector3D& reflectPos, const CVector3D& normal) const;

	// 光線と平面の交差点を探す
	bool Intersect(
		const std::vector<CVector3D>& polygon,	// 平面
		CVector3D* point,						// [out]交点
		double* dist							// [out]交点までの距離
	) const; 

	// 光線のベクトル取得
	inline const CVector3D& GetVector() const { return m_vec; }
	// 光源取得
	inline const CVector3D& GetPos() const { return m_pos; }

private:
	CVector3D m_vec;	// 光線の長さと向き
	CVector3D m_pos;	// 光線の原点

private:
	// 平面上の点の内外判定
	bool IsPointInPolygon(const CVector3D& pointTarget, const std::vector<CVector3D>& polygon) const;
	// バウンディングボックス内の判定
	bool IsPointInBB(const CVector3D& pointTarget, const std::vector<CVector3D>& polygon) const;
};
