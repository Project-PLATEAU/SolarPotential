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

	// ������
	m_vecPointData.clear();
	std::vector<std::wstring> vecItemName;	// ���ږ��z��
	int	iDataCount = 0;						// 1�n�_�̃f�[�^��
	bool bEnableItemVal[3] = { true, false, false };	// ���ڂ��Ƃ̗L���f�[�^
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
					// �ŏ��̃w�b�_�܂œǂݔ�΂�
				}
				else if (lineCnt == ROW_HEADER1_NUM)
				{
					for (int i = 0; i < aryData.size(); i++)
					{
						if (aryData[i].empty())	continue;
						if (m_vecPointData.size() == 0)
						{	// 1�ڂ̒n�_
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
					// ���ڐ��𐔂���
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
					// ���p�㒍�ӂ��K�v�ȃf�[�^�A�l���s�ώ��ƂȂ����f�[�^��������\���̏ꍇ�́A
					// 5�s�ڂ���f�[�^���n�܂�̂ł����͒ʂ�Ȃ�
					if (aryData[(int)eFormat] == L"")	// �ʏ�f�[�^�̓w�b�_������
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
					// �n�_�����[�v
					for (int i = 0; i < m_vecPointData.size(); i++)
					{
						CPointData& pData = m_vecPointData[i];
						CWeatherData wData;

						// ���t����
						switch (eFormat)
						{
						case eDateFormat::LITERAL:
						{
							// �G�N�Z���ŊJ����Jan-10(mmm-yy�`��)�ƕ\������邪�A�e�L�X�g�f�[�^�Ƃ��Ă�2010/1�ƂȂ��Ă���
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

						// �f�[�^����
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
	
	if (m_vecPointData.size() > 0)	bRet = true;

	return bRet;
}

// �w�肵�����t���炻�̌��̓��Ǝ��Ԃ��擾
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
				if (item.strItemName.compare(L"���Ǝ���(����)") == 0)
				{
					if (date.iYear == data.m_iYear)
					{	// �Ώ۔N�̃f�[�^�����݂���
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

	// �Ώ۔N�f�[�^��������Ȃ������ꍇ
	if (!bFind)
	{
		dVal = dSumVal / nDataCount;
	}
	return dVal;
}
