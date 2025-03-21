#pragma once
#include <string>
#include <algorithm>
#include <iterator>
#include <vector>
#include "AggregateData.h"
#include "../../LIB/CommonUtil/CGeoUtil.h"

// ����
class CBuilding
{
public:
	std::string	m_strAreaId;					// �G���AID
	int			m_iMeshId;						// ���b�V��ID
	std::string m_strBuildingId;				// ����ID
	std::vector<ROOFSURFACES>* m_pRoofSurfaceList;	// �������X�g

	double dBldHeight;					// ��������
	double dSolorRadiation;				// �N�ԗ\�����˗�
	int iBldStructureType;				// �\�����
	double dFloodDepth;					// �^���Z���z��̐Z���[(���[�g��) 
										// �����̐�̍^���Z���z��̃^�O�����ꍇ�͐[�����̐Z���[���󂯎��
	double dTsunamiHeight;				// �Ôg�Z���z��(���[�g��)
	bool bLandslideArea;				// �y���ЊQ�x�����
	int iBldStructureType2;				// �s�s���Ƃ̓Ǝ��敪�Ɋ�Â����z�\���̎��
	int iFloorType;						// �s�s���Ƃ̓Ǝ��敪�Ɋ�Â��n��K���͈̔�

	// ���˗ʌv�Z���̉����̕��ʊp
	std::vector<double> vecRoofAzimuth;	// ��͏����̑ΏۂƂȂ��������ʂ̕��ʊp

	// �R���X�g���N�^
	CBuilding()
		: m_strAreaId("")
		, m_iMeshId(0)
		, m_strBuildingId("")
		, m_pRoofSurfaceList(NULL)
		, dBldHeight(-1.0)
		, dSolorRadiation(-1.0)
		, iBldStructureType(-1)
		, dFloodDepth(-1.0)
		, dTsunamiHeight(-1.0)
		, bLandslideArea(false)
		, iBldStructureType2(-1)
		, iFloorType(-1)
	{
	};
	void CalcBounding(double* pMinX = NULL, double* pMinY = NULL, double* pMaxX = NULL, double* pMaxY = NULL, double* pMinZ = NULL, double* pMaxZ = NULL) const;
	bool IsBuildingInPolygon(unsigned int uiCountPoint, const CPoint2D* pPoint);
};

// �����Q
class CBuildingData
{
public:
	CBuildingData(void);
	~CBuildingData(void);

	size_t GetBuildingSize()
	{
		return m_vecBuilding.size();
	};
	CBuilding* GetBuildingAt(int idx)
	{
		return &m_vecBuilding.at(idx);
	};
	void Add(CBuilding building)
	{
		m_vecBuilding.push_back(building);
	};
	int GetSolorRadiationOrder(std::string strBuildingID);
	bool ReadAzimuthCSV(std::wstring strFilePath);

private:
	
	std::vector<CBuilding> m_vecBuilding;	// �f�[�^�z��
	std::vector<CBuilding> m_vecSortCopy;			// �\�[�g�ς݃f�[�^�z��

	// �\�[�g(���˗�)
	void sortSolorRadiation(std::vector<CBuilding>* vecSortData);
	static int compareSolorRadiation(void* context, const void* a1, const void* a2);

};

