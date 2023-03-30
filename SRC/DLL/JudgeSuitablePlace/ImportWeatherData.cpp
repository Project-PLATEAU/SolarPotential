#include "pch.h"
#include "ImportWeatherData.h"
#include "../../LIB/CommonUtil/CFileIO.h"
#include "../../LIB/CommonUtil/CFileUtil.h"
#include "../../LIB/CommonUtil/ReadINIParam.h"
#include "shapefil.h"

#ifdef _DEBUG
#pragma comment(lib,"shapelib_i.lib")
#else
#pragma comment(lib,"shapelib_i.lib")
#endif

CImportWeatherData::CImportWeatherData(void)
{

}

CImportWeatherData::~CImportWeatherData(void)
{
	Initialize();
}

void CImportWeatherData::Initialize()
{
	if (m_arySnowDepthData.size() > 0)
	{
		for (SnowDepth* pRestrictArea : m_arySnowDepthData)
		{
			delete pRestrictArea;
		}
	}
	m_arySnowDepthData.clear();
}

bool CImportWeatherData::ReadData()
{
	if (!GetFUtil()->IsExistPath(m_strFilePath))
	{
		return	false;
	}

	std::string strFilePath = CStringEx::ToString(m_strFilePath);

	// Dbfファイル名取得
	std::string strDbfPath = CFileUtil::ChangeFileNameExt(strFilePath, ".dbf");
	if (!GetFUtil()->IsExistPath(strDbfPath))
	{
		return false;
	}

	// シェープファイルをオープン
	SHPHandle hSHP;
	hSHP = SHPOpen(strFilePath.c_str(), "r");
	if (hSHP == NULL)
	{
		// ファイルのオープンに失敗
		return false;
	}

	// Dbfファイルをオープン
	DBFHandle hDBF;
	hDBF = DBFOpen(strDbfPath.c_str(), "rb");
	if (hDBF == NULL)
	{
		// ファイルのオープンに失敗
		SHPClose(hSHP);
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
		DBFClose(hDBF);
		return false;
	}
	// 種別のチェック
	if (nShapeType != SHPT_POLYGON &&
		nShapeType != SHPT_POLYGONZ &&
		nShapeType != SHPT_POLYGONM)
	{
		// シェープタイプが対象外
		SHPClose(hSHP);
		DBFClose(hDBF);
		return false;
	}

	// フィールド数
	int nField = DBFGetFieldCount(hDBF);

	// 3次メッシュコードフィールド
	const char* pMeshField = "G02_001";
	// 年最深積雪フィールド
	const char* pSnowDepthField = "G02_058";

	// 対象の属性フィールドIndex
	int meshFieldIdx = -1;
	int snowDepthFieldIdx = -1;

	char szTitle[20];
	int nWidth, nDecimals;
	for (int ic = 0; ic < nField; ic++)
	{
		DBFFieldType eType = DBFGetFieldInfo(hDBF, ic, szTitle, &nWidth, &nDecimals);
		switch (eType)
		{
		case FTString:
			if (strcmp(szTitle, pMeshField) == 0)
			{
				meshFieldIdx = ic;
			}
			break;
		case FTDouble:
			if (strcmp(szTitle, pSnowDepthField) == 0)
			{
				snowDepthFieldIdx = ic;
			}
			break;
		}
	}
	if (meshFieldIdx < 0 || snowDepthFieldIdx < 0)
	{
		// メッシュIDおよび積雪深の属性がない場合は失敗
		SHPClose(hSHP);
		DBFClose(hDBF);
		return false;
	}

	// ファイルからデータを読み込んで頂点配列に追加
	SHPObject* psElem;
	for (int ic = 0; ic < nEntities; ic++)
	{
		psElem = SHPReadObject(hSHP, ic);

		if (psElem)
		{
			SnowDepth* pSnowDepth = new SnowDepth;

			// 頂点列の取得
			for (int jc = 0; jc < psElem->nVertices; jc++)
			{	// 緯度経度　→　平面直角座標系に変換
				int JPZONE = GetINIParam()->GetJPZone();
				double dEast, dNorth;
				CGeoUtil::LonLatToXY(psElem->padfX[jc], psElem->padfY[jc], JPZONE, dEast, dNorth);

				// 変換した座標の追加
				CPointBase pt(dEast, dNorth, psElem->padfZ[jc]);
				pSnowDepth->vecPolyline.push_back(pt);
			}

			// 属性の取得
			pSnowDepth->iMeshID = DBFReadIntegerAttribute(hDBF, ic, meshFieldIdx);
			pSnowDepth->iSnowDepth = DBFReadIntegerAttribute(hDBF, ic, snowDepthFieldIdx);
			if (pSnowDepth->iMeshID >= 0 && pSnowDepth->iSnowDepth >= 0 && pSnowDepth->vecPolyline.size() >= 0)
			{
				m_arySnowDepthData.push_back(pSnowDepth);
			}
		}
		SHPDestroyObject(psElem);
	}

	SHPClose(hSHP);
	DBFClose(hDBF);

	return true;
}


// 積雪深を取得
int CImportWeatherData::GetSnowDepth(const CPoint2D& pointTarget)
{
	// メッシュ数
	size_t iAreaNum = m_arySnowDepthData.size();

	// 積雪深(cm)
	int iSnowDepth = -1;

	for (SnowDepth* pSnowDepth : m_arySnowDepthData)
	{
		int iCountPoint = (int)pSnowDepth->vecPolyline.size();
		CPoint2D* pPoint = new CPoint2D[iCountPoint];
		for (int n = 0; n < iCountPoint; n++)
		{
			pPoint[n] = CPoint2D(pSnowDepth->vecPolyline[n].x, pSnowDepth->vecPolyline[n].y);
		}

		// 内外判定
		bool bRet = CGeoUtil::IsPointInPolygon(pointTarget, (int)pSnowDepth->vecPolyline.size(), pPoint);

		delete[] pPoint;

		if (bRet)
		{	// 該当メッシュの場合
			iSnowDepth = pSnowDepth->iSnowDepth;
			break;
		}
	}
	return iSnowDepth;

}

// 積雪荷重計算
double CImportWeatherData::CalSnowLoad(const CPoint2D& pointTarget, const double dp)
{
	// 積雪深(cm)を取得
	int iSnowDepth = GetSnowDepth(pointTarget);

	if (iSnowDepth >= 0)
	{
		// 積雪荷重を計算する
		// 積雪荷重(kgf/㎡) = 年最深積雪量cm × □N/㎡ 
		double dS = iSnowDepth * dp / 10.0; // 10N/㎡ = 1kgf/㎡

		return dS;
	}

	return -1;
}
