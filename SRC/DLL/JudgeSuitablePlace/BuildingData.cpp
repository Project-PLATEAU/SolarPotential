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

// 建物の内外判定
bool CBuilding::IsBuildingInPolygon(	unsigned int uiCountPoint,		// 頂点数
										const CPoint2D* pPoint			// 多角形の座標
)
{
	// 屋根ごとに内外判定する
	for (const auto& roofSurfaces : *m_pRoofSurfaceList)
	{
		for (const auto& surfaceMembers : roofSurfaces.roofSurfaceList)
		{
			for (const auto& pos : surfaceMembers.posList)
			{
				CPoint2D target2d(pos.x, pos.y);
				bool bRet = CGeoUtil::IsPointInPolygon(target2d, uiCountPoint, pPoint);
				// 範囲内に屋根がある
				if (bRet) return true;
			}
		}
	}
	return false;
}
// 建物のバウンディングボックスを計算
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


// 日射量順(昇順)に並び替えた場合の順位を取得
int CBuildingData::GetSolorRadiationOrder(std::string strBuildingID)
{
	if (m_vecSortCopy.size() <= 0)
	{
		// ソート用にコピー
		std::copy(m_vecBuilding.begin(), m_vecBuilding.end(), std::back_inserter(m_vecSortCopy));
		// ソートする
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
// ソート(日射量)
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

// 方位角データを読み込む
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
			{	// ヘッダ部分
			}
			else
			{	// 2行目以降
				std::string strMeshID = CStringEx::ToString(aryData[0]);
				std::string strBldID = CStringEx::ToString(aryData[1]);
				int iCount = stoi(aryData[2]);
				for (int i = 0; i < GetBuildingSize(); i++)
				{   // 建物IDで検索
					CBuilding* pBuilding = GetBuildingAt(i);
					if (strBldID.compare(pBuilding->m_strBuildingId) == 0)
					{
						// 初期化
						pBuilding->vecRoofAzimuth.clear();
						// 屋根面方位角を格納する
						for (int j = 3; j < aryData.size(); j++)
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
