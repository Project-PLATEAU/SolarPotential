#include <float.h>
#include "pch.h"
#include "BuildingData.h"
#include "../../LIB/CommonUtil/CEpsUtil.h"
#include "../../LIB/CommonUtil/CFileIO.h"
#include "../../LIB/CommonUtil/CFileUtil.h"

CBuildingData::CBuildingData(void)
{

}

CBuildingData::~CBuildingData(void)
{


}

// �����̓��O����
bool CBuilding::IsBuildingInPolygon(	unsigned int uiCountPoint,		// ���_��
										const CPoint2D* pPoint			// ���p�`�̍��W
)
{
	// �������Ƃɓ��O���肷��
	for (const auto& roofSurfaces : *m_pRoofSurfaceList)
	{
		for (const auto& surfaceMembers : roofSurfaces.roofSurfaceList)
		{
			for (const auto& pos : surfaceMembers.posList)
			{
				CPoint2D target2d(pos.x, pos.y);
				bool bRet = CGeoUtil::IsPointInPolygon(target2d, uiCountPoint, pPoint);
				// �͈͓��ɉ���������
				if (bRet) return true;
			}
		}
	}
	return false;
}
// �����̃o�E���f�B���O�{�b�N�X���v�Z
void CBuilding::CalcBounding(double* pMinX, double* pMinY, double* pMaxX, double* pMaxY, double* pMinZ, double* pMaxZ) const
{
	double	  minX = DBL_MAX
			, minY = DBL_MAX
			, maxX = -DBL_MAX
			, maxY = -DBL_MAX
			, minZ = DBL_MAX
			, maxZ = -DBL_MAX
			;
	for (const auto& roofSurfaces : *m_pRoofSurfaceList)
	{
		for (const auto& surfaceMembers : roofSurfaces.roofSurfaceList)
		{
			for (const auto& pos : surfaceMembers.posList)
			{
				const double& x = pos.x;
				const double& y = pos.y;
				const double& z = pos.z;
				if (CEpsUtil::Less(x, minX))	minX = x;
				if (CEpsUtil::Less(y, minY))	minY = y;
				if (CEpsUtil::Less(maxX, x))	maxX = x;
				if (CEpsUtil::Less(maxY, y))	maxY = y;
				if (CEpsUtil::Less(z, minZ))	minZ = z;
				if (CEpsUtil::Less(maxZ, z))	maxZ = z;
			}
		}
	}
	if (pMinX != NULL)	*pMinX = minX;
	if (pMinY != NULL)	*pMinY = minY;
	if (pMaxX != NULL)	*pMaxX = maxX;
	if (pMaxY != NULL)	*pMaxY = maxY;
	if (pMinZ != NULL)	*pMinZ = minZ;
	if (pMaxZ != NULL)	*pMaxZ = maxZ;
};


// ���˗ʏ�(����)�ɕ��ёւ����ꍇ�̏��ʂ��擾
int CBuildingData::GetSolorRadiationOrder(std::string strBuildingID)
{
	if (m_vecSortCopy.size() <= 0)
	{
		// �\�[�g�p�ɃR�s�[
		std::copy(m_vecBuilding.begin(), m_vecBuilding.end(), std::back_inserter(m_vecSortCopy));
		// �\�[�g����
		sortSolorRadiation(&m_vecSortCopy);
	}
	for (int i = 0; i < m_vecSortCopy.size(); i++)
	{
		CBuilding Building = m_vecSortCopy.at(i);
		if (strBuildingID.compare(Building.m_strBuildingId) == 0)
		{
			return i + 1;
		}
	}
	return -1;
}
// �\�[�g(���˗�)
void CBuildingData::sortSolorRadiation(std::vector<CBuilding>* vecSortData)
{
	CBuilding* pBuilding = vecSortData->data();
	qsort_s((LPVOID)pBuilding, vecSortData->size(), sizeof(CBuilding), compareSolorRadiation, NULL);
}
int CBuildingData::compareSolorRadiation(void* context, const void* a1, const void* a2)
{
	CBuilding* p1 = (CBuilding*)a1;
	CBuilding* p2 = (CBuilding*)a2;
	if (CEpsUtil::Equal(p1->dSolorRadiation, p2->dSolorRadiation))
		return 0;
	else if (CEpsUtil::Less(p1->dSolorRadiation, p2->dSolorRadiation))
		return -1;
	else
		return 1;
}

// ���ʊp�f�[�^��ǂݍ���
bool CBuildingData::ReadAzimuthCSV(std::wstring strFilePath)
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
				std::string strMeshID = CStringEx::ToString(aryData[1]);
				std::string strBldID = CStringEx::ToString(aryData[2]);
				int iCount = stoi(aryData[3]);
				for (int i = 0; i < GetBuildingSize(); i++)
				{   // ����ID�Ō���
					CBuilding* pBuilding = GetBuildingAt(i);
					if (strBldID.compare(pBuilding->m_strBuildingId) == 0)
					{
						// ������
						pBuilding->vecRoofAzimuth.clear();
						// �����ʕ��ʊp���i�[����
						for (int j = 4; j < aryData.size(); j++)
						{
							std::wstring str = aryData[j];
							if (str == L"") break;
							pBuilding->vecRoofAzimuth.push_back(stof(str));
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
