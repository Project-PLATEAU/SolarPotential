#pragma once
#include <vector>
#include <map>
#include <string>
#include "ReflectionSimulator.h"
#include "UIParam.h"

class CResultKeyData;

class CReflectionSimulateMng
{
public:
	CReflectionSimulateMng()
		: m_outDir("")
		, m_year(2021)
		, m_pParam(nullptr)
	{}

	void Exec(const std::string& outDir, UIParam* pUIParam, int year);

private:
	// ŒõŠQŠÔ
	struct ReflectionEffectTime
	{
		int summer{ 0 };	// ‰ÄŠ
		int spring{ 0 };	// t•ª
		int winter{ 0 };	// “~Š
	};

	// ‰ğÍŒ‹‰ÊŠi”[æ
	std::string m_outDir;
	// ‰ğÍ”N
	int m_year;
	// İ’èƒpƒ‰ƒ[ƒ^
	UIParam* m_pParam;

	bool ReflectionSim(std::vector<CAnalysisReflectionOneDay>& result);

	bool ReflectionEffect(const std::vector<CAnalysisReflectionOneDay>& result);

	bool OutReflectionEffect(const std::string csvfile,
		const std::map<CResultKeyData, ReflectionEffectTime>& effectResult);

	// Œš•¨–¼‚©‚çƒƒbƒVƒ…ID‚ğæ“¾
	bool GetMeshID(const std::string& buildingID, std::string& meshID) const;

};

class CResultKeyData
{
public:
	string buildingId;	// Œš•¨ID
	int index;			// Œš•¨ID•À‚Ñ‡

public:
	CResultKeyData() : buildingId(""), index(0) {}
	CResultKeyData(const string& buildingId, int index)
		: buildingId(buildingId), index(index) {}

	bool operator <(const CResultKeyData& a) const
	{
		// index‚Å”äŠr‚µ‚Ä“¯‚¶‚Å‚ ‚ê‚ÎbuildingId‚Å”äŠr‚·‚é
		return tie(index, buildingId) < tie(a.index, a.buildingId);
	}
};
