using System;
using System.Text;
using System.IO;

namespace SolarPotential
{
    class LogManager
    {
        /// <summary>
        /// ログファイル名
        /// </summary>
        public static readonly string FILE_SYSTEM_LOG = "/log/SolarPotential.log";

        /// <summary>
        /// ログ区分
        /// </summary>
        public enum LogType
        {
            START = -2,     // 処理開始
            END,            // 処理終了
            ERROR = 0,      // エラー
            WARN,           // 警告
            LOG,            // ログ
            INFO,           // 情報
            DEBUG,          // デバッグログ
        }

        /// <summary>
        /// ログ出力レベル
        /// </summary>
#if DEBUG
        public static int LogLevel { get; set; } = (int)LogType.DEBUG;
#else
        public static int LogLevel { get; set; } = (int)LogType.LOG;
#endif

        /// <summary>
        /// ログファイルパス
        /// </summary>
        public static string LogFilePath { get; set; } = "";

        /// <summary>
        /// ログメッセージ出力
        /// </summary>
        /// <param name="path">ログファイルパス</param>
        /// <param name="strlog">メッセージ</param>
        /// <param name="type">ログ区分</param>
        public void OutputLogMessage(string strlog, LogType type = LogType.LOG)
        {
            if ((int)type > LogLevel) return;

            // 出力文字列格納用
            string strNowDate = $"{DateTime.Now.ToString("yyyy/MM/dd HH:mm:ss")}";
            string strLogType = $"[{Enum.GetName(typeof(LogType), type)}]";
            string strOutputLog = $"{strNowDate} {strLogType} {strlog}";

            // 書き込み
            //WriteLogFile(LogFilePath, strOutputLog);
        }

        /// <summary>
        /// ログファイル書き込み
        /// </summary>
        /// <param name="path">出力パス</param>
        /// <param name="content">出力内容</param>
        private void WriteLogFile(string path, string content)
        {
            try
            {
                Encoding enc = Encoding.GetEncoding("UTF-8");

                // 追記モードで書き込み
                using (StreamWriter writer = new StreamWriter(path, true, enc))
                {
                    // 解析処理入力データ選択
                    writer.WriteLine(content);
                }
            }
            catch
            {
                //
            }
        }

        /// <summary>
        /// メソッドの開始ログを出力
        /// </summary>
        /// <param name="strlog"></param>
        /// <param name="type"></param>
        public void MethodStartLog(string className, string methodName, LogType type = LogType.DEBUG)
        {
            OutputLogMessage($"{className}::{methodName} Start", type);
        }

        /// <summary>
        /// Exeption発生時のログを出力
        /// </summary>
        /// <param name="strlog"></param>
        /// <param name="type"></param>
        public void ExceptionLog(Exception ex)
        {
            OutputLogMessage(ex.Message + Environment.NewLine + ex.StackTrace, LogType.ERROR);
        }

        private LogManager()
        {

        }

        private static LogManager _instance = null;

        public static LogManager Instance
        {
            get
            {
                if (_instance == null)
                {
                    _instance = new LogManager();

                    try
                    {
                        LogFilePath = Directory.GetCurrentDirectory() + FILE_SYSTEM_LOG;

                        if (!Directory.Exists(Directory.GetParent(LogFilePath).FullName))
                        {
                            Directory.CreateDirectory(Directory.GetParent(LogFilePath).FullName);
                        }

                        if (File.Exists(LogFilePath))
                        {
                            File.Delete(LogFilePath);
                        }
                    }
                    catch (Exception ex)
                    {
                        CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
                    }
                }
                return _instance;
            }
        }
    }
}
