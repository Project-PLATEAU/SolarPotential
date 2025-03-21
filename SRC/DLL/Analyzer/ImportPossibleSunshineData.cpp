#include "pch.h"
#include "DataImport.h"
#include "ImportPossibleSunshineData.h"
#include "../../LIB/CommonUtil/CFileIO.h"
#include "../../LIB/CommonUtil/CFileUtil.h"

CImportPossibleSunshineData::CImportPossibleSunshineData(void)
	: m_dLat(0.0), m_dLon(0.0)
	, m_dHeight(0.0)
{

}

CImportPossibleSunshineData::~CImportPossibleSunshineData(void)
{
}

bool CImportPossibleSunshineData::ReadData()
{
	if (!GetFUtil()->IsExistPath(m_strFilePath))
	{
		return	false;
	}

	bool bRet = false;

	// ������
	m_vecSunshineData.clear();

	setlocale(LC_ALL, "");

	try
	{
		CFileIO fio;
		if (fio.Open(m_strFilePath, L"rt"))
		{
			std::wstring strLine;
			int lineCnt = 0;
			wchar_t cBuff[1024];

			while (fio.ReadLineW(cBuff, 1024) != NULL)
			{
				strLine = cBuff;
				if (lineCnt == 1)
				{
					// �ܓx:XX�� �o�x:XX�� �W��: 0.0 m �W����:UT+9h
					std::vector<std::wstring> aryData;
					GetFUtil()->SplitCSVData(strLine, &aryData, ' ');
					std::wstring strLat = aryData[0];
					strLat = strLat.substr(3);
					strLat.pop_back();
					m_dLat = stof(strLat);
					std::wstring strLon = aryData[1];
					strLon = strLon.substr(3);
					strLon.pop_back();
					m_dLon = stof(strLon);
				}
				else if (lineCnt != 0 && lineCnt != 2)
				{	// 4�s�ڈȍ~
					std::vector<std::wstring> aryData;
					GetFUtil()->SplitCSVData(strLine, &aryData, ',');

					CSunshineData tmpData;

					std::string strDate = CStringEx::ToString(aryData[0]);
					tmpData.m_SunriseTime.SetDateTime(strDate, CStringEx::ToString(aryData[1]));
					tmpData.m_dSunriseAngle = stof(aryData[2]);
					tmpData.m_MeridianTransit.SetDateTime(strDate, CStringEx::ToString(aryData[3]));
					tmpData.m_dMeridianAltitude = stof(aryData[4]);
					tmpData.m_SunsetTime.SetDateTime(strDate, CStringEx::ToString(aryData[5]));
					tmpData.m_dSunsetAngle = stof(aryData[6]);

					m_vecSunshineData.push_back(tmpData);
				}
				lineCnt++;
			}
			fio.Close();
			// ��t�@�C���̏ꍇ(�ǂݍ��߂����C�������O)�ُ͈�I���ɂ���B
			if (lineCnt == 0)
			{
				return false;
			}
		}
	}
	catch (...)
	{
		return false;
	}

	if (m_vecSunshineData.size() > 0)	bRet = true;

	return bRet;
}

// �ǂݍ��݃f�[�^����N���擾
int CImportPossibleSunshineData::GetYear()
{
	return m_vecSunshineData[0].m_SunsetTime.iYear;
}

// �w�肵�����t���炻�̌��̉Ǝ��Ԃ��擾
double CImportPossibleSunshineData::GetPossibleSunshineDuration(const CTime& date)
{
	int dataNum = (int)m_vecSunshineData.size();
	double dSumDiff = 0.0;
	for (int i = 0; i < dataNum; i++)
	{
		CSunshineData tmpData = m_vecSunshineData.at(i);
		if (date.iYear != tmpData.m_SunsetTime.iYear)	continue;
		if (date.iMonth != tmpData.m_SunsetTime.iMonth)	continue;
		double diffSec = CTime::DiffTime(tmpData.m_SunriseTime, tmpData.m_SunsetTime);
		double diff = diffSec / 3600;
		dSumDiff += diff;
	}
	return dSumDiff;
}
