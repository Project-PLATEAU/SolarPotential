
namespace SolarPotential._3.Class
{
    class cls_analyzeParam
    {
        // 解析処理入力データ選択
        // 3D都市モデル
        public string txt_Param_1 { get; set; }
        // 発電ポテンシャル推計
        // 月毎の可照時間
        public string txt_Param_2 { get; set; }
        // 毎月の平均日照時間
        public string txt_Param_3 { get; set; }
        // 月毎の積雪深
        public string txt_Param_4 { get; set; }
        // DEMデータ
        public string txt_Param_5 { get; set; }

        // 解析結果出力フォルダ
        public string txt_Output_Directory { get; set; }

        // 発電ポテンシャル推移
        // 面積
        public string txt_Param_1_1 { get; set; }

        // 向き
        public int cmb_Param_1_2 { get; set; }

        // 傾き1
        public string txt_Param_1_3 { get; set; }

        // 傾き2
        public string txt_Param_1_4 { get; set; }


        // 傾斜が少ない(水平に近い)屋根面の補正
        // 傾き1
        public string txt_Param_2_1 { get; set; }

        // 傾き2
        public string txt_Param_2_2 { get; set; }


        // 太陽光パネル単位面積当たりの発電容量
        // 発電容量
        public string txt_Param_3_1 { get; set; }


        // 屋根の傾きによる反射シミュレーション時の屋根面の向き・傾きの補正
        // 3度未満
        // 屋根面と同値
        public int rdo_Param_4_1 { get; set; }

        // 指定
        public int rdo_Param_4_2 { get; set; }

        // 向き
        public int cmb_Param_4_3 { get; set; }

        // 傾き
        public string txt_Param_4_4 { get; set; }


        // 3度以上
        // 屋根面と同値
        public int rdo_Param_4_5 { get; set; }

        // 指定
        public int rdo_Param_4_6 { get; set; }

        // 向き
        public int cmb_Param_4_7 { get; set; }

        // 傾き
        public string txt_Param_4_8 { get; set; }

        // 出力フォルダ
        public string dir_Output_Result { get; set; }
    }
}
