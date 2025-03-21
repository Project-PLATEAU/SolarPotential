#include "pch.h"
#include "ResultJudgment.h"
#include "JudgeSuitablePlacePriorityIniFile.h"
#include "../../LIB/CommonUtil/CFileUtil.h"
#include "../../LIB/CommonUtil/CFileIO.h"

#include <fstream>
using namespace std;


CResultJudgment::CResultJudgment(void)
{

}

CResultJudgment::~CResultJudgment(void)
{
}

void CResultJudgment::Prioritization()
{
	std::string strFilePath = GetFUtil()->Combine(GetFUtil()->GetModulePath(), "judge_suitable_place_priority.ini");
	CJudgeSuitablePlacePriorityIniFile JudgeSuitablePlacePriorityIniFile(strFilePath);

	// �D��x�̔������擾
	int iRank5 = JudgeSuitablePlacePriorityIniFile.GetCriterion_5();
	int iRank4 = JudgeSuitablePlacePriorityIniFile.GetCriterion_4();
	int iRank3 = JudgeSuitablePlacePriorityIniFile.GetCriterion_3();
	int iRank2 = JudgeSuitablePlacePriorityIniFile.GetCriterion_2();

	// �����̃|�C���g�ݒ���擾
	int iPoint1_1 = JudgeSuitablePlacePriorityIniFile.GetJudgementCondition_1_1();
	int iPoint1_2 = JudgeSuitablePlacePriorityIniFile.GetJudgementCondition_1_2();
	int iPoint1_3 = JudgeSuitablePlacePriorityIniFile.GetJudgementCondition_1_3();
	int iPoint2_1 = JudgeSuitablePlacePriorityIniFile.GetJudgementCondition_2_1();
	int iPoint2_2 = JudgeSuitablePlacePriorityIniFile.GetJudgementCondition_2_2();
	int iPoint2_3 = JudgeSuitablePlacePriorityIniFile.GetJudgementCondition_2_3();
	int iPoint2_4 = JudgeSuitablePlacePriorityIniFile.GetJudgementCondition_2_4();
	int iPoint3_1 = JudgeSuitablePlacePriorityIniFile.GetJudgementCondition_3_1();
	int iPoint3_2 = JudgeSuitablePlacePriorityIniFile.GetJudgementCondition_3_2();
	int iPoint3_3 = JudgeSuitablePlacePriorityIniFile.GetJudgementCondition_3_3();

	//  �������ƂɗD��x�����肷��
	for ( auto& result : m_result )
	{
		int iPoint = 0;

		// ��������ɂ���ď��O�ƂȂ������ڂ̃}�C�i�X�|�C���g�����Z���Ă���
		if (result.m_strSuitable1_1_1 == "�~" || result.m_strSuitable1_1_2 == "�~") iPoint += iPoint1_1;
		if (result.m_strSuitable1_2 == "�~") iPoint += iPoint1_2;
		if (result.m_strSuitable1_3 == "�~") iPoint += iPoint1_3;
		if (result.m_strSuitable2_1 == "�~") iPoint += iPoint2_1;
		if (result.m_strSuitable2_2 == "�~") iPoint += iPoint2_2;
		if (result.m_strSuitable2_3 == "�~") iPoint += iPoint2_3;
		if (result.m_strSuitable2_4 == "�~") iPoint += iPoint2_4;
		if (result.m_strSuitable3_1 == "�~") iPoint += iPoint3_1;
		if (result.m_strSuitable3_2 == "�~") iPoint += iPoint3_2;
		if (result.m_strSuitable3_3 == "�~") iPoint += iPoint3_3;

		// ���v�|�C���g���烉���N�t�����s��
		if(iRank5 <= iPoint) result.m_ePriority = ePriority::PRIORITY_RANK_5;
		else if (iRank4 <= iPoint) result.m_ePriority = ePriority::PRIORITY_RANK_4;
		else if (iRank3 <= iPoint) result.m_ePriority = ePriority::PRIORITY_RANK_3;
		else if (iRank2 <= iPoint) result.m_ePriority = ePriority::PRIORITY_RANK_2;
		else result.m_ePriority = ePriority::PRIORITY_RANK_1;
	}
}

ePriority CResultJudgment::GetPriority(std::string strId)
{
	for (const auto& result : m_result)
	{
		switch (m_eTarget)
		{
		case eTarget::TARGET_BUILD:
		{
			if (result.m_strBuildingId == strId)
			{
				return result.m_ePriority;
			}
			break;
		}

		case eTarget::TARGET_LAND:
		{
			if (result.m_strAreaId == strId)
			{
				return result.m_ePriority;
			}
			break;
		}

		default:
			break;
		}
	}
	return ePriority::PRIORITY_RANK_UNKNOWN;
}

bool CResultJudgment::OutputResultCSV(const std::wstring& filepath)
{
	bool ret = false;

	CFileIO file;
	if (!file.Open(filepath, L"w"))
	{
		return false;
	}

	// �w�b�_��
	if (!file.WriteLineA("�G���AID,���b�V��ID,����ID,�D��x,\
�������1_1_1,�������1_1_2,�������1_2,�������1_3,\
�������2_1,�������2_2,�������2_3,�������2_4,\
�������3_1,�������3_2,�������3_3"))
	{
		return false;
	}

	// �f�[�^��
	for (const auto& result : m_result)
	{
		std::string strLine = CStringEx::Format("%s,%d,%s,%d,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",
				result.m_strAreaId.c_str(),
				result.m_iMeshId,
				result.m_strBuildingId.c_str(),
				(int)result.m_ePriority,
				result.m_strSuitable1_1_1.c_str(),
				result.m_strSuitable1_1_2.c_str(),
				result.m_strSuitable1_2.c_str(),
				result.m_strSuitable1_3.c_str(),
				result.m_strSuitable2_1.c_str(),
				result.m_strSuitable2_2.c_str(),
				result.m_strSuitable2_3.c_str(),
				result.m_strSuitable2_4.c_str(),
				result.m_strSuitable3_1.c_str(),
				result.m_strSuitable3_2.c_str(),
				result.m_strSuitable3_3.c_str());
		file.WriteLineA(strLine);
	}

	file.Close();
	
	return true;

}