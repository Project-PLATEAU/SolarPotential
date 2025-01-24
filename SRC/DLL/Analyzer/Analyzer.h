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

	// 入力データ
    struct AnalyzeInputData
    {
        // 入力データ
		char* strKashoData;
		char* strNisshoData;
		char* strSnowDepthData;
		char* strLandData;
		bool bUseDemData;
    };

	// 発電ポテンシャル推計条件
	struct SolarPotentialParam
	{
		// 屋根面
		double	dArea2D_Roof;			// 除外する面積
		int		eDirection_Roof;		// 除外する方位
		double	dDirectionDegree_Roof;	// 除外する方位＋傾き
		double	dSlopeDegree_Roof;		// 除外する屋根面の傾き
		double	dCorrectionCaseDeg_Roof;			// 補正対象となる傾き(基準)
		int		eCorrectionDirection_Roof;			// 補正する方位
		double	dCorrectionDirectionDegree_Roof;	// 補正する方位＋傾き
		bool	bExclusionInterior_Roof;			// interior面の除外(除外:true)

		// 土地面
		double	dArea2D_Land;			// 除外する面積
		double	dSlopeDegree_Land;		// 除外する屋根面の傾き
		int		eCorrectionDirection_Land;			// 補正する方位
		double	dCorrectionDirectionDegree_Land;	// 補正する方位＋傾き

		double	dPanelMakerSolarPower;	// 発電容量
		double	dPanelRatio;			// パネル設置割合
	};

	// 反射シミュレーション条件
	struct ReflectionParam
	{
		// 屋根面3度未満
		bool	bCustom_Roof_Lower;
		int		eAzimuth_Roof_Lower;
		double	dSlopeAngle_Roof_Lower;
		// 屋根面3度以上
		bool	bCustom_Roof_Upper;
		int		eAzimuth_Roof_Upper;
		double	dSlopeAngle_Roof_Upper;

		// 土地面3度未満
		bool	bCustom_Land_Lower;
		int		eAzimuth_Land_Lower;
		double	dSlopeAngle_Land_Lower;
		// 土地面3度以上
		bool	bCustom_Land_Upper;
		int		eAzimuth_Land_Upper;
		double	dSlopeAngle_Land_Upper;

		double dReflectionRange;
	};

	struct AnalyzeParam
	{
		// 解析対象
		bool bExecSolarPotantial;		// 発電ポテンシャル推計実行フラグ
		bool bExecReflection;			// 反射シミュレーション実行フラグ
		bool bExecBuild;				// 建物シミュレーション実行フラグ
		bool bExecLand;					// 土地シミュレーション実行フラグ

	};

	struct AnalyzeDateParam
	{
		// 期間
		int eDateType;
		int iMonth;
		int iDay;

	};

	// 解析処理開始
	ANALYZER_API int AnalyzeStart(const char* outDir);

    ANALYZER_API void InitializeUIParam();

	ANALYZER_API void SetAnalyzeParam(AnalyzeParam* p);
	ANALYZER_API void SetAnalyzeInputData(AnalyzeInputData* p);
	ANALYZER_API void SetSolarPotentialParam(SolarPotentialParam* p);
	ANALYZER_API void SetReflectionParam(ReflectionParam* p);
	ANALYZER_API void SetAnalyzeDateParam(AnalyzeDateParam* p);

}