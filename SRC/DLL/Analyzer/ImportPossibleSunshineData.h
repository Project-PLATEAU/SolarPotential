#pragma once
#include "DataImport.h"
#include "../../LIB/CommonUtil/CTime.h"

// �Ǝ���

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
		// �N����	�o	����[��]	�쒆	���x[��]	����	����[��]
		CTime			m_SunriseTime;			// ���̏o����
		double			m_dSunriseAngle;		// ����[��]
		CTime			m_MeridianTransit;		// �쒆����
		double			m_dMeridianAltitude;	// �쒆���x[��]
		CTime			m_SunsetTime;			// ���̓��莞��
		double			m_dSunsetAngle;			// ����[��]

		CSunshineData()
		{
			m_dSunriseAngle = 0.0;
			m_dMeridianAltitude = 0.0;
			m_dSunsetAngle = 0.0;

		};
	};

	// �w�肵�����t���炻�̌��̉Ǝ��Ԃ��擾
	double GetPossibleSunshineDuration(const CTime& date);

	// �f�[�^�̔N���擾
	int GetYear();


private:
	double		m_dLat;			// �ܓx
	double		m_dLon;			// �o�x
	double		m_dHeight;		// �W��

	std::vector<CSunshineData> m_vecSunshineData;	// �f�[�^�z��

};