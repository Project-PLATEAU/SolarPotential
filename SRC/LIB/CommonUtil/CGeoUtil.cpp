#include "pch.h"
#include "CGeoUtil.h"
#include "CPoint2DPolygon.h"
#include <cmath>
#include <random>

#pragma region 幾何計算

//ベクトル内積
inline double CGeoUtil::InnerProduct(const CVector3D& vl, const CVector3D& vr)
{
	return vl.x * vr.x + vl.y * vr.y + vl.z * vr.z;
}

//ベクトル外積
inline void CGeoUtil::OuterProduct(const CVector3D& v1, const CVector3D& v2, CVector3D& vOut)
{
	vOut.x = v1.y * v2.z - v1.z * v2.y;
	vOut.y = v1.z * v2.x - v1.x * v2.z;
	vOut.z = v1.x * v2.y - v1.y * v2.x;
}

// ベクトル2D内積
inline double CGeoUtil::InnerProduct(const CVector2D& vl, const CVector2D& vr)
{
	return vl.x * vr.x + vl.y * vr.y;
}

// 平面の式取得
void CGeoUtil::GetPlaneParameter
(
	const CPointBase& pos1,	//!< in 平面上の任意の点1
	const CPointBase& pos2,	//!< in 平面上の任意の点2
	const CPointBase& pos3,	//!< in 平面上の任意の点3
	CPlane& plane			//!< out	平面
)
{
	CVector3D v1(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z);
	CVector3D v2(pos3.x - pos1.x, pos3.y - pos1.y, pos3.z - pos1.z);

	CVector3D vNormal;
	OuterProduct(v1, v2, vNormal);
	if (vNormal.z < 0)
	{
		// 上向きに反転
		vNormal.Inverse();
	}
	vNormal.Normalize();

	plane.a = vNormal.x;
	plane.b = vNormal.y;
	plane.c = vNormal.z;
	plane.d = -vNormal.x * pos1.x - vNormal.y * pos1.y - vNormal.z * pos1.z;
}

// 傾斜角
// 上向きベクトルと面法線の成す角度
void CGeoUtil::CalcSlope(CVector3D& vP, double& slopeDegree)
{
	// 上向き
	CVector3D vUp(0, 0, 1);

	double dL1 = vP.Length();
	if (abs(dL1) < _MAX_TOL) return;

	// cosθ
	double costh = InnerProduct(vP, vUp) / dL1;
	if (costh > 1.0) costh = 1.0;
	if (-1.0 > costh)costh = -1.0;
	double dAngle = acos(costh);
	slopeDegree = dAngle * _COEF_RAD_TO_DEG;
}

// 方位角
// 2次元の法線ベクトルと真北のなす角度
void CGeoUtil::CalcAzimuth(CVector3D& vP, double& azDegree, int iJPZone)
{
	CVector3D vP2d(vP.x, vP.y, 0);

	double dL1 = vP2d.Length();
	if (abs(dL1) < _MAX_TOL) return;

	CVector3D vNup(0, 1, 0);
	// cosθ
	double costh = InnerProduct(vP2d, vNup) / dL1;
	if (costh > 1.0) costh = 1.0;
	if (-1.0 > costh)costh = -1.0;
	double dAngle = acos(costh);	// 方向角

	// 西側の場合
	if (vP.x < 0.0)	dAngle = _PI * 2.0 - dAngle;

	// 平面直角の北と真北の補正
	// 配置座標を緯度経度で真北方向に移動後の座標を算出
	double dN = calcNorth(iJPZone, vP2d.y, vP2d.x);

	azDegree = (dAngle + dN) * _COEF_RAD_TO_DEG;
}

//点と平面の距離を求める ( P=平面上の点 N=平面の法線 )
double CGeoUtil::CalcDistancePointToFace(const CVector3D& A, const CVector3D& P, const CVector3D& N)
{
	//PAベクトル(A-P)
	CVector3D PA(0, 0, 0);
	PA.x = A.x - P.x;
	PA.y = A.y - P.y;
	PA.z = A.z - P.z;

	//法線NとPAを内積,その絶対値が点と平面の距離
	return abs(InnerProduct(N, PA));
}

// 点と平面との距離の算出
// 平面はax + by + cz + d = 0で与えるものとする
// 点と平面との距離
double CGeoUtil::CalcDistancePointToFace
(
	const double& x, //!< in 点座標X
	const double& y, //!< in 点座標Y
	const double& z, //!< in 点座標Z
	const double& a, //!< in 平面のパラメータa
	const double& b, //!< in 平面のパラメータb
	const double& c, //!< in 平面のパラメータc
	const double& d	 //!< in 平面のパラメータd
)
{
	// 点と平面の距離はabs(a * x + b *y + c * z + d) / sqrt(a * a + b * b + c * c)
	return abs(a * x + b * y + c * z + d) / sqrt(a * a + b * b + c * c);
}

// 内外判定
bool CGeoUtil::IsPointInPolygon(const CPoint2D& pointTarget, unsigned int uiCountPoint, const CPoint2D* pPoint)
{
	int iCountCrossing = 0;

	CPoint2D point0 = pPoint[0];
	bool bFlag0x = (pointTarget.x <= point0.x);
	bool bFlag0y = (pointTarget.y <= point0.y);

	// レイの方向は、Ｘプラス方向
	for (unsigned int ui = 1; ui < uiCountPoint + 1; ui++)
	{
		CPoint2D point1 = pPoint[ui % uiCountPoint];	// 最後は始点が入る（多角形データの始点と終点が一致していないデータ対応）
		bool bFlag1x = (pointTarget.x <= point1.x);
		bool bFlag1y = (pointTarget.y <= point1.y);
		if (bFlag0y != bFlag1y)
		{	// 線分はレイを横切る可能性あり。
			if (bFlag0x == bFlag1x)
			{	// 線分の２端点は対象点に対して両方右か両方左にある
				if (bFlag0x)
				{	// 完全に右。⇒線分はレイを横切る
					iCountCrossing += (bFlag0y ? -1 : 1);	// 上から下にレイを横切るときには、交差回数を１引く、下から上は１足す。
				}
			}
			else
			{	// レイと交差するかどうか、対象点と同じ高さで、対象点の右で交差するか、左で交差するかを求める。
				if (pointTarget.x <= (point0.x + (point1.x - point0.x) * (pointTarget.y - point0.y) / (point1.y - point0.y)))
				{	// 線分は、対象点と同じ高さで、対象点の右で交差する。⇒線分はレイを横切る
					iCountCrossing += (bFlag0y ? -1 : 1);	// 上から下にレイを横切るときには、交差回数を１引く、下から上は１足す。
				}
			}
		}
		// 次の判定のために、
		point0 = point1;
		bFlag0x = bFlag1x;
		bFlag0y = bFlag1y;
	}

	// クロスカウントがゼロのとき外、ゼロ以外のとき内。
	return (0 != iCountCrossing);
}

// 単位ベクトル
CVector3D CGeoUtil::Normalize(const CVector3D& vec)
{
	CVector3D result = vec;
	result.Normalize();
	return result;
}

#pragma endregion


#pragma region 座標変換


/*! 最適な系番号を求める
@note 本機能では距離で判断しているため実際とは異なる系番号となりうる。
*/
int CGeoUtil::GetNearJPZone(
	const double& dLon,		//!<in 経度[度]
	const double& dLat		//!<in 緯度[度]
)
{
	int jpzone = 0;
	double dMinDist = DBL_MAX;
	double dX, dY;
	for (int ic = 1; ic < 20; ic++)
	{
		LonLatToXY(dLon, dLat, ic, dX, dY);
		double dTmpDist = dX * dX + dY * dY;
		if (dTmpDist < dMinDist)
		{
			jpzone = ic;
			dMinDist = dTmpDist;
		}
	}
	return jpzone;
}

/*! 緯度経度から任意原点の平面直角座標を求める
*/
void CGeoUtil::LonLatToXY(
	const double& dLon,		//!<in 経度[度]
	const double& dLat,		//!<in 緯度[度]
	int JPZone,				//!<in 系番号
	double& dEast,				//!<out X座標(東西方向、m)
	double& dNorth				//!<out Y座標(南北方向、m)
)
{
	double dLat0 = JPN_ORG_LAT_SEC[JPZone] / 3600.0 * _COEF_DEG_TO_RAD;
	double dLon0 = JPN_ORG_LON_SEC[JPZone] / 3600.0 * _COEF_DEG_TO_RAD;

	double dn = 1.0 / (2 * dF - 1);

	//ラジアン単位に
	double dLonRad = dLon * _COEF_DEG_TO_RAD;
	double dLatRad = dLat * _COEF_DEG_TO_RAD;

	double dt = sinh(atanh(sin(dLatRad)) - (2 * sqrt(dn)) / (1 + dn) * atanh(2 * sqrt(dn) / (1 + dn) * sin(dLatRad)));
	double dtb = sqrt(1 + pow(dt, 2));
	double dLmc = cos(dLonRad - dLon0);
	double dLms = sin(dLonRad - dLon0);
	double dXi = atan(dt / dLmc);
	double dEt = atanh(dLms / dtb);

	//α1→0～α5→4
	double dal[6];
	dal[0] = 0;
	dal[1] = 1.0 / 2.0 * dn - 2.0 / 3.0 * pow(dn, 2) + 5.0 / 16.0 * pow(dn, 3) + 41.0 / 180.0 * pow(dn, 4) - 127.0 / 288.0 * pow(dn, 5);
	dal[2] = 13.0 / 48.0 * pow(dn, 2) - 3.0 / 5.0 * pow(dn, 3) + 557.0 / 1440.0 * pow(dn, 4) + 281.0 / 630.0 * pow(dn, 5);
	dal[3] = 61.0 / 240.0 * pow(dn, 3) - 103.0 / 140.0 * pow(dn, 4) + 15061.0 / 26880.0 * pow(dn, 5);
	dal[4] = 49561.0 / 161280.0 * pow(dn, 4) - 179.0 / 168.0 * pow(dn, 5);
	dal[5] = 34729.0 / 80640.0 * pow(dn, 5);
	double dSg = 0; double dTu = 0;
	for (int j = 1; j <= 5; j++)
	{
		dSg = dSg + 2 * j * dal[j] * cos(2 * j * dXi) * cosh(2 * j * dEt);
		dTu = dTu + 2 * j * dal[j] * sin(2 * j * dXi) * sinh(2 * j * dEt);
	}
	dSg = 1 + dSg;

	//A0-A5
	double dA[6];
	dA[0] = 1 + pow(dn, 2) / 4 + pow(dn, 4) / 64;
	dA[1] = -3.0 / 2.0 * (dn - pow(dn, 3) / 8 - pow(dn, 5) / 64);
	dA[2] = 15.0 / 16.0 * (pow(dn, 2) - pow(dn, 4) / 4);
	dA[3] = -35.0 / 48.0 * (pow(dn, 3) - 5.0 / 16.0 * pow(dn, 5));
	dA[4] = 315.0 / 512.0 * pow(dn, 4);
	dA[5] = -693.0 / 1280.0 * pow(dn, 5);
	double dAb = dM0 * daa / (1 + dn) * dA[0];
	double dSb = 0;
	for (int j = 1; j <= 5; j++)
	{
		dSb = dSb + dA[j] * sin(2 * j * dLat0);
	}
	dSb = dM0 * daa / (1 + dn) * (dA[0] * dLat0 + dSb);

	double dNorthTmp = 0;
	double dEastTmp = 0;
	for (int j = 1; j <= 5; j++)
	{
		dNorthTmp += dal[j] * sin(2 * j * dXi) * cosh(2 * j * dEt);
		dEastTmp += dal[j] * cos(2 * j * dXi) * sinh(2 * j * dEt);
	}
	dNorth = dAb * (dXi + dNorthTmp) - dSb;
	dEast = dAb * (dEt + dEastTmp);
}

/*! 平面直角座標から緯度経度を求める
*/
void CGeoUtil::XYToLatLon(
	int JPZone,				//!< in 緯度原点
	const double& dNorth, 	//!< in X（北, m）
	const double& dEast, 	//!< in Y（東, m）
	double& dLat, 			//!< out 緯度（度）
	double& dLon			//!< out 経度（度）
)
{
	// 平面直角座標系原点の緯度及び経度 
	double dLat0 = JPN_ORG_LAT_SEC[JPZone] / 3600.0 * _COEF_DEG_TO_RAD;
	double dLon0 = JPN_ORG_LON_SEC[JPZone] / 3600.0 * _COEF_DEG_TO_RAD;
	double M0 = 0.9999; // 中心の縮尺係数

	double dn = 1.0 / (2 * dF - 1);

	//Sφ0、A
	double dA[6];
	dA[0] = 1.0 + pow(dn, 2) / 4.0 + pow(dn, 4) / 64.0;
	dA[1] = -3.0 / 2.0 * (dn - pow(dn, 3) / 8.0 - pow(dn, 5) / 64.0);
	dA[2] = 15.0 / 16.0 * (pow(dn, 2) - pow(dn, 4) / 4);
	dA[3] = -35.0 / 48.0 * (pow(dn, 3) - 5.0 / 16.0 * pow(dn, 5));
	dA[4] = 315.0 / 512.0 * pow(dn, 4);
	dA[5] = -693.0 / 1280.0 * pow(dn, 5);

	//β
	double dBt[6];
	dBt[1] = 1 / 2.0 * dn - 2 / 3.0 * pow(dn, 2) + 37.0 / 96.0 * pow(dn, 3) - 1 / 360.0 * pow(dn, 4) - 81 / 512.0 * pow(dn, 5);
	dBt[2] = 1 / 48.0 * pow(dn, 2) + 1 / 15.0 * pow(dn, 3) - 437 / 1440.0 * pow(dn, 4) + 46.0 / 105.0 * pow(dn, 5);
	dBt[3] = 17 / 480.0 * pow(dn, 3) - 37 / 840.0 * pow(dn, 4) - 209 / 4480.0 * pow(dn, 5);
	dBt[4] = 4397 / 161280.0 * pow(dn, 4) - 11 / 504.0 * pow(dn, 5);
	dBt[5] = 4583 / 161280.0 * pow(dn, 5);

	//δ
	double dDt[7];
	dDt[1] = 2 * dn - 2.0 / 3.0 * pow(dn, 2) - 2 * pow(dn, 3) + 116.0 / 45.0 * pow(dn, 4)
		+ 26.0 / 45.0 * pow(dn, 5) - 2854.0 / 675.0 * pow(dn, 6);
	dDt[2] = 7.0 / 3.0 * pow(dn, 2) - 8.0 / 5.0 * pow(dn, 3) - 227.0 / 45.0 * pow(dn, 4)
		+ 2704.0 / 315.0 * pow(dn, 5) + 2323.0 / 945.0 * pow(dn, 6);
	dDt[3] = 56.0 / 15.0 * pow(dn, 3) - 136.0 / 35.0 * pow(dn, 4)
		- 1262.0 / 105.0 * pow(dn, 5) + 73814.0 / 2835.0 * pow(dn, 6);
	dDt[4] = 4279.0 / 630.0 * pow(dn, 4) - 332.0 / 35.0 * pow(dn, 5) - 399572.0 / 14175.0 * pow(dn, 6);
	dDt[5] = 4174.0 / 315.0 * pow(dn, 5) - 144838.0 / 6237.0 * pow(dn, 6);
	dDt[6] = 601676.0 / 22275.0 * pow(dn, 6);

	double dAb = dM0 * daa / (1 + dn) * dA[0];
	double dSb0 = 0;
	for (int j = 1; j <= 5; j++)
	{
		dSb0 += dA[j] * sin(2 * j * dLat0);
	}
	double dSb = dM0 * daa / (1 + dn) * (dA[0] * dLat0 + dSb0);

	//ξ
	double dXi = (dNorth + dSb) / dAb;
	// η
	double dEt = dEast / dAb;

	//ξ’・η'・σ'・τ'・χ
	double dXi2tmp = 0;
	double dEt2tmp = 0;
	double dSg2tmp = 0;
	double dTu2tmp = 0;
	for (int j = 1; j <= 5; j++)
	{
		dXi2tmp += dBt[j] * sin(2 * j * dXi) * cosh(2 * j * dEt);
		dEt2tmp += dBt[j] * cos(2 * j * dXi) * sinh(2 * j * dEt);
		dSg2tmp += dBt[j] * cos(2 * j * dXi) * cosh(2 * j * dEt);
		dTu2tmp += dBt[j] * sin(2 * j * dXi) * sinh(2 * j * dEt);
	}
	double dXi2 = dXi - dXi2tmp;
	double dEt2 = dEt - dEt2tmp;
	double dSg2 = 1 - dSg2tmp;
	double dCi = asin(sin(dXi2) / cosh(dEt2));
	double dLatRad = dCi;
	for (int j = 1; j <= 6; j++)
	{
		dLatRad += dDt[j] * sin(2 * j * dCi);
	}
	//ラジアン単位の緯度経度
	double dLonRad = dLon0 + atan(sinh(dEt2) / cos(dXi2));

	//度単位に
	dLon = dLonRad * _COEF_RAD_TO_DEG;
	dLat = dLatRad * _COEF_RAD_TO_DEG;

	return;
}

// 子午線収差角(=-真北方向角)を求める
double CGeoUtil::calcNorth(int JPZone, const double& dNorth, const double& dEast)
{
	//
	double dGm = 0.0;

	// 平面直角座標系原点の緯度
	double dLat0 = JPN_ORG_LAT_SEC[JPZone] / 3600.0 * _COEF_DEG_TO_RAD;

	double dn = 1.0 / (2 * dF - 1);

	//Sφ0、A
	double dA[6];
	dA[0] = 1.0 + pow(dn, 2) / 4.0 + pow(dn, 4) / 64.0;
	dA[1] = -3.0 / 2.0 * (dn - pow(dn, 3) / 8.0 - pow(dn, 5) / 64.0);
	dA[2] = 15.0 / 16.0 * (pow(dn, 2) - pow(dn, 4) / 4);
	dA[3] = -35.0 / 48.0 * (pow(dn, 3) - 5.0 / 16.0 * pow(dn, 5));
	dA[4] = 315.0 / 512.0 * pow(dn, 4);
	dA[5] = -693.0 / 1280.0 * pow(dn, 5);

	//β
	double dBt[6];
	dBt[1] = 1 / 2.0 * dn - 2 / 3.0 * pow(dn, 2) + 37.0 / 96.0 * pow(dn, 3) - 1 / 360.0 * pow(dn, 4) - 81 / 512.0 * pow(dn, 5);
	dBt[2] = 1 / 48.0 * pow(dn, 2) + 1 / 15.0 * pow(dn, 3) - 437 / 1440.0 * pow(dn, 4) + 46.0 / 105.0 * pow(dn, 5);
	dBt[3] = 17 / 480.0 * pow(dn, 3) - 37 / 840.0 * pow(dn, 4) - 209 / 4480.0 * pow(dn, 5);
	dBt[4] = 4397 / 161280.0 * pow(dn, 4) - 11 / 504.0 * pow(dn, 5);
	dBt[5] = 4583 / 161280.0 * pow(dn, 5);

	double dAb = dM0 * daa / (1 + dn) * dA[0];
	double dSb0 = 0;
	for (int j = 1; j <= 5; j++)
	{
		dSb0 += dA[j] * sin(2 * j * dLat0);
	}
	double dSb = dM0 * daa / (1 + dn) * (dA[0] * dLat0 + dSb0);

	//ξ
	double dXi = (dNorth + dSb) / dAb;
	// η
	double dEt = dEast / dAb;

	//ξ’・η'・σ'・τ'・χ
	double dXi2tmp = 0;
	double dEt2tmp = 0;
	double dSg2tmp = 0;
	double dTu2tmp = 0;
	for (int j = 1; j <= 5; j++)
	{
		dXi2tmp += dBt[j] * sin(2 * j * dXi) * cosh(2 * j * dEt);
		dEt2tmp += dBt[j] * cos(2 * j * dXi) * sinh(2 * j * dEt);
		dSg2tmp += dBt[j] * cos(2 * j * dXi) * cosh(2 * j * dEt);
		dTu2tmp += dBt[j] * sin(2 * j * dXi) * sinh(2 * j * dEt);
	}
	double dXi2 = dXi - dXi2tmp;
	double dEt2 = dEt - dEt2tmp;
	double dSg2 = 1 - dSg2tmp;

	dGm = atan((dTu2tmp + dSg2 * tan(dXi2) * tanh(dEt2)) / (dSg2 - dTu2tmp * tan(dXi2) * tanh(dEt2)));

	return dGm;

}

// ３次メッシュコードから緯度経度を求める
bool CGeoUtil::MeshIDToLatLon(const std::string& meshid, double& lat, double& lon)
{
	if (meshid.length() != 8)
		return false;

	// 1次メッシュ用計算
	double first_two = std::stod(meshid.substr(0, 2));
	double last_two = std::stod(meshid.substr(2, 2));
	lat = first_two * 2 / 3;
	lon = last_two + 100;

	// 2次メッシュ用計算
	double fifth = std::stod(meshid.substr(4, 1));
	double sixth = std::stod(meshid.substr(5, 1));
	lat += fifth * 2 / 3 / 8;
	lon += sixth / 8;

	// 3次メッシュ用計算
	double seventh = std::stod(meshid.substr(6, 1));
	double eighth = std::stod(meshid.substr(7, 1));
	lat += seventh * 2 / 3 / 8 / 10;
	lon += eighth / 8 / 10;

	return true;
}

// 2次元空間での内外判定とZ値取得
bool CTriangle::CalcZBySpecEps(const CVector3D& inputPoint, double& dz, double dEps)
{
	std::vector<CPoint2D> point2DList{};
	point2DList.push_back(CPoint2D(posTriangle[0].x, posTriangle[0].y));
	point2DList.push_back(CPoint2D(posTriangle[1].x, posTriangle[1].y));
	point2DList.push_back(CPoint2D(posTriangle[2].x, posTriangle[2].y));
	CPoint2D * pt2D = &point2DList[0];

	// 内外判定
	if (!CGeoUtil::IsPointInPolygon(CPoint2D(inputPoint.x, inputPoint.y), posTriangle.size(), pt2D))
	{
		return false;
	}

	CPoint2DPolygon poly2d;
	poly2d.push_back(CPoint2D(posTriangle[0].x, posTriangle[0].y));
	poly2d.push_back(CPoint2D(posTriangle[1].x, posTriangle[1].y));
	poly2d.push_back(CPoint2D(posTriangle[2].x, posTriangle[2].y));
	CPointBase vec[3];
	if (poly2d.Clockwise())
	{
		vec[0] = posTriangle[2];
		vec[1] = posTriangle[1];
		vec[2] = posTriangle[0];
		poly2d.Reverse();
	}
	else
	{
		vec[0] = posTriangle[0];
		vec[1] = posTriangle[1];
		vec[2] = posTriangle[2];
	}
	CVector3D vec01(vec[1], vec[0]);
	CVector3D vec02(vec[2], vec[0]);

	// 平面法線
	// ベクトルを外積
	CVector3D normal;
	CGeoUtil::OuterProduct(vec01, vec02, normal);

	// 面積
	double dArea = normal.Length();
	if (fabs(dArea) < dEps * 2.0)
	{
		return false;
	}

	double crsY[2], crsZ[2];
	int crsCnt = 0;
	for (int ic = 0; ic < 3; ic++)
	{
		// 頂点間の2次元ベクトル
		CVector2D vecXY(poly2d[(ic + 1) % 3], poly2d[ic]);
		double dLenTotal = vecXY.Length();

		if (fabs(vecXY.x) < dEps)
		{	// 線分がY軸と平行な場合
			CPoint2D pt(inputPoint.x, inputPoint.y);
			double dLen0pt = CVector2D(poly2d[ic], pt).Length();
			double dLen1pt = CVector2D(poly2d[(ic + 1) % 3], pt).Length();
			if (fabs((dLen0pt + dLen1pt) - dLenTotal) < dEps)
			{	// 線分上に交点がある場合、比でZを求める
				dz = vec[ic].z + (vec[(ic + 1) % 3].z - vec[ic].z) * (dLen0pt / dLenTotal);
				return true;
			}
		}
		else
		{
			// 線分がY軸と平行でない場合
			// X固定で三角形の線分との交点取得
			double dYonSeg = (vecXY.y / vecXY.x * (inputPoint.x - poly2d[ic].x) + poly2d[ic].y);
			CPoint2D pt(inputPoint.x, dYonSeg);
			// 頂点とptの2次元距離
			double dLen0pt = CVector2D(poly2d[ic], pt).Length();
			double dLen1pt = CVector2D(poly2d[(ic + 1) % 3], pt).Length();
			if (fabs((dLen0pt + dLen1pt) - dLenTotal) < dEps)
			{
				if (dLen0pt >= dEps && dLen1pt >= dEps)
				{
					// 線分上に交点がある場合、比でZを求める
					crsY[crsCnt] = dYonSeg;
					crsZ[crsCnt] = vec[ic].z + (vec[(ic + 1) % 3].z - vec[ic].z) * (dLen0pt / dLenTotal);
					crsCnt++;
					if (2 == crsCnt)
					{
						// 求める点と交点の2次元距離
						double dLenInputCrs = fabs(crsY[0] - inputPoint.y);
						double dLenCrsCrs = fabs(crsY[0] - crsY[1]);
						// 比でZを求める
						dz = crsZ[0] + (crsZ[1] - crsZ[0]) * (dLenInputCrs / dLenCrsCrs);
						return true;
					}
				}
				else if (dLen0pt < dEps)
				{	// dLen0pt≒0 頂点上に交点
					if (fabs(posTriangle[ic].y - inputPoint.y) < dEps)
					{	// 頂点と一致
						dz = posTriangle[ic].z;
						return true;
					}
					CPoint2D& v1 = poly2d[(ic + 1) % 3];
					CPoint2D& v2 = poly2d[(ic + 2) % 3];
					double z2 = posTriangle[(ic + 2) % 3].z;
					double z1 = posTriangle[(ic + 1) % 3].z;
					// 2頂点を結ぶ線分上のY値
					double dYtmp = v1.y + (v2.y - v1.y) * (posTriangle[ic].x - v1.x) / (v2.x - v1.x);
					CPoint2D v12(posTriangle[ic].x, dYtmp);
					// 2頂点を結ぶ線分上のZ値
					double dZtmp = z1 + (z2 - z1) * CVector2D(v12, v1).Length() / CVector2D(v2, v1).Length();
					dz = posTriangle[ic].z + (dZtmp - posTriangle[ic].x) * (inputPoint.y - posTriangle[ic].y) / (dYtmp - posTriangle[ic].y);
					return true;
				}
			}
		}
	}

	return false;

}

#pragma endregion
