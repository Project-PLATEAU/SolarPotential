#include "pch.h"
#include "DataImport.h"
#include "ImportAverageSunshineData.h"
#include "../../LIB/CommonUtil/CFileIO.h"
#include "../../LIB/CommonUtil/CFileUtil.h"


CImportAverageSunshineData::CImportAverageSunshineData(void)
{

}

CImportAverageSunshineData::~CImportAverageSunshineData(void)
{
}

bool CImportAverageSunshineData::ReadData()
{
	if (!GetFUtil()->IsExistPath(m_strFilePath))
	{
		return	false;
	}

	bool bRet = false;

	// 初期化
	m_vecPointData.clear();
	std::vector<std::wstring> vecItemName;	// 項目名配列
	int	iDataCount = 0;						// 1地点のデータ数
	bool bEnableItemVal[3] = { true, false, false };	// 項目ごとの有効データ
	eDateFormat eFormat = eDateFormat::UNKNOWN;

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
				std::vector<std::wstring> aryData;
				GetFUtil()->SplitCSVData(strLine, &aryData, ',');
				if (lineCnt < ROW_HEADER1_NUM)
				{
					// 最初のヘッダまで読み飛ばす
				}
				else if (lineCnt == ROW_HEADER1_NUM)
				{
					for (int i = 0; i < aryData.size(); i++)
					{
						if (aryData[i].empty())	continue;
						if (m_vecPointData.size() == 0)
						{	// 1つ目の地点
							if (i == (int)eDateFormat::LITERAL)	eFormat = eDateFormat::LITERAL;
							if (i == (int)eDateFormat::NUMERIC)	eFormat = eDateFormat::NUMERIC;
							CPointData tmpData;
							tmpData.strPointName = aryData[i];
							m_vecPointData.push_back(tmpData);
						}
						else if (!aryData[(int64_t)i - 1].empty() && (aryData[(int64_t)i - 1] != aryData[i]))
						{
							CPointData tmpData;
							tmpData.strPointName = aryData[i];
							m_vecPointData.push_back(tmpData);
						}
						if (m_vecPointData.size() == 1)		iDataCount++;
					}


				}
				else if (lineCnt == ROW_HEADER2_NUM)
				{
					// 項目数を数える
					std::wstring  strPrevItemName = L"";
					for (int i = (int)eFormat; i < iDataCount; i++)
					{
						if (strPrevItemName.empty())
						{
							strPrevItemName = aryData[i];
							vecItemName.push_back(aryData[i]);
						}
						else if (strPrevItemName != aryData[i])
						{
							strPrevItemName = aryData[i];
							vecItemName.push_back(aryData[i]);
						}
					}
				}
				else if (aryData[0].empty() && lineCnt == ROW_HEADER3_NUM)
				{
					// 利用上注意が必要なデータ、値が不均質となったデータが両方非表示の場合は、
					// 5行目からデータが始まるのでここは通らない
					if (aryData[(int)eFormat] == L"")	// 通常データはヘッダ名が空
					{
						bEnableItemVal[0] = true;
					}
					if (aryData[(int64_t)eFormat + 1] == STR_DATA_QUALITY)
					{
						bEnableItemVal[1] = true;
					}
					if (aryData[(int64_t)eFormat + 2] == STR_DATA_HOMO_NUMBER)
					{
						bEnableItemVal[2] = true;
					}
				}
				else
				{
					// 地点数ループ
					for (int i = 0; i < m_vecPointData.size(); i++)
					{
						CPointData& pData = m_vecPointData[i];
						CWeatherData wData;

						// 日付部分
						switch (eFormat)
						{
						case eDateFormat::LITERAL:
						{
							// エクセルで開くとJan-10(mmm-yy形式)と表示されるが、テキストデータとしては2010/1となっている
							std::vector<std::wstring> aryTmp;
							GetFUtil()->SplitCSVData(aryData[0], &aryTmp, '/');
							if (aryTmp.size() == 2)
							{
								wData.m_iYear = stoi(aryTmp[0]);
								wData.m_iMonth = stoi(aryTmp[1]);
							}
						}
						break;

						case eDateFormat::NUMERIC:
						{
							wData.m_iYear = stoi(aryData[0]);
							wData.m_iMonth = stoi(aryData[1]);
						}
						break;

						default:
							break;
						}

						// データ部分
						int idx = (int)eFormat + i * iDataCount;
						for (int j = 0; j < vecItemName.size(); j++)
						{
							CItem iData;
							iData.strItemName = vecItemName[j];
							if (bEnableItemVal[0])
							{
								if (!aryData[idx].empty()) iData.info = stof(aryData[idx]);
								idx++;
							}
							if (bEnableItemVal[1])
							{
								if (!aryData[idx].empty()) iData.qualityInfo = stoi(aryData[idx]);
								idx++;
							}
							if (bEnableItemVal[2])
							{
								if (!aryData[idx].empty()) iData.homoNumber = stoi(aryData[idx]);
								idx++;
							}
							wData.vecItemVals.push_back(iData);
						}

						pData.vecData.push_back(wData);
					}

				}
				lineCnt++;
			}
			fio.Close();
			// 空ファイルの場合(読み込めたライン数＝０)は異常終了にする。
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
	
	if (m_vecPointData.size() > 0)	bRet = true;

	return bRet;
}

// 指定した日付からその月の日照時間を取得
double CImportAverageSunshineData::GetAverageSunshineTime(
	const CTime& date
)
{
	int pointNum = (int)m_vecPointData.size();
	double dVal = 0.0;
	double dSumVal = 0.0; int nDataCount = 0;
	bool bFind = false;
	for (int ic = 0; ic < pointNum; ic++)
	{
		CPointData pointData = m_vecPointData[ic];
		int dataNum = (int)pointData.vecData.size();
		for (int jc = 0; jc < dataNum; jc++)
		{
			CWeatherData data = pointData.vecData[jc];
			if (date.iMonth != data.m_iMonth)	continue;
			int itemNum = (int)data.vecItemVals.size();
			for (int kc = 0; kc < itemNum; kc++)
			{ 
				CItem item = data.vecItemVals[kc];
				if (item.strItemName.compare(L"日照時間(時間)") == 0)
				{
					if (date.iYear == data.m_iYear)
					{	// 対象年のデータが存在する
						dVal = item.info;
						bFind = true;
						break;
					}
					else
					{
						dSumVal += item.info;
						nDataCount++;
						break;
					}
				}
			}
			if (bFind)	break;
		}
		if (bFind)	break;
	}

	// 対象年データが見つからなかった場合
	if (!bFind)
	{
		dVal = dSumVal / nDataCount;
	}
	return dVal;
}
