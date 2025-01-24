using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Windows;
using System.Runtime.InteropServices;

namespace SolarPotential
{
    class CommonManager
    {

        #region Parameters

        /// <summary>
        /// 実行処理
        /// </summary>
        public enum Process
        {
            None = -1,
            Analyze = 0,
            Aggregate,
        }

        private Process _CurrentProcess = Process.None;
        /// <summary>
        /// 実行中の処理
        /// </summary>
        public Process CurrentProcess
        {
            get => this._CurrentProcess;
            set
            {
                this._CurrentProcess = value;
                switch (value)
                {
                    case Process.Analyze:
                        TitleText = "解析・シミュレーション";
                        OutputDirectoryName = "解析";
                        break;
                    case Process.Aggregate:
                        TitleText = "適地判定・集計";
                        OutputDirectoryName = "適地判定";
                        break;
                    case Process.None:
                    default:
                        break;
                };
            }
        }

        /// <summary>
        /// 画面タイトル
        /// </summary>
        public static string TitleText;

        /// <summary>
        /// 出力結果フォルダ名
        /// </summary>
        public static string OutputDirectoryName;

        /// <summary>
        /// 範囲画像の一時保存ファイルパス
        /// </summary>
        public static string AreaImageTempPath;

        /// <summary>
        /// 範囲KMLの一時保存ファイルパス
        /// </summary>
        public static string AreaKmlTempPath;

        /// <summary>
        /// 系番号
        /// </summary>
        public int JPZone = 0;

        /// <summary>
        /// 前回のパラメータを読み込むかどうかのフラグ
        /// </summary>
        public bool ReadPreParam { get; set; } = true;

        /// <summary>
        /// 4方位
        /// </summary>
        public static readonly List<string> Directions4 = new List<string> { "北向き", "東向き", "南向き", "西向き" };

        /// <summary>
        /// 8方位
        /// </summary>
        public static readonly List<string> Directions8 = new List<string> { "北向き", "北東向き", "東向き", "南東向き",
                                                            "南向き", "南西向き", "西向き", "北西向き" };

        /// <summary>
        /// 16方位
        /// </summary>
        public static readonly List<string> Directions16 = new List<string> { "北向き", "北北東向き", "北東向き", "東北東向き",
                                                            "東向き", "東南東向き", "南東向き", "南南東向き",
                                                            "南向き", "南南西向き", "南西向き", "西南西向き",
                                                            "西向き", "西北西向き", "北西向き", "北北西向き" };

        /// <summary>
        /// 座標系設定
        /// </summary>
        public static readonly List<string> DatumTypeList = new List<string> { "緯度経度", "平面直角座標" };

        /// <summary>
        /// 座標系設定
        /// </summary>
        public enum DatumTypes
        {
            None = -1,
            LatLon = 0,
            XY,
        }

        /// <summary>
        /// KML出力データ
        /// </summary>
        public struct KMLData
        {
            /// <summary>
            /// ID
            /// </summary>
            public string Id;

            /// <summary>
            /// name
            /// </summary>
            public string Name;

            /// <summary>
            /// 座標
            /// </summary>
            public List<Point2D> Points;

            public KMLData(string id, string name)
            {
                Id = id; Name = name;
                Points = new List<Point2D>();
            }
        }

        #endregion

        #region Strings

        /// <summary>
        /// 共通キャプション
        /// </summary>
        public static readonly string TEXT_SYSTEM_CAPTION = "カーボンニュートラル施策推進支援システム";

        /// <summary>
        /// Assetsフォルダパス
        /// </summary>
        public static string AssetsDirectory { get; } = Directory.GetCurrentDirectory() + "/Assets";

        /// <summary>
        /// 設定ファイルパス
        /// </summary>
        public static string SystemIniFilePath { get; } = Directory.GetCurrentDirectory() + "/SolarPotential.ini";

        /// <summary>
        /// ログファイル
        /// </summary>
        public static readonly string FILE_SYSTEM_LOG = "/log/SolarPotential.log";

        /// <summary>
        /// キャンセルファイル
        /// </summary>
        public static string FILE_CANCEL = "cancel.txt";

        #region KML
        /// <summary>
        /// KMLテンプレートファイル
        /// </summary>
        public static string FILE_KML_TEMPLATE = AssetsDirectory + "/KMLTemplate.kml";

        /// <summary>
        /// KMLテンプレートファイル
        /// </summary>
        public static string FILE_KML_PLACEMARK_TEXT = AssetsDirectory + "/KMLPlacemark.txt";

        /// <summary>
        /// KMLテンプレートファイルの置換用文字列(coordinates)
        /// </summary>
        public static string TEXT_KML_PLACEMARK = "<!--Placemark-->";

        /// <summary>
        /// KMLテンプレートファイルの置換用文字列(coordinates)
        /// </summary>
        public static string TEXT_KML_REPLACE_COORDINATES = "<!--REPLACE_COORDINATES-->";

        /// <summary>
        /// KMLテンプレートファイルの置換用文字列(id)
        /// </summary>
        public static string TEXT_KML_REPLACE_ID = "<!--REPLACE_ID-->";

        /// <summary>
        /// KMLテンプレートファイルの置換用文字列(name)
        /// </summary>
        public static string TEXT_KML_REPLACE_NAME = "<!--REPLACE_NAME-->";
        #endregion

        /// <summary>
        /// 拡張子フィルタ(CSVファイル)
        /// </summary>
        public static readonly string EXT_FILTER_CSV = "CSVファイル(*.csv)|*.csv";

        /// <summary>
        /// 拡張子フィルタ(Shapeファイル)
        /// </summary>
        public static readonly string EXT_FILTER_SHP = "Shapeファイル(*.shp)|*.shp";

        /// <summary>
        /// 拡張子フィルタ(テキストファイル)
        /// </summary>
        public static readonly string EXT_FILTER_TXT = "テキストファイル(*.txt)|*.txt";

        #endregion

        private CommonManager()
        {

        }

        private static CommonManager _instance = null;

        public static CommonManager Instance
        {
            get
            {
                if (_instance == null)
                {
                    _instance = new CommonManager();
                }
                return _instance;
            }
        }

        public void ShowExceptionMessageBox(string message)
        {
            string errMessage = $"System Exception:\n{message}";
            MessageBox.Show(errMessage, TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Error);
        }

        #region 入力値チェック関数

        /// <summary>
        /// 符号なし数値（小数を含む）チェック 
        /// </summary>
        /// <param name="target"></param>
        /// <returns></returns>
        public static bool IsDecimal(string target, int digits = 0)
        {
            bool ret = true;
            double target_d;
            if (!double.TryParse(target, out target_d))
            {
                ret = false;
            }
            else if (digits != 0)
            {
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
        /// 符号なし数値（整数値）チェック
        /// </summary>
        /// <param name="target"></param>
        /// <returns></returns>
        public static bool IsInteger(string target)
        {
            int target_i;
            
            return int.TryParse(target, out target_i);
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
            if (!Directory.Exists(directoryPath)) return false;

            // 指定拡張子のファイル件数をチェック
            var files = Directory.EnumerateFiles(directoryPath).Where(x => Path.GetExtension(x) == chk_extension);
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

        /// <summary>
        /// フォルダ選択ダイアログを表示
        /// </summary>
        public void ShowFolderBrowserDialog(out string path, string description = "", string directory = "")
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            path = string.Empty;

            // 指定パス存在チェック
            if (!Directory.Exists(directory))
            {
                // 存在しない場合はデフォルトを設定
                //directory = @"C:\";
                directory = Directory.GetCurrentDirectory();
            }

            System.Windows.Forms.FolderBrowserDialog fbd = new System.Windows.Forms.FolderBrowserDialog();
            // 説明
            fbd.Description = string.IsNullOrEmpty(description) ? "フォルダを指定してください。" : description;
            // ルートフォルダを指定(デフォルトはDesktop)
            fbd.RootFolder = Environment.SpecialFolder.Desktop;
            // 最初に選択するフォルダを指定する
            fbd.SelectedPath = directory;
            // ユーザーが新しいフォルダを作成できるようにする
            fbd.ShowNewFolderButton = true;
            // ダイアログを表示する
            if (fbd.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                path = fbd.SelectedPath;
            }
        }

        /// <summary>
        /// ファイル選択ダイアログを表示
        /// </summary>
        public void ShowSelectFileDialog(out string path, string description = "", string file = "", string filter = "")
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            path = string.Empty;

            string directory = string.Empty;
            string filename = string.Empty;

            // 指定パス存在チェック
            if (File.Exists(file))
            {
                // 存在する場合はフォルダ設定
                directory = Path.GetDirectoryName(file);
                filename = Path.GetFileName(file);
            }
            else
            {
                // 存在しない場合はデフォルトを設定
                //directory = @"C:\";
                directory = Directory.GetCurrentDirectory();
            }

            System.Windows.Forms.OpenFileDialog ofd = new System.Windows.Forms.OpenFileDialog();
            // 最初に選択するフォルダを指定する
            ofd.InitialDirectory = directory;
            // 最初に選択するファイルを指定する
            ofd.FileName = filename;
            // [ファイルの種類]に表示される選択肢を指定
            //指定なしの場合はすべてのファイルを表示
            ofd.Filter = string.IsNullOrEmpty(filter) ? "すべてのファイル(*.*)|*.*" : filter;
            // [ファイルの種類]で初期選択を指定
            ofd.FilterIndex = 1;
            // タイトルを設定
            ofd.Title = string.IsNullOrEmpty(description) ? "ファイルを指定してください。" : description;
            // ダイアログボックスを閉じる前に現在のディレクトリを復元するようにする
            ofd.RestoreDirectory = true;
            // ダイアログを表示する
            if (ofd.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                path = ofd.FileName;
            }
        }

        /// <summary>
        /// ファイル選択ダイアログを表示(複数ファイル)
        /// </summary>
        public void ShowSelectMultiFileDialog(out string[] paths, string description = "", string file = "", string filter = "")
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            paths = null;

            string directory = string.Empty;
            string filename = string.Empty;

            // 指定パス存在チェック
            if (File.Exists(file))
            {
                // 存在する場合はフォルダ設定
                directory = Path.GetDirectoryName(file);
                filename = Path.GetFileName(file);
            }
            else
            {
                // 存在しない場合はデフォルトを設定
                //directory = @"C:\";
                directory = Directory.GetCurrentDirectory();
            }

            System.Windows.Forms.OpenFileDialog ofd = new System.Windows.Forms.OpenFileDialog();
            // 最初に選択するフォルダを指定する
            ofd.InitialDirectory = directory;
            // 最初に選択するファイルを指定する
            ofd.FileName = filename;
            // [ファイルの種類]に表示される選択肢を指定
            //指定なしの場合はすべてのファイルを表示
            ofd.Filter = string.IsNullOrEmpty(filter) ? "すべてのファイル(*.*)|*.*" : filter;
            // [ファイルの種類]で初期選択を指定
            ofd.FilterIndex = 1;
            // タイトルを設定
            ofd.Title = description;
            // ダイアログボックスを閉じる前に現在のディレクトリを復元するようにする
            ofd.RestoreDirectory = true;
            // 複数ファイルを許可
            ofd.Multiselect = true;
            // ダイアログを表示する
            if (ofd.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                paths = ofd.FileNames;
            }
        }

        /// <summary>
        /// フォルダを作成(存在チェックしてから作成)
        /// </summary>
        public void CreateDirectory(string path)
        {
            if (Directory.Exists(path)) return;
            Directory.CreateDirectory(path);
        }

        /// <summary>
        /// 地図表示が可能か確認
        /// </summary>
        /// <returns></returns>
        public static bool CheckEnableDispMap()
        {
            LogManager.Instance.MethodStartLog("CommonManager", System.Reflection.MethodBase.GetCurrentMethod().Name);

            bool result = true;

            //WebRequestの作成
            System.Net.HttpWebRequest webreq =
                (System.Net.HttpWebRequest)System.Net.WebRequest.Create(_3_Class.ClassHTML.MapUrl);

            System.Net.HttpWebResponse webres = null;
            try
            {
                //サーバーからの応答を受信するためのWebResponseを取得
                webres = (System.Net.HttpWebResponse)webreq.GetResponse();

                //応答したURIを表示する
                Console.WriteLine(webres.ResponseUri);
                //応答ステータスコードを表示する
                Console.WriteLine("{0}:{1}",
                    webres.StatusCode, webres.StatusDescription);
            }
            catch (System.Net.WebException ex)
            {
                //HTTPプロトコルエラーかどうか調べる
                if (ex.Status == System.Net.WebExceptionStatus.ProtocolError)
                {
                    //HttpWebResponseを取得
                    System.Net.HttpWebResponse errres =
                        (System.Net.HttpWebResponse)ex.Response;
                    //応答したURIを表示する
                    Console.WriteLine(errres.ResponseUri);
                    //応答ステータスコードを表示する
                    Console.WriteLine("{0}:{1}",
                        errres.StatusCode, errres.StatusDescription);

                }
                else
                {
                    //Console.WriteLine(ex.Message);
                    LogManager.Instance.ExceptionLog(ex);
                }
                result = false;
            }
            finally
            {
                //閉じる
                if (webres != null)
                    webres.Close();

            }

            return result;

        }

        /// <summary>
        /// 座標情報をKMLファイルに出力
        /// </summary>
        /// <param name="points">座標リスト</param>
        /// <param name="path">出力先のKMLファイルパス</param>
        public bool WriteKMLFile(List<KMLData> kmlDataList, string path)
        {
            if (!File.Exists(FILE_KML_TEMPLATE)) return false;
            if (!File.Exists(FILE_KML_PLACEMARK_TEXT)) return false;
            
            // 出力先フォルダを作成
            if (!Directory.Exists(Directory.GetParent(path).FullName))
            {
                Directory.CreateDirectory(Directory.GetParent(path).FullName);
            }

            // ファイルコピー
            File.Copy(FILE_KML_TEMPLATE, path, true);

            try
            {
                string kmlText = "";
                using (StreamReader sr = new StreamReader(FILE_KML_TEMPLATE))
                {
                    kmlText = sr.ReadToEnd();
                }

                string orgText = "";
                using (StreamReader sr = new StreamReader(FILE_KML_PLACEMARK_TEXT))
                {
                    orgText = sr.ReadToEnd();
                }

                string placemarkText = "";

                // Placemark部分の文字列を作成
                foreach (var kmlData in kmlDataList)
                {
                    string tempText = orgText;
                    tempText = tempText.Replace(TEXT_KML_REPLACE_ID, kmlData.Id);
                    tempText = tempText.Replace(TEXT_KML_REPLACE_NAME, kmlData.Name);

                    // 出力する座標の文字列リスト
                    string pointsText = "";
                    foreach (var p in kmlData.Points)
                    {
                        // 緯度、経度を選択範囲に置換する
                        pointsText += $"{p.Lon},{p.Lat},0 ";
                    }
                    // 末尾のスペースを削除
                    pointsText = pointsText.Remove(pointsText.Length - 1);
                    tempText = tempText.Replace(TEXT_KML_REPLACE_COORDINATES, pointsText);

                    placemarkText += tempText + "\n";
                }

                // 出力テキスト
                kmlText = kmlText.Replace(TEXT_KML_PLACEMARK, placemarkText);

                using (StreamWriter writer = new StreamWriter(path, false))
                {
                    // 解析処理入力データ選択
                    writer.Write(kmlText);
                }

            }
            catch (Exception ex)
            {
                ShowExceptionMessageBox(ex.Message);
                LogManager.Instance.ExceptionLog(ex);
                return false;
            }

            return true;
        }
    }

    /// <summary>
    /// 方位＋傾き(解析/集計ともに使用)
    /// </summary>
    class DirectionDegree : BindableBase
    {
        /// <summary>
        /// 方位
        /// </summary>
        public int Direction
        {
            get => this._Direction;
            set => base.SetProperty(ref _Direction, value);
        }

        /// <summary>
        /// 傾き
        /// </summary>
        public string Degree
        {
            get => this._Degree;
            set => base.SetProperty(ref _Degree, value);
        }

        #region プロパティ
        private int _Direction;
        private string _Degree;
        #endregion
    }

    /// <summary>
    /// 解析設定
    /// </summary>
    class AnalyzeTargets : BindableBase
    {
        public bool SolarPotential
        {
            get => this._SolarPotential;
            set => base.SetProperty(ref _SolarPotential, value);
        }

        public bool Reflection
        {
            get => this._Reflection;
            set => base.SetProperty(ref _Reflection, value);
        }

        public bool Build
        {
            get => this._Build;
            set => base.SetProperty(ref _Build, value);
        }

        public bool Land
        {
            get => this._Land;
            set => base.SetProperty(ref _Land, value);
        }


        #region プロパティ
        private bool _SolarPotential;
        private bool _Reflection;
        private bool _Build;
        private bool _Land;
        #endregion
    }

    class INIFile
    {
        [DllImport("kernel32.dll", EntryPoint = "GetPrivateProfileStringW", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern uint GetPrivateProfileString(string lpAppName, string lpKeyName, string lpDefault, StringBuilder lpReturnedString, uint nSize, string lpFileName);

        [DllImport("kernel32.dll")]
        private static extern uint GetPrivateProfileInt(string lpAppName, string lpKeyName, int nDefault, string lpFileName);

        [DllImport("kernel32.dll", EntryPoint = "WritePrivateProfileStringW", CharSet = CharSet.Auto, SetLastError = true)]
        private static extern uint WritePrivateProfileString(string lpAppName, string lpKeyName, string lpString, string lpFileName);

        public string FilePath { get; set; }

        public INIFile(string path)
        {
            FilePath = path;
        }

        public string GetString(string section, string key, string defaultValue = "")
        {
            var sb = new StringBuilder(32767);
            var r = GetPrivateProfileString(section, key, defaultValue, sb, (uint)sb.Capacity, FilePath);
            return sb.ToString();
        }

        public int GetInt(string section, string key, int defaultValue = 0)
        {
            return (int)GetPrivateProfileInt(section, key, defaultValue, FilePath);
        }

        public double GetDouble(string section, string key, double defaultValue = 0.0)
        {
            var str = GetString(section, key, defaultValue.ToString());
            double ret = 0.0;
            double.TryParse(str, out ret);
            return ret;
        }

        public void WriteString(string section, string key, string value)
        {
            var r = WritePrivateProfileString(section, key, value, FilePath);
        }
    }

}
