#pragma once
#include "DataImport.h"
#include "../../LIB/CommonUtil/CTime.h"

// 毎月の平均日照時間

class CImportAverageSunshineData : public CDataImport
{

public:
	CImportAverageSunshineData(void);
	~CImportAverageSunshineData(void);


	virtual bool	ReadData();

public:
	const int ROW_HEADER1_NUM = 2;	// 地点名ヘッダ行
	const int ROW_HEADER2_NUM = 3;	// 項目名ヘッダ行
	const int ROW_HEADER3_NUM = 4;	// データ分類ヘッダ行

	const int ITEM_DATA_MAX = 3;	// 1項目の最大データ数

	const std::wstring STR_DATA_QUALITY = L"品質情報";
	const std::wstring STR_DATA_HOMO_NUMBER = L"均質番号";


	// 取得時の日付形式
	// データ開始列数にも対応する
	enum class eDateFormat
	{
		UNKNOWN	= 0,
		LITERAL = 1,
		NUMERIC = 2
	};

	class CItem
	{
	public:
		std::wstring	strItemName;	// 項目名
		double			info;			// [0]現象なし情報 or 空欄
		int				qualityInfo;	// [1]品質情報
		int				homoNumber;		// [2]均質番号

		CItem() { info = 0.0; qualityInfo = 0; homoNumber = 0; };
		CItem(double d, int i1, int i2) { info = d; qualityInfo = i1; homoNumber = i2; };
	};
	class CWeatherData
	{
	public:
		// 「年月日などに分けて格納」で取得したデータのみ対応
		int			m_iYear;
		int			m_iMonth;
		std::vector<CItem> vecItemVals;

		CWeatherData()
		{
			m_iYear = 0; m_iMonth = 0;
			vecItemVals.clear();
		};
	};

	// 地点別のデータ
	class CPointData
	{
	public:
		std::wstring				strPointName;	// 地点名
		std::vector<CWeatherData>	vecData;		// データ

		CPointData()
		{
			strPointName = L"";
			vecData.clear();
		};
	};


	// 指定した日付からその月の平均日照時間を取得
	double GetAverageSunshineTime(const CTime& date);

private:
	std::vector<CPointData> m_vecPointData;		// 地点別データ配列

};