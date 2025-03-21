#include "pch.h"
#include "ReflectionSimulateMng.h"
#include <fstream>
#include <thread>
#include <vector>
#include <filesystem>
#include <iomanip>

#include <CommonUtil/CTime.h>
#include "AnalyzeData.h"

using namespace std;

bool CReflectionSimulateMng::Exec(const std::string& outDir, CUIParam* pUIParam, int year)
{
	m_outDir = outDir;
	m_year = year;
	m_pParam = pUIParam;

	// �Ď��A�t���A�~���A�w����̏��Ŕ��˃V�~�����[�V�������ʂ��擾����
	vector<CAnalysisReflectionOneDay> result;
	bool res = ReflectionSim(result);
	if (!res)
	{
		return false;
	}

	// ���Q�v�Z
	res = ReflectionEffect(result);

	if (res)
	{
		m_eExitCode = eExitCode::Normal;
	}

	return res;
}

// ���˃V�~�����[�V������͎��{
bool CReflectionSimulateMng::ReflectionSim(
	vector<CAnalysisReflectionOneDay>& result
)
{
	// ��̓G���A�����擾
	vector<AREADATA>* allList;
	allList = reinterpret_cast<vector<AREADATA>*>(GetAllAreaList());

	// ���ʊi�[�p
	CAnalysisReflectionOneDay summerResult;
	CAnalysisReflectionOneDay springResult;
	CAnalysisReflectionOneDay winterResult;
	CAnalysisReflectionOneDay onedayResult;

	switch (m_pParam->eAnalyzeDate)
	{
	case eDateType::OneMonth:
	{
		int nDay = 1;	// 1���ŌŒ�

		// �Ď��E�~���E�t���̓��t�擾
		CTime date = CTime(m_year, m_pParam->nMonth, nDay, 0, 0, 0);
		CReflectionSimulator refSim(date, m_pParam);

		// �X���b�h
		std::thread thread(&CReflectionSimulator::Exec, &refSim, *allList);
		thread.join();

		// ��͎��s���ʃ`�F�b�N
		if (refSim.GetExecResult() != eExitCode::Normal) { m_eExitCode = refSim.GetExecResult(); return false; }

		// ��͌��ʏo��
		filesystem::path filepath;

		filepath = m_outDir;
		filepath /= CStringEx::Format("���˃V�~�����[�V��������_%02d%02d.csv", m_pParam->nMonth, nDay);
		refSim.OutResult(filepath.string());
		onedayResult = refSim.GetResult();

		// CZML�t�@�C���o��
		filesystem::path czmlfilepath;

		czmlfilepath = m_outDir;
		czmlfilepath /= CStringEx::Format("���˃V�~�����[�V��������_%02d%02d.czml", m_pParam->nMonth, nDay);
		refSim.OutResultCZML(czmlfilepath.string());

		break;
	}
	case eDateType::OneDay:
	{
		// �Ď��E�~���E�t���̓��t�擾
		CTime date = CTime(m_year, m_pParam->nMonth, m_pParam->nDay, 0, 0, 0);
		CReflectionSimulator refSim(date, m_pParam);

		// �X���b�h
		std::thread thread(&CReflectionSimulator::Exec, &refSim, *allList);
		thread.join();

		// ��͎��s���ʃ`�F�b�N
		if (refSim.GetExecResult() != eExitCode::Normal) { m_eExitCode = refSim.GetExecResult(); return false; }

		// ��͌��ʏo��
		filesystem::path filepath;

		filepath = m_outDir;
		filepath /= CStringEx::Format("���˃V�~�����[�V��������_%02d%02d.csv", m_pParam->nMonth, m_pParam->nDay);
		refSim.OutResult(filepath.string());
		onedayResult = refSim.GetResult();

		// CZML�t�@�C���o��
		filesystem::path czmlfilepath;

		czmlfilepath = m_outDir;
		czmlfilepath /= CStringEx::Format("���˃V�~�����[�V��������_%02d%02d.czml", m_pParam->nMonth, m_pParam->nDay);
		refSim.OutResultCZML(czmlfilepath.string());

		break;
	}
	case eDateType::Summer:
	{
		// �Ď��̓��t�擾
		CTime summer = CTime::GetSummerSolstice(m_year);
		CReflectionSimulator summerSim(summer, m_pParam);

		// �X���b�h
		std::thread threadSummer(&CReflectionSimulator::Exec, &summerSim, *allList);
		threadSummer.join();

		// ��͎��s���ʃ`�F�b�N
		if (summerSim.GetExecResult() != eExitCode::Normal) { m_eExitCode = summerSim.GetExecResult(); return false; }

		// ��͌��ʏo��
		filesystem::path filepath;

		filepath = m_outDir;
		filepath /= "���˃V�~�����[�V��������_�Ď�.csv";
		summerSim.OutResult(filepath.string());
		summerResult = summerSim.GetResult();

		// CZML�t�@�C���o��
		filesystem::path czmlfilepath;

		czmlfilepath = m_outDir;
		czmlfilepath /= "���˃V�~�����[�V��������_�Ď�.czml";
		summerSim.OutResultCZML(czmlfilepath.string());

		break;
	}

	case eDateType::Winter:
	{
		// �~���̓��t�擾
		CTime winter = CTime::GetWinterSolstice(m_year);
		CReflectionSimulator winterSim(winter, m_pParam);

		// �X���b�h
		std::thread threadWinter(&CReflectionSimulator::Exec, &winterSim, *allList);
		threadWinter.join();

		// ��͎��s���ʃ`�F�b�N
		if (winterSim.GetExecResult() != eExitCode::Normal) { m_eExitCode = winterSim.GetExecResult(); return false; }

		// ��͌��ʏo��
		filesystem::path filepath;

		filepath = m_outDir;
		filepath /= "���˃V�~�����[�V��������_�~��.csv";
		winterSim.OutResult(filepath.string());
		winterResult = winterSim.GetResult();

		// CZML�t�@�C���o��
		filesystem::path czmlfilepath;

		czmlfilepath = m_outDir;
		czmlfilepath /= "���˃V�~�����[�V��������_�~��.czml";
		winterSim.OutResultCZML(czmlfilepath.string());

		break;
	}
	case eDateType::Year:
	{
		// �Ď��E�~���E�t���̓��t�擾
		CTime summer = CTime::GetSummerSolstice(m_year);
		CTime winter = CTime::GetWinterSolstice(m_year);
		CTime sprint = CTime::GetVernalEquinox(m_year);

		CReflectionSimulator summerSim(summer, m_pParam);
		CReflectionSimulator winterSim(winter, m_pParam);
		CReflectionSimulator springSim(sprint, m_pParam);

		// �X���b�h
		std::thread threadSummer(&CReflectionSimulator::Exec, &summerSim, *allList);
		std::thread threadSpring(&CReflectionSimulator::Exec, &springSim, *allList);
		std::thread threadWinter(&CReflectionSimulator::Exec, &winterSim, *allList);
		threadSummer.join();
		threadSpring.join();
		threadWinter.join();

		// ��͎��s���ʃ`�F�b�N
		if (summerSim.GetExecResult() != eExitCode::Normal) { m_eExitCode = summerSim.GetExecResult(); return false; }
		if (springSim.GetExecResult() != eExitCode::Normal) { m_eExitCode = springSim.GetExecResult(); return false; }
		if (winterSim.GetExecResult() != eExitCode::Normal) { m_eExitCode = winterSim.GetExecResult(); return false; }

		// ��͌��ʏo��
		filesystem::path filepath;

		filepath = m_outDir;
		filepath /= "���˃V�~�����[�V��������_�Ď�.csv";
		summerSim.OutResult(filepath.string());
		summerResult = summerSim.GetResult();

		filepath = m_outDir;
		filepath /= "���˃V�~�����[�V��������_�t��.csv";
		springSim.OutResult(filepath.string());
		springResult = springSim.GetResult();

		filepath = m_outDir;
		filepath /= "���˃V�~�����[�V��������_�~��.csv";
		winterSim.OutResult(filepath.string());
		winterResult = winterSim.GetResult();

		// CZML�t�@�C���o��
		filesystem::path czmlfilepath;

		czmlfilepath = m_outDir;
		czmlfilepath /= "���˃V�~�����[�V��������_�Ď�.czml";
		summerSim.OutResultCZML(czmlfilepath.string());

		czmlfilepath = m_outDir;
		czmlfilepath /= "���˃V�~�����[�V��������_�t��.czml";
		springSim.OutResultCZML(czmlfilepath.string());

		czmlfilepath = m_outDir;
		czmlfilepath /= "���˃V�~�����[�V��������_�~��.czml";
		winterSim.OutResultCZML(czmlfilepath.string());

		break;
	}

	default:
		return false;
	}

	// ���ʂ��i�[
	result.push_back(summerResult);
	result.push_back(springResult);
	result.push_back(winterResult);
	result.push_back(onedayResult);

	return true;
}

// ���Q��͎��{
bool CReflectionSimulateMng::ReflectionEffect(
	const vector<CAnalysisReflectionOneDay>& result
)
{
	// ���Q��͌��ʂ��i�[����
	// key�͌���ID�Avalue�͉Ď��E�t���E�~���E�w������Ƃ̌��Q����
	map<CResultKeyData, ReflectionEffectTime> effectResult;
	// �����Ƃ̌���
	// �Ď��A�t���A�~���A�w����̏��ɓ����Ă���
	int dateCount = 0;
	for (const auto& dateResult : result)
	{
		// 1���Ԃ��Ƃ̌���
		for (const auto& oneHourResult : dateResult)
		{
			map<CResultKeyData, ReflectionEffectTime> tmpEffect;
			// 1���b�V������
			for (const auto& meshResult : oneHourResult)
			{
				// ���ː�ID
				string targetBuildingId = meshResult.reflectionTarget.buildingId;
				// ���ˌ�ID
				string reflectionBuildingId	= meshResult.reflectionRoof.buildingId;
				int index					= meshResult.reflectionRoof.buildingIndex;

				// ���������͌��Q����̃J�E���g���Ȃ�
				if (targetBuildingId == reflectionBuildingId)
					continue;

				// ���ˌ�ID���J�E���g����
				CResultKeyData keyData(reflectionBuildingId, index);
				if (dateCount == 0)
					tmpEffect[keyData].summer++;
				else if (dateCount == 1)
					tmpEffect[keyData].spring++;
				else if (dateCount == 2)
					tmpEffect[keyData].winter++;
				else if (dateCount == 3)
					tmpEffect[keyData].oneday++;
			}
			// 1������1����1�J�E���g�ɂ���
			for (const auto& effect : tmpEffect)
			{
				if (dateCount == 0)
					effectResult[effect.first].summer++;
				else if (dateCount == 1)
					effectResult[effect.first].spring++;
				else if (dateCount == 2)
					effectResult[effect.first].winter++;
				else if (dateCount == 3)
					effectResult[effect.first].oneday++;
			}
		}
		dateCount++;
	}

	// ���ʂ��t�@�C���ɏo�͂���
	filesystem::path filepath = m_outDir;
	filepath /= "�������ƌ��Q��������.csv";
	OutReflectionEffect(filepath.string(), effectResult);

	return true;
}

bool CReflectionSimulateMng::OutReflectionEffect(
	const std::string csvfile,
	const map<CResultKeyData, ReflectionEffectTime>& effectResult
)
{
	ofstream ofs;
	ofs.open(csvfile);
	if (!ofs.is_open())
		return false;

	// �w�b�_�[��
	ofs << "�G���AID,���b�V��ID,����ID,�Ď�(����),�t��(����),�~��(����),�w���(����)" << endl;

	// �f�[�^��
	for (const auto& [key, value] : effectResult)
	{
		// �G���AID, ���b�V��ID���擾
		string areaID;
		string meshID;
		if (GetIDs(key.buildingId, areaID, meshID))
		{
			ofs << areaID << ","
				<< meshID << ","
				<< key.buildingId << ","
				<< value.summer << ","
				<< value.spring << ","
				<< value.winter << ","
				<< value.oneday << endl;
		}
	}

	ofs.close();

	return true;
}

// ����������G���AID, ���b�V��ID���擾
bool CReflectionSimulateMng::GetIDs(const string& buildingID, string& areaID, string& meshID) const
{
	// �G���A�����擾
	vector<AREADATA>* allList;
	allList = reinterpret_cast<vector<AREADATA>*>(GetAllAreaList());
	if (!allList)
		return false;

	bool ret = false;

	for (const auto& area : *allList)
	{
		// �G���A���̌����f�[�^�擾
		for (const auto& bldList : area.neighborBldgList)
		{
			// �������X�g���擾
			for (const auto& building : bldList->buildingList)
			{
				if (building.building == buildingID)
				{
					ret = true;
					areaID = area.areaID;
					meshID = bldList->meshID;
					break;
				}
			}

			// �������Ă���̂Ń��[�v�𔲂���
			if (ret)
				break;
		}
		// �������Ă���̂Ń��[�v�𔲂���
		if (ret)
			break;
	}

	return ret;
}
