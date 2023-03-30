#pragma once
#include <string>
#include <algorithm>
#include <iterator>
#include <locale.h>
#include "BuildingData.h"
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "../../LIB/CommonUtil/StringEx.h"

class CImportRestrictAreaData
{

public:
	CImportRestrictAreaData(void);
	~CImportRestrictAreaData(void);

	struct RestrictArea
	{
		std::string				strCityName;	// エリア名
		std::vector<CPointBase> vecPolyline;	// ポリラインの点列

		/* コンストラクタ
		*/
		RestrictArea()
		{
		}
		/* デストラクタ
		*/
		virtual ~RestrictArea()
		{
		}

		/* コピーコンストラクタ
		*/
		RestrictArea(const RestrictArea& x) { *this = x; }
		/* 代入演算子
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
	bool ReadData();
	bool IsBuildingInRestrictArea(const std::vector<ROOFSURFACES> pRoofSurfaceList);

private:
	std::wstring				m_strFilePath;				// 入力ファイルパス
	std::vector<RestrictArea*>	m_aryRestrictAreaData;		// 制限区域データ

};