#include "pch.h"
#include "TiffDataManager.h"
#include <cmath>
#include "CTiffCreator.h"
#include "CFileUtil.h"

CTiffDataManager::CTiffDataManager()
	: m_dMeshSize(0.0)
	, m_iNoDataVal(0)
	, m_iEPSGCode(0)
	, m_dOffsetX(0.0)
	, m_dOffsetY(0.0)
	, m_dWidth(0.0)
	, m_dHeight(0.0)
	, m_iBufWidth(0)
	, m_iBufHeight(0)
{

}

CTiffDataManager::~CTiffDataManager()
{
	for (CTiffData* tiff : m_vecTiffDatas)
	{
		const std::vector<CPointBase>* pData = tiff->GetData();
		delete pData;
		delete tiff;
	}
}

bool CTiffDataManager::AddTiffData(std::vector<CPointBase>* pData)
{
	if (nullptr == pData)
	{
		return false;
	}

	try
	{
		CTiffData* tiff = new CTiffData(pData);
		m_vecTiffDatas.push_back(tiff);
	}
	catch (...)
	{
		return false;
	}

	return true;
}

bool CTiffDataManager::CalcIntegRectangle()
{
	if (0 == m_vecTiffDatas.size())
	{
		return false;
	}

	CPointBase min = { FLT_MAX, FLT_MAX, FLT_MAX };
	CPointBase max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

	// 統合領域の最大値、最小値算出
	for (int i = 0; i < m_vecTiffDatas.size(); i++)
	{
		CTiffData* _tiff = m_vecTiffDatas[i];

		CPointBase _min = { FLT_MAX, FLT_MAX, FLT_MAX };
		CPointBase _max = { FLT_MAX, FLT_MAX, FLT_MAX };

		_tiff->CalcMaxMinPoint();

		_min = _tiff->GetPointMin();
		_max = _tiff->GetPointMax();

		if (min.x > _min.x) min.x = _min.x;
		if (min.y > _min.y) min.y = _min.y;
		if (min.z > _min.z) min.z = _min.z;
		if (max.x < _max.x) max.x = _max.x;
		if (max.y < _max.y) max.y = _max.y;
		if (max.z < _max.z) max.z = _max.z;
	}

	// オフセット算出
	m_dOffsetX = min.x;
	m_dOffsetY = max.y;

	// 縦横サイズ算出
	m_dWidth = max.x - min.x;
	m_dHeight = max.y - min.y;

	m_iBufWidth = (int)std::ceil(m_dWidth / m_dMeshSize);
	m_iBufHeight = (int)std::ceil(m_dHeight / m_dMeshSize);

	return true;
}

int CTiffDataManager::CalcGeoTiffBufIndex(CPointBase point)
{
	int ibuf_h = (int)((m_dOffsetY - point.y) / m_dMeshSize);
	int ibuf_w = (int)((point.x - m_dOffsetX) / m_dMeshSize);

	if (ibuf_w >= m_iBufWidth) ibuf_w = m_iBufWidth - 1;
	if (ibuf_h >= m_iBufHeight) ibuf_h = m_iBufHeight - 1;

	int index = ibuf_w + m_iBufWidth * ibuf_h;

	return index;
}

bool CTiffDataManager::InitGeoTiffBuf()
{
	return true;

}

bool CTiffDataManager::CreateGeoTiffBuf()
{
	return true;
}

bool CTiffDataManager::CallGeoTiffCreator(float* buf)
{
	// パラメータ設定
	CTiffCreator::GeoTiffInfo gti;
	gti.offsetX = m_dOffsetX;
	gti.offsetY = m_dOffsetY;
	gti.meshSize = m_dMeshSize;
	gti.width = m_iBufWidth;
	gti.height = m_iBufHeight;
	gti.noDataValue = m_iNoDataVal;
	gti.EPSGCode = m_iEPSGCode;

	std::string _msg("");

	// GeoTiff生成
	CTiffCreator gtc;
	bool ret = gtc.CreateColorTiff(m_strFilePath, gti, buf, m_strColorSetting);
	if (ret)
	{
		// ワールドファイル
		std::wstring strWldFilePath = GetFUtil()->ChangeFileNameExt(m_strFilePath, L"tfw");
		ret = gtc.CreateWldFile(strWldFilePath, gti);
	}
	return ret; 
}

bool CTiffDataManager::Create()
{
	// GeoTiff出力用バッファ領域の縦横サイズ算出
	if (!CalcIntegRectangle())
	{
		return false;
	}
	  
	ULONG _iBufSize = m_iBufWidth * m_iBufHeight;

	// GeoTiff出力用バッファ領域の確保、初期化
	float* dstBuf = new float[_iBufSize];
	for (ULONG i = 0; i < _iBufSize; i++)
	{
		dstBuf[i] = (float)m_iNoDataVal;
	}

	// 出力用バッファにZ値のデータを格納
	{
		for (size_t i = 0; i < m_vecTiffDatas.size(); i++)
		{
			CTiffData* tiff = m_vecTiffDatas[i];
			const std::vector<CPointBase>* pData = tiff->GetData();

			for (size_t ipt = 0; ipt < pData->size(); ipt++)
			{
				CPointBase _pt = pData->at(ipt);
				int _indexBuf = CalcGeoTiffBufIndex(_pt);
				float _bufVal = dstBuf[_indexBuf];

				dstBuf[_indexBuf] = (float)_pt.z;
			}
		}
	} 

	// GeoTiff生成処理実行
	bool ret = CallGeoTiffCreator(dstBuf);

	return ret;
}

