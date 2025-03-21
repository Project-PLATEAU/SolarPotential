#include "pch.h"
#include "CGeoUtil.h"
#include "CPoint2DPolygon.h"
#include <cmath>
#include <random>

// �ȉ��AGetCrossingPolygon�Ŏg�p���邽�߂̃e���|�����N���X�B
// ��`�����Ɏg���e���|�����N���X�B
// �|���S���̍\���_�����Ƃ̑��p�`�̃|���S���̒��_(0)���A��_(1)���A�������͕������ŋ����I�ɍ쐬���ꂽ�_(2)�����L�^����z��
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
// ��_�Ǘ��p�̃e���|�����_�N���X
class __CPoint2D : public CPoint2D
{
public:
	int iIndex[2];
};

// Y���W�ƃC���f�b�N�X���i�[���邽�߂̍\����
class  _C_Y_AND_INDEX
{
public:
	double m_dY;
	int m_iIndex;
};

// vector<double> �\�[�g�p�̊֐�
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

// CAASVec2dArray��qsort�p�֐��Q
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

/*! Y���W�ɂ��\�[�g(�~���p)
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
/*! Y���W�ɂ��\�[�g(�~���p)
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

/*! X���W�ɂ��\�[�g
*/
void CPoint2DArray::SortX(bool bAscending)
{
	if (bAscending)
		qsort(this->data(), this->size(), sizeof(CVector2D), _compareXAscending);
	else
		qsort(this->data(), this->size(), sizeof(CVector2D), _compareXDescending);
}

/*! Y���W�ɂ��\�[�g
*/
void CPoint2DArray::SortY(bool bAscending)
{
	if (bAscending)
		qsort(this->data(), this->size(), sizeof(CVector2D), _compareYAscending);
	else
		qsort(this->data(), this->size(), sizeof(CVector2D), _compareYDescending);
}

/*! �������n�_�Ƃ��ĕ��ёւ�(�n�_�ƏI�_�͈�v���Ȃ�)
*/
void CPoint2DArray::StartLeft()
{
	CPoint2DArray polyTmp = *this;

	// �n�_�ƂȂ�C���f�b�N�X���擾
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
	if (nStartIndex == 0) return;	// �������n�_�ɂȂ��Ă���ΏI��

	// ���̔z��̊J�n�ʒu����Ō�܂�
	int addIndex = 0;
	for (int i = nStartIndex; i < polyTmp.size(); i++)
	{
		this->at(addIndex) = polyTmp.at(i);
		addIndex++;
	}

	// ���̔z��̐擪����J�n�ʒu�̂ЂƂO�܂�
	CPoint2D tmp;
	for (int i = 0; i < nStartIndex; i++)
	{
		// �Ō�ɒǉ������_�Ǝn�_�������ꍇ�̓X�L�b�v
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

/*!	�ʃ|���S�����m�̌����|���S���𓾂�
@retval	true	��������
@retval false	�������Ȃ�
@note	�ʃ|���S�����m�̌����|���S���𓾂�
<br>	���͂��ʃ|���S���łȂ��Ƃ��A����͕ۏ؂���Ȃ��B
*/
bool CPoint2DPolygon::GetCrossingPolygon(
	CPoint2DPolygon& polygon,		//!<	in	�|���S��
	CPoint2DPolygon& polyCommon,	//!<	out	���p�`
	const bool bBL /*= false*/		//!<	in	�ܓx�o�x���ǂ����i���t�ύX���l���L���j
)
{
	//�o�̓|���S���̏�����
	polyCommon.clear();

	if (CEpsUtil::Equal(GetMinX(), DBL_MAX))
	{
		RenewMinMax();
	}
	if (CEpsUtil::Equal(polygon.GetMinX(), DBL_MAX))
	{
		polygon.RenewMinMax();
	}

	// �ő��`�̊O���Ȃ�false
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

	// y=aryX[i]��y=aryX[i+1]�̂Q�������̑�`���쐬���A���̋��ʕ�����o�^����B


	std::vector<__CPoint2DArray> aryTrapezoid0Upper, aryTrapezoid0Lower, aryTrapezoid1Upper, aryTrapezoid1Lower;
	std::vector<__CPoint2DArray>* pAryTrapezoid[2][2];
	pAryTrapezoid[0][0] = &aryTrapezoid0Upper;
	pAryTrapezoid[0][1] = &aryTrapezoid0Lower;
	pAryTrapezoid[1][0] = &aryTrapezoid1Upper;
	pAryTrapezoid[1][1] = &aryTrapezoid1Lower;

	//�@�����̃|���S���̍�Ɨp�|���S��
	CPoint2DPolygon polyTmp[2];
	polyTmp[0] = *this;
	polyTmp[1] = polygon;

	int iIndexUpper = 0;
	int iIndexLower = 1;

	// �|���S���ɑ΂���O����
		// �|���S���̏㑤�̃C���f�b�N�X�Ɖ����̃C���f�b�N�X
	int iIndexLeft[2][2];

	for (int iPoly = 0; iPoly < 2; iPoly++)
	{
		// ���v���ɓ���
		if (!polyTmp[iPoly].Clockwise(bBL))
			polyTmp[iPoly].Reverse();

		// Y���W�������_���R�_�ȏ㑱���ꍇ�A��菜���Ă����B�܂��A�d���_����菜���Ă����B
		for (int jCorner = 0; jCorner < polyTmp[iPoly].size(); jCorner++)
		{
			// 1�_�ڂ͎�菜���Ȃ�
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

		// X���W���ŏ�����Y���W���ő�̃C���f�b�N�X��iIndexUpper�ɁAX���W���ŏ�����Y���W���ŏ��̃C���f�b�N�X��iIndexLower�Ɋi�[����
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

	// �����̃|���S����X���W��𓾂�B
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

	// �����ɕ��ׂ�
	qsort(aryX.data(), aryX.size(), sizeof(double), _compareDoubleAscending);

	// �d�����W�����
	for (int i = 1; i < aryX.size(); i++)
	{
		if (CEpsUtil::Equal(aryX.at(i), aryX.at(i - 1)))
		{
			aryX.erase(aryX.begin() + i);
			i--;
		}
	}



	// ��`�����̒��o
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
		// �����ƉE����X���W
		double dXLeft, dXRight;
		dXLeft = aryX[iDivX];
		dXRight = aryX[iDivX + 1];

		// ��`�����邩�ǂ����̔���
		{
			bool bBreak = false;
			for (int iPoly = 0; iPoly < 2; iPoly++)
			{
				pPoly = &polyTmp[iPoly];
				pPos = &(pPoly->GetAtExt(iIndexLeft[iPoly][iIndexUpper]));
				pPosNext = &(pPoly->GetAtExt(iIndexLeft[iPoly][iIndexUpper] + 1));

				// �؂�o���͈͂����݂̓_�̍����̂Ƃ��́A�X�L�b�v
				if (CEpsUtil::Greater(pPos->x, dXLeft))
				{
					continue;
				}
				// ���W���߂�Ƃ��͏I���
				if (CEpsUtil::GreaterEqual(pPos->x, pPosNext->x))
				{
					bBreak = true;
					break;
				}
				// ���ׂĂ̒��_��X���W��aryX�ɓo�^���Ă���̂ŁA����͂��肦�Ȃ��͂��I
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

			//��̃|���S����ǉ�
			__CPoint2DArray poly;
			pAryTrapezoid[iPoly][iUpper]->push_back(poly);
			pAryTrapezoid[iPoly][iLower]->push_back(poly);



			// iUL = 0: �㑤
			// iUL = 1: ����

			for (int iUL = 0; iUL < 2; iUL++)
			{
				__CPoint2DArray* pTrap = &(pAryTrapezoid[iPoly][iUL]->at(iDivX));
				// �_�̑���
				std::vector<int>* pAtt = &(pTrap->m_aryAtt);

				// �E���̓_�ւ̃C���f�b�N�X�̃C���N�������g
				int iInc = 1;
				// ���̂Ƃ��̓C���f�b�N�X��߂�
				if (iUL == 1)
					iInc = -1;

				int* pIndex = &iIndexLeft[iPoly][iUL];

				// ����
				pPos = &(pPoly->GetAtExt(*pIndex));
				pPosNext = &(pPoly->GetAtExt(*pIndex + iInc));

				// ���̓_
				if (CEpsUtil::Greater(pPos->x, dXLeft))
				{
					continue;
				}
				else if (CEpsUtil::Equal(pPos->x, dXLeft))
				{
					pos[iUL][iLeft] = *pPos;
					// �|���S���̒��_(0)
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
					// �������ŋ����I�ɍ쐬���ꂽ�_(2)
					iAtt[iUL][iLeft] = 2;
				}
				// ����͂��肦�Ȃ��͂�
				else
				{
					assert(FALSE);
					return false;
				}

				// �E�̓_
				if (CEpsUtil::Equal(pPosNext->x, dXRight))
				{
					pos[iUL][iRight] = *pPosNext;
					(*pIndex) += iInc;
					// �|���S���̒��_(0)
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
					// �������ŋ����I�ɍ쐬���ꂽ�_(2)
					iAtt[iUL][iRight] = 2;
				}
				// ��`�̕ӂ̓o�^
				pTrap->Add(pos[iUL][iLeft]);
				pTrap->Add(pos[iUL][iRight]);
				pAtt->push_back(iAtt[iUL][iLeft]);
				pAtt->push_back(iAtt[iUL][iRight]);
			}
		}

		// ��`���m�Ɍ�_������ꍇ�͂���ɕ���

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

		// p1 ����Ɓ@p2�@����̌����ʒu
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
					// ��_���\��������̃C���f�b�N�X���L�^
					postmp.iIndex[iPoly0] = iUL0;
					postmp.iIndex[iPoly1] = iUL1;
					aryPosTmp.push_back(postmp);
				}
			}

		// ��_���Ȃ��ꍇ�͎��ɂ���
		if (aryPosTmp.size() == 0)
			continue;

		// ��_������ꍇ�AaryXTmp���\�[�g
		qsort(aryPosTmp.data(), aryPosTmp.size(), sizeof(__CPoint2D), _compareXAscending);

		// ��_��X���W�ő�`�𕪊�
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


	// ��`�̋��ʕ����̎擾

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
				// �[�ł�Y���W�̍������߂�
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

				// ���L�̂Ȃ��P�[�X
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
				// iPoly0 �̏オiPoly1�̏��艺
				if (CEpsUtil::GreaterEqual(dy[iLeft][iUpper][iUpper], 0.0) && CEpsUtil::GreaterEqual(dy[iRight][iUpper][iUpper], 0.0))
				{
					// iPoly0 �̏オiPoly1�̉�����
					if (CEpsUtil::LessEqual(dy[iLeft][iUpper][iLower], 0.0) && CEpsUtil::LessEqual(dy[iRight][iUpper][iLower], 0.0))
					{
						iUp = iPoly0;
					}
				}
				if (iUp == -1)
				{
					// iPoly1 �̏オiPoly0�̏��艺
					if (CEpsUtil::LessEqual(dy[iLeft][iUpper][iUpper], 0.0) && CEpsUtil::LessEqual(dy[iRight][iUpper][iUpper], 0.0))
					{
						// iPoly1 �̏オiPoly0�̏��艺
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

				// iPoly0 �̉���iPoly1�̏��艺
				if (CEpsUtil::GreaterEqual(dy[iLeft][iLower][iUpper], 0.0) && CEpsUtil::GreaterEqual(dy[iRight][iLower][iUpper], 0.0))
				{
					// iPoly0 �̉���iPoly1�̉�����
					if (CEpsUtil::LessEqual(dy[iLeft][iLower][iLower], 0.0) && CEpsUtil::LessEqual(dy[iRight][iLower][iLower], 0.0))
					{
						iLow = iPoly0;
					}
				}
				if (iLow == -1)
				{
					// iPoly1 �̉���iPoly0�̏��艺
					if (CEpsUtil::LessEqual(dy[iLeft][iUpper][iLower], 0.0) && CEpsUtil::LessEqual(dy[iRight][iUpper][iLower], 0.0))
					{
						// iPoly1 �̉���iPoly0�̏��艺
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
						// �ŏ��̓_�������_�̏ꍇ�A�����I�Ɍ�_(1)�ɂ���
						if (aryUpper.m_aryAtt.at(aryUpper.m_aryAtt.size() - 1) == 2)
							aryUpper.m_aryAtt.at(aryUpper.m_aryAtt.size() - 1) = iAttCross;

						aryLower.Add(ppoly[iLow][iLower]->at(iDivSub));
						aryLower.m_aryAtt.push_back(ppoly[iLow][iLower]->m_aryAtt.at(iDivSub));

						// �ŏ��̓_�������_�̏ꍇ�A�����I�Ɍ�_(1)�ɂ���
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

						// �O��iUp���ς��Ƃ��͌�_�i�����_���O�̒i�K�œ���ꍇ������̂ŁA�����I�Ɍ�_(1)�ɂ���j
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
					// �_��I��
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

		// �Ō�̓_�������_�̏ꍇ�A�����I�Ɍ�_(1)�ɂ���
		if (aryUpper.m_aryAtt.at(aryUpper.m_aryAtt.size() - 1) == 2)
			aryUpper.m_aryAtt.at(aryUpper.m_aryAtt.size() - 1) = iAttCross;

		// �Ō�̓_�������_�̏ꍇ�A�����I�Ɍ�_(1)�ɂ���
		if (aryLower.m_aryAtt.at(aryLower.m_aryAtt.size() - 1) == 2)
			aryLower.m_aryAtt.at(aryLower.m_aryAtt.size() - 1) = iAttCross;


		// �|���S���̓���(��j
		for (int i = 0; i < (int)aryUpper.size(); i++)
		{
			// ���[�łȂ��A�����_�̂Ƃ��͂Ƃ΂�
			if (i != 0 && i != (int)aryUpper.size() - 1)
			{
				if (aryUpper.m_aryAtt.at(i) == 2)
					continue;
			}
			polyCommon.Add((CPoint2D&)aryUpper.at(i));
		}
		// �|���S���̓����i���j
		for (int i = (int)aryLower.size() - 1; i >= 0; i--)
		{
			// ���[�łȂ��A�����_�̂Ƃ��͂Ƃ΂�
			if (i != 0 && i != (int)aryUpper.size() - 1)
			{
				if (aryLower.m_aryAtt.at(i) == 2)
					continue;
			}
			polyCommon.Add((CPoint2D&)aryLower.at(i));
		}

		// �d���_�̍폜
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


/*!	���|���S���𕪊����ēʃ|���S����𓾂�
@retval	true	���̓|���S�������|���S��
@retval false	���̓|���S�����ʃ|���S��
@note	���|���S���𕪊����ēʃ|���S����𓾂�B
<br>	���͂��ʃ|���S���̏ꍇ�́A���̓|���S���P��Ԃ�
@note	�����_��ʂ�AY���ɕ��s�ȕ������Ń|���S���𕪊�����B
*/
bool CPoint2DPolygon::GetConvexPolygons(
	vector<CPoint2DPolygon>& cvxPolys,		//!< out	�����|���S����
	const bool bBL /*= false*/				//!< in		�ܓx�o�x���ǂ����i���t�ύX���l���L���j
)
{
	cvxPolys.clear();

	if (this->size() < 3)
	{
		cvxPolys.push_back(*this);
		return false;
	}

	//	�����_�̃C���f�b�N�X�̔z��
	vector<int>	aryNonConvexIndex;

	// ��Ɨp�|���S��
	CPoint2DPolygon polyTmp = *this;

	// ���v���ɓ���
	if (!polyTmp.Clockwise(bBL))
	{
		polyTmp.Reverse();
	}

	// ��������X���W��
	vector<double>	aryDivX;

	// �����_��X���W��𓾂�
	for (int i = 0; i < polyTmp.size(); i++)
	{
		if (!polyTmp.IsConvexAngle(i, true, bBL))
		{
			double dtmp = polyTmp.at(i).x;
			aryDivX.push_back(dtmp);
		}
	}

	// �����_���Ȃ��i�ʑ��p�`�j�̂Ƃ��Afalse��Ԃ�
	if (aryDivX.size() == 0)
	{
		cvxPolys.push_back(*this);
		return false;
	}

	//�@���_�̍ő�X�E�ŏ�X�����ߕ�������X���W��ɒǉ�����B
	CPoint2DPolygon polyTmp2 = *this;
	polyTmp2.SortX(true);
	double dtmp = polyTmp2.at(polyTmp2.size() - 1).x;
	aryDivX.push_back(dtmp);
	dtmp = polyTmp2.at(0).x;
	aryDivX.push_back(dtmp);

	//�@��������X���W��̃\�[�g
	qsort(aryDivX.data(), aryDivX.size(), sizeof(double), _compareDoubleAscending);

	// �d������X���W�����X�g����폜����B
	for (int i = 1; i < aryDivX.size(); i++)
	{
		if (CEpsUtil::Equal(aryDivX[i - 1], aryDivX[i]))
		{
			aryDivX.erase(aryDivX.begin() + i);
			i--;
		}
	}

	//	��������̓_�̃C���f�b�N�X�̏�����
	vector<vector<_C_Y_AND_INDEX>> aryIndexOnDiv;
	aryIndexOnDiv.resize(aryDivX.size());
	for (int i = 0; i < aryIndexOnDiv.size(); i++)
	{
		aryIndexOnDiv.at(i).clear();
	}

	// �|���S���ƕ������̌�_���|���S���ɒǉ�����B
	{
		double dx, dy, dx1, dx2;
		// �����́A�|�C���^�ɂ���ƁA�z��ɓ_��}�������Ƃ��ɂ��������Ȃ�̂ł����ăC���X�^���X���e���|�����ɍ��B
		// p1,p2���|�C���^�ɂ��Ȃ����ƁB
		CPoint2D p1, p2;
		/// iCorner�Ԗڂ̓_��(iCorner+1)�Ԗڂ̓_�̂Ȃ������ɂ���
		for (int iCorner = 0; iCorner < polyTmp.size(); iCorner++)
		{
			p1 = polyTmp.GetAtExt(iCorner);
			p2 = polyTmp.GetAtExt(iCorner + 1);
			dx = p2.x - p1.x;
			dy = p2.y - p1.y;

			if (!CEpsUtil::Zero(dx))
			{
				int jDivStart, jDivEnd, jdDiv;
				// ���[�͍ő�E�ŏ���X���W�̂͂��Ȃ̂ŁA��_�͂Ȃ�
				// X��iCorner�Ԗڂ̓_��(iCorner+1)�Ԗڂ̓_�ő����Ă���Ƃ��AaryDivX�������ɒT������
				if (CEpsUtil::Greater(dx, 0.0))
				{
					jDivStart = 1;
					jDivEnd = (int)aryDivX.size() - 1;
					jdDiv = 1;
				}
				// X��iCorner�Ԗڂ̓_��(iCorner+1)�Ԗڂ̓_�Ō����Ă���Ƃ��AaryDivX���~���ɒT������
				else
				{
					jDivStart = (int)aryDivX.size() - 2;
					jDivEnd = 0;
					jdDiv = -1;
				}

				// �����Ɛ����̌�_��polyTmp�ɒǉ����A����Index��Y���W��aryIndexOnDiv�ɋL�^����B
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

	// �|���S����̏d���_�̍폜
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


	//	��������̓_�̃C���f�b�N�X�ɓo�^����
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


	// ��������̓_�̃C���f�b�N�X���AY�ɂ������č~���Ƀ\�[�g����B
	for (int iDiv = 0; iDiv < aryIndexOnDiv.size(); iDiv++)
	{
		qsort(aryIndexOnDiv.at(iDiv).data(), aryIndexOnDiv.at(iDiv).size(), sizeof(_C_Y_AND_INDEX), _compareY_AND_INDEXDescending);
	}

	// �|���S����̐��̏o�̓��O
	vector<int> arySegOutputLog;
	arySegOutputLog.resize(polyTmp.size());
	for (int i = 0; i < arySegOutputLog.size(); i++)
		arySegOutputLog[i] = 0;

	// X���������ق�����|���S��������B
	// 
	// ����3�̃^�C�v�̃|���S��������B
	// (1) �^�C�v�P�F2�̕������ł͂��܂��|���S��
	// |---|
	// |   |
	// |---|
	//
	// (2) �^�C�v�Q�F�����̕���������o�����A�E���̕������ɓ��B����O�ɍ����ɖ߂�|���S��
	// |-- |
	// | | |
	// |-- |
	//
	// (3) �^�C�v�R�F�E���̕���������o�����A�����̕������ɓ��B����O�ɉE���ɖ߂�|���S��
	// | --|
	// | | |
	// | --|

	// �܂��́A�����̕���������o�����Ė߂�悤�ȃg���[�X�����āA�^�C�v�P�E�^�C�v�Q���o�͂���B

	bool hasError = false;
	for (int iDiv = 0; iDiv < aryIndexOnDiv.size() - 1; iDiv++)
	{
		// �o�͗p�|���S��
		CPoint2DPolygon polyDiv;
		// �����������id
		int iDivSubLeft;
		// �E���������id
		int iDivSubRight;

		CPoint2D* pStart = NULL;
		CPoint2D* p = NULL;
		CPoint2D* pRight = NULL;
		CPoint2D* pRight2 = NULL;

		double dLastY = DBL_MAX;

		// �����̕�������̃C���f�b�N�X
		vector< _C_Y_AND_INDEX>* pLeftIndex = &(aryIndexOnDiv.at(iDiv));
		// �E���̕�������̃C���f�b�N�X
		vector< _C_Y_AND_INDEX>* pRightIndex = &(aryIndexOnDiv.at(iDiv + 1));

		for (iDivSubLeft = 0; iDivSubLeft < pLeftIndex->size(); iDivSubLeft++)
		{
			// �o�͗p�|���S���̏�����
			polyDiv.clear();
			bool bFailed = false;

			int iStartIndex = pLeftIndex->at(iDivSubLeft).m_iIndex;
			int iIndex = iStartIndex;
			// ���ォ������ɒT��
			pStart = &(polyTmp.GetAtExt(iStartIndex));
			p = &(polyTmp.GetAtExt(iStartIndex + 1));
			// �|���S����X���W������Ƃ��́A���̌��Ɉڂ�B
			if (CEpsUtil::LessEqual(p->x, pStart->x))
			{
				continue;
			}

			//�@�ŏ��̓_��ǉ�
			polyDiv.Add(*pStart);
			bool bLeft = false;
			bool bRight = false;

			// ��������E���֒T���i�E���ɓ��B���Ȃ��Ŗ߂��Ă���ꍇ������(�^�C�v�Q)�j
			while (1)
			{
				iIndex++;
				iIndex = iIndex % (int)polyTmp.size();
				p = &(polyTmp.GetAtExt(iIndex));

				// �E���ɓ��B���Ȃ��ō����ɖ߂��Ă����Ƃ��i�^�C�v�Q�j
				if (CEpsUtil::Equal(p->x, aryDivX[iDiv])) // �����̕�������ɗ����ꍇ�I��
				{
					// �J�n�_�Ɠ����łȂ���Γ_��ǉ�
					if (p->y != pStart->y)
					{
						polyDiv.Add(*p);
						arySegOutputLog[(iIndex + arySegOutputLog.size()) % arySegOutputLog.size()] = 1;
					}
					bLeft = true;
					break;
				}
				// �E���̕�������ɗ����ꍇ
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
				// ����ȊO�͒P���ɒǉ�
				else
				{
					polyDiv.Add(*p);
					arySegOutputLog[(iIndex + arySegOutputLog.size()) % arySegOutputLog.size()] = 1;
				}
			}

			if (bFailed)
				continue;

			// �����ɖ߂��Ă���ꍇ�́A�|���S�����o�^���A���̃|���S���̒T�����J�n����B
			if (bLeft)
			{
				cvxPolys.push_back(polyDiv);
				continue;
			}
			// �E���ɓ��B�����ꍇ
			else if (bRight)
			{
				// �E���������T����n�_(pRight2)�̒T���B
				// X����������Ƀ|���S���_���W���ړ�����ꍇ�A���̂܂ܒT���𑱂���
				while (1)
				{
					pRight2 = &polyTmp.GetAtExt(pRightIndex->at(iDivSubRight).m_iIndex);
					p = &polyTmp.GetAtExt(pRightIndex->at(iDivSubRight).m_iIndex + 1);

					// �E���ɓ��B����̓_�������_�̂Ƃ��́A���̓_�Ɉړ�����B
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
						// �E�����B�_(pRight)�ƉE���������T����n�_(pRight2)���قȂ�ꍇ�́ApRight2��ǉ�����
						// �����ł́A�E���̕��������|���S�����`������̂ŁAarySegOutputLog�ɂ͏o�͂��L�^���Ȃ��B
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

			// �E�����獶���֒T��
			while (1)
			{
				iIndex++;
				iIndex = iIndex % (int)polyTmp.size();
				p = &(polyTmp.GetAtExt(iIndex));

				// �E���ɓ��B���Ȃ��ō����ɖ߂��Ă����Ƃ�
				if (CEpsUtil::Equal(p->x, aryDivX[iDiv])) // �����̕�������ɗ����ꍇ�I��
				{
					// �J�n�_�Ɠ����łȂ���Γ_��ǉ�
					if (!CEpsUtil::Equal(p->y, pStart->y))
					{
						polyDiv.Add(*p);
						arySegOutputLog[(iIndex + arySegOutputLog.size()) % arySegOutputLog.size()] = 1;
					}
					bLeft = true;
					break;
				}
				// ����ȊO�͒P���ɒǉ�
				else
				{
					polyDiv.Add(*p);
					arySegOutputLog[(iIndex + arySegOutputLog.size()) % arySegOutputLog.size()] = 1;
				}
			}

			// �����ɖ߂��Ă���ꍇ�́A�|���S�����o�^���A���̃|���S���̒T�����J�n����B
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

	// �^�C�v�R�̃|���S����T������B

	// ��������̓_�̃C���f�b�N�X���AY�ɑ΂��ď����Ƀ\�[�g����B
	for (int iDiv = 0; iDiv < aryIndexOnDiv.size(); iDiv++)
	{
		qsort(aryIndexOnDiv.at(iDiv).data(), aryIndexOnDiv.at(iDiv).size(), sizeof(_C_Y_AND_INDEX), _compareY_AND_INDEXAscending);
	}

	// iDiv = 1, ���Ȃ킿��ԍ��[�͖�������B
	for (int iDiv = 1; iDiv < aryIndexOnDiv.size() - 1; iDiv++)
	{
		// �o�͗p�|���S��
		CPoint2DPolygon polyDiv;
		// �E���������id
		int iDivSubRight;
		CPoint2D* pStart, *p;
		double dLastY = DBL_MAX;

		// �E���̕�������̃C���f�b�N�X
		vector<_C_Y_AND_INDEX>* pRightIndex = &(aryIndexOnDiv.at(iDiv));

		for (iDivSubRight = 0; iDivSubRight < pRightIndex->size(); iDivSubRight++)
		{
			bool bFailed = false;
			// �o�͗p�|���S���̏�����
			polyDiv.clear();

			int iStartIndex = pRightIndex->at(iDivSubRight).m_iIndex;
			int iIndex = iStartIndex;

			// ��������̓_���炻�̎��̓_�ɂ�������������ɒT���ς݂̂Ƃ��́A���Ɉڂ�B
			if (arySegOutputLog.at((iStartIndex + 1 + arySegOutputLog.size()) % arySegOutputLog.size()))
				continue;

			pStart = &(polyTmp.GetAtExt(iStartIndex));
			p = &(polyTmp.GetAtExt(iStartIndex + 1));

			// �|���S����X���W��������Ƃ��́A���̌��Ɉڂ�B
			if (CEpsUtil::GreaterEqual(p->x, pStart->x))
			{
				continue;
			}

			//�@�ŏ��̓_��ǉ�
			polyDiv.Add(*pStart);
			bool bLeft = false;
			bool bRight = false;

			// �^�C�v�R�̒T��
			while (1)
			{
				iIndex++;
				iIndex = iIndex % (int)polyTmp.size();
				p = &(polyTmp.GetAtExt(iIndex));

				// �����ɓ��B���Ȃ��ŉE���ɖ߂��Ă����Ƃ��i�^�C�v�R�j
				if (CEpsUtil::Equal(p->x, aryDivX[iDiv])) // �E���̕�������ɗ����ꍇ�I��
				{
					// �J�n�_�Ɠ����łȂ���Γ_��ǉ�
					if (!CEpsUtil::Equal(p->y, pStart->y))
					{
						polyDiv.Add(*p);
						arySegOutputLog[(iIndex + arySegOutputLog.size()) % arySegOutputLog.size()] = 1;
					}
					bRight = true;
					break;
				}
				// �����̕�������ɗ����ꍇ�̓G���[
				else if (CEpsUtil::Equal(p->x, aryDivX[iDiv - 1]))
				{
					bLeft = true;
					break;
				}
				// ����ȊO�͒P���ɒǉ�
				else
				{
					polyDiv.Add(*p);
					arySegOutputLog[(iIndex + arySegOutputLog.size()) % arySegOutputLog.size()] = 1;
				}
			}

			// �E���ɖ߂��Ă���ꍇ�́A�|���S�����o�^���A���̃|���S���̒T�����J�n����B
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
