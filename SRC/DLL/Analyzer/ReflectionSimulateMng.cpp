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

	// 夏至、春分、冬至、指定日の順で反射シミュレーション結果を取得する
	vector<CAnalysisReflectionOneDay> result;
	bool res = ReflectionSim(result);
	if (!res)
	{
		return false;
	}

	// 光害計算
	res = ReflectionEffect(result);

	if (res)
	{
		m_eExitCode = eExitCode::Normal;
	}

	return res;
}

// 反射シミュレーション解析実施
bool CReflectionSimulateMng::ReflectionSim(
	vector<CAnalysisReflectionOneDay>& result
)
{
	// 解析エリア情報を取得
	vector<AREADATA>* allList;
	allList = reinterpret_cast<vector<AREADATA>*>(GetAllAreaList());

	// 結果格納用
	CAnalysisReflectionOneDay summerResult;
	CAnalysisReflectionOneDay springResult;
	CAnalysisReflectionOneDay winterResult;
	CAnalysisReflectionOneDay onedayResult;

	switch (m_pParam->eAnalyzeDate)
	{
	case eDateType::OneMonth:
	{
		int nDay = 1;	// 1日で固定

		// 夏至・冬至・春分の日付取得
		CTime date = CTime(m_year, m_pParam->nMonth, nDay, 0, 0, 0);
		CReflectionSimulator refSim(date, m_pParam);

		// スレッド
		std::thread thread(&CReflectionSimulator::Exec, &refSim, *allList);
		thread.join();

		// 解析実行結果チェック
		if (refSim.GetExecResult() != eExitCode::Normal) { m_eExitCode = refSim.GetExecResult(); return false; }

		// 解析結果出力
		filesystem::path filepath;

		filepath = m_outDir;
		filepath /= CStringEx::Format("反射シミュレーション結果_%02d%02d.csv", m_pParam->nMonth, nDay);
		refSim.OutResult(filepath.string());
		onedayResult = refSim.GetResult();

		// CZMLファイル出力
		filesystem::path czmlfilepath;

		czmlfilepath = m_outDir;
		czmlfilepath /= CStringEx::Format("反射シミュレーション結果_%02d%02d.czml", m_pParam->nMonth, nDay);
		refSim.OutResultCZML(czmlfilepath.string());

		break;
	}
	case eDateType::OneDay:
	{
		// 夏至・冬至・春分の日付取得
		CTime date = CTime(m_year, m_pParam->nMonth, m_pParam->nDay, 0, 0, 0);
		CReflectionSimulator refSim(date, m_pParam);

		// スレッド
		std::thread thread(&CReflectionSimulator::Exec, &refSim, *allList);
		thread.join();

		// 解析実行結果チェック
		if (refSim.GetExecResult() != eExitCode::Normal) { m_eExitCode = refSim.GetExecResult(); return false; }

		// 解析結果出力
		filesystem::path filepath;

		filepath = m_outDir;
		filepath /= CStringEx::Format("反射シミュレーション結果_%02d%02d.csv", m_pParam->nMonth, m_pParam->nDay);
		refSim.OutResult(filepath.string());
		onedayResult = refSim.GetResult();

		// CZMLファイル出力
		filesystem::path czmlfilepath;

		czmlfilepath = m_outDir;
		czmlfilepath /= CStringEx::Format("反射シミュレーション結果_%02d%02d.czml", m_pParam->nMonth, m_pParam->nDay);
		refSim.OutResultCZML(czmlfilepath.string());

		break;
	}
	case eDateType::Summer:
	{
		// 夏至の日付取得
		CTime summer = CTime::GetSummerSolstice(m_year);
		CReflectionSimulator summerSim(summer, m_pParam);

		// スレッド
		std::thread threadSummer(&CReflectionSimulator::Exec, &summerSim, *allList);
		threadSummer.join();

		// 解析実行結果チェック
		if (summerSim.GetExecResult() != eExitCode::Normal) { m_eExitCode = summerSim.GetExecResult(); return false; }

		// 解析結果出力
		filesystem::path filepath;

		filepath = m_outDir;
		filepath /= "反射シミュレーション結果_夏至.csv";
		summerSim.OutResult(filepath.string());
		summerResult = summerSim.GetResult();

		// CZMLファイル出力
		filesystem::path czmlfilepath;

		czmlfilepath = m_outDir;
		czmlfilepath /= "反射シミュレーション結果_夏至.czml";
		summerSim.OutResultCZML(czmlfilepath.string());

		break;
	}

	case eDateType::Winter:
	{
		// 冬至の日付取得
		CTime winter = CTime::GetWinterSolstice(m_year);
		CReflectionSimulator winterSim(winter, m_pParam);

		// スレッド
		std::thread threadWinter(&CReflectionSimulator::Exec, &winterSim, *allList);
		threadWinter.join();

		// 解析実行結果チェック
		if (winterSim.GetExecResult() != eExitCode::Normal) { m_eExitCode = winterSim.GetExecResult(); return false; }

		// 解析結果出力
		filesystem::path filepath;

		filepath = m_outDir;
		filepath /= "反射シミュレーション結果_冬至.csv";
		winterSim.OutResult(filepath.string());
		winterResult = winterSim.GetResult();

		// CZMLファイル出力
		filesystem::path czmlfilepath;

		czmlfilepath = m_outDir;
		czmlfilepath /= "反射シミュレーション結果_冬至.czml";
		winterSim.OutResultCZML(czmlfilepath.string());

		break;
	}
	case eDateType::Year:
	{
		// 夏至・冬至・春分の日付取得
		CTime summer = CTime::GetSummerSolstice(m_year);
		CTime winter = CTime::GetWinterSolstice(m_year);
		CTime sprint = CTime::GetVernalEquinox(m_year);

		CReflectionSimulator summerSim(summer, m_pParam);
		CReflectionSimulator winterSim(winter, m_pParam);
		CReflectionSimulator springSim(sprint, m_pParam);

		// スレッド
		std::thread threadSummer(&CReflectionSimulator::Exec, &summerSim, *allList);
		std::thread threadSpring(&CReflectionSimulator::Exec, &springSim, *allList);
		std::thread threadWinter(&CReflectionSimulator::Exec, &winterSim, *allList);
		threadSummer.join();
		threadSpring.join();
		threadWinter.join();

		// 解析実行結果チェック
		if (summerSim.GetExecResult() != eExitCode::Normal) { m_eExitCode = summerSim.GetExecResult(); return false; }
		if (springSim.GetExecResult() != eExitCode::Normal) { m_eExitCode = springSim.GetExecResult(); return false; }
		if (winterSim.GetExecResult() != eExitCode::Normal) { m_eExitCode = winterSim.GetExecResult(); return false; }

		// 解析結果出力
		filesystem::path filepath;

		filepath = m_outDir;
		filepath /= "反射シミュレーション結果_夏至.csv";
		summerSim.OutResult(filepath.string());
		summerResult = summerSim.GetResult();

		filepath = m_outDir;
		filepath /= "反射シミュレーション結果_春分.csv";
		springSim.OutResult(filepath.string());
		springResult = springSim.GetResult();

		filepath = m_outDir;
		filepath /= "反射シミュレーション結果_冬至.csv";
		winterSim.OutResult(filepath.string());
		winterResult = winterSim.GetResult();

		// CZMLファイル出力
		filesystem::path czmlfilepath;

		czmlfilepath = m_outDir;
		czmlfilepath /= "反射シミュレーション結果_夏至.czml";
		summerSim.OutResultCZML(czmlfilepath.string());

		czmlfilepath = m_outDir;
		czmlfilepath /= "反射シミュレーション結果_春分.czml";
		springSim.OutResultCZML(czmlfilepath.string());

		czmlfilepath = m_outDir;
		czmlfilepath /= "反射シミュレーション結果_冬至.czml";
		winterSim.OutResultCZML(czmlfilepath.string());

		break;
	}

	default:
		return false;
	}

	// 結果を格納
	result.push_back(summerResult);
	result.push_back(springResult);
	result.push_back(winterResult);
	result.push_back(onedayResult);

	return true;
}

// 光害解析実施
bool CReflectionSimulateMng::ReflectionEffect(
	const vector<CAnalysisReflectionOneDay>& result
)
{
	// 光害解析結果を格納する
	// keyは建物ID、valueは夏至・春分・冬至・指定日ごとの光害時間
	map<CResultKeyData, ReflectionEffectTime> effectResult;
	// 日ごとの結果
	// 夏至、春分、冬至、指定日の順に入っている
	int dateCount = 0;
	for (const auto& dateResult : result)
	{
		// 1時間ごとの結果
		for (const auto& oneHourResult : dateResult)
		{
			map<CResultKeyData, ReflectionEffectTime> tmpEffect;
			// 1メッシュごと
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
				else if (dateCount == 3)
					tmpEffect[keyData].oneday++;
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
				else if (dateCount == 3)
					effectResult[effect.first].oneday++;
			}
		}
		dateCount++;
	}

	// 結果をファイルに出力する
	filesystem::path filepath = m_outDir;
	filepath /= "建物ごと光害発生時間.csv";
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
	ofs << "エリアID,メッシュID,建物ID,夏至(時間),春分(時間),冬至(時間),指定日(時間)" << endl;

	// データ部
	for (const auto& [key, value] : effectResult)
	{
		// エリアID, メッシュIDを取得
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

// 建物名からエリアID, メッシュIDを取得
bool CReflectionSimulateMng::GetIDs(const string& buildingID, string& areaID, string& meshID) const
{
	// エリア情報を取得
	vector<AREADATA>* allList;
	allList = reinterpret_cast<vector<AREADATA>*>(GetAllAreaList());
	if (!allList)
		return false;

	bool ret = false;

	for (const auto& area : *allList)
	{
		// エリア内の建物データ取得
		for (const auto& bldList : area.neighborBldgList)
		{
			// 建物リストを取得
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

			// 見つかっているのでループを抜ける
			if (ret)
				break;
		}
		// 見つかっているのでループを抜ける
		if (ret)
			break;
	}

	return ret;
}
