#include <float.h>
#include "pch.h"
#include "LandData.h"
#include "../../LIB/CommonUtil/CEpsUtil.h"
#include "../../LIB/CommonUtil/CFileIO.h"
#include "../../LIB/CommonUtil/CFileUtil.h"

CLandData::CLandData(void)
{

}

CLandData::~CLandData(void)
{


}

// �y�n�̓��O����
bool CLand::IsLandInPolygon(unsigned int uiCountPoint,		// ���_��
	const CPoint2D* pPoint			// ���p�`�̍��W
)
{
	for (const auto& pos : *m_pSurface)
	{
		CPoint2D target2d(pos.x, pos.y);
		bool bRet = CGeoUtil::IsPointInPolygon(target2d, uiCountPoint, pPoint);
		if (bRet) return true;
	}
	return false;
}

// �y�n�̃o�E���f�B���O�{�b�N�X���v�Z
void CLand::CalcBounding(double* pMinX, double* pMinY, double* pMaxX, double* pMaxY, double* pMinZ, double* pMaxZ) const
{
	double	  minX = DBL_MAX
		, minY = DBL_MAX
		, maxX = -DBL_MAX
		, maxY = -DBL_MAX
		;

	for (const auto& pos : *m_pSurface)
	{
		const double& x = pos.x;
		const double& y = pos.y;
		if (CEpsUtil::Less(x, minX))	minX = x;
		if (CEpsUtil::Less(y, minY))	minY = y;
		if (CEpsUtil::Less(maxX, x))	maxX = x;
		if (CEpsUtil::Less(maxY, y))	maxY = y;
	}

	if (pMinX != NULL)	*pMinX = minX;
	if (pMinY != NULL)	*pMinY = minY;
	if (pMaxX != NULL)	*pMaxX = maxX;
	if (pMaxY != NULL)	*pMaxY = maxY;

};


// ���˗ʏ�(����)�ɕ��ёւ����ꍇ�̏��ʂ��擾
int CLandData::GetSolorRadiationOrder(std::string strAreaId)
{
	if (m_vecSortCopy.size() <= 0)
	{
		// �\�[�g�p�ɃR�s�[
		std::copy(m_vecLand.begin(), m_vecLand.end(), std::back_inserter(m_vecSortCopy));
		// �\�[�g����
		sortSolorRadiation(&m_vecSortCopy);
	}
	for (int i = 0; i < m_vecSortCopy.size(); i++)
	{
		CLand land = m_vecSortCopy.at(i);
		if (strAreaId.compare(land.m_strAreaId) == 0)
		{
			return i + 1;
		}
	}
	return -1;
}
// �\�[�g(���˗�)
void CLandData::sortSolorRadiation(std::vector<CLand>* vecSortData)
{
	CLand* pLand = vecSortData->data();
	qsort_s((LPVOID)pLand, vecSortData->size(), sizeof(CLand), compareSolorRadiation, NULL);
}
int CLandData::compareSolorRadiation(void* context, const void* a1, const void* a2)
{
	CLand* p1 = (CLand*)a1;
	CLand* p2 = (CLand*)a2;
	if (CEpsUtil::Equal(p1->dSolorRadiation, p2->dSolorRadiation))
		return 0;
	else if (CEpsUtil::Less(p1->dSolorRadiation, p2->dSolorRadiation))
		return -1;
	else
		return 1;
}

// ���ʊp�f�[�^��ǂݍ���
bool CLandData::ReadAzimuthCSV(std::wstring strFilePath)
{
	if (!GetFUtil()->IsExistPath(strFilePath))
	{
		return	false;
	}
	CFileIO fio;
	if (fio.Open(strFilePath, L"rt"))
	{
		std::wstring strLine;
		int lineCnt = 0;
		wchar_t cBuff[1024];

		while (fio.ReadLineW(cBuff, 1024) != NULL)
		{
			strLine = cBuff;
			std::vector<std::wstring> aryData;
			GetFUtil()->SplitCSVData(strLine, &aryData, ',');
			if (lineCnt == 0)
			{	// �w�b�_����
			}
			else
			{	// 2�s�ڈȍ~
				std::string strAreaID = CStringEx::ToString(aryData[0]);
				int iCount = stoi(aryData[3]);
				for (int i = 0; i < GetLandSize(); i++)
				{   // ����ID�Ō���
					CLand* pLand = GetLandAt(i);
					if (strAreaID.compare(pLand->m_strAreaId) == 0)
					{
						// ������
						pLand->vecAzimuth.clear();
						// ���ʊp���i�[����
						for (int j = 4; j < aryData.size(); j++)
						{
							std::wstring str = aryData[j];
							if (str == L"") break;
							pLand->vecAzimuth.push_back(stof(str));
						}
						break;
					}
				}
			}
			lineCnt++;
		}
		fio.Close();
	}

	return true;

}
