#pragma once

#include <string>
#include <map>

#include <CommonUtil/CTime.h>
#include <CommonUtil/CSunVector.h>
#include "AnalysisReflectionMesh.h"
#include "AnalyzeData.h"
#include "UIParam.h"
#include "CommonUtil/CGeoid.h"
#include "CommonUtil/ExitCode.h"

typedef std::vector<CAnalysisReflectionMesh> CAnalysisReflection;
typedef std::vector<CAnalysisReflection> CAnalysisReflectionOneDay;

class CReflectionSimulator
{
public:
	// 反射シミュレーション解析を行う年月日を指定する
	// 設定した日の24時間で解析を行う
	CReflectionSimulator(const CTime& date, CUIParam* pUIParam);
	virtual ~CReflectionSimulator();

	// 反射シミュレーション解析を実行する
	// 反射光が建物にあたっている解析結果があるときtrueを返す
	eExitCode Exec(
		const vector<AREADATA>& analyzeAreas);			// 解析エリア

	// 結果をCSVファイルに出力する
	// 結果はExec()を実施後のtrueが返ったときに格納されたデータを出力する
	bool OutResult(const std::string& csvfile) const;

	// 結果をCZMLファイルに出力する
	bool OutResultCZML(const std::string& czmlfile) const;

	// 結果CSVをCZMLに変換する
	static bool ConvertResultCSVtoCZML(const std::string& csvfile, const std::string& czmlfile);

	// 解析結果の24時間分データを取得する
	// 結果はExec()を実施後にtrueが返ったときに格納されたデータを出力する
	const std::vector<CAnalysisReflection>& GetResult() const;

	// Exec()実行結果
	eExitCode GetExecResult() const;

private:
	// 年月日
	CTime m_date;

	// 太陽光ベクトル
	CSunVector* m_pSunVector{ nullptr };

	// 24時間分の反射シミュレーションの解析結果を格納
	CAnalysisReflectionOneDay m_result;

	// 屋根面の向き・傾きの補正
	CReflectionCorrect m_roofCorrectLower;	// 屋根面傾き3度未満の補正
	CReflectionCorrect m_roofCorrectUpper;	// 屋根面傾き3度以上の補正

	// Exec()実行結果
	eExitCode m_bExec;

	CUIParam* m_pParam;

	// 処理中の建物IDの数、建物IDの順番を管理する
	int m_buildingIndex;
	map<string, int> m_mapBuildingIndex;

	// 全て初期化
	void Init();

	// 解析結果の初期化
	void InitResult();

	// 太陽位置の緯度経度を設定する
	// 太陽位置を元に反射シミュレーションの解析を行う
	void SetSunVector(double lat, double lon);

	// 解析結果の設定
	void SetResult(uint8_t hour, const CAnalysisReflection& result);

	// 全建物での解析
	bool AnalyzeBuildings(
		const std::vector<BLDGLIST>& targetBuildings,
		const std::vector<BLDGLIST*>& buildings,
		uint8_t hour,
		CAnalysisReflection& result);

	// 1建物での解析
	bool AnalyzeBuilding(
		const BUILDINGS& building,
		const std::vector<BLDGLIST>& buildings,
		const CVector3D& sunVector,
		CAnalysisReflection& result);

	// 1屋根での解析
	bool AnalyzeRoof(
		const ROOFSURFACES& roof, const BUILDINGS& building,
		const std::vector<BLDGLIST>& buildings,
		const CVector3D& sunVector,
		CAnalysisReflection& result);

	// 1屋根メッシュでの解析
	bool AnalyzeMesh(
		const MESHPOSITION_XY& mesh,
		const vector<CPointBase>& posList,
		const std::vector<BLDGLIST>& buildings,
		const CVector3D& sunVector,
		CAnalysisReflectionMesh& result);

	// 土地での解析
	bool AnalyzeLand(
		const AREADATA& targetArea,
		const std::vector<BLDGLIST*>& buildings,
		uint8_t hour,
		CAnalysisReflection& result);

	// 対象メッシュの隣接するメッシュを取得
	void GetNeighborBuildings(
		const BLDGLIST& targetBuildings,			// 対象メッシュ
		const std::vector<BLDGLIST*>& buildings,	// 全メッシュ
		std::vector<BLDGLIST>& neighborBuildings);	// 近隣メッシュ

	// 対象メッシュの隣接するメッシュを取得(土地面)
	void GetNeighborBuildings(
		const AREADATA& targetArea,
		const std::vector<BLDGLIST*>& buildings,
		std::vector<BLDGLIST>& neighborBuildings);

	// キャンセル中かどうか
	bool IsCancel();

	// 結果に建物IDの順番を設定する
	void SetBuildingIndex(CAnalysisReflection& result);

	// 結果に建物IDの順番を設定する
	void SetAreaIndex(CAnalysisReflection& result);

	// 結果をCZMLファイルに出力する
	static bool OutResultCZML(const std::string& czmlfile, const CAnalysisReflectionOneDay& result);

	// CZMLのLineを出力
	static void OutResultCZMLLine(
		ofstream& ofs, const CAnalysisReflectionOneDay& result,
		int JPZONE, CGeoid* pGeoid, const string& rgba);

	// CZMLのPointを出力
	static void OutResultCZMLPoint(
		ofstream& ofs, const CAnalysisReflectionOneDay& result,
		int JPZONE, CGeoid* pGeoid, const string& rgba, const string& czmlfilename);

	// XYZをCZMLの緯度経度、高さに変換する
	static void ConvertXYZToLatLonZ(
		double x, double y, double z,
		int JPZONE, CGeoid* pGeoid,
		double& lat, double& lon, double& h);

	// 結果ファイルの全行を読み込む
	static bool ReadResultCSVLines(const string& csvfile, std::vector<string>& lines);
};
