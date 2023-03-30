#pragma once
#include <string>
#include <algorithm>
#include <iterator>
#include <locale.h>
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "../../LIB/CommonUtil/StringEx.h"

class CImportWeatherData
{

public:
	CImportWeatherData(void);
	~CImportWeatherData(void);

	struct SnowDepth
	{
		int						iMeshID;		// メッシュID
		std::vector<CPointBase> vecPolyline;	// ポリラインの点列
		int						iSnowDepth;		// 年間積雪量(cm)

		/* コンストラクタ
		*/
		SnowDepth()
		{
			iMeshID = -1;
			iSnowDepth = -1;
		}
		/* デストラクタ
		*/
		virtual ~SnowDepth()
		{
		}

		/* コピーコンストラクタ
		*/
		SnowDepth(const SnowDepth& x) { *this = x; }
		/* 代入演算子
		*/
		SnowDepth& operator=(const SnowDepth& x)
		{
			if (this != &x)
			{
				iMeshID = x.iMeshID;
				copy(x.vecPolyline.begin(), x.vecPolyline.end(), back_inserter(vecPolyline));
				iSnowDepth = x.iSnowDepth;
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
	int GetSnowDepth(const CPoint2D& pointTarget);
	double CalSnowLoad(const CPoint2D& pointTarget, const double dp);

private:
	std::wstring			m_strFilePath;			// 入力ファイルパス
	std::vector<SnowDepth*>	m_arySnowDepthData;		// 積雪データ
};