#pragma once

// 終了コード(各DLL共通)
enum eExitCode
{
	// 共通
	Normal = 0,					// 正常終了
	Error = 1,					// 処理エラー
	Cancel = 2,					// キャンセル

	// 解析・シミュレーション
	Err_KashoData = 10,			// 可照時間データ不正エラー
	Err_NisshoData = 11,		// 日照時間データ不正エラー
	Err_SnowDepthData = 12,		// 積雪深データ不正エラー
	Err_NoTarget = 13,			// 対象データ無し


};
