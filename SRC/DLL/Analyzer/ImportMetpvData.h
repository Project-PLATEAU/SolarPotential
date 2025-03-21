#pragma once
#include "DataImport.h"
#include "../../LIB/CommonUtil/CTime.h"

// �ϐ�[

class CImportMetpvData : public CDataImport
{

public:
	CImportMetpvData(void);
	~CImportMetpvData(void);

	virtual bool	ReadData();

public:
	// �C�ۗv�f�ԍ�
	enum class eWeatherElementNo
	{
		GlobalSolarRadiation = 1,		// �����ʑS�V���˗�
		DirectSolarRadiation,			// �����ʑS�V���˗ʂ̒��B����
		ScatterSolarRadiation,			// �����ʑS�V���˗ʂ̓V��U������
		SunshineDuration,				// ���Ǝ���
		Temperature,					// �C��
		WindDirection,					// ����
		WindSpeed,						// ����
		Precipitation,					// �~����
		SnowDepth,						// �ϐ�[
		PossibleSunshineDuration,		// �Ǝ���

		Unknown = 0
	};

	class CMetpvData
	{
	public:
		eWeatherElementNo	m_eElementNo;		// �C�ۗv�f�ԍ�
		int					m_iMonth;			// ��
		int					m_iDay;				// ��
		int					m_iYear;			// ��\�N
		int					m_aryTimeVal[24];	// ���Ԃ��Ƃ̒l(24h) [0]=1��
		int					m_aryStats[4];		// �����v�l
		int					m_iDayCount;		// 1��1����1�Ƃ���ʎZ����

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
				m_aryStats[n] = 8888;	// �f�[�^�Ȃ�
			}
			m_iDayCount = 0;

		};
	};

	int GetSpotNo() { return this->m_iSpotNo; };

	double GetSnowDepth(const CTime& time);		// �Ώۓ����̐ϐ�[���擾

private:
	int					m_iSpotNo;			// �n�_�ԍ�
	std::wstring		m_strSpotName;		// �n�_��
	int					m_iNLatDeg;			// �k��(�x)
	double				m_dNLatMin;			// �k��(��)
	int					m_iELonDeg;			// ���o(�x)
	double				m_dELonMin;			// ���o(��)
	double				m_dHeight;			// �ϑ��n�_�̕W��(m)

	std::vector<CMetpvData> m_vecMetpvData;	// �f�[�^�z��

};
