#pragma once
#include <string>
#include <vector>

// 対象
enum class eTarget
{
	TARGET_NONE = 0,		// 不明(エラー)
	TARGET_BUILD = 1,		// 建物
	TARGET_LAND = 2,		// 土地
};


// 建築構造の種類
enum class ePriority
{
	PRIORITY_RANK_UNKNOWN = 0,		// 優先度ランクなし
	PRIORITY_RANK_1 = 1,			// 優先度ランク1
	PRIORITY_RANK_2 = 2,			// 優先度ランク2
	PRIORITY_RANK_3 = 3,			// 優先度ランク3
	PRIORITY_RANK_4 = 4,			// 優先度ランク4
	PRIORITY_RANK_5 = 5,			// 優先度ランク5
};

struct ResultJudgment
{
	// エリアID
	std::string m_strAreaId;
	// メッシュID
	int m_iMeshId;
	// 建物ID
	std::string m_strBuildingId;
	// 優先度
	ePriority m_ePriority;
	// 各判定条件の適・不適
	std::string m_strSuitable1_1_1;			// 判定条件1_1_1
	std::string m_strSuitable1_1_2;			// 判定条件1_1_2
	std::string m_strSuitable1_2;			// 判定条件1_2
	std::string m_strSuitable1_3;			// 判定条件1_3
	std::string m_strSuitable2_1;			// 判定条件2_1
	std::string m_strSuitable2_2;			// 判定条件2_2
	std::string m_strSuitable2_3;			// 判定条件2_3
	std::string m_strSuitable2_4;			// 判定条件2_4
	std::string m_strSuitable3_1;			// 判定条件3_1
	std::string m_strSuitable3_2;			// 判定条件3_2
	std::string m_strSuitable3_3;			// 判定条件3_3

	// コンストラクタ
	ResultJudgment()
		: m_strAreaId("")
		, m_iMeshId(0)
		, m_strBuildingId("")
		, m_ePriority(ePriority::PRIORITY_RANK_5)
		, m_strSuitable1_1_1("-")
		, m_strSuitable1_1_2("-")
		, m_strSuitable1_2("-")
		, m_strSuitable1_3("-")
		, m_strSuitable2_1("-")
		, m_strSuitable2_2("-")
		, m_strSuitable2_3("-")
		, m_strSuitable2_4("-")
		, m_strSuitable3_1("-")
		, m_strSuitable3_2("-")
		, m_strSuitable3_3("-")
	{
	};
};

class CResultJudgment
{
public:
	CResultJudgment(void);
	~CResultJudgment(void);

	// 結果データの追加
	void Add(const ResultJudgment& result)
	{
		m_result.push_back(result);
	}
	// 結果データ数
	size_t GetSize()
	{
		return m_result.size();
	};

	// 優先度を設定する
	void Prioritization();

	// 優先度を取得する
	ePriority GetPriority(std::string strBuildingId);

	// 判定結果CSV出力
	bool OutputResultCSV(const std::wstring& filepath);

	// 対象を設定する
	void SetTarget(eTarget target) { m_eTarget = target; };

private:
	std::vector<ResultJudgment> m_result;
	eTarget m_eTarget;

};




