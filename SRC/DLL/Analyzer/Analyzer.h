// 以下の ifdef ブロックは、DLL からのエクスポートを容易にするマクロを作成するための
// 一般的な方法です。この DLL 内のすべてのファイルは、コマンド ラインで定義された ANALYZER_EXPORTS
// シンボルを使用してコンパイルされます。このシンボルは、この DLL を使用するプロジェクトでは定義できません。
// ソースファイルがこのファイルを含んでいる他のプロジェクトは、
// ANALYZER_API 関数を DLL からインポートされたと見なすのに対し、この DLL は、このマクロで定義された
// シンボルをエクスポートされたと見なします。
#ifdef ANALYZER_EXPORTS
#define ANALYZER_API __declspec(dllexport)
#else
#define ANALYZER_API __declspec(dllimport)
#endif

// CS側で使用する関数を定義
extern "C" {

	// 解析処理開始
	ANALYZER_API bool AnalyzeStart(const char* outDir);

	ANALYZER_API void SetPossibleSunshineDataPath(char* path);		// 月毎の可照時間
	ANALYZER_API void SetAverageSunshineDataPath(char* path);		// 毎月の平均日照時間
	ANALYZER_API void SetMetpvDataPath(char* path);					// 月毎の積雪深
	ANALYZER_API void SetOutputPath(char* path);					// 出力フォルダ

	ANALYZER_API void InitializeUIParam();

	// 解析処理
	ANALYZER_API void SetElecPotential(double d1, int e, double d2, double d3);				// 発電ポテンシャル推計
	ANALYZER_API void SetRoofSurfaceCorrect(double d1, double d2);							// 屋根面補正
	ANALYZER_API void SetAreaSolarPower(double d);											// 太陽光パネル単位面積当たりの発電容量
	ANALYZER_API void SetReflectRoofCorrect_Lower(bool b1, bool b2, int e, double d);	// 反射シミュレーション時の屋根面の向き・傾き補正(3度未満)
	ANALYZER_API void SetReflectRoofCorrect_Upper(bool b1, bool b2, int e, double d);	// 反射シミュレーション時の屋根面の向き・傾き補正(3度以上)
	ANALYZER_API void SetEnableDEMData(bool b);											// DEMデータを使用するか
	ANALYZER_API void SetExecSolarPotantial(bool b);									// 発電ポテンシャル推計実行フラグ
	ANALYZER_API void SetExecReflection(bool b);										// 反射シミュレーション実行フラグ

}
