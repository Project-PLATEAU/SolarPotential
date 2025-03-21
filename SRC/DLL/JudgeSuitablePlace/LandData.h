#pragma once
#include <string>
#include <algorithm>
#include <iterator>
#include <vector>
#include "AggregateData.h"
#include "../../LIB/CommonUtil/CGeoUtil.h"

// ����
class CLand
{
public:
	std::string	m_strAreaId;			// �G���A(�y�n)ID
	std::vector<CPoint2D>* m_pSurface;	// �y�n��

	double dLandHeight;					// ��������
	double dSolorRadiation;				// �\�����˗�
	double dFloodDepth;					// �^���Z���z��̐Z���[(���[�g��) 
										// �����̐�̍^���Z���z��̃^�O�����ꍇ�͐[�����̐Z���[���󂯎��
	double dTsunamiHeight;				// �Ôg�Z���z��(���[�g��)
	bool bLandslideArea;				// �y���ЊQ�x�����

	// ���˗ʌv�Z���̕��ʊp
	std::vector<double> vecAzimuth;		// ��͏����̑ΏۂƂȂ����y�n�ʂ̕��ʊp

	// �R���X�g���N�^
	CLand()
		: m_strAreaId("")
		, m_pSurface(NULL)
		, dLandHeight(-1.0)
		, dSolorRadiation(-1.0)
		, dFloodDepth(-1.0)
		, dTsunamiHeight(-1.0)
		, bLandslideArea(false)
	{
	};
	void CalcBounding(double* pMinX = NULL, double* pMinY = NULL, double* pMaxX = NULL, double* pMaxY = NULL, double* pMinZ = NULL, double* pMaxZ = NULL) const;
	bool IsLandInPolygon(unsigned int uiCountPoint, const CPoint2D* pPoint);
};

// �����Q
class CLandData
{
public:
	CLandData(void);
	~CLandData(void);

	size_t GetLandSize()
	{
		return m_vecLand.size();
	};
	CLand* GetLandAt(int idx)
	{
		return &m_vecLand.at(idx);
	};
	void Add(CLand land)
	{
		m_vecLand.push_back(land);
	};
	int GetSolorRadiationOrder(std::string strLandID);
	bool ReadAzimuthCSV(std::wstring strFilePath);

private:

	std::vector<CLand> m_vecLand;			// �f�[�^�z��
	std::vector<CLand> m_vecSortCopy;		// �\�[�g�ς݃f�[�^�z��

	// �\�[�g(���˗�)
	void sortSolorRadiation(std::vector<CLand>* vecSortData);
	static int compareSolorRadiation(void* context, const void* a1, const void* a2);

};

