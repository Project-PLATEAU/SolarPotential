// 以下の ifdef ブロックは、DLL からのエクスポートを容易にするマクロを作成するための
// 一般的な方法です。この DLL 内のすべてのファイルは、コマンド ラインで定義された JUDGESUITABLEPLACE_EXPORTS
// シンボルを使用してコンパイルされます。このシンボルは、この DLL を使用するプロジェクトでは定義できません。
// ソースファイルがこのファイルを含んでいる他のプロジェクトは、
// JUDGESUITABLEPLACE_API 関数を DLL からインポートされたと見なすのに対し、この DLL は、このマクロで定義された
// シンボルをエクスポートされたと見なします。
#ifdef JUDGESUITABLEPLACE_EXPORTS
#define JUDGESUITABLEPLACE_API __declspec(dllexport)
#else
#define JUDGESUITABLEPLACE_API __declspec(dllimport)
#endif

// CS側で使用する関数を定義
extern "C" {
	
    // 集計画面入力パラメータ
    struct AggregateParam
    {

        // 集計範囲
        bool bAggregateRange;// 集計範囲
        double dMaxLat;      // 最大緯度
        double dMinLon;      // 最小経度
        double dMaxLon;      // 最大経度
        double dMinLat;      // 最小緯度

        // 太陽光パネルの設置に関して優先度が低い施設の判定
        // 日射量が少ない施設を除外
        double dParam_1_1_1;   // kWh/m2未満
        int iParam_1_1_2;      // 下位％

        // 建物構造による除外
        bool bParam_1_2_1;      // 木造・土蔵造
        bool bParam_1_2_2;      // 鉄骨鉄筋コンクリート造
        bool bParam_1_2_3;      // 鉄筋コンクリート造
        bool bParam_1_2_4;      // 鉄骨造
        bool bParam_1_2_5;      // 軽量鉄骨造
        bool bParam_1_2_6;      // レンガ造・コンクリートブロック造・石造
        bool bParam_1_2_7;      // 不明
        bool bParam_1_2_8;      // 非木造

        // 特定の階層の施設を除外
        int iParam_1_3_1;      // 〇階以下
        int iParam_1_3_2;      // 〇階以上

        // 災害時に太陽光パネルが破損、消失する危険性の判定
        bool bParam_2_1;                // 高さが想定される最大津波高さを下回る建物を除外
        bool bParam_2_2;                // 建物高さが想定される河川浸水想定浸水深を下回る建物を除外
        bool bParam_2_3;                // 土砂災害警戒区域内に存在する建物を除外
        char* strParam_2_4;             // 気象データ(積雪)フォルダパス
        double dParam_2_4_1;            // 気象データ(積雪)_cm以上
        double dParam_2_4_2;            // 積雪荷重(kgf/m3)
        double dParam_2_4_3;            // 単位荷重(N/m3)

        // 太陽光パネルの設置に制限がある施設の判定
        char*        strParam_3_1;            // 制限を設ける範囲のシェープファイル_フォルダパス１
        double       dParam_3_1_1;            // 制限を設ける範囲のシェープファイル_高さ１
        int          iParam_3_1_2;            // 制限を設ける範囲のシェープファイル_方位１
        int          iParam_3_1_3;            // 制限を設ける範囲のシェープファイル_座標系１
        char*        strParam_3_2;            // 制限を設ける範囲のシェープファイル_フォルダパス２
        double       dParam_3_2_1;            // 制限を設ける範囲のシェープファイル_高さ２
        int          iParam_3_2_2;            // 制限を設ける範囲のシェープファイル_方位２
        int          iParam_3_2_3;            // 制限を設ける範囲のシェープファイル_座標系２
        char*        strParam_3_3;            // 制限を設ける範囲のシェープファイル_フォルダパス３
        double       dParam_3_3_1;            // 制限を設ける範囲のシェープファイル_高さ３
        int          iParam_3_3_2;            // 制限を設ける範囲のシェープファイル_方位３
        int          iParam_3_3_3;            // 制限を設ける範囲のシェープファイル_座標系３
    };

    struct AggregateTarget
    {
        bool bExecBuild;				// 建物実行フラグ
        bool bExecLand;					// 土地実行フラグ

    };

	// パラメータ設定
    JUDGESUITABLEPLACE_API void InitializeUIParam();
	JUDGESUITABLEPLACE_API void SetAggregateParam(AggregateParam* aggregateParam);	        // 画面パラメータ
    JUDGESUITABLEPLACE_API bool SetOutputPath(char* aggregatePath);		    // 出力フォルダ
    JUDGESUITABLEPLACE_API bool SetAnalyzeResultPath(char* analyzePath);	// 解析結果フォルダ
    JUDGESUITABLEPLACE_API void SetAggregateTarget(AggregateTarget* target);    // 集計対象

    // 適地判定処理開始
    JUDGESUITABLEPLACE_API int JadgeStart(); 

}

