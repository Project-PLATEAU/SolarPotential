#pragma once
#include "pch.h"
#include "../../LIB/CommonUtil/CINIFileIO.h"

/*! 優先度設定ファイル読み込みクラス
	読み取り専用
*/
class CJudgeSuitablePlacePriorityIniFile : public CINIFileIO
{
public:
	CJudgeSuitablePlacePriorityIniFile(std::string strFilePath)
	{
		// iniファイルOPEN
		this->Open(strFilePath);
	}
	~CJudgeSuitablePlacePriorityIniFile(void)
	{
		this->Close();
	}

	// 優先度の判定基準
	/*! 優先度ランク5
	*/
	int GetCriterion_5()
	{
		return this->GetInt("Criterion", "JudgementCriterion_5", 0);
	}
	/*! 優先度ランク4
	*/
	int GetCriterion_4()
	{
		return this->GetInt("Criterion", "JudgementCriterion_4", -5);
	}
	/*! 優先度ランク3
	*/
	int GetCriterion_3()
	{
		return this->GetInt("Criterion", "JudgementCriterion_3", -15);
	}
	/*! 優先度ランク2
	*/
	int GetCriterion_2()
	{
		return this->GetInt("Criterion", "JudgementCriterion_2", -25);
	}
	// 建物に付随する条件のポイントの設定
	/*! 日射量が少ない施設
	*/
	int GetJudgementCondition_1_1()
	{
		return this->GetInt("Building", "JudgementCondition_1_1", -1);
	}
	/*! 建物構造による除外
	*/
	int GetJudgementCondition_1_2()
	{
		return this->GetInt("Building", "JudgementCondition_1_2", -1);
	}
	/*! 特定の階層の施設
	*/
	int GetJudgementCondition_1_3()
	{
		return this->GetInt("Building", "JudgementCondition_1_3", -1);
	}
	// 災害時のリスクによる条件のポイントの設定
	/*! 高さが想定される最大津波高さを下回る建物
	*/
	int GetJudgementCondition_2_1()
	{
		return this->GetInt("Hazard", "JudgementCondition_2_1", -1);
	}
	/*! 建物高さが想定される河川浸水想定浸水深を下回る建物
	*/
	int GetJudgementCondition_2_2()
	{
		return this->GetInt("Hazard", "JudgementCondition_2_2", -1);
	}
	/*! 土砂災害警戒区域内に存在する建物
	*/
	int GetJudgementCondition_2_3()
	{
		return this->GetInt("Hazard", "JudgementCondition_2_3", -1);
	}
	/*! 積雪が多い地域の建物
	*/
	int GetJudgementCondition_2_4()
	{
		return this->GetInt("Hazard", "JudgementCondition_2_4", -1);
	}
	// 区域による条件のポイントの設定
	/*! 設置に制限がある区域1
	*/
	int GetJudgementCondition_3_1()
	{
		return this->GetInt("Restrict", "JudgementCondition_3_1", -1);
	}
	/*! 設置に制限がある区域2
	*/
	int GetJudgementCondition_3_2()
	{
		return this->GetInt("Restrict", "JudgementCondition_3_2", -1);
	}
	/*! 設置に制限がある区域3
	*/
	int GetJudgementCondition_3_3()
	{
		return this->GetInt("Restrict", "JudgementCondition_3_3", -1);
	}
};


