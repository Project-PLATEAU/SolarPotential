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
	// 光害時間
	struct ReflectionEffectTime
	{
		int summer{ 0 };	// 夏至
		int spring{ 0 };	// 春分
		int winter{ 0 };	// 冬至
		int oneday{ 0 };	// 指定日
	};

	// 解析結果格納先
	std::string m_outDir;
	// 解析年
	int m_year;
	// 設定パラメータ
	CUIParam* m_pParam;

	// 終了コード
	eExitCode m_eExitCode;

	bool ReflectionSim(std::vector<CAnalysisReflectionOneDay>& result);

	bool ReflectionEffect(const std::vector<CAnalysisReflectionOneDay>& result);

	bool OutReflectionEffect(const std::string csvfile,
		const std::map<CResultKeyData, ReflectionEffectTime>& effectResult);

	// 建物名からエリアID, メッシュIDを取得
	bool GetIDs(const std::string& buildingID, std::string& areaID, std::string& meshID) const;

};

class CResultKeyData
{
public:
	string buildingId;	// 建物ID
	int index;			// 建物ID並び順

public:
	CResultKeyData() : buildingId(""), index(0) {}
	CResultKeyData(const string& buildingId, int index)
		: buildingId(buildingId), index(index) {}

	bool operator <(const CResultKeyData& a) const
	{
		// indexで比較して同じであればbuildingIdで比較する
		return tie(index, buildingId) < tie(a.index, a.buildingId);
	}
};
