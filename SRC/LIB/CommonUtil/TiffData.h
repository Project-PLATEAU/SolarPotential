#pragma once
#include <vector>
#include "CGeoUtil.h"
#include "StringEx.h"

// TIFF(画像)出力用データクラス

class CTiffData
{
public:
	CTiffData();
	CTiffData(std::vector<CPointBase>* pData);
	~CTiffData();

private:
	// 出力データ
	std::vector<CPointBase>* m_pData;
	// 点群データの各座標最小値
	CPointBase m_minPoint;
	// 点群データの各座標最大値
	CPointBase m_maxPoint;

	// ファイル名
	std::wstring m_fileName;

public:
	// 初期化
	void Initialize();
	// データ解析
	void Analysis();

	bool CalcMaxMinPoint();

	const CPointBase GetPointMin();
	const CPointBase GetPointMax();

	void SetData(std::vector<CPointBase>* pData) { m_pData = pData; };
	const std::vector<CPointBase>* GetData() { return m_pData; };

	void SetFileName(const std::wstring& str) { m_fileName = str; };
	const std::wstring GetFileName() { return m_fileName; };

};

