#pragma once
#include <math.h>
#include <vector>
#include <cassert>
#include <string>

const double _PI = 3.141592653589793;
const double _COEF_RAD_TO_DEG = 180.0 / _PI;
const double _COEF_DEG_TO_RAD = _PI / 180.0;
const double _MAX_TOL = 0.00001;		// 許容誤差(仮値)

const double daa = 6378137;			//長半径
const double dF = 298.257222101;	//逆扁平率
const double dM0 = 0.9999;			//平面直角座標系のY軸上における縮尺係数(UTM座標系の場合→0.9996)

// 平面直角座標系の原点(単位：秒)
static const double JPN_ORG_LAT_SEC[20] = {
		 0.0,
	118800.0, 118800.0, 129600.0, 118800.0,129600.0, 129600.0,129600.0,
	129600.0, 129600.0, 144000.0, 158400.0, 158400.0,158400.0, 93600.0,
	 93600.0,  93600.0,  93600.0,72000.0, 93600.0 };
static const double JPN_ORG_LON_SEC[20] = {
		 0.0,
	466200.0, 471600.0, 475800.0, 480600.0, 483600.0, 489600.0, 493800.0,
	498600.0, 503400.0, 507000.0, 504900.0, 512100.0, 519300.0, 511200.0,
	459000.0, 446400.0, 471600.0, 489600.0, 554400.0 };

// 座標
class CPointBase
{
public:
	double x, y, z;

	CPointBase()
	{
		x = 0; y = 0; z = 0;
	}

	CPointBase(const double& x0, const double& y0, const double& z0)
	{
		x = x0; y = y0; z = z0;
	}
};

// ベクトル(3d)
class CVector3D : public CPointBase
{
public:
	CVector3D(const double& x, const double& y, const double& z) : CPointBase(x, y, z) {}
	CVector3D() {}
	// p2からp1へのベクトル
	CVector3D(const CPointBase& p1, const CPointBase& p2)
		: CPointBase(p1.x - p2.x, p1.y - p2.y, p1.z - p2.z) {}

	CVector3D& operator = (const CVector3D& v) {
		if (&v != this) { x = v.x; y = v.y; z = v.z; }
		return *this;
	}
	CVector3D  operator + (const CVector3D& v) const { return CVector3D(x + v.x, y + v.y, z + v.z); }
	CVector3D  operator - (const CVector3D& v) const { return CVector3D(x - v.x, y - v.y, z - v.z); }
	CVector3D  operator * (double a) const { return CVector3D(x * a, y * a, z * a); }
	CVector3D  operator *=(double a) { x *= a; y *= a; z *= a; return *this; }
	CVector3D& operator /=(double a) { assert(!(abs(a) < DBL_EPSILON)); x /= a; y /= a; z /= a; return *this; }
	CVector3D& operator +=(const CVector3D& a) { x += a.x; y += a.y; z += a.z; return *this; }
	CVector3D& operator -=(const CVector3D& a) { x -= a.x; y -= a.y; z -= a.z; return *this; }

	// ベクトル反転
	void Inverse() { x = -1 * x; y = -1 * y; z = -1 * z; }

	// ベクトル大きさ
	inline double Length() const { return sqrt(x * x + y * y + z * z); }

	// 単位ベクトル
	void Normalize() {
		double dL = Length();
		x /= dL; y /= dL; z /= dL;
	}
};

// スカラーとベクトルの掛け算
inline CVector3D operator * (double a, const CVector3D& b) { return CVector3D(a * b.x, a * b.y, a * b.z); }

// 座標(2d)
class CPoint2D
{
public:
	double x, y;

	CPoint2D()
	{
		x = 0; y = 0;
	}

	CPoint2D(const double& x0, const double& y0)
	{
		x = x0; y = y0;
	}

};

// ベクトル(2d)
class CVector2D : public CPoint2D
{
public:
	CVector2D(const double& x, const double& y) : CPoint2D(x, y) {}
	CVector2D() {}

	CVector2D& operator = (const CVector2D& v) {
		if (&v != this) { x = v.x; y = v.y; }
		return *this;
	}
	CVector2D  operator + (const CVector2D& v) const { return CVector2D(x + v.x, y + v.y); }
	CVector2D  operator - (const CVector2D& v) const { return CVector2D(x - v.x, y - v.y); }
	CVector2D  operator * (double a) const { return CVector2D(x * a, y * a); }
	CVector2D  operator *=(double a) { x *= a; y *= a; return *this; }

	// Pos2からPos1方向へのベクトルで初期化
	CVector2D(const CPoint2D& p1,	//!< in Pos1
		const CPoint2D& p2			//!< in Pos2
	) {
		x = p1.x - p2.x; y = p1.y - p2.y;
	}

	// ベクトル反転
	void Inverse() { x = -1 * x; y = -1 * y; }

	// ベクトル大きさ
	double Length() const { return sqrt(x * x + y * y); }

	// 単位ベクトル
	void Normalize() {
		double dL = Length();
		x /= dL; y /= dL;
	}

	// ２点間の距離を求める
	double Distance(const CPoint2D& p) const { return(sqrt(pow(x - p.x, 2) + pow(y - p.y, 2))); }
	double Distance(const double& x, const double& y) const {
		return(Distance(CPoint2D(x, y)));
	}
	static double Distance(const CPoint2D& p1, const CPoint2D& p2) {
		CVector2D tmp(p1.x, p1.y);
		return(tmp.Distance(p2));
	}
	static double Distance(const double& x1, const double& y1,
		const double& x2, const double& y2) {
		CVector2D tmp(x1, y1);
		return(tmp.Distance(x2, y2));
	}

};

// スカラーとベクトルの掛け算
inline CVector2D operator * (double a, const CVector2D& b) { return CVector2D(a * b.x, a * b.y); }


// 平面( ax+by+cz+d=0 )
class CPlane
{
public:
	double a, b, c, d;
};

// 三角形クラス
class CTriangle
{
public:
	std::vector<CPointBase> posTriangle;

	CTriangle(CPointBase p1, CPointBase p2, CPointBase p3)
	{
		posTriangle.push_back(p1);
		posTriangle.push_back(p2);
		posTriangle.push_back(p3);

	}

	CTriangle()
	{
		posTriangle.clear();
	}
};


class CGeoUtil
{
public:

#pragma region 幾何計算
	//ベクトル内積
	static double InnerProduct(const CVector3D& vl, const CVector3D& vr);

	//ベクトル2D内積
	static double InnerProduct(const CVector2D& vl, const CVector2D& vr);

	//ベクトル外積
	static void OuterProduct(const CVector3D& v1, const CVector3D& v2, CVector3D& vOut);

	// 平面の式取得
	static void GetPlaneParameter(const CPointBase& pos1, const CPointBase& pos2, const CPointBase& pos3, CPlane& plane);

	// 傾斜角
	// 上向きベクトルと面法線の成す角度
	static void CalcSlope(CVector3D& vP, double& slopeDegree);

	// 方位角
	// 2次元の法線ベクトルと真北のなす角度
	static void CalcAzimuth(CVector3D& vP, double& azDegree, int iJPZone);

	//点と平面の距離を求める ( P=平面上の点 N=平面の法線 )
	static double CalcDistancePointToFace(const CVector3D& A, const CVector3D& P, const CVector3D& N);

	// 点と平面との距離の算出
	// 平面はax + by + cz + d = 0で与えるものとする
	// 点と平面との距離
	static double CalcDistancePointToFace(const double& x, const double& y, const double& z, const double& a, const double& b, const double& c, const double& d);

	// 内外判定
	static bool IsPointInPolygon(
		const CPoint2D& pointTarget,		// 内外判定を行う点座標
		unsigned int uiCountPoint,			// 頂点数
		const CPoint2D* pPoint);			// 多角形の座標

	// 単位ベクトル
	static CVector3D Normalize(const CVector3D& vec);

#pragma endregion

#pragma region 座標変換
	// 最適な系番号を求める
	static int GetNearJPZone(const double& dLon, const double& dLat);
	
	// 緯度経度から任意原点の平面直角座標を求める
	static void LonLatToXY(const double& dLon, const double& dLat, int JPZone, double& X, double& Y );

	// 平面直角座標から緯度経度を求める
	static void XYToLatLon(int JPZone, const double& dNorth, const double& dEast, double& dLat, double& dLon);

	// 子午線収差角(=-真北方向角)を求める
	static double calcNorth(int JPZone, const double& dNorth, const double& dEast);

	// ３次メッシュコードから緯度経度を求める
	static bool MeshIDToLatLon(const std::string& meshID, double& lat, double& lon);
#pragma endregion	
};

