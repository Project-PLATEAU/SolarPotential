#pragma once
#include "DataImport.h"
#include "../../LIB/CommonUtil/CTime.h"

// ���ϓ��Ǝ���

class CImportAverageSunshineData : public CDataImport
{

public:
	CImportAverageSunshineData(void);
	~CImportAverageSunshineData(void);


	virtual bool	ReadData();

public:
	const int ROW_HEADER1_NUM = 2;	// �n�_���w�b�_�s
	const int ROW_HEADER2_NUM = 3;	// ���ږ��w�b�_�s
	const int ROW_HEADER3_NUM = 4;	// �f�[�^���ރw�b�_�s

	const int ITEM_DATA_MAX = 3;	// 1���ڂ̍ő�f�[�^��

	const std::wstring STR_DATA_QUALITY = L"�i�����";
	const std::wstring STR_DATA_HOMO_NUMBER = L"�ώ��ԍ�";


	// �擾���̓��t�`��
	// �f�[�^�J�n�񐔂ɂ��Ή�����
	enum class eDateFormat
	{
		UNKNOWN	= 0,
		LITERAL = 1,
		NUMERIC = 2
	};

	class CItem
	{
	public:
		std::wstring	strItemName;	// ���ږ�
		double			info;			// [0]���ۂȂ���� or ��
		int				qualityInfo;	// [1]�i�����
		int				homoNumber;		// [2]�ώ��ԍ�

		CItem() { info = 0.0; qualityInfo = 0; homoNumber = 0; };
		CItem(double d, int i1, int i2) { info = d; qualityInfo = i1; homoNumber = i2; };
	};
	class CWeatherData
	{
	public:
		// �u�N�����Ȃǂɕ����Ċi�[�v�Ŏ擾�����f�[�^�̂ݑΉ�
		int			m_iYear;
		int			m_iMonth;
		std::vector<CItem> vecItemVals;

		CWeatherData()
		{
			m_iYear = 0; m_iMonth = 0;
			vecItemVals.clear();
		};
	};

	// �n�_�ʂ̃f�[�^
	class CPointData
	{
	public:
		std::wstring				strPointName;	// �n�_��
		std::vector<CWeatherData>	vecData;		// �f�[�^

		CPointData()
		{
			strPointName = L"";
			vecData.clear();
		};
	};


	// �w�肵�����t���炻�̌��̕��ϓ��Ǝ��Ԃ��擾
	double GetAverageSunshineTime(const CTime& date);

private:
	std::vector<CPointData> m_vecPointData;		// �n�_�ʃf�[�^�z��

};