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

void CReflectionSimulateMng::Exec(const std::string& outDir, UIParam* pUIParam, int year)
{
	m_outDir = outDir;
	m_year = year;
	m_pParam = pUIParam;

	// 夏至、春分、冬至の順で反射シミュレーション結果を取得する
	vector<CAnalysisReflectionOneDay> result;
	bool res = ReflectionSim(result);
	if (!res)
	{
		return;
	}

	// 光害計算
	ReflectionEffect(result);
}

// 反射シミュレーション解析実施
bool CReflectionSimulateMng::ReflectionSim(
	vector<CAnalysisReflectionOneDay>& result
)
{
	// 夏至・冬至・春分の日付取得
	CTime summer = CTime::GetSummerSolstice(m_year);
	CTime winter = CTime::GetWinterSolstice(m_year);
	CTime sprint = CTime::GetVernalEquinox(m_year);

	CReflectionSimulator summerSim(summer, m_pParam);
	CReflectionSimulator winterSim(winter, m_pParam);
	CReflectionSimulator springSim(sprint, m_pParam);


	// 建物情報を取得
	vector<BLDGLIST>* allList;
	allList = reinterpret_cast<vector<BLDGLIST>*>(GetAllList());


#	// スレッド
	std::thread threadSummer(&CReflectionSimulator::Exec, &summerSim, *allList, *allList);
	std::thread threadSpring(&CReflectionSimulator::Exec, &springSim, *allList, *allList);
	std::thread threadWinter(&CReflectionSimulator::Exec, &winterSim, *allList, *allList);
	threadSummer.join();
	threadSpring.join();
	threadWinter.join();

	// 解析実行結果チェック
	if (!summerSim.GetExecResult() ||
		!springSim.GetExecResult() ||
		!winterSim.GetExecResult())
	{
		return false;
	}


	// 解析結果出力
	filesystem::path filepath;

	filepath = m_outDir;
	filepath /= "1_summer.csv";
	summerSim.OutResult(filepath.string());
	CAnalysisReflectionOneDay summerResult = summerSim.GetResult();
	result.push_back(summerResult);

	filepath = m_outDir;
	filepath /= "2_spring.csv";
	springSim.OutResult(filepath.string());
	CAnalysisReflectionOneDay springResult = springSim.GetResult();
	result.push_back(springResult);

	filepath = m_outDir;
	filepath /= "3_winter.csv";
	winterSim.OutResult(filepath.string());
	CAnalysisReflectionOneDay winterResult = winterSim.GetResult();
	result.push_back(winterResult);


	// CZMLファイル出力
	filesystem::path czmlfilepath;

	czmlfilepath = m_outDir;
	czmlfilepath /= "summer.czml";
	summerSim.OutResultCZML(czmlfilepath.string());

	czmlfilepath = m_outDir;
	czmlfilepath /= "spring.czml";
	springSim.OutResultCZML(czmlfilepath.string());

	czmlfilepath = m_outDir;
	czmlfilepath /= "winter.czml";
	winterSim.OutResultCZML(czmlfilepath.string());

	return true;
}

// 光害解析実施
bool CReflectionSimulateMng::ReflectionEffect(
	const vector<CAnalysisReflectionOneDay>& result
)
{
	// 光害解析結果を格納する
	// keyは建物ID、valueは夏至・春分・冬至ごとの光害時間
	map<CResultKeyData, ReflectionEffectTime> effectResult;
	// 日毎の結果
	// 夏至、春分、冬至の順に入っている
	int dateCount = 0;
	for (const auto& dateResult : result)
	{
		// 1時間ごとの結果
		for (const auto& oneHourResult : dateResult)
		{
			map<CResultKeyData, ReflectionEffectTime> tmpEffect;
			// 1メッシュ毎
			for (const auto& meshResult : oneHourResult)
			{
				// 反射先ID
				string targetBuildingId = meshResult.reflectionTarget.buildingId;
				// 反射元ID
				string reflectionBuildingId	= meshResult.reflectionRoof.buildingId;
				int index					= meshResult.reflectionRoof.buildingIndex;

				// 同じ建物は光害判定のカウントしない
				if (targetBuildingId == reflectionBuildingId)
					continue;

				// 反射元IDをカウントする
				CResultKeyData keyData(reflectionBuildingId, index);
				if (dateCount == 0)
					tmpEffect[keyData].summer++;
				else if (dateCount == 1)
					tmpEffect[keyData].spring++;
				else if (dateCount == 2)
					tmpEffect[keyData].winter++;
			}
			// 1建物は1時間1カウントにする
			for (const auto& effect : tmpEffect)
			{
				if (dateCount == 0)
					effectResult[effect.first].summer++;
				else if (dateCount == 1)
					effectResult[effect.first].spring++;
				else if (dateCount == 2)
					effectResult[effect.first].winter++;
			}
		}
		dateCount++;
	}

	// 結果をファイルに出力する
	filesystem::path filepath = m_outDir;
	filepath /= "建物毎光害発生時間.csv";
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

	// ヘッダー部
	ofs << "メッシュID,建物ID,夏至,春分,冬至" << endl;

	// データ部
	for (const auto& [key, value] : effectResult)
	{
		// メッシュIDをを取得
		string meshID;
		GetMeshID(key.buildingId, meshID);

		ofs << meshID << ","
			<< key.buildingId << ","
			<< value.summer << ","
			<< value.spring << ","
			<< value.winter << endl;
	}

	ofs.close();

	return true;
}

// 建物名からメッシュIDを取得
bool CReflectionSimulateMng::GetMeshID(const string& buildingID, string& meshID) const
{
	// 建物情報を取得
	vector<BLDGLIST>* allList;
	allList = reinterpret_cast<vector<BLDGLIST>*>(GetAllList());
	if (!allList)
		return false;

	bool ret = false;

	for (const auto& mesh : *allList)
	{
		for (const auto& building : mesh.buildingList)
		{
			if (building.building == buildingID)
			{
				ret = true;
				meshID = mesh.meshID;
				break;
			}
		}
		// 見つかっているのでループを抜ける
		if (ret)
			break;

		for (const auto& building : mesh.buildingListLOD1)
		{
			if (building.building == buildingID)
			{
				ret = true;
				meshID = mesh.meshID;
				break;
			}
		}
		// 見つかっているのでループを抜ける
		if (ret)
			break;
	}

	return ret;
}
