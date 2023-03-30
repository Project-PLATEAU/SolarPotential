
namespace SolarPotential._3.Class
{
    class  cls_message
    {

        /// <summary>
        /// 全般 - 共通
        /// </summary>
        public static class general
        {
            // メッセージ定義
            public static readonly string MSG_SAVE_COMPLETE = "入力内容保存完了";
            public static readonly string MSG_SELECT_SAVE_FILE = "保存先のファイルを選択してください";
            public static readonly string MSG_SELECT_INPUT_FILE = "保存した入力内容ファイルを指定してください";

            // エラーメッセージ定義
            public static readonly string ERRMSG_DEFAULT_LOADING = "初期値の読み込みに失敗しました";
            public static readonly string ERRMSG_INPUT_LOADING = "入力内容の読み込みに失敗しました";
            public static readonly string ERRMSG_INPUT_DATA = "入力エラーを確認してください";
            public static readonly string ERRMSG_SELECTED_RANGE = "選択範囲エラー";
            public static readonly string ERRMSG_ERROR_CHACK = "エラーチェック失敗";
            public static readonly string ERRMSG_FOLDER_EXIST = "指定されたフォルダが存在しません";
            public static readonly string ERRMSG_FILE_EXIST = "指定されたファイルが存在しません";
            public static readonly string ERRMSG_EXTENSION_EXIST = "ファイルが存在しません";       // 先頭にファイル拡張子を付与
            public static readonly string ERRMSG_CHECK_REQUIRED = "の選択を確認してください";
            public static readonly string ERRMSG_INPUT_REQUIRED = "の入力を確認してください";
            public static readonly string ERRMSG_SET_PARAMETER = "パラメータの設定に失敗しました";

            // 入力バリデーションエラー
            public static readonly string ERRMSG_NUMBER = "数値入力エラー";

        }

        /// <summary>
        /// 集計画面メッセージ
        /// </summary>
        public static class　frm_Aggregate
        {
            // フォルダ選択ダイアログ定義
            public static readonly string MSG_001 = "解析結果データを指定してください。";
            public static readonly string MSG_002 = "集計結果出力フォルダを指定してください。";
            public static readonly string MSG_003 = "気象データ積雪のシェープファイルを指定してください。";
            public static readonly string MSG_004 = "制限を設ける範囲のシェープファイルを指定してください。";

        }

        /// <summary>
        /// 解析画面メッセージ
        /// </summary>
        public partial class frm_Analyze
        {
            // ファイル選択ダイアログ
            public static readonly string MSG_001 = "月毎の可照時間を指定してください。";
            public static readonly string MSG_002 = "毎月の平均日照時間を指定してください。";
            public static readonly string MSG_003 = "月毎の積雪深を指定してください。";
            public static readonly string MSG_004 = "3D都市モデルを指定してください。";
            public static readonly string MSG_005 = "DEMデータを指定してください。";
            public static readonly string MSG_006 = "気象データ（積雪）を指定してください。";
            public static readonly string MSG_008 = "解析結果出力フォルダを指定してください。";

        }

    }

}
