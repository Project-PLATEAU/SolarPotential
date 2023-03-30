#pragma once
#include <vector>
#include "StringEx.h"
#include "CGeoUtil.h"
#include "TiffData.h"

const float CTRLVALUE_MESH_SIZE_DEFAULT = 1.0;
const int CTRLVALUE_NO_DATA_VALUE_DEFAULT = -9999;

const int JGD2011_EPSG_CODE_TABLE[] = {
	6669,
	6670,
	6671,
	6672,
	6673,
	6674,
	6675,
	6676,
	6677,
	6678,
	6679,
	6680,
	6681,
	6682,
	6683,
	6684,
	6685,
	6686,
	6687,
};

class CTiffDataManager
{
public:
	CTiffDataManager();
	~CTiffDataManager();

protected:
	std::vector<CTiffData*> m_vecTiffDatas;

	double m_dMeshSize;
	int m_iNoDataVal;
	int m_iEPSGCode;

	double m_dOffsetX;
	double m_dOffsetY;
	double m_dWidth;
	double m_dHeight;
	int m_iBufWidth;
	int m_iBufHeight;

	std::wstring m_strFilePath;
	std::wstring m_strColorSetting;

protected:
	bool CalcIntegRectangle();
	int CalcGeoTiffBufIndex(CPointBase point);
	bool InitGeoTiffBuf();
	bool CreateGeoTiffBuf();
	bool CallGeoTiffCreator(float* buf);

public:
	bool AddTiffData(std::vector<CPointBase>* pData);
	void SetEPSGCode(int code) { m_iEPSGCode = code; };
	void SetMeshSize(float size) { m_dMeshSize = size; };
	void SetNoDataVal(int val) { m_iNoDataVal = val; };
	void SetFilePath(std::wstring path) { m_strFilePath = path; };
	void SetColorSetting(std::wstring colorSetting) { m_strColorSetting = colorSetting; };
	
	bool Create();

};

