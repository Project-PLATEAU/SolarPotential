#pragma once
#include "DataImport.h"
#include "../../LIB/CommonUtil/CTime.h"

// 月毎の可照時間

class CImportPossibleSunshineData : public CDataImport
{

public:
	CImportPossibleSunshineData(void);
	~CImportPossibleSunshineData(void);


	virtual bool	ReadData();


public:
	class CSunshineData
	{
	public:
		// 年月日	出	方位[°]	南中	高度[°]	入り	方位[°]
		CTime			m_SunriseTime;			// 日の出時刻
		double			m_dSunriseAngle;		// 方位[°]
		CTime			m_MeridianTransit;		// 南中時刻
		double			m_dMeridianAltitude;	// 南中高度[°]
		CTime			m_SunsetTime;			// 日の入り時刻
		double			m_dSunsetAngle;			// 方位[°]

		CSunshineData()
		{
			m_dSunriseAngle = 0.0;
			m_dMeridianAltitude = 0.0;
			m_dSunsetAngle = 0.0;

		};
	};

	// 指定した日付からその月の可照時間を取得
	double GetPossibleSunshineDuration(const CTime& date);

	// データの年を取得
	int GetYear();


private:
	double		m_dLat;			// 緯度
	double		m_dLon;			// 経度
	double		m_dHeight;		// 標高

	std::vector<CSunshineData> m_vecSunshineData;	// データ配列

};