#include "pch.h"
#include "CGeoUtil.h"
#include "CPoint2DPolygon.h"
#include <cmath>
#include <random>

// 以下、GetCrossingPolygonで使用するためのテンポラリクラス。
// 台形分割に使うテンポラリクラス。
// ポリゴンの構成点がもとの多角形のポリゴンの頂点(0)か、交点(1)か、もしくは分割線で強制的に作成された点(2)かを記録する配列
class __CPoint2DArray : public CPoint2DArray
{
public:
	std::vector<int> m_aryAtt;
	__CPoint2DArray()
	{
	}
	__CPoint2DArray(const __CPoint2DArray& in)
	{
		*this = in;
	}
	__CPoint2DArray& operator = (const __CPoint2DArray& x)
	{
		((CPoint2DArray&)*this) = (CPoint2DArray&)x;
		m_aryAtt = x.m_aryAtt;
		return *this;
	}

};
// 交点管理用のテンポラリ点クラス
class __CPoint2D : public CPoint2D
{
public:
	int iIndex[2];
};

// Y座標とインデックスを格納するための構造体
class  _C_Y_AND_INDEX
{
public:
	double m_dY;
	int m_iIndex;
};

// vector<double> ソート用の関数
int _compareDoubleAscending(const void* pa, const void* pb)
{
	double* a;
	double* b;
	a = (double*)pa;
	b = (double*)pb;
	if (CEpsUtil::Equal(*a, *b))
		return 0;
	else if (CEpsUtil::Less(*a, *b))
		return -1;
	else
		return 1;
}

// CAASVec2dArrayのqsort用関数群
int _compareXAscending(const void* pa, const void* pb)
{
	CVector2D* a;
	CVector2D* b;
	a = (CVector2D*)pa;
	b = (CVector2D*)pb;
	if (CEpsUtil::Equal(a->x, b->x))
		return 0;
	else if (CEpsUtil::Less(a->x, b->x))
		return -1;
	else
		return 1;
}
int _compareXDescending(const void* pa, const void* pb)
{
	CVector2D* a;
	CVector2D* b;
	a = (CVector2D*)pa;
	b = (CVector2D*)pb;
	if (CEpsUtil::Equal(a->x, b->x))
		return 0;
	else if (CEpsUtil::Greater(a->x, b->x))
		return -1;
	else
		return 1;
}
int _compareYAscending(const void* pa, const void* pb)
{
	CVector2D* a;
	CVector2D* b;
	a = (CVector2D*)pa;
	b = (CVector2D*)pb;
	if (CEpsUtil::Equal(a->y, b->y))
		return 0;
	else if (CEpsUtil::Less(a->y, b->y))
		return -1;
	else
		return 1;
}
int _compareYDescending(const void* pa, const void* pb)
{
	CVector2D* a;
	CVector2D* b;
	a = (CVector2D*)pa;
	b = (CVector2D*)pb;
	if (CEpsUtil::Equal(a->y, b->y))
		return 0;
	else if (CEpsUtil::Greater(a->y, b->y))
		return -1;
	else
		return 1;
}

/*! Y座標によるソート(降順用)
*/
int _compareY_AND_INDEXDescending(const void* pa, const void* pb)
{
	_C_Y_AND_INDEX* a;
	_C_Y_AND_INDEX* b;
	a = (_C_Y_AND_INDEX*)pa;
	b = (_C_Y_AND_INDEX*)pb;
	if (CEpsUtil::Equal(a->m_dY, b->m_dY))
		return 0;
	else if (CEpsUtil::Greater(a->m_dY, b->m_dY))
		return -1;
	else
		return 1;
}
/*! Y座標によるソート(降順用)
*/
int _compareY_AND_INDEXAscending(const void* pa, const void* pb)
{
	_C_Y_AND_INDEX* a;
	_C_Y_AND_INDEX* b;
	a = (_C_Y_AND_INDEX*)pa;
	b = (_C_Y_AND_INDEX*)pb;
	if (CEpsUtil::Equal(a->m_dY, b->m_dY))
		return 0;
	else if (CEpsUtil::Less(a->m_dY, b->m_dY))
		return -1;
	else
		return 1;
}

/*! X座標によるソート
*/
void CPoint2DArray::SortX(bool bAscending)
{
	if (bAscending)
		qsort(this->data(), this->size(), sizeof(CVector2D), _compareXAscending);
	else
		qsort(this->data(), this->size(), sizeof(CVector2D), _compareXDescending);
}

/*! Y座標によるソート
*/
void CPoint2DArray::SortY(bool bAscending)
{
	if (bAscending)
		qsort(this->data(), this->size(), sizeof(CVector2D), _compareYAscending);
	else
		qsort(this->data(), this->size(), sizeof(CVector2D), _compareYDescending);
}

/*! 左側を始点として並び替え(始点と終点は一致しない)
*/
void CPoint2DArray::StartLeft()
{
	CPoint2DArray polyTmp = *this;

	// 始点となるインデックスを取得
	int nStartIndex = 0;
	double dMinX = DBL_MAX;
	for (int i = 0; i < polyTmp.size(); i++)
	{
		if (CEpsUtil::Less(polyTmp.at(i).x, dMinX))
		{
			dMinX = polyTmp.at(i).x;
			nStartIndex = i;
		}
	}
	if (nStartIndex == 0) return;	// 左側が始点になっていれば終了

	// 元の配列の開始位置から最後まで
	int addIndex = 0;
	for (int i = nStartIndex; i < polyTmp.size(); i++)
	{
		this->at(addIndex) = polyTmp.at(i);
		addIndex++;
	}

	// 元の配列の先頭から開始位置のひとつ前まで
	CPoint2D tmp;
	for (int i = 0; i < nStartIndex; i++)
	{
		// 最後に追加した点と始点が同じ場合はスキップ
		if (this->at(addIndex - 1) == polyTmp.at(i))
		{
			tmp = polyTmp.at(nStartIndex);
			continue;
		}
		this->at(addIndex) = polyTmp.at(i);
		addIndex++;
	}

	if (addIndex < polyTmp.size())
	{
		this->at(this->size() - 1) = tmp;
	}
}

/*!	凸ポリゴン同士の交差ポリゴンを得る
@retval	true	交差する
@retval false	交差しない
@note	凸ポリゴン同士の交差ポリゴンを得る
<br>	入力が凸ポリゴンでないとき、動作は保証されない。
*/
bool CPoint2DPolygon::GetCrossingPolygon(
	CPoint2DPolygon& polygon,		//!<	in	ポリゴン
	CPoint2DPolygon& polyCommon,	//!<	out	多角形
	const bool bBL /*= false*/		//!<	in	緯度経度かどうか（日付変更線考慮有無）
)
{
	//出力ポリゴンの初期化
	polyCommon.clear();

	if (CEpsUtil::Equal(GetMinX(), DBL_MAX))
	{
		RenewMinMax();
	}
	if (CEpsUtil::Equal(polygon.GetMinX(), DBL_MAX))
	{
		polygon.RenewMinMax();
	}

	// 最大矩形の外部ならfalse
	if (CEpsUtil::GreaterEqual(GetMinX(), polygon.GetMaxX()))
	{
		return false;
	}
	if (CEpsUtil::GreaterEqual(GetMinY(), polygon.GetMaxY()))
	{
		return false;
	}
	if (CEpsUtil::LessEqual(GetMaxX(), polygon.GetMinX()))
	{
		return false;
	}
	if (CEpsUtil::LessEqual(GetMaxY(), polygon.GetMinY()))
	{
		return false;
	}

	// y=aryX[i]とy=aryX[i+1]の２線分内の台形を作成し、その共通部分を登録する。


	std::vector<__CPoint2DArray> aryTrapezoid0Upper, aryTrapezoid0Lower, aryTrapezoid1Upper, aryTrapezoid1Lower;
	std::vector<__CPoint2DArray>* pAryTrapezoid[2][2];
	pAryTrapezoid[0][0] = &aryTrapezoid0Upper;
	pAryTrapezoid[0][1] = &aryTrapezoid0Lower;
	pAryTrapezoid[1][0] = &aryTrapezoid1Upper;
	pAryTrapezoid[1][1] = &aryTrapezoid1Lower;

	//　両方のポリゴンの作業用ポリゴン
	CPoint2DPolygon polyTmp[2];
	polyTmp[0] = *this;
	polyTmp[1] = polygon;

	int iIndexUpper = 0;
	int iIndexLower = 1;

	// ポリゴンに対する前処理
		// ポリゴンの上側のインデックスと下側のインデックス
	int iIndexLeft[2][2];

	for (int iPoly = 0; iPoly < 2; iPoly++)
	{
		// 時計回りに統一
		if (!polyTmp[iPoly].Clockwise(bBL))
			polyTmp[iPoly].Reverse();

		// Y座標が同じ点が３点以上続く場合、取り除いておく。また、重複点を取り除いておく。
		for (int jCorner = 0; jCorner < polyTmp[iPoly].size(); jCorner++)
		{
			// 1点目は取り除かない
			if (jCorner == 0)	continue;

			if (polyTmp[iPoly].GetAtExt(jCorner).x == polyTmp[iPoly].GetAtExt(jCorner - 1).x
				&& polyTmp[iPoly].GetAtExt(jCorner).x == polyTmp[iPoly].GetAtExt(jCorner + 1).x)
			{
				polyTmp[iPoly].RemoveAt(jCorner);
				jCorner--;
			}
			else if (polyTmp[iPoly].GetAtExt(jCorner).x == polyTmp[iPoly].GetAtExt(jCorner - 1).x
				&& polyTmp[iPoly].GetAtExt(jCorner).y == polyTmp[iPoly].GetAtExt(jCorner - 1).y)
			{
				polyTmp[iPoly].RemoveAt(jCorner);
				jCorner--;
			}
		}

		// X座標が最小かつY座標が最大のインデックスをiIndexUpperに、X座標が最小かつY座標が最小のインデックスをiIndexLowerに格納する
		CPoint2D* pPosTmp = &(polyTmp[iPoly].GetAtExt(0));
		iIndexLeft[iPoly][iIndexUpper] = 0;
		iIndexLeft[iPoly][iIndexLower] = 0;
		for (int jCorner = 1; jCorner < polyTmp[iPoly].size(); jCorner++)
		{
			if (CEpsUtil::Less(polyTmp[iPoly].GetAtExt(jCorner).x, pPosTmp->x))
			{
				iIndexLeft[iPoly][iIndexUpper] = jCorner;
				iIndexLeft[iPoly][iIndexLower] = jCorner;
				pPosTmp = &(polyTmp[iPoly].GetAtExt(jCorner));
			}
			else if (CEpsUtil::Equal(polyTmp[iPoly].GetAtExt(jCorner).x, pPosTmp->x))
			{
				if (CEpsUtil::Less(polyTmp[iPoly].GetAtExt(jCorner).y, pPosTmp->y))
					iIndexLeft[iPoly][iIndexLower] = jCorner;
				else
					iIndexLeft[iPoly][iIndexUpper] = jCorner;
				pPosTmp = &(polyTmp[iPoly].GetAtExt(jCorner));
			}
		}
	}

	// 両方のポリゴンのX座標列を得る。
	std::vector<double> aryX;
	double dTmp;
	for (int iPoly = 0; iPoly < 2; iPoly++)
	{
		for (int i = 0; i < polyTmp[iPoly].size(); i++)
		{
			dTmp = polyTmp[iPoly].at(i).x;
			aryX.push_back(dTmp);
		}
	}

	// 昇順に並べる
	qsort(aryX.data(), aryX.size(), sizeof(double), _compareDoubleAscending);

	// 重複座標を取る
	for (int i = 1; i < aryX.size(); i++)
	{
		if (CEpsUtil::Equal(aryX.at(i), aryX.at(i - 1)))
		{
			aryX.erase(aryX.begin() + i);
			i--;
		}
	}



	// 台形部分の抽出
	CPoint2D pos[2][2];
	int iAtt[2][2];
	CPoint2D* pPos, * pPosNext;
	CPoint2DPolygon* pPoly;

	const int iLeft = 0;
	const int iRight = 1;
	const int iUpper = 0;
	const int iLower = 1;
	const int iPoly0 = 0;
	const int iPoly1 = 1;


	for (int iDivX = 0; iDivX < aryX.size() - 1; iDivX++)
	{
		// 左側と右側のX座標
		double dXLeft, dXRight;
		dXLeft = aryX[iDivX];
		dXRight = aryX[iDivX + 1];

		// 台形があるかどうかの判定
		{
			bool bBreak = false;
			for (int iPoly = 0; iPoly < 2; iPoly++)
			{
				pPoly = &polyTmp[iPoly];
				pPos = &(pPoly->GetAtExt(iIndexLeft[iPoly][iIndexUpper]));
				pPosNext = &(pPoly->GetAtExt(iIndexLeft[iPoly][iIndexUpper] + 1));

				// 切り出し範囲が現在の点の左側のときは、スキップ
				if (CEpsUtil::Greater(pPos->x, dXLeft))
				{
					continue;
				}
				// 座標が戻るときは終わり
				if (CEpsUtil::GreaterEqual(pPos->x, pPosNext->x))
				{
					bBreak = true;
					break;
				}
				// すべての頂点のX座標をaryXに登録しているので、これはありえないはず！
				if (CEpsUtil::Less(pPosNext->x, dXLeft))
				{
					assert(FALSE);
					return false;
				}
			}
			if (bBreak)
				break;
		}


		for (int iPoly = 0; iPoly < 2; iPoly++)
		{
			pPoly = &polyTmp[iPoly];

			//空のポリゴンを追加
			__CPoint2DArray poly;
			pAryTrapezoid[iPoly][iUpper]->push_back(poly);
			pAryTrapezoid[iPoly][iLower]->push_back(poly);



			// iUL = 0: 上側
			// iUL = 1: 下側

			for (int iUL = 0; iUL < 2; iUL++)
			{
				__CPoint2DArray* pTrap = &(pAryTrapezoid[iPoly][iUL]->at(iDivX));
				// 点の属性
				std::vector<int>* pAtt = &(pTrap->m_aryAtt);

				// 右側の点へのインデックスのインクリメント
				int iInc = 1;
				// 下のときはインデックスを戻す
				if (iUL == 1)
					iInc = -1;

				int* pIndex = &iIndexLeft[iPoly][iUL];

				// 左側
				pPos = &(pPoly->GetAtExt(*pIndex));
				pPosNext = &(pPoly->GetAtExt(*pIndex + iInc));

				// 左の点
				if (CEpsUtil::Greater(pPos->x, dXLeft))
				{
					continue;
				}
				else if (CEpsUtil::Equal(pPos->x, dXLeft))
				{
					pos[iUL][iLeft] = *pPos;
					// ポリゴンの頂点(0)
					iAtt[iUL][iLeft] = 0;
				}
				else if (CEpsUtil::Less(pPos->x, dXLeft))
				{
					double dx = pPosNext->x - pPos->x;
					double dy = pPosNext->y - pPos->y;
					double dx1 = dXLeft - pPos->x;
					double dY = dx1 / dx * dy + pPos->y;
					pos[iUL][iLeft].x = dXLeft;
					pos[iUL][iLeft].y = dY;
					// 分割線で強制的に作成された点(2)
					iAtt[iUL][iLeft] = 2;
				}
				// これはありえないはず
				else
				{
					assert(FALSE);
					return false;
				}

				// 右の点
				if (CEpsUtil::Equal(pPosNext->x, dXRight))
				{
					pos[iUL][iRight] = *pPosNext;
					(*pIndex) += iInc;
					// ポリゴンの頂点(0)
					iAtt[iUL][iRight] = 0;
				}
				else if (CEpsUtil::Greater(pPosNext->x, dXRight))
				{
					double dx = pPosNext->x - pPos->x;
					double dy = pPosNext->y - pPos->y;
					double dx1 = dXRight - pPos->x;
					double dY = dx1 / dx * dy + pPos->y;
					pos[iUL][iRight].x = dXRight;
					pos[iUL][iRight].y = dY;
					// 分割線で強制的に作成された点(2)
					iAtt[iUL][iRight] = 2;
				}
				// 台形の辺の登録
				pTrap->Add(pos[iUL][iLeft]);
				pTrap->Add(pos[iUL][iRight]);
				pAtt->push_back(iAtt[iUL][iLeft]);
				pAtt->push_back(iAtt[iUL][iRight]);
			}
		}

		// 台形同士に交点がある場合はさらに分割

		__CPoint2DArray* ppoly[2][2];
		int iIndexOrg = (int)pAryTrapezoid[iPoly1][iIndexUpper]->size() - 1;

		for (int iPoly = 0; iPoly < 2; iPoly++)
			for (int iUL = 0; iUL < 2; iUL++)
			{
				ppoly[iPoly][iUL] = &(pAryTrapezoid[iPoly][iUL]->at(iIndexOrg));
			}

		if (ppoly[0][0]->size() == 0)
			continue;

		if (ppoly[1][0]->size() == 0)
			continue;

		if (ppoly[0][1]->size() == 0)
			continue;

		if (ppoly[1][1]->size() == 0)
			continue;


		std::vector<__CPoint2D> aryPosTmp;
		double dXTmp, dYTmp;

		double dy[2][2][2];

		for (int iLR = 0; iLR < 2; iLR++)
		{
			for (int iUL1 = 0; iUL1 < 2; iUL1++)
			{
				for (int iUL2 = 0; iUL2 < 2; iUL2++)
				{
					dy[iLR][iUL1][iUL2] = ppoly[iPoly1][iUL2]->at(iLR).y - ppoly[iPoly0][iUL1]->at(iLR).y;
				}
			}
		}

		if (CEpsUtil::GreaterEqual(dy[iLeft][iUpper][iLower], 0.0) && CEpsUtil::GreaterEqual(dy[iRight][iUpper][iLower], 0.0))
			continue;

		if (CEpsUtil::LessEqual(dy[iLeft][iLower][iUpper], 0.0) && CEpsUtil::LessEqual(dy[iRight][iLower][iUpper], 0.0))
			continue;

		// p1 上線と　p2　上線の交差位置
		__CPoint2D postmp;
		double dRatio;
		for (int iUL0 = 0; iUL0 < 2; iUL0++)
			for (int iUL1 = 0; iUL1 < 2; iUL1++)
			{
				if (CEpsUtil::Less(dy[iLeft][iUL0][iUL1] * dy[iRight][iUL0][iUL1], 0.0))
				{
					dRatio = fabs(dy[iLeft][iUL0][iUL1] / (dy[iLeft][iUL0][iUL1] - dy[iRight][iUL0][iUL1]));
					dXTmp = (dXRight - dXLeft) * dRatio;
					dYTmp = (ppoly[iPoly0][iUL0]->at(iRight).y - ppoly[iPoly0][iUL0]->at(iLeft).y) * dRatio;
					postmp.x = dXTmp + dXLeft;
					postmp.y = dYTmp + ppoly[iPoly0][iUL0]->at(iLeft).y;
					// 交点を構成する線のインデックスを記録
					postmp.iIndex[iPoly0] = iUL0;
					postmp.iIndex[iPoly1] = iUL1;
					aryPosTmp.push_back(postmp);
				}
			}

		// 交点がない場合は次にいく
		if (aryPosTmp.size() == 0)
			continue;

		// 交点がある場合、aryXTmpをソート
		qsort(aryPosTmp.data(), aryPosTmp.size(), sizeof(__CPoint2D), _compareXAscending);

		// 交点のX座標で台形を分割
		for (int iPoly = 0; iPoly < 2; iPoly++)
		{
			for (int iUL = 0; iUL < 2; iUL++)
			{
				double dd = (ppoly[iPoly][iUL]->at(iRight).y - ppoly[iPoly][iUL]->at(iLeft).y) / (dXRight - dXLeft);

				for (int iDivSub = 0; iDivSub < aryPosTmp.size(); iDivSub++)
				{
					CPoint2D postmp;
					__CPoint2D* pPostmp = &(aryPosTmp.at(iDivSub));
					int iAtt;
					if (pPostmp->iIndex[iPoly] == iUL)
					{
						postmp.x = pPostmp->x;
						postmp.y = pPostmp->y;
						iAtt = 1;
					}
					else
					{
						postmp.x = pPostmp->x;
						postmp.y = ppoly[iPoly][iUL]->at(iLeft).y + dd * (pPostmp->x - dXLeft);
						iAtt = 2;
					}
					auto it1 = ppoly[iPoly][iUL]->begin() + iDivSub + 1;
					ppoly[iPoly][iUL]->insert(it1, postmp);
					auto it2 = ppoly[iPoly][iUL]->m_aryAtt.begin() + iDivSub + 1;
					ppoly[iPoly][iUL]->m_aryAtt.insert(it2, iAtt);
				}
			}
		}

	}

	if (pAryTrapezoid[0][0]->size() == 0)
		return false;
	if (pAryTrapezoid[1][0]->size() == 0)
		return false;


	// 台形の共通部分の取得

	{
		__CPoint2DArray aryUpper;
		__CPoint2DArray aryLower;

		int iAttCross = 1;

		bool bStart = false;
		double dy[2][2][2];
		__CPoint2DArray* ppoly[2][2];

		int iLastLow = -1;
		int iLastUp = -1;

		for (int iDivX = 0; iDivX < pAryTrapezoid[0][1]->size() && iDivX < pAryTrapezoid[0][1]->size(); iDivX++)
		{
			for (int iPoly = 0; iPoly < 2; iPoly++)
				for (int iUL = 0; iUL < 2; iUL++)
				{
					ppoly[iPoly][iUL] = &(pAryTrapezoid[iPoly][iUL]->at(iDivX));
				}

			if (ppoly[0][0]->size() == 0)
				continue;

			if (ppoly[1][0]->size() == 0)
				continue;

			if (ppoly[0][1]->size() == 0)
				continue;

			if (ppoly[1][1]->size() == 0)
				continue;

			for (int iDivSub = 0; iDivSub < ppoly[iPoly0][iUpper]->size() - 1; iDivSub++)
			{
				// 端でのY座標の差を求める
				for (int iLR = 0; iLR < 2; iLR++)
				{
					for (int iUL1 = 0; iUL1 < 2; iUL1++)
					{
						for (int iUL2 = 0; iUL2 < 2; iUL2++)
						{
							dy[iLR][iUL1][iUL2] = ppoly[iPoly1][iUL2]->at(iDivSub + iLR).y - ppoly[iPoly0][iUL1]->at(iDivSub + iLR).y;
						}
					}
				}

				// 共有のないケース
				if (CEpsUtil::GreaterEqual(dy[iLeft][iUpper][iLower], 0.0) && CEpsUtil::GreaterEqual(dy[iRight][iUpper][iLower], 0.0))
				{
					if (bStart)
						goto MERGELOOP_END;
					else
						continue;
				}

				if (CEpsUtil::LessEqual(dy[iLeft][iLower][iUpper], 0.0) && CEpsUtil::LessEqual(dy[iRight][iLower][iUpper], 0.0))
				{
					if (bStart)
						goto MERGELOOP_END;
					else
						continue;
				}

				int iUp = -1;
				int iLow = -1;
				// iPoly0 の上がiPoly1の上より下
				if (CEpsUtil::GreaterEqual(dy[iLeft][iUpper][iUpper], 0.0) && CEpsUtil::GreaterEqual(dy[iRight][iUpper][iUpper], 0.0))
				{
					// iPoly0 の上がiPoly1の下より上
					if (CEpsUtil::LessEqual(dy[iLeft][iUpper][iLower], 0.0) && CEpsUtil::LessEqual(dy[iRight][iUpper][iLower], 0.0))
					{
						iUp = iPoly0;
					}
				}
				if (iUp == -1)
				{
					// iPoly1 の上がiPoly0の上より下
					if (CEpsUtil::LessEqual(dy[iLeft][iUpper][iUpper], 0.0) && CEpsUtil::LessEqual(dy[iRight][iUpper][iUpper], 0.0))
					{
						// iPoly1 の上がiPoly0の上より下
						if (CEpsUtil::GreaterEqual(dy[iLeft][iLower][iUpper], 0.0) && CEpsUtil::GreaterEqual(dy[iRight][iLower][iUpper], 0.0))
						{
							iUp = iPoly1;
						}
					}
					else
					{
						if (bStart)
							goto MERGELOOP_END;
						else
							continue;
					}
				}

				// iPoly0 の下がiPoly1の上より下
				if (CEpsUtil::GreaterEqual(dy[iLeft][iLower][iUpper], 0.0) && CEpsUtil::GreaterEqual(dy[iRight][iLower][iUpper], 0.0))
				{
					// iPoly0 の下がiPoly1の下より上
					if (CEpsUtil::LessEqual(dy[iLeft][iLower][iLower], 0.0) && CEpsUtil::LessEqual(dy[iRight][iLower][iLower], 0.0))
					{
						iLow = iPoly0;
					}
				}
				if (iLow == -1)
				{
					// iPoly1 の下がiPoly0の上より下
					if (CEpsUtil::LessEqual(dy[iLeft][iUpper][iLower], 0.0) && CEpsUtil::LessEqual(dy[iRight][iUpper][iLower], 0.0))
					{
						// iPoly1 の下がiPoly0の上より下
						if (CEpsUtil::GreaterEqual(dy[iLeft][iLower][iLower], 0.0) && CEpsUtil::GreaterEqual(dy[iRight][iLower][iLower], 0.0))
						{
							iLow = iPoly1;
						}
					}
					else
					{
						if (bStart)
							goto MERGELOOP_END;
						else
							continue;
					}
				}


				if (iLow != -1 && iUp != -1)
				{
					if (bStart == false)
					{
						aryUpper.Add(ppoly[iUp][iUpper]->at(iDivSub));
						aryUpper.m_aryAtt.push_back(ppoly[iUp][iUpper]->m_aryAtt.at(iDivSub));
						// 最初の点が分割点の場合、強制的に交点(1)にする
						if (aryUpper.m_aryAtt.at(aryUpper.m_aryAtt.size() - 1) == 2)
							aryUpper.m_aryAtt.at(aryUpper.m_aryAtt.size() - 1) = iAttCross;

						aryLower.Add(ppoly[iLow][iLower]->at(iDivSub));
						aryLower.m_aryAtt.push_back(ppoly[iLow][iLower]->m_aryAtt.at(iDivSub));

						// 最初の点が分割点の場合、強制的に交点(1)にする
						if (aryLower.m_aryAtt.at(aryLower.m_aryAtt.size() - 1) == 2)
							aryLower.m_aryAtt.at(aryLower.m_aryAtt.size() - 1) = iAttCross;
					}
					else
					{
						if (aryUpper.m_aryAtt.at(aryUpper.m_aryAtt.size() - 1) == 2)
						{
							aryUpper.m_aryAtt.at(aryUpper.m_aryAtt.size() - 1) = ppoly[iUp][iUpper]->m_aryAtt.at(iDivSub);
						}
						if (aryLower.m_aryAtt.at(aryLower.m_aryAtt.size() - 1) == 2)
						{
							aryLower.m_aryAtt.at(aryLower.m_aryAtt.size() - 1) = ppoly[iLow][iLower]->m_aryAtt.at(iDivSub);
						}

						// 前とiUpが変わるときは交点（分割点が前の段階で入る場合があるので、強制的に交点(1)にする）
						if (iLastUp != iUp)
						{
							aryUpper.m_aryAtt.at(aryUpper.m_aryAtt.size() - 1) = iAttCross;
						}
						if (iLastLow != iLow)
						{
							aryLower.m_aryAtt.at(aryLower.m_aryAtt.size() - 1) = iAttCross;
						}
					}
					aryUpper.Add(ppoly[iUp][iUpper]->at(iDivSub + 1));
					aryUpper.m_aryAtt.push_back(ppoly[iUp][iUpper]->m_aryAtt.at(iDivSub + 1));
					aryLower.Add(ppoly[iLow][iLower]->at(iDivSub + 1));
					aryLower.m_aryAtt.push_back(ppoly[iLow][iLower]->m_aryAtt.at(iDivSub + 1));

					iLastUp = iUp;
					iLastLow = iLow;

					bStart = true;
				}
				else
				{
					// 点列終了
					if (bStart)
					{
						goto MERGELOOP_END;

					}
					else
						continue;
				}
			}
		}

	MERGELOOP_END:

		if (aryUpper.size() == 0)
			return false;
		if (aryLower.size() == 0)
			return false;

		// 最後の点が分割点の場合、強制的に交点(1)にする
		if (aryUpper.m_aryAtt.at(aryUpper.m_aryAtt.size() - 1) == 2)
			aryUpper.m_aryAtt.at(aryUpper.m_aryAtt.size() - 1) = iAttCross;

		// 最後の点が分割点の場合、強制的に交点(1)にする
		if (aryLower.m_aryAtt.at(aryLower.m_aryAtt.size() - 1) == 2)
			aryLower.m_aryAtt.at(aryLower.m_aryAtt.size() - 1) = iAttCross;


		// ポリゴンの統合(上）
		for (int i = 0; i < (int)aryUpper.size(); i++)
		{
			// 両端でなく、分割点のときはとばす
			if (i != 0 && i != (int)aryUpper.size() - 1)
			{
				if (aryUpper.m_aryAtt.at(i) == 2)
					continue;
			}
			polyCommon.Add((CPoint2D&)aryUpper.at(i));
		}
		// ポリゴンの統合（下）
		for (int i = (int)aryLower.size() - 1; i >= 0; i--)
		{
			// 両端でなく、分割点のときはとばす
			if (i != 0 && i != (int)aryUpper.size() - 1)
			{
				if (aryLower.m_aryAtt.at(i) == 2)
					continue;
			}
			polyCommon.Add((CPoint2D&)aryLower.at(i));
		}

		// 重複点の削除
		CPoint2D* p1, * p2;
		double dX, dY;
		for (int iCorner = 0; iCorner < polyCommon.size(); iCorner++)
		{
			p1 = &(polyCommon.GetAtExt(iCorner));
			p2 = &(polyCommon.GetAtExt(iCorner + 1));
			dX = p2->x - p1->x;
			dY = p2->y - p1->y;
			if (CEpsUtil::Zero(dX) && CEpsUtil::Zero(dY))
			{
				polyCommon.erase(polyCommon.begin() + iCorner);
				iCorner--;
			}
		}
	}

	if (polyCommon.size() > 2)
		return true;
	else
		return false;
}


/*!	凹ポリゴンを分割して凸ポリゴン列を得る
@retval	true	入力ポリゴンが凹ポリゴン
@retval false	入力ポリゴンが凸ポリゴン
@note	凹ポリゴンを分割して凸ポリゴン列を得る。
<br>	入力が凸ポリゴンの場合は、入力ポリゴン１つを返す
@note	凹頂点を通り、Y軸に平行な分割線でポリゴンを分割する。
*/
bool CPoint2DPolygon::GetConvexPolygons(
	vector<CPoint2DPolygon>& cvxPolys,		//!< out	分割ポリゴン列
	const bool bBL /*= false*/				//!< in		緯度経度かどうか（日付変更線考慮有無）
)
{
	cvxPolys.clear();

	if (this->size() < 3)
	{
		cvxPolys.push_back(*this);
		return false;
	}

	//	凹頂点のインデックスの配列
	vector<int>	aryNonConvexIndex;

	// 作業用ポリゴン
	CPoint2DPolygon polyTmp = *this;

	// 時計回りに統一
	if (!polyTmp.Clockwise(bBL))
	{
		polyTmp.Reverse();
	}

	// 分割線のX座標列
	vector<double>	aryDivX;

	// 凹頂点のX座標列を得る
	for (int i = 0; i < polyTmp.size(); i++)
	{
		if (!polyTmp.IsConvexAngle(i, true, bBL))
		{
			double dtmp = polyTmp.at(i).x;
			aryDivX.push_back(dtmp);
		}
	}

	// 凹頂点がない（凸多角形）のとき、falseを返す
	if (aryDivX.size() == 0)
	{
		cvxPolys.push_back(*this);
		return false;
	}

	//　頂点の最大X・最小Xを求め分割線のX座標列に追加する。
	CPoint2DPolygon polyTmp2 = *this;
	polyTmp2.SortX(true);
	double dtmp = polyTmp2.at(polyTmp2.size() - 1).x;
	aryDivX.push_back(dtmp);
	dtmp = polyTmp2.at(0).x;
	aryDivX.push_back(dtmp);

	//　分割線のX座標列のソート
	qsort(aryDivX.data(), aryDivX.size(), sizeof(double), _compareDoubleAscending);

	// 重複するX座標をリストから削除する。
	for (int i = 1; i < aryDivX.size(); i++)
	{
		if (CEpsUtil::Equal(aryDivX[i - 1], aryDivX[i]))
		{
			aryDivX.erase(aryDivX.begin() + i);
			i--;
		}
	}

	//	分割線上の点のインデックスの初期化
	vector<vector<_C_Y_AND_INDEX>> aryIndexOnDiv;
	aryIndexOnDiv.resize(aryDivX.size());
	for (int i = 0; i < aryIndexOnDiv.size(); i++)
	{
		aryIndexOnDiv.at(i).clear();
	}

	// ポリゴンと分割線の交点をポリゴンに追加する。
	{
		double dx, dy, dx1, dx2;
		// ここは、ポインタにすると、配列に点を挿入したときにおかしくなるのであえてインスタンスをテンポラリに作る。
		// p1,p2をポインタにしないこと。
		CPoint2D p1, p2;
		/// iCorner番目の点と(iCorner+1)番目の点のなす線分について
		for (int iCorner = 0; iCorner < polyTmp.size(); iCorner++)
		{
			p1 = polyTmp.GetAtExt(iCorner);
			p2 = polyTmp.GetAtExt(iCorner + 1);
			dx = p2.x - p1.x;
			dy = p2.y - p1.y;

			if (!CEpsUtil::Zero(dx))
			{
				int jDivStart, jDivEnd, jdDiv;
				// 両端は最大・最小のX座標のはずなので、交点はない
				// XがiCorner番目の点と(iCorner+1)番目の点で増えているとき、aryDivXを昇順に探索する
				if (CEpsUtil::Greater(dx, 0.0))
				{
					jDivStart = 1;
					jDivEnd = (int)aryDivX.size() - 1;
					jdDiv = 1;
				}
				// XがiCorner番目の点と(iCorner+1)番目の点で減っているとき、aryDivXを降順に探索する
				else
				{
					jDivStart = (int)aryDivX.size() - 2;
					jDivEnd = 0;
					jdDiv = -1;
				}

				// 分割と線分の交点をpolyTmpに追加し、そのIndexとY座標をaryIndexOnDivに記録する。
				for (int jDiv = jDivStart; jDiv != jDivEnd; jDiv += jdDiv)
				{
					dx1 = aryDivX[jDiv] - p1.x;
					dx2 = p2.x - aryDivX[jDiv];

					if (CEpsUtil::Less(dx1 * jdDiv, 0.0))
						continue;

					if (CEpsUtil::Greater(dx1 * dx2, 0.0))
					{
						double x = aryDivX[jDiv];
						double y = p1.y + dy * dx1 / dx;
						CPoint2D postmp;
						postmp.x = x; postmp.y = y;

						auto it = polyTmp.begin() + iCorner + 1;
						polyTmp.insert(it, postmp);
						iCorner++;
					}
				}

			}
		}
	}

	// ポリゴン上の重複点の削除
	{
		CPoint2D* p1; CPoint2D* p2;
		double dx, dy;
		for (int iCorner = 0; iCorner < polyTmp.size(); iCorner++)
		{
			p1 = &(polyTmp.GetAtExt(iCorner));
			p2 = &(polyTmp.GetAtExt(iCorner + 1));
			dx = p2->x - p1->x;
			dy = p2->y - p1->y;
			if (CEpsUtil::Zero(dx) && CEpsUtil::Zero(dy))
			{
				polyTmp.RemoveAt(iCorner);
				iCorner--;
			}
		}
	}


	//	分割線上の点のインデックスに登録する
	{
		for (int iCorner = 0; iCorner < polyTmp.size(); iCorner++)
		{
			CPoint2D* p = &(polyTmp.GetAtExt(iCorner));
			for (int jDiv = 0; jDiv != aryDivX.size(); jDiv++)
			{
				if (CEpsUtil::Equal(aryDivX.at(jDiv), p->x))
				{
					_C_Y_AND_INDEX _y_and_index;
					_y_and_index.m_dY = p->y;
					_y_and_index.m_iIndex = iCorner;

					aryIndexOnDiv.at(jDiv).push_back(_y_and_index);
				}
			}
		}
	}


	// 分割線上の点のインデックスを、Yにたいして降順にソートする。
	for (int iDiv = 0; iDiv < aryIndexOnDiv.size(); iDiv++)
	{
		qsort(aryIndexOnDiv.at(iDiv).data(), aryIndexOnDiv.at(iDiv).size(), sizeof(_C_Y_AND_INDEX), _compareY_AND_INDEXDescending);
	}

	// ポリゴン上の線の出力ログ
	vector<int> arySegOutputLog;
	arySegOutputLog.resize(polyTmp.size());
	for (int i = 0; i < arySegOutputLog.size(); i++)
		arySegOutputLog[i] = 0;

	// Xが小さいほうからポリゴン化する。
	// 
	// 次の3つのタイプのポリゴンがある。
	// (1) タイプ１：2つの分割線ではさまれるポリゴン
	// |---|
	// |   |
	// |---|
	//
	// (2) タイプ２：左側の分割線から出発し、右側の分割線に到達する前に左側に戻るポリゴン
	// |-- |
	// | | |
	// |-- |
	//
	// (3) タイプ３：右側の分割線から出発し、左側の分割線に到達する前に右側に戻るポリゴン
	// | --|
	// | | |
	// | --|

	// まずは、左側の分割線から出発して戻るようなトレースをして、タイプ１・タイプ２を出力する。

	bool hasError = false;
	for (int iDiv = 0; iDiv < aryIndexOnDiv.size() - 1; iDiv++)
	{
		// 出力用ポリゴン
		CPoint2DPolygon polyDiv;
		// 左分割線上のid
		int iDivSubLeft;
		// 右分割線上のid
		int iDivSubRight;

		CPoint2D* pStart = NULL;
		CPoint2D* p = NULL;
		CPoint2D* pRight = NULL;
		CPoint2D* pRight2 = NULL;

		double dLastY = DBL_MAX;

		// 左側の分割線上のインデックス
		vector< _C_Y_AND_INDEX>* pLeftIndex = &(aryIndexOnDiv.at(iDiv));
		// 右側の分割線上のインデックス
		vector< _C_Y_AND_INDEX>* pRightIndex = &(aryIndexOnDiv.at(iDiv + 1));

		for (iDivSubLeft = 0; iDivSubLeft < pLeftIndex->size(); iDivSubLeft++)
		{
			// 出力用ポリゴンの初期化
			polyDiv.clear();
			bool bFailed = false;

			int iStartIndex = pLeftIndex->at(iDivSubLeft).m_iIndex;
			int iIndex = iStartIndex;
			// 左上から方向に探索
			pStart = &(polyTmp.GetAtExt(iStartIndex));
			p = &(polyTmp.GetAtExt(iStartIndex + 1));
			// ポリゴンのX座標が減るときは、次の候補に移る。
			if (CEpsUtil::LessEqual(p->x, pStart->x))
			{
				continue;
			}

			//　最初の点を追加
			polyDiv.Add(*pStart);
			bool bLeft = false;
			bool bRight = false;

			// 左側から右側へ探索（右側に到達しないで戻ってくる場合もある(タイプ２)）
			while (1)
			{
				iIndex++;
				iIndex = iIndex % (int)polyTmp.size();
				p = &(polyTmp.GetAtExt(iIndex));

				// 右側に到達しないで左側に戻ってきたとき（タイプ２）
				if (CEpsUtil::Equal(p->x, aryDivX[iDiv])) // 左側の分割線上に来た場合終了
				{
					// 開始点と同じでなければ点を追加
					if (p->y != pStart->y)
					{
						polyDiv.Add(*p);
						arySegOutputLog[(iIndex + arySegOutputLog.size()) % arySegOutputLog.size()] = 1;
					}
					bLeft = true;
					break;
				}
				// 右側の分割線上に来た場合
				else if (CEpsUtil::Equal(p->x, aryDivX[iDiv + 1]))
				{
					polyDiv.Add(*p);
					arySegOutputLog[(iIndex + arySegOutputLog.size()) % arySegOutputLog.size()] = 1;
					pRight = p;
					iDivSubRight = -1;
					bRight = true;
					for (int i = 0; i < pRightIndex->size(); i++)
					{
						if (iIndex == pRightIndex->at(i).m_iIndex)
						{
							iDivSubRight = i;
							break;
						}
					}
					if (iDivSubRight == -1)
					{
						bFailed = true;
						hasError = true;
					}
					break;
				}
				// それ以外は単純に追加
				else
				{
					polyDiv.Add(*p);
					arySegOutputLog[(iIndex + arySegOutputLog.size()) % arySegOutputLog.size()] = 1;
				}
			}

			if (bFailed)
				continue;

			// 左側に戻っている場合は、ポリゴン列を登録し、次のポリゴンの探索を開始する。
			if (bLeft)
			{
				cvxPolys.push_back(polyDiv);
				continue;
			}
			// 右側に到達した場合
			else if (bRight)
			{
				// 右側→左側探索会始点(pRight2)の探索。
				// Xが減る方向にポリゴン点座標が移動する場合、そのまま探索を続ける
				while (1)
				{
					pRight2 = &polyTmp.GetAtExt(pRightIndex->at(iDivSubRight).m_iIndex);
					p = &polyTmp.GetAtExt(pRightIndex->at(iDivSubRight).m_iIndex + 1);

					// 右側に到達直後の点が凹頂点のときは、下の点に移動する。
					if (pRight2 == pRight && !polyTmp.IsConvexAngle(pRightIndex->at(iDivSubRight).m_iIndex, true, bBL))
					{
						iDivSubRight++;
						if (iDivSubRight >= pRightIndex->size())
						{
							bFailed = true;
							hasError = true;
							break;
						}
					}

					else if (CEpsUtil::Less(p->x, pRight2->x))
					{
						// 右側到達点(pRight)と右側→左側探索会始点(pRight2)が異なる場合は、pRight2を追加する
						// ここでは、右側の分割線がポリゴンを形成するので、arySegOutputLogには出力を記録しない。
						if (pRight2 != pRight)
							polyDiv.Add(*pRight2);
						iIndex = pRightIndex->at(iDivSubRight).m_iIndex;
						break;
					}
					else
					{
						iDivSubRight++;
						if (iDivSubRight >= pRightIndex->size())
						{
							bFailed = true;
							hasError = true;
							break;
						}
					}
				}
			}
			else
			{
				hasError = true;
				continue;
			}

			// 右側から左側へ探索
			while (1)
			{
				iIndex++;
				iIndex = iIndex % (int)polyTmp.size();
				p = &(polyTmp.GetAtExt(iIndex));

				// 右側に到達しないで左側に戻ってきたとき
				if (CEpsUtil::Equal(p->x, aryDivX[iDiv])) // 左側の分割線上に来た場合終了
				{
					// 開始点と同じでなければ点を追加
					if (!CEpsUtil::Equal(p->y, pStart->y))
					{
						polyDiv.Add(*p);
						arySegOutputLog[(iIndex + arySegOutputLog.size()) % arySegOutputLog.size()] = 1;
					}
					bLeft = true;
					break;
				}
				// それ以外は単純に追加
				else
				{
					polyDiv.Add(*p);
					arySegOutputLog[(iIndex + arySegOutputLog.size()) % arySegOutputLog.size()] = 1;
				}
			}

			// 左側に戻っている場合は、ポリゴン列を登録し、次のポリゴンの探索を開始する。
			if (bLeft)
			{
				cvxPolys.push_back(polyDiv);
				continue;
			}
			else
			{
				hasError = true;
				continue;
			}
		}
	}

	// タイプ３のポリゴンを探索する。

	// 分割線上の点のインデックスを、Yに対して昇順にソートする。
	for (int iDiv = 0; iDiv < aryIndexOnDiv.size(); iDiv++)
	{
		qsort(aryIndexOnDiv.at(iDiv).data(), aryIndexOnDiv.at(iDiv).size(), sizeof(_C_Y_AND_INDEX), _compareY_AND_INDEXAscending);
	}

	// iDiv = 1, すなわち一番左端は無視する。
	for (int iDiv = 1; iDiv < aryIndexOnDiv.size() - 1; iDiv++)
	{
		// 出力用ポリゴン
		CPoint2DPolygon polyDiv;
		// 右分割線上のid
		int iDivSubRight;
		CPoint2D* pStart, *p;
		double dLastY = DBL_MAX;

		// 右側の分割線上のインデックス
		vector<_C_Y_AND_INDEX>* pRightIndex = &(aryIndexOnDiv.at(iDiv));

		for (iDivSubRight = 0; iDivSubRight < pRightIndex->size(); iDivSubRight++)
		{
			bool bFailed = false;
			// 出力用ポリゴンの初期化
			polyDiv.clear();

			int iStartIndex = pRightIndex->at(iDivSubRight).m_iIndex;
			int iIndex = iStartIndex;

			// 分割線上の点からその次の点にいたる線分が既に探索済みのときは、次に移る。
			if (arySegOutputLog.at((iStartIndex + 1 + arySegOutputLog.size()) % arySegOutputLog.size()))
				continue;

			pStart = &(polyTmp.GetAtExt(iStartIndex));
			p = &(polyTmp.GetAtExt(iStartIndex + 1));

			// ポリゴンのX座標が増えるときは、次の候補に移る。
			if (CEpsUtil::GreaterEqual(p->x, pStart->x))
			{
				continue;
			}

			//　最初の点を追加
			polyDiv.Add(*pStart);
			bool bLeft = false;
			bool bRight = false;

			// タイプ３の探索
			while (1)
			{
				iIndex++;
				iIndex = iIndex % (int)polyTmp.size();
				p = &(polyTmp.GetAtExt(iIndex));

				// 左側に到達しないで右側に戻ってきたとき（タイプ３）
				if (CEpsUtil::Equal(p->x, aryDivX[iDiv])) // 右側の分割線上に来た場合終了
				{
					// 開始点と同じでなければ点を追加
					if (!CEpsUtil::Equal(p->y, pStart->y))
					{
						polyDiv.Add(*p);
						arySegOutputLog[(iIndex + arySegOutputLog.size()) % arySegOutputLog.size()] = 1;
					}
					bRight = true;
					break;
				}
				// 左側の分割線上に来た場合はエラー
				else if (CEpsUtil::Equal(p->x, aryDivX[iDiv - 1]))
				{
					bLeft = true;
					break;
				}
				// それ以外は単純に追加
				else
				{
					polyDiv.Add(*p);
					arySegOutputLog[(iIndex + arySegOutputLog.size()) % arySegOutputLog.size()] = 1;
				}
			}

			// 右側に戻っている場合は、ポリゴン列を登録し、次のポリゴンの探索を開始する。
			if (bRight)
			{
				cvxPolys.push_back(polyDiv);
				continue;
			}
			else if (bLeft)
			{
				continue;
			}
		}
	}
	if (hasError)
	{
		return false;
	}
	return true;
}
