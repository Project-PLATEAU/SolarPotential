
namespace SolarPotential._3.Class
{
    class cls_aggregateParam
    {
     // 解析処理入力データ選択
        // 解析結果データフォルダパス
        public string txt_Param_1 { get; set; }

    // 選択範囲
        // 最大緯度
        public string txt_Max_Lat { get; set; }

        // 最小経度
        public string txt_Min_Lon { get; set; }

        // 最大経度
        public string txt_Max_Lon { get; set; }

        // 最小緯度
        public string txt_Min_Lat { get; set; }

        // 太陽光パネルの設置に関して優先度が低い施設の判定
        // 日射量が少ない施設を除外
        // kWh/m2未満
        public string txt_Param_1_1_1 { get; set; }

        // 下位％
        public string txt_Param_1_1_2 { get; set; }

    // 建物構造による除外
        // 木造・土蔵造
        public int chk_Param_1_2_1 { get; set; }


        // 鉄骨鉄筋コンクリート造
        public int chk_Param_1_2_2 { get; set; }

        // 鉄筋コンクリート造
        public int chk_Param_1_2_3 { get; set; }

        // 鉄骨造
        public int chk_Param_1_2_4 { get; set; }

        // 軽量鉄骨造
        public int chk_Param_1_2_5 { get; set; }

        // レンガ造・コンクリートブロック造・石造
        public int chk_Param_1_2_6 { get; set; }
        // 不明
        public int chk_Param_1_2_7 { get; set; }

        // 非木造
        public int chk_Param_1_2_8 { get; set; }

    // 特定の階層の施設を除外
        // 〇階以下
        public string txt_Param_1_3_1 { get; set; }

        // 〇階以上
        public string txt_Param_1_3_2 { get; set; }

    // 災害時に太陽光パネルが破損、消失する危険性の判定
        // 高さが想定される最大津波高さを下回る建物を除外
        public int chk_Param_2_1 { get; set; }

        // 建物高さが想定される河川浸水想定浸水深を下回る建物を除外
        public int chk_Param_2_2 { get; set; }

        // 土砂災害警戒区域内に存在する建物を除外
        public int chk_Param_2_3 { get; set; }

        // 気象データ(積雪)
        public int chk_Param_2_4 { get; set; }

        // 気象データ(積雪)フォルダパス
        public string txt_Param_2_4 { get; set; }

        // 気象データ(積雪)_cm以上
        public string txt_Param_2_4_1 { get; set; }

        // 積雪荷重(kgf/m3) = 年最深積雪量  ×　N/m3
        public string txt_Param_2_4_2 { get; set; }

        // 積雪荷重(kgf/m3) = 年最深積雪量  ×　N/m3
        public string txt_Param_2_4_3 { get; set; }
        // 太陽光パネルの設置に制限がある施設の判定
        // 制限を設ける範囲のシェープファイル_フォルダパス１
        public string txt_Param_3_1 { get; set; }

        // 制限を設ける範囲のシェープファイル_高さ１
        public string txt_Param_3_1_1 { get; set; }

        // 制限を設ける範囲のシェープファイル_方位１
        public int cmb_Param_3_1_2 { get; set; }

        // 制限を設ける範囲のシェープファイル_フォルダパス２
        public string txt_Param_3_2 { get; set; }

        // 制限を設ける範囲のシェープファイル_高さ２
        public string txt_Param_3_2_1 { get; set; }

        // 制限を設ける範囲のシェープファイル_方位２
        public int cmb_Param_3_2_2 { get; set; }

        // 制限を設ける範囲のシェープファイル_フォルダパス３
        public string txt_Param_3_3 { get; set; }

        // 制限を設ける範囲のシェープファイル_高さ３
        public string txt_Param_3_3_1 { get; set; }

        // 制限を設ける範囲のシェープファイル_方位３
        public int cmb_Param_3_3_2 { get; set; }

        // 出力フォルダ
        public string dir_Output_Result { get; set; }
    }
}
