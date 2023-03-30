#include "pch.h"
#include "ImportRestrictAreaData.h"
#include "../../LIB/CommonUtil/CFileIO.h"
#include "../../LIB/CommonUtil/CFileUtil.h"
#include "../../LIB/CommonUtil/ReadINIParam.h"
#include "shapefil.h"

#ifdef _DEBUG
#pragma comment(lib,"shapelib_i.lib")
#else
#pragma comment(lib,"shapelib_i.lib")
#endif

CImportRestrictAreaData::CImportRestrictAreaData(void)
	: m_strFilePath(L"")
{

}

CImportRestrictAreaData::~CImportRestrictAreaData(void)
{
	Initialize();
}

void CImportRestrictAreaData::Initialize()
{
	if (m_aryRestrictAreaData.size() > 0)
	{
		for (RestrictArea* pRestrictArea : m_aryRestrictAreaData)
		{
			delete pRestrictArea;
		}
		m_aryRestrictAreaData.clear();
	}
}
bool CImportRestrictAreaData::ReadData()
{
	if (!GetFUtil()->IsExistPath(m_strFilePath))
	{
		return	false;
	}

	// 初期化
	Initialize();

	// ファイル名一覧を取得
	std::string strFilePath = CStringEx::ToString(m_strFilePath);

	// シェープファイルをオープン
	SHPHandle hSHP;
	hSHP = SHPOpen(strFilePath.c_str(), "r");
	if (hSHP == NULL)
	{
		// ファイルのオープンに失敗
		return false;
	}

	// 種別
	int nShapeType = SHPT_NULL;
	// 要素数
	int nEntities = 0;
	//　バウンディング
	double adfMinBound[4], adfMaxBound[4];
	SHPGetInfo(hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound);

	if (nEntities <= 0)
	{
		// シェープファイルレコードの取得に失敗
		SHPClose(hSHP);
		return false;
	}
	// 種別のチェック
	if (nShapeType != SHPT_POLYGON &&
		nShapeType != SHPT_POLYGONZ &&
		nShapeType != SHPT_POLYGONM)
	{
		// シェープタイプが対象外
		SHPClose(hSHP);
		return false;
	}

	// ファイルからデータを読み込んで頂点配列に追加
	SHPObject* psElem;
	for (int ic = 0; ic < nEntities; ic++)
	{
		psElem = SHPReadObject(hSHP, ic);

		if (psElem)
		{
			RestrictArea* pRestrictArea = new RestrictArea;

			// 頂点列の取得
			for (int jc = 0; jc < psElem->nVertices; jc++)
			{
				// 変換した座標の追加
				CPointBase pt(psElem->padfX[jc], psElem->padfY[jc], psElem->padfZ[jc]);
				pRestrictArea->vecPolyline.push_back(pt);
			}

			if (pRestrictArea->vecPolyline.size() >= 0)
			{
				m_aryRestrictAreaData.push_back(pRestrictArea);
			}

		}
		SHPDestroyObject(psElem);
	}

	SHPClose(hSHP);
	return true;
}

// 制限区域内外判定
bool CImportRestrictAreaData::IsBuildingInRestrictArea(const std::vector<ROOFSURFACES> pRoofSurfaceList)
{
	// 制限区域数
	size_t iAreaNum = m_aryRestrictAreaData.size();

	// 制限区域ごとに判定
	for (RestrictArea* pRestrictArea : m_aryRestrictAreaData)
	{
		std::vector<CPoint2D> restrictArea;
		for(int i = 0; i < pRestrictArea->vecPolyline.size(); i++)
		{
			CPointBase pos = pRestrictArea->vecPolyline.at(i);
			restrictArea.push_back(CPoint2D(pos.x, pos.y));
		}
		
		// 屋根面座標ごとに判定
		for (const auto& roofSurfaces : pRoofSurfaceList)
		{
			for (const auto& surfaceMembers : roofSurfaces.roofSurfaceList)
			{
				for (const auto& pos : surfaceMembers.posList)
				{
					CPoint2D target2d(pos.x, pos.y);
					// 内外判定
					bool bRet = CGeoUtil::IsPointInPolygon(target2d, (int)restrictArea.size(), restrictArea.data());
					// 制限区域内にある
					if (bRet) return true;
				}
			}
		}
	}

	return false;
}