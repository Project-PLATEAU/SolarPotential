#pragma once
#include <math.h>
#include <vector>
#include <cassert>
#include <string>
#include "CGeoUtil.h"
#include "CEpsUtil.h"

/*!
@brief	２次元頂点列クラス
@note	左下・右上の矩形情報を保存する。
*/
// 
typedef std::vector<CPoint2D> _CPoint2DArray;
class CPoint2DPolygon;
class CPoint2DArray : public _CPoint2DArray
{
	friend CPoint2DPolygon;
	double m_dMin[2];		//!<	座標の最小値
	double m_dMax[2];		//!<	座標の最大値
public:
	double GetMaxX() const { return m_dMax[0]; }
	double GetMaxY() const { return m_dMax[1]; }
	double GetMinX() const { return m_dMin[0]; }
	double GetMinY() const { return m_dMin[1]; }

	/*! 最大・最小の更新
	*/
	void RenewMinMax(int iStart = 0)
	{
		if (iStart == 0)
		{
			m_dMin[0] = m_dMin[1] = DBL_MAX;
			m_dMax[0] = m_dMax[1] = -DBL_MAX;
		}
		for (int i = iStart; i < this->size(); i++)
		{
			m_dMin[0] = CEpsUtil::Greater(m_dMin[0], this->at(i).x) ? this->at(i).x : m_dMin[0];
			m_dMin[1] = CEpsUtil::Greater(m_dMin[1], this->at(i).y) ? this->at(i).y : m_dMin[1];
			m_dMax[0] = CEpsUtil::Less(m_dMax[0], this->at(i).x) ? this->at(i).x : m_dMax[0];
			m_dMax[1] = CEpsUtil::Less(m_dMax[1], this->at(i).y) ? this->at(i).y : m_dMax[1];
		}
	}
	/*! 最大・最小の矩形の内部
	@retval	true	内部
	@retval	false	外部
	@note	境界値上は内部
	*/
	bool IsInsideMinMax(const CPoint2D& pos) const
	{
		if (CEpsUtil::Less(pos.x, GetMinX()))
			return false;
		if (CEpsUtil::Greater(pos.x, GetMaxX()))
			return false;
		if (CEpsUtil::Less(pos.y, GetMinY()))
			return false;
		if (CEpsUtil::Greater(pos.y, GetMaxY()))
			return false;
		return true;
	}

	CPoint2DArray()
	{
		m_dMin[0] = 0.0;
		m_dMin[1] = 0.0;
		m_dMax[0] = 0.0;
		m_dMax[1] = 0.0;
	}
	CPoint2DArray(const CPoint2DArray& in)
	{
		*this = in;
	}
	virtual ~CPoint2DArray()
	{
	}
	INT_PTR Add(CPoint2D& pos)
	{
		((_CPoint2DArray*)this)->push_back(pos);
		INT_PTR iTMP = this->size() - 1;
		if (this->size())
		{
			RenewMinMax((int)this->size() - 1);
		}
		return iTMP;
	}
	void RemoveAt(int pos)
	{
		((_CPoint2DArray*)this)->erase(((_CPoint2DArray*)this)->begin() + pos);
	}
	CPoint2DArray& operator = (const CPoint2DArray& x)
	{
		this->insert(this->end(), x.begin(), x.end());
		for (int i = 0; i < 2; i++)
		{
			m_dMax[i] = x.m_dMax[i];
			m_dMin[i] = x.m_dMin[i];
		}
		return *this;
	}
	/*! 点列の反転
	*/
	void Reverse()
	{
		CPoint2D tmp;
		int iHalfCount = (int)this->size() / 2;
		for (int i = 0; i < iHalfCount; i++)
		{
			tmp = this->at(i);
			this->at(i) = this->at(this->size() - i - 1);
			this->at(this->size() - i - 1) = tmp;
		}
	}

	void SortX(const bool bAscending = true);
	void SortY(const bool bAscending = true);

	void StartLeft();

};

// 2Dポリゴン
// 最後の点から最初の点に戻るとする。重複格納しない。
class CPoint2DPolygon : public CPoint2DArray
{
public:
	CPoint2DPolygon()
	{
	}
	CPoint2DPolygon(const CPoint2DPolygon& in)
	{
		*this = in;
	}
	virtual ~CPoint2DPolygon()
	{
	}

	// 凸ポリゴン同士の交差ポリゴンを得る
	bool GetCrossingPolygon(CPoint2DPolygon& polygon, CPoint2DPolygon& polyCommon, const bool bBL = false);

	// 凸ポリゴン列を取得
	bool GetConvexPolygons(vector<CPoint2DPolygon>& cvxPolys, const bool bBL = false);

	CPoint2DPolygon& operator = (const CPoint2DPolygon& x)
	{
		(*(CPoint2DArray*)this) = (CPoint2DArray&)x;
		return *this;
	}

	bool operator == (const CPoint2DPolygon& x) const
	{
		if (this->size() != x.size())	return false;

		for (int i = 0; i < this->size(); i++)
		{
			const CPoint2D& a = this->at(i);
			const CPoint2D& b = x.at(i);
			if (!CEpsUtil::Equal(a.x, b.x)) return false;
			if (!CEpsUtil::Equal(a.y, b.y)) return false;
		}
		return true;
	}

	bool operator != (const CPoint2DPolygon& x) const
	{
		return !(*this == x);
	}


	/*! 日付変更線考慮
	@note	日付変更線を考慮したCPoint2DPolygonを生成します。
	*/
	void	Normal2dPolygon(CPoint2DPolygon* pPolygon2d)
	{
		// 日付変更線を考慮
		// 繋がる2頂点のLon値を必ず180度以内に収める
		double				NowLon = 0.0;
		double				dOldLon = 0.0;
		for (int i = 0; i < this->size(); i++)
		{
			CPoint2D		pos;
			double	dLat = this->at(i).y;
			double	dLon = this->at(i).x;
			if (i == 0)
			{
				// 先頭
				dOldLon = NowLon = dLon;
			}
			else
			{
				// 先頭以外
				double	AddLon = dLon - dOldLon;
				if (CEpsUtil::Less(180.0, abs(AddLon)))
				{
					AddLon = (360.0 - abs(AddLon)) * (CEpsUtil::Less(AddLon, 0.0) ? 1.0 : -1.0);
				}
				NowLon += AddLon;
				dOldLon = dLon;
			}

			pos = CPoint2D(NowLon, dLat);
			pPolygon2d->Add(pos);
		}

	}

	/*! 時計回り（右手座標系の場合）
	@retval	true	時計回り
	@retval	false	反時計回り
	@note	面積を計算し、＋のときは時計回り、－のときは反時計回りと判定する。
	@note	先に単純ポリゴンあることをチェックすること。
	*/
	bool Clockwise(const bool bBL = false			//!<	in	緯度経度かどうか（日付変更線考慮有無）
	)
	{
		double dArea = 0.0;

		if (bBL)
		{
			// 日付変更線考慮頂点
			CPoint2DPolygon Polygon2d;
			Normal2dPolygon(&Polygon2d);

			for (int i = 0; i < (int)Polygon2d.size(); i++)
			{
				const double dx = Polygon2d.at(((i + 1) % (Polygon2d.size()))).x - Polygon2d.at(i).x;
				const double dy = Polygon2d.at(((i + 1) % (Polygon2d.size()))).y + Polygon2d.at(i).y;
				dArea += dx * dy;
			}
		}
		else
		{
			for (int i = 0; i < this->size(); i++)
			{
				const double dx = this->at((i + 1) % this->size()).x - this->at(i).x;
				const double dy = this->at((i + 1) % this->size()).y + this->at(i).y;
				dArea += dx * dy;
			}
		}
		return CEpsUtil::GreaterEqual(dArea, 0.0);
	}

	/*! 凸ポリゴン
	@retval	true	凸ポリゴン(Convex)
	@retval	false	凹ポリゴン(Non-Convex)
	@note	先に単純ポリゴンであることをチェックすること。
	*/
	bool IsConvexPolygon(const bool bBL = false			//!<	in	緯度経度かどうか（日付変更線考慮有無）
	)
	{
		bool bClockwise = Clockwise(bBL);
		for (int i = 0; i < this->size(); i++)
		{
			if (!IsConvexAngle(i, bClockwise, bBL))
				return false;
		}
		return true;
	}

	///*! 単純ポリゴン
	//@note	単純ポリゴン（辺同士が交差しない）かどうかの判定
	//@retval	true	単純ポリゴン(Simple)
	//@retval	false	複雑ポリゴン(Complex)
	//*/
	//bool IsSimplePolygon();

	/*! 凸角
	@retval	true	凸ポリゴン(Convex)
	@retval	false	凹ポリゴン(Non-Convex)
	@note			180度は凸とする
	@note			単純ポリゴンでない場合は動作は保証されない
	*/
	bool IsConvexAngle(
		const int iIndex,				//!<	in	頂点のインデックス
		const bool bClockwise = true,	//!<	in	時計回りかどうか
		const bool bBL = false			//!<	in	緯度経度かどうか（日付変更線考慮有無）
	)
	{
		bool bRet;

		if (bBL)
		{
			// 日付変更線考慮頂点
			CPoint2DPolygon Polygon2d;
			Normal2dPolygon(&Polygon2d);

			CVector2D v1 = CVector2D(Polygon2d.at(iIndex), Polygon2d.at((iIndex - 1 + (int)this->size()) % (int)this->size()));
			CVector2D v2 = CVector2D(Polygon2d.at((iIndex + 1) % ((int)this->size())), Polygon2d.at(iIndex));
			double dCrsP = v1.CrsP(v2);

			bRet = false;

			if (bClockwise)
			{
				if (CEpsUtil::LessEqual(dCrsP, 0.0))
					bRet = true;
			}
			else
			{
				if (CEpsUtil::GreaterEqual(dCrsP, 0.0))
					bRet = true;
			}
		}
		else
		{
			CVector2D v1 = CVector2D(GetAtExt(iIndex), GetAtExt(iIndex - 1));
			CVector2D v2 = CVector2D(GetAtExt(iIndex + 1), GetAtExt(iIndex));
			double dCrsP = v1.CrsP(v2);

			bRet = false;
			if (bClockwise)
			{
				if (CEpsUtil::LessEqual(dCrsP, 0.0))
					bRet = true;
			}
			else
			{
				if (CEpsUtil::GreaterEqual(dCrsP, 0.0))
					bRet = true;
			}
		}

		return bRet;
	}

	/*! 指定するインデックスの点
	@return	指定するインデックスの点
	@note	-1のとき、最後の点、GetCount() のとき、最初の点を返す
	*/
	CPoint2D& GetAtExt(int iIndex)
	{
		return this->at((iIndex + this->size()) % this->size());
	}

};