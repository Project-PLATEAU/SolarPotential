#pragma once
#include <string>
#include <algorithm>
#include <iterator>
#include <locale.h>
#include "BuildingData.h"
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "../../LIB/CommonUtil/StringEx.h"
#include "UIParam.h"

class CImportRestrictAreaData
{

public:
	CImportRestrictAreaData(void);
	~CImportRestrictAreaData(void);

	struct RestrictArea
	{
		std::string				strCityName;	// �G���A��
		std::vector<CPointBase> vecPolyline;	// �|�����C���̓_��

		/* �R���X�g���N�^
		*/
		RestrictArea()
		{
		}
		/* �f�X�g���N�^
		*/
		virtual ~RestrictArea()
		{
		}

		/* �R�s�[�R���X�g���N�^
		*/
		RestrictArea(const RestrictArea& x) { *this = x; }
		/* ������Z�q
		*/
		RestrictArea& operator=(const RestrictArea& x)
		{
			if (this != &x)
			{
				copy(x.vecPolyline.begin(), x.vecPolyline.end(), back_inserter(vecPolyline));
			}
			return *this;
		}

	};

	void Initialize();
	void SetReadFilePath(std::string path)
	{
		setlocale(LC_ALL, "");
		m_strFilePath = CStringEx::ToWString(path);
	};
	bool ReadData(eDatum datum);
	bool IsBuildingInRestrictArea(const std::vector<ROOFSURFACES> pRoofSurfaceList);
	bool IsLandInRestrictArea(const std::vector<CPoint2D> pSurface);

private:
	std::wstring				m_strFilePath;				// ���̓t�@�C���p�X
	std::vector<RestrictArea*>	m_aryRestrictAreaData;		// �������f�[�^

};