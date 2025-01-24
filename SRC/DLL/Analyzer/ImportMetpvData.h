#pragma once
#include "DataImport.h"
#include "../../LIB/CommonUtil/CTime.h"

// 積雪深

class CImportMetpvData : public CDataImport
{

public:
	CImportMetpvData(void);
	~CImportMetpvData(void);

	virtual bool	ReadData();

public:
	// 気象要素番号
	enum class eWeatherElementNo
	{
		GlobalSolarRadiation = 1,		// 水平面全天日射量
		DirectSolarRadiation,			// 水平面全天日射量の直達成分
		ScatterSolarRadiation,			// 水平面全天日射量の天空散乱成分
		SunshineDuration,				// 日照時間
		Temperature,					// 気温
		WindDirection,					// 風向
		WindSpeed,						// 風速
		Precipitation,					// 降水量
		SnowDepth,						// 積雪深
		PossibleSunshineDuration,		// 可照時間

		Unknown = 0
	};

	class CMetpvData
	{
	public:
		eWeatherElementNo	m_eElementNo;		// 気象要素番号
		int					m_iMonth;			// 月
		int					m_iDay;				// 日
		int					m_iYear;			// 代表年
		int					m_aryTimeVal[24];	// 時間ごとの値(24h) [0]=1時
		int					m_aryStats[4];		// 日統計値
		int					m_iDayCount;		// 1月1日を1とする通算日数

		CMetpvData()
		{
			m_eElementNo = eWeatherElementNo::Unknown;
			m_iYear = 0; m_iMonth = 0;  m_iDay = 0;
			m_iDayCount = 0;

			for (int n = 0; n < 24; n++)
			{
				m_aryTimeVal[n] = 0;
			}
			for (int n = 0; n < 4; n++)
			{
				m_aryStats[n] = 8888;	// データなし
			}
			m_iDayCount = 0;

		};
	};

	int GetSpotNo() { return this->m_iSpotNo; };

	double GetSnowDepth(const CTime& time);		// 対象日時の積雪深を取得

private:
	int					m_iSpotNo;			// 地点番号
	std::wstring		m_strSpotName;		// 地点名
	int					m_iNLatDeg;			// 北緯(度)
	double				m_dNLatMin;			// 北緯(分)
	int					m_iELonDeg;			// 東経(度)
	double				m_dELonMin;			// 東経(分)
	double				m_dHeight;			// 観測地点の標高(m)

	std::vector<CMetpvData> m_vecMetpvData;	// データ配列

};
