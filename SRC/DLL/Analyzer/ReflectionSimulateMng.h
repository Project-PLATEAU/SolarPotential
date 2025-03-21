#pragma once
#include <vector>
#include <map>
#include <string>
#include "ReflectionSimulator.h"
#include "UIParam.h"
#include "../../LIB/CommonUtil/ExitCode.h"

class CResultKeyData;

class CReflectionSimulateMng
{
public:
	CReflectionSimulateMng()
		: m_outDir("")
		, m_year(2021)
		, m_pParam(nullptr)
	{}

	bool Exec(const std::string& outDir, CUIParam* pUIParam, int year);

	eExitCode GetExitCode() { return m_eExitCode; };

private:
	// ���Q����
	struct ReflectionEffectTime
	{
		int summer{ 0 };	// �Ď�
		int spring{ 0 };	// �t��
		int winter{ 0 };	// �~��
		int oneday{ 0 };	// �w���
	};

	// ��͌��ʊi�[��
	std::string m_outDir;
	// ��͔N
	int m_year;
	// �ݒ�p�����[�^
	CUIParam* m_pParam;

	// �I���R�[�h
	eExitCode m_eExitCode;

	bool ReflectionSim(std::vector<CAnalysisReflectionOneDay>& result);

	bool ReflectionEffect(const std::vector<CAnalysisReflectionOneDay>& result);

	bool OutReflectionEffect(const std::string csvfile,
		const std::map<CResultKeyData, ReflectionEffectTime>& effectResult);

	// ����������G���AID, ���b�V��ID���擾
	bool GetIDs(const std::string& buildingID, std::string& areaID, std::string& meshID) const;

};

class CResultKeyData
{
public:
	string buildingId;	// ����ID
	int index;			// ����ID���я�

public:
	CResultKeyData() : buildingId(""), index(0) {}
	CResultKeyData(const string& buildingId, int index)
		: buildingId(buildingId), index(index) {}

	bool operator <(const CResultKeyData& a) const
	{
		// index�Ŕ�r���ē����ł����buildingId�Ŕ�r����
		return tie(index, buildingId) < tie(a.index, a.buildingId);
	}
};
