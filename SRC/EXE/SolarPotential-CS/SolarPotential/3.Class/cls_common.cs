using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace SolarPotential._3.Class
{
    class cls_common
    {
        // 定数 ※後で別定数ファイルにする
        public const string CON_TEXT = "txt";
        public const string CON_COMB = "cbo";
        public const string CON_CHECK = "chk";
        public const string CON_RADIO = "rdo";
        public const string UNVERIFIED = "unexecuted";      // 未検証

        // ファイル拡張子
        public const string FILE_CSV = ".csv";
        public const string FILE_GML = ".gml";
        public const string FILE_SHP = ".shp";
        public const string FILE_SHX = ".shx";
        public const string FILE_DBF = ".dbf";
        public const string FILE_KML = ".kml";
        public const string FILE_JPG = ".jpg";

        // ファイル名
        public const string FILE_RANGE_COORDINATES = "/output/initFile_Coordinates.txt";
        public const string FILE_PARAMETER_LOG = "parameter.log";
        public const string FILE_AGGREGATE_RANGE = "集計範囲";
        public const string FILE_CANCEL = "cancel.txt";

        // 出力フォルダ名
        public const string OUTPUT_DIR_ANALYSIS = "解析";
        public const string OUTPUT_DIR_AGGREGATE = "適地判定";

        #region メッセージ出力関連
        /// <summary>
        /// メッセージ区分
        /// </summary>
        public enum MessageType
        {
            error,      // エラーメッセージ
            info,       // メッセージ
        }

        /// <summary>
        /// メッセージ出力
        /// </summary>
        /// <param name="msg">出力文字列</param>
        /// <param name="type">メッセージ区分</param>
        /// <param name="title">メッセージボックスタイトル ※規定値あり</param>
        public static void showMessage(string msg, MessageType type, string title = "")
        {
            switch (type) {
                // エラーメッセージ
                case MessageType.error :
                    title = string.IsNullOrEmpty(title) ? "エラー" : title;
                    System.Windows.Forms.MessageBox.Show(msg, title, MessageBoxButtons.OK, MessageBoxIcon.Error);

                    break;
                // 出力メッセージ
                case MessageType.info:
                    title = string.IsNullOrEmpty(title) ? "情報" : title;
                    System.Windows.Forms.MessageBox.Show(msg, title, MessageBoxButtons.OK, MessageBoxIcon.Information);
                    break;
            }

        }
        #endregion

        #region 入力パラメータログ出力関連
        /// <summary>
        /// ログ区分
        /// </summary>
        public enum LogType
        {
            no,         // 任意の文字列
            start,      // 処理開始   [yyyyMMddHHmmss] 開始 xxxxxxxxxxx
            end,        // 処理終了   [yyyyMMddHHmmss] 終了 xxxxxxxxxxx
        }

        /// <summary>
        /// メッセージ出力
        /// </summary>
        /// <param name="type">メッセージ区分</param>
        /// <param name="msg">出力文字列</param>
        /// <param name="title">メッセージボックスタイトル ※規定値あり</param>

        /// <summary>
        /// ログメッセージ出力
        /// </summary>
        /// <param name="path">出力ログファイルパス</param>
        /// <param name="msg">出力文字列</param>
        /// <param name="type">ログ区分（省略可）</param>
        public static void outputLogMessage(string path,string strlog, LogType type = LogType.no)
        {
            // 出力文字列格納用
            string strOutputLog;
            
            // 出力文字列の生成
            switch (type)
            {
                // 処理開始、処理終了ログ
                case LogType.start:
                case LogType.end:
                    // システム日付
                    string now = DateTime.Now.ToString("yyyyMMddHHmmss");
                    string status = "";
                    if (type == LogType.start) status = "開始";
                    if (type == LogType.end) status = "終了";

                    // [日時] 開始／終了 指定文字列 の形式で出力する
                    strOutputLog = $"[{now}] {status} {strlog}";
                    logFile_Output(path, strOutputLog);
                    break;

                default:
                    // 指定の文字列を出力する
                    logFile_Output(path, strlog);
                    break;
            }
        }

        /// <summary>
        /// ログファイル出力
        /// </summary>
        /// <param name="path">出力パス</param>
        /// <param name="content">出力内容</param>
        private static void logFile_Output(string path, string content)
        {
            Encoding enc = Encoding.GetEncoding("UTF-8");
            // 追記モードで書き込み
            using (StreamWriter writer = new StreamWriter(path, true, enc))
            {
                // 解析処理入力データ選択
                writer.WriteLine(content);
            }

        }
        #endregion

        #region エラーチェック関数
        /// <summary>
        /// 符号なし数値（小数を含む）チェック 
        /// </summary>
        /// <param name="target"></param>
        /// <returns></returns>
        public static bool IsDecimal(string target , int digits = 0)
        {
            bool ret = true;
            double target_d;
            if (!double.TryParse(target, out target_d))
            {
                ret = false;
            }
            else if (digits != 0) {
                // 小数点以下の指定がある場合は桁数までチェック
                if (digits < GetDecimalLength(target_d))
                {
                    ret = false;
                }

            }

            return ret;

        }

        /// <summary>
        /// 小数点以下の桁数を返却する
        /// </summary>
        /// <param name="val"></param>
        /// <returns></returns>
        public static int GetDecimalLength(double val)
        {
            int result = 0;
            string valStr = val.ToString().TrimEnd('0');
            int idx = valStr.IndexOf('.');
            if (idx != -1) result = valStr.Substring(idx + 1).Length;

            return result;
        }

        /// <summary>
        /// 符号なし数値（整数値）チェック ※後ほどcommonに移動予定
        /// </summary>
        /// <param name="target"></param>
        /// <returns></returns>
        public static bool IsInteger(string target)
        {
            int target_i;
            return int.TryParse(target, out target_i);

        }

        /// <summary>
        /// ファイル存在チェック
        /// </summary>
        /// <param name="filePath">対象ファイルパス</param>
        /// <returns></returns>
        public static bool IsFileExists(string filePath)
        {
            // ファイル存在チェック
            return File.Exists(filePath);

        }

        /// <summary>
        /// ディレクトリ存在チェック
        /// </summary>
        /// <param name="directoryPath">対象パス</param>
        /// <param name="extension">拡張子の指定　※指定の拡張子ファイルの存在チェックが必要な場合は記載</param>
        /// <returns name="ret">チェック結果</returns>
        /// <returns name="extension">拡張子チェック結果/returns> 
        public static bool IsDirectoryExists(string directoryPath)
        {
             // フォルダ存在チェック結果
            return Directory.Exists(directoryPath);

        }

        /// <summary>
        /// ファイル拡張子チェック
        /// </summary>
        /// <param name="filePath">対象パス</param>
        /// <param name="chk_extension">ファイル拡張子</param>
        /// <returns>チェック結果</returns>
        public static bool IsExtensionExists_File(string filePath, string chk_extension)
        {
            var extension = Path.GetExtension(filePath);
            // 指定拡張子と一致するかチェック結果を返却
            return (chk_extension == extension) ? true : false;

        }
        /// <summary>
        /// フォルダ内指定拡張子ファイル存在チェック
        /// </summary>
        /// <param name="directoryPath">対象フォルダ</param>
        /// <param name="chk_extension">ファイル拡張子</param>
        /// <returns></returns>
        public static bool IsExtensionExists_Directory(string directoryPath, string chk_extension)
        {
            // 指定拡張子のファイル件数をチェック
            var files = Directory.EnumerateFiles(directoryPath).Where(x => (Path.GetExtension(x) == chk_extension));
            //1件でもあればOK
            if (files.Count() > 0)
            {
                return true;
            }
            else
            {
                return false;
            }

        }

        #endregion

        #region 入力保存内容の保存／読み込み関連
        // 画面コントロールの配列用
        public struct controlObj
        {
            public string ControlName { get; set; }
            public string ControlName_JP { get; set; }
            public string Value { get; set; }
            public string Type { get; set; }

            public controlObj(string controlName, string controlName_JP, string value, string type)
            {
                ControlName = controlName;
                ControlName_JP = controlName_JP;
                Value = value;
                Type = type;
            }
        }

        // コントロール名からコントロール名日本語を取得
        public static string get_controlNameJP(string name , List<controlObj> listControl)
        {
            var nameJP = string.IsNullOrEmpty(listControl.Find(x => x.ControlName == name).ControlName_JP) ? "名称未取得 " + name : listControl.Find(x => x.ControlName == name).ControlName_JP;

            return nameJP;

        }

        // コントロール名からコントロール名日本語を取得
        public static string get_controlName(string nameJP, List<controlObj> listControl)
        {
            var name = string.IsNullOrEmpty(listControl.Find(x => x.ControlName == nameJP).ControlName_JP) ? "名称未取得 " + nameJP : listControl.Find(x => x.ControlName_JP == nameJP).ControlName;

            return name;

        }


        // 保存した入力内容をファイル読み込み→コントロール配列で返却
        public static List<controlObj> read_SaveFile(string fileName)
        {
            // 読み込むテキストを保存する変数
            var saveControl = new List<controlObj>();
    
            try
            {
                //ファイルをオープンする
                using (StreamReader sr = new StreamReader(fileName, Encoding.GetEncoding("Shift_JIS")))
                {
                    while (0 <= sr.Peek())
                    {
                        var text = sr.ReadLine();
                        var lineItems = text.Split(',');
                        // 1コントロール毎に分割（カンマ）
                        foreach (string lineItem in lineItems)
                        {
                            if (lineItem.Contains(":")) {
                                // コントロールと値に分割（セミコロン）
                                int count = lineItem.IndexOf(':');
                                var name = lineItem.Substring(0, count);
                                var value = lineItem.Substring(count+1);

                                // コントロール配列に格納
                                saveControl.Add(new controlObj { ControlName_JP = name, ControlName = "", Value = value, Type = "" });

                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                showMessage(ex.Message, cls_common.MessageType.error);
            }

            return saveControl;
        }
        #endregion

    }
}
