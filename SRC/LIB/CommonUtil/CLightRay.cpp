#include "pch.h"
#include "CLightRay.h"
#include <algorithm>

using namespace std;

CLightRay::CLightRay(const CVector3D& pos, const CVector3D& vec)
	: m_pos(pos), m_vec(vec)
{
}

// 反射光を取得
// 反射光の長さは入射光と同じにする
CLightRay CLightRay::Reflect(const CVector3D& reflectPos, const CVector3D& normal) const
{
	if ((m_vec.Length() < DBL_EPSILON) ||
		(normal.Length() < DBL_EPSILON))
		return CLightRay(reflectPos, CVector3D());

	CVector3D inVec = CGeoUtil::Normalize(m_vec);
	CVector3D nVec = CGeoUtil::Normalize(normal);

	// 裏側からのときは法線を逆にして裏面を表にする
	double dot = CGeoUtil::InnerProduct(inVec, nVec);
	if (dot > 0.0f)
		nVec *= -1;

	// 反射ベクトル = inVec * 2 * a * normal (a : 入射ベクトル逆向きと法線の内積)
	CVector3D reflectVec = inVec + nVec * 2 * CGeoUtil::InnerProduct(inVec * (-1), nVec);
	reflectVec.Normalize();
	reflectVec *= m_vec.Length();

	return CLightRay(reflectPos, reflectVec);
}

// 光線が平面と交差しているか
bool CLightRay::Intersect(
	const std::vector<CVector3D>& polygon,
	CVector3D* point,
	double* dist
) const
{
	if (polygon.size() < 3)
		return false;

	// 平面の法線
	CVector3D n;
	{
		CVector3D vec1;
		CVector3D vec2;
		// [0]からの各点のベクトル
		vector<CVector3D> vecPolyList;
		for (int i = 1; i < polygon.size(); ++i)
		{
			CVector3D vec(polygon[i], polygon[0]);
			vecPolyList.push_back(vec);
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
	}

	// 平面に対する光線の方向をチェック
	CVector3D inVec = CGeoUtil::Normalize(m_vec);
	double dot = CGeoUtil::InnerProduct(n, inVec);
	// 平面と平行のときは交差しない
	if (abs(dot) < DBL_EPSILON)
		return false;
	// 裏側からのときは法線を逆にして裏面を表にする
	if (dot > 0.0)
	{
		n   *= -1;
		dot *= -1;
	}


	// 平面の式
	// ax + by + cz = d から
	// p ・ n = d (p:平面上の全ての点, n:法線[a,b,c]) と表せる
	CVector3D p(polygon[0].x, polygon[0].y, polygon[0].z);
	double d = CGeoUtil::InnerProduct(p, n);

	// 平面と光線の交点の式
	// p(t) = p0 + t * v (p0:光線の原点、t:原点から交点までの距離、v:光線の単位ベクトル)
	// 交点の式と平面の式からtの式にする
	CVector3D p0(m_pos.x, m_pos.y, m_pos.z);
	double t = (d - CGeoUtil::InnerProduct(p0, n)) / dot;
	
	if ((t > m_vec.Length()) ||	// 離れすぎているときは交差していないとする
		(t <= 0.0))				// 平面上または逆方向の時は交差していないとする
		return false;
	double tempDist = t;

	// 交点
	CVector3D tempPoint = p0 + t * inVec;

	// 交点がポリゴン平面内にあるか
	bool result = IsPointInPolygon(tempPoint, polygon);
	if (result)
	{
		*point = tempPoint;
		*dist = tempDist;
	}

	return result;
}

// 内外判定
bool CLightRay::IsPointInPolygon(const CVector3D& point, const vector<CVector3D>& polygon) const
{
	// まずは大まかにバウンディングボックス内にあるかでふるいにかける
	if (!IsPointInBB(point, polygon))
		return false;

	// 平面の法線
	CVector3D n;
	CGeoUtil::OuterProduct(CVector3D(polygon[1], polygon[0]), CVector3D(polygon[2], polygon[1]), n);

	// 平面に投影する
	// 投影する方向は法線の方向が長い方向
	vector<CVector2D> plane2d;
	CVector2D targetPoint;
	// x方向に伸びている時yz平面に投影
	if (abs(n.x) > abs(n.y) && abs(n.x) > abs(n.z))
	{
		for (const auto& vertex : polygon)
		{
			plane2d.push_back(CVector2D(vertex.y, vertex.z));
		}
		targetPoint.x = point.y;
		targetPoint.y = point.z;
	}
	// y方向に伸びている時xz平面に投影
	else if (abs(n.y) > abs(n.x) && abs(n.y) > abs(n.z))
	{
		for (const auto& vertex : polygon)
		{
			plane2d.push_back(CVector2D(vertex.x, vertex.z));
		}
		targetPoint.x = point.x;
		targetPoint.y = point.z;
	}
	// z方向に伸びている時xy平面に投影
	else
	{
		for (const auto& vertex : polygon)
		{
			plane2d.push_back(CVector2D(vertex.x, vertex.y));
		}
		targetPoint.x = point.x;
		targetPoint.y = point.y;
	}

	// 平面上の内外判定
	return CGeoUtil::IsPointInPolygon(targetPoint, static_cast<unsigned int>(polygon.size()), plane2d.data());
}


// ポリゴンのバウンディングボックス内に点が存在するか判定
// ※polygonは平面データで使用する想定のため、BBは少し余裕を持たせて判定する
bool CLightRay::IsPointInBB(const CVector3D& pointTarget, const std::vector<CVector3D>& polygon) const
{
	constexpr double margine = 0.1;	// BBの余裕

	CVector3D bbMin(DBL_MAX, DBL_MAX, DBL_MAX);
	CVector3D bbMax(-DBL_MAX, -DBL_MAX, -DBL_MAX);

	for (const auto& vertex : polygon)
	{
		if (vertex.x < bbMin.x) bbMin.x = vertex.x;
		if (vertex.y < bbMin.y) bbMin.y = vertex.y;
		if (vertex.z < bbMin.z) bbMin.z = vertex.z;
		if (vertex.x > bbMax.x) bbMax.x = vertex.x;
		if (vertex.y > bbMax.y) bbMax.y = vertex.y;
		if (vertex.z > bbMax.z) bbMax.z = vertex.z;
	}

	bbMin -= CVector3D(margine, margine, margine);
	bbMax += CVector3D(margine, margine, margine);

	// 範囲内
	if ((pointTarget.x >= bbMin.x && pointTarget.x <= bbMax.x) &&
		(pointTarget.y >= bbMin.y && pointTarget.y <= bbMax.y) &&
		(pointTarget.z >= bbMin.z && pointTarget.z <= bbMax.z))
		return true;

	return false;
}
