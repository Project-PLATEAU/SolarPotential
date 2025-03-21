#pragma once
#include <math.h>
#include <vector>
#include <cassert>
#include <string>
#include "CGeoUtil.h"
#include "CEpsUtil.h"

/*!
@brief	�Q�������_��N���X
@note	�����E�E��̋�`����ۑ�����B
*/
// 
typedef std::vector<CPoint2D> _CPoint2DArray;
class CPoint2DPolygon;
class CPoint2DArray : public _CPoint2DArray
{
	friend CPoint2DPolygon;
	double m_dMin[2];		//!<	���W�̍ŏ��l
	double m_dMax[2];		//!<	���W�̍ő�l
public:
	double GetMaxX() const { return m_dMax[0]; }
	double GetMaxY() const { return m_dMax[1]; }
	double GetMinX() const { return m_dMin[0]; }
	double GetMinY() const { return m_dMin[1]; }

	/*! �ő�E�ŏ��̍X�V
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
	/*! �ő�E�ŏ��̋�`�̓���
	@retval	true	����
	@retval	false	�O��
	@note	���E�l��͓���
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
	/*! �_��̔��]
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

// 2D�|���S��
// �Ō�̓_����ŏ��̓_�ɖ߂�Ƃ���B�d���i�[���Ȃ��B
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

	// �ʃ|���S�����m�̌����|���S���𓾂�
	bool GetCrossingPolygon(CPoint2DPolygon& polygon, CPoint2DPolygon& polyCommon, const bool bBL = false);

	// �ʃ|���S������擾
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


	/*! ���t�ύX���l��
	@note	���t�ύX�����l������CPoint2DPolygon�𐶐����܂��B
	*/
	void	Normal2dPolygon(CPoint2DPolygon* pPolygon2d)
	{
		// ���t�ύX�����l��
		// �q����2���_��Lon�l��K��180�x�ȓ��Ɏ��߂�
		double				NowLon = 0.0;
		double				dOldLon = 0.0;
		for (int i = 0; i < this->size(); i++)
		{
			CPoint2D		pos;
			double	dLat = this->at(i).y;
			double	dLon = this->at(i).x;
			if (i == 0)
			{
				// �擪
				dOldLon = NowLon = dLon;
			}
			else
			{
				// �擪�ȊO
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

	/*! ���v���i�E����W�n�̏ꍇ�j
	@retval	true	���v���
	@retval	false	�����v���
	@note	�ʐς��v�Z���A�{�̂Ƃ��͎��v���A�|�̂Ƃ��͔����v���Ɣ��肷��B
	@note	��ɒP���|���S�����邱�Ƃ��`�F�b�N���邱�ƁB
	*/
	bool Clockwise(const bool bBL = false			//!<	in	�ܓx�o�x���ǂ����i���t�ύX���l���L���j
	)
	{
		double dArea = 0.0;

		if (bBL)
		{
			// ���t�ύX���l�����_
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

	/*! �ʃ|���S��
	@retval	true	�ʃ|���S��(Convex)
	@retval	false	���|���S��(Non-Convex)
	@note	��ɒP���|���S���ł��邱�Ƃ��`�F�b�N���邱�ƁB
	*/
	bool IsConvexPolygon(const bool bBL = false			//!<	in	�ܓx�o�x���ǂ����i���t�ύX���l���L���j
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

	///*! �P���|���S��
	//@note	�P���|���S���i�ӓ��m���������Ȃ��j���ǂ����̔���
	//@retval	true	�P���|���S��(Simple)
	//@retval	false	���G�|���S��(Complex)
	//*/
	//bool IsSimplePolygon();

	/*! �ʊp
	@retval	true	�ʃ|���S��(Convex)
	@retval	false	���|���S��(Non-Convex)
	@note			180�x�͓ʂƂ���
	@note			�P���|���S���łȂ��ꍇ�͓���͕ۏ؂���Ȃ�
	*/
	bool IsConvexAngle(
		const int iIndex,				//!<	in	���_�̃C���f�b�N�X
		const bool bClockwise = true,	//!<	in	���v��肩�ǂ���
		const bool bBL = false			//!<	in	�ܓx�o�x���ǂ����i���t�ύX���l���L���j
	)
	{
		bool bRet;

		if (bBL)
		{
			// ���t�ύX���l�����_
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

	/*! �w�肷��C���f�b�N�X�̓_
	@return	�w�肷��C���f�b�N�X�̓_
	@note	-1�̂Ƃ��A�Ō�̓_�AGetCount() �̂Ƃ��A�ŏ��̓_��Ԃ�
	*/
	CPoint2D& GetAtExt(int iIndex)
	{
		return this->at((iIndex + this->size()) % this->size());
	}

};