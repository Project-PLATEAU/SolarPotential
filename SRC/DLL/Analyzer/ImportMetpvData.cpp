#include "pch.h"
#include "DataImport.h"
#include "ImportMetpvData.h"
#include "../../LIB/CommonUtil/CFileIO.h"
#include "../../LIB/CommonUtil/CFileUtil.h"


CImportMetpvData::CImportMetpvData(void)
	: m_iSpotNo(0)
	, m_strSpotName(L"")
	, m_iNLatDeg(0), m_dNLatMin(0.0)
	, m_iELonDeg(0), m_dELonMin(0.0)
	, m_dHeight(0.0)
{

}

CImportMetpvData::~CImportMetpvData(void)
{
}

bool CImportMetpvData::ReadData()
{
	if (!GetFUtil()->IsExistPath(m_strFilePath))
	{
		return	false;
	}

	// 初期化
	m_vecMetpvData.clear();

	CFileIO fio;
	if (fio.Open(m_strFilePath, L"rt"))
	{
		std::wstring strLine;
		int lineCnt = 0;
		wchar_t cBuff[1024];

		while (fio.ReadLineW(cBuff, 1024) != NULL)
		{
			strLine = cBuff;
			std::vector<std::wstring> aryData;
			GetFUtil()->SplitCSVData(strLine, &aryData, ',');
			if ( lineCnt == 0 )
			{	// ヘッダ部分
				m_iSpotNo = stoi(aryData[0]);
				m_strSpotName = aryData[1];
				m_iNLatDeg = stoi(aryData[2]);
				m_dNLatMin = stof(aryData[3]);
				m_iELonDeg = stoi(aryData[4]);
				m_dELonMin = stof(aryData[5]);
				m_dHeight = stof(aryData[6]);
			}
			else
			{	// 2行目以降
				CMetpvData tmpData;

				int iElementNo = stoi(aryData[0]);
				tmpData.m_eElementNo = (eWeatherElementNo)iElementNo;
				if (tmpData.m_eElementNo == eWeatherElementNo::GlobalSolarRadiation ||
					tmpData.m_eElementNo == eWeatherElementNo::DirectSolarRadiation ||
					tmpData.m_eElementNo == eWeatherElementNo::ScatterSolarRadiation ||
					tmpData.m_eElementNo == eWeatherElementNo::SunshineDuration ||
					tmpData.m_eElementNo == eWeatherElementNo::SnowDepth
					)
				{
					int nReadCnt = 1;
					tmpData.m_iMonth = stoi(aryData[nReadCnt]);	nReadCnt++;
					tmpData.m_iDay = stoi(aryData[nReadCnt]);	nReadCnt++;
					tmpData.m_iYear = stoi(aryData[nReadCnt]);	nReadCnt++;
					for (int i = 0; i < 24; i++)
					{
						tmpData.m_aryTimeVal[i] = stoi(aryData[(int64_t)nReadCnt+i]);
					}
					nReadCnt += 24;
					for (int i = 0; i < 4; i++)
					{
						tmpData.m_aryStats[i] = stoi(aryData[(int64_t)nReadCnt+i]);
					}
					nReadCnt += 4;
					tmpData.m_iDayCount = stoi(aryData[nReadCnt]);	nReadCnt++;
				}

				m_vecMetpvData.push_back(tmpData);
			}
			lineCnt++;
		}
		fio.Close();
	}

	return true;
}

// 対象日時の積雪深を取得
// 平均年のデータを使用
double CImportMetpvData::GetSnowDepth(const CTime& time)
{
	double dVal = 0.0;
	int dataSize = (int)m_vecMetpvData.size();
	for (int i = 0; i < dataSize; i++)
	{
		if (this->m_vecMetpvData[i].m_eElementNo != eWeatherElementNo::SnowDepth)	continue;
		// NEDOデータは年統一されていないので年は考慮しない
		if (this->m_vecMetpvData[i].m_iMonth != time.iMonth)	continue;
		if (this->m_vecMetpvData[i].m_iDay != time.iDay)	continue;

		int hourIdx = (time.iHour == 0) ? 23 : (time.iHour - 1);
		dVal = this->m_vecMetpvData[i].m_aryTimeVal[hourIdx];

	}

	return dVal;

}
