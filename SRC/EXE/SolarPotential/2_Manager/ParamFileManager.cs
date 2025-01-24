using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Collections.ObjectModel;

namespace SolarPotential
{
    class ParamFileManager
    {
        /// <summary>
        /// 実行履歴保存フォルダ
        /// </summary>
        public string HistoryDirectory { get; } = Directory.GetCurrentDirectory() + "/Assets/History";

        /// <summary>
        /// 実行履歴情報ファイル
        /// </summary>
        static readonly string FILE_HISTORY_LOG = "HistoryInfo.csv";

        /// <summary>
        /// 実行履歴ファイル名 yyyyMMDDhhmm.param
        /// </summary>
        static readonly string NAME_HISTORY_FILE = "{0}.param";

        /// <summary>
        /// まる
        /// </summary>
        public static readonly string TEXT_MARU = "○";

        /// <summary>
        /// ばつ
        /// </summary>
        public static readonly string TEXT_BATSU = "×";

        /// <summary>
        /// 実行履歴情報ファイルの列数
        /// </summary>
        const int NUM_HISTORY_LOG_COLUMN = 6;

        /// <summary>
        /// 履歴リスト
        /// </summary>
        private List<ParamInfo> _ParamHistoryList = null;
        public List<ParamInfo> ParamHistoryList
        {
            get
            {
                if (_ParamHistoryList == null)
                {
                    ReadHistoryFile();
                }
                return _ParamHistoryList;
            }
        }

        /// <summary>
        /// パラメータ情報
        /// </summary>
        public class ParamInfo
        {
            /// <summary>
            /// 実行日時
            /// </summary>
            public DateTime Date;

            /// <summary>
            /// 実行日時(文字列)
            /// </summary>
            public string DateStr { get { return Date.ToString("yyyy/MM/dd HH:mm"); } }

            /// <summary>
            /// 解析対象
            /// </summary>
            public AnalyzeTargets Target;

            /// <summary>
            /// 発電ポテンシャル推計
            /// </summary>
            public string Potential { get{ return Target.SolarPotential ? TEXT_MARU : TEXT_BATSU; } }

            /// <summary>
            /// 反射シミュレーション
            /// </summary>
            public string Reflection { get{ return Target.Reflection ? TEXT_MARU : TEXT_BATSU; } }

            /// <summary>
            /// 建物
            /// </summary>
            public string Build { get { return Target.Build ? TEXT_MARU : TEXT_BATSU; } }

            /// <summary>
            /// 土地
            /// </summary>
            public string Land { get { return Target.Land ? TEXT_MARU : TEXT_BATSU; } }

            /// <summary>
            /// 備考
            /// </summary>
            public string Explanation { get; set; }

            public ParamInfo()
            {
                Target = new AnalyzeTargets(); Explanation = "";
            }
        }

        /// <summary>
        /// 最後に実行したパラメータを読み込む
        /// </summary>
        /// <param name="file"></param>
        /// <returns></returns>
        public bool ReadLastParamFile()
        {
            bool ret = false;

            string file = "";
            switch (CommonManager.Instance.CurrentProcess)
            {
                case CommonManager.Process.Analyze:
                    file = Path.Combine(HistoryDirectory, AnalyzeParam.FILE_PRE_ANALYZE);
                    ret = ReadAnalyzeParams(file);
                    break;

                case CommonManager.Process.Aggregate:
                    file = Path.Combine(HistoryDirectory, AggregateParam.FILE_PRE_AGGREGATE);
                    ret = ReadAggregateParams(file);
                    break;

                default:
                    break;
            }

            if (ret)
            {
                // 読み込んだパラメータの各データパをスチェック
                DataPathExists();
            }

            return ret;
		}

        /// <summary>
        /// データパスの存在チェック
        /// 存在しないデータは再入力させるため空にする
        /// </summary>
        /// <returns></returns>
        public bool DataPathExists()
        {
            List<string> errList = new List<string>();

            switch (CommonManager.Instance.CurrentProcess)
            {
                case CommonManager.Process.Analyze:
                    if (!string.IsNullOrEmpty(AnalyzeParam.Instance.InputData.CityModel) && !Directory.Exists(AnalyzeParam.Instance.InputData.CityModel))
                    {
                        AnalyzeParam.Instance.InputData.CityModel = "";
                        errList.Add("3D都市モデルデータ");
                    }
                    if (!string.IsNullOrEmpty(AnalyzeParam.Instance.InputData.KashoData) && !File.Exists(AnalyzeParam.Instance.InputData.KashoData))
                    {
                        AnalyzeParam.Instance.InputData.KashoData = "";
                        errList.Add("可照時間CSVファイル");
                    }
                    if (!string.IsNullOrEmpty(AnalyzeParam.Instance.InputData.NisshoData) && !File.Exists(AnalyzeParam.Instance.InputData.NisshoData))
                    {
                        AnalyzeParam.Instance.InputData.NisshoData = "";
                        errList.Add("平均日照時間CSVファイル");
                    }
                    if (!string.IsNullOrEmpty(AnalyzeParam.Instance.InputData.SnowDepthData) && !File.Exists(AnalyzeParam.Instance.InputData.SnowDepthData))
                    {
                        AnalyzeParam.Instance.InputData.SnowDepthData = "";
                        errList.Add("積雪深CSVファイル");
                    }
                    if (!string.IsNullOrEmpty(AnalyzeParam.Instance.InputData.LandData) && !File.Exists(AnalyzeParam.Instance.InputData.LandData))
                    {
                        AnalyzeParam.Instance.InputData.LandData = "";
                        errList.Add("土地範囲指定シェープファイル");
                    }
                    if (!string.IsNullOrEmpty(AnalyzeParam.Instance.OutputResultDirectory) && !Directory.Exists(AnalyzeParam.Instance.OutputResultDirectory))
                    {
                        AnalyzeParam.Instance.OutputResultDirectory = "";
                        errList.Add("結果出力フォルダ");
                    }
                    break;

                case CommonManager.Process.Aggregate:
                    if (!string.IsNullOrEmpty(AggregateParam.Instance.AnalyzeResultPath) && !Directory.Exists(AggregateParam.Instance.AnalyzeResultPath))
                    {
                        AggregateParam.Instance.AnalyzeResultPath = "";
                        errList.Add("解析結果フォルダ");
                    }
                    if (!string.IsNullOrEmpty(AggregateParam.Instance.OutputResultDirectory) && !Directory.Exists(AggregateParam.Instance.OutputResultDirectory))
                    {
                        AggregateParam.Instance.OutputResultDirectory = "";
                        errList.Add("結果出力フォルダ");
                    }
                    break;

                default:
                    break;

            }

            // 存在しないデータパスがあった場合
            if (errList.Count() > 0)
            {
                string strdata = "";
                foreach (var str in errList)
                {
                    strdata += $"・{str}\n";
				}
                System.Windows.MessageBox.Show($"読み込んだパラメータに存在しないパスが指定されています。\n以下の項目を再設定してください。\n\n{strdata}",
                        CommonManager.TEXT_SYSTEM_CAPTION, System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Warning);
                return false;
            }

            return true;
        }

        /// <summary>
        /// パラメータファイルパスを取得
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        public string GetParamFilePath(int index)
        {
            ParamInfo info = _ParamHistoryList[index];
            string historyDir = Path.Combine(HistoryDirectory, Enum.GetName(typeof(CommonManager.Process), CommonManager.Instance.CurrentProcess));
            return  Path.Combine(historyDir, string.Format(NAME_HISTORY_FILE, info.Date.ToString("yyyyMMddHHmm")));
        }

        /// <summary>
        /// 実行履歴情報ファイルを読み込み
        /// </summary>
        /// <param name="file"></param>
        /// <returns></returns>
        public void ReadHistoryFile()
        {
            try
            {
                LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

                if (_ParamHistoryList == null) _ParamHistoryList = new List<ParamInfo>();

                // 履歴データ読み込み
                string file = Path.Combine(HistoryDirectory, Enum.GetName(typeof(CommonManager.Process), CommonManager.Instance.CurrentProcess), FILE_HISTORY_LOG);
                if (!File.Exists(file)) return;

                var csvData = new List<string>();

                using (StreamReader sr = new StreamReader(file, Encoding.GetEncoding("Shift_JIS")))
                {
                    while (0 <= sr.Peek())
                    {
                        csvData.Add(sr.ReadLine());
                    }
                }

                foreach (var line in csvData)
                {
                    string[] arr = line.Split(',');
                    if (arr.Count() == NUM_HISTORY_LOG_COLUMN)
                    {
                        ParamInfo info = new ParamInfo();
                        if (DateTime.TryParse(arr[0], out DateTime dt))
                        {
                            info.Date = dt;
                        }
                        else
                        {
                            throw new Exception("履歴ファイルの日付が不正です。");
                        }
                        if (arr[1] == "1") info.Target.SolarPotential = true; 
                        if (arr[2] == "1") info.Target.Reflection = true; 
                        if (arr[3] == "1") info.Target.Build = true; 
                        if (arr[4] == "1") info.Target.Land = true;
                        info.Explanation = arr[5];
                        ParamHistoryList.Add(info);
                    }
                    else
                    {
                        throw new Exception("履歴ファイルのフォーマットが不正です。");
                    }
                }
            }
            catch (Exception ex)
            {
                CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
                LogManager.Instance.ExceptionLog(ex);
                _ParamHistoryList = null;
            }
        }

        /// <summary>
        /// 実行履歴を追加
        /// </summary>
        /// <param name="paramfile">パラメータファイル</param>
        /// <returns></returns>
        public bool AddHistory(string paramfile, DateTime date)
        {
            bool ret = false;

            try
            {
                LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

                // 履歴情報ファイルに追記
                string historyDir = Path.Combine(HistoryDirectory, Enum.GetName(typeof(CommonManager.Process), CommonManager.Instance.CurrentProcess));
                string file = Path.Combine(historyDir, FILE_HISTORY_LOG);
                if (!Directory.Exists(historyDir))
                {
                    Directory.CreateDirectory(historyDir);
                }

                using (StreamWriter sw = new StreamWriter(file, true, Encoding.GetEncoding("Shift_JIS")))
                {
                    string targets = "";
                    if (CommonManager.Instance.CurrentProcess == CommonManager.Process.Analyze)
                    {
                        targets += $"{(AnalyzeParam.Instance.Target.SolarPotential ? "1," : "0,")}";
                        targets += $"{(AnalyzeParam.Instance.Target.Reflection ? "1," : "0,")}";
                        targets += $"{(AnalyzeParam.Instance.Target.Build ? "1," : "0,")}";
                        targets += $"{(AnalyzeParam.Instance.Target.Land ? "1" : "0")}";
                    }
                    else
                    {
                        targets = "0,0,0,0";
                    }
                    string line = $"{date.ToString("yyyy/MM/dd HH:mm")},{targets},";
                    sw.WriteLine(line);
                }

                // パラメータファイルを履歴フォルダにコピー
                string dstFile = Path.Combine(historyDir, string.Format(NAME_HISTORY_FILE, date.ToString("yyyyMMddHHmm")));
                File.Copy(paramfile, dstFile, true);

                ret = true;
            }
            catch (Exception ex)
            {
                CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
                LogManager.Instance.ExceptionLog(ex);
                ret = false;
            }

            return ret;
        }

        /// <summary>
        /// 履歴情報ファイルを更新
        /// </summary>
        /// <returns></returns>
        public bool UpdateHistoryFile()
        {
            if (_ParamHistoryList == null) return false;
            if (_ParamHistoryList.Count <= 0) return false;

            bool ret = false;

            try
            {
                LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

                // 既存ファイルを削除
                string file = Path.Combine(HistoryDirectory, Enum.GetName(typeof(CommonManager.Process), CommonManager.Instance.CurrentProcess), FILE_HISTORY_LOG);
                File.Delete(file);

                using (StreamWriter sw = new StreamWriter(file, true, Encoding.GetEncoding("Shift_JIS")))
                {
                    foreach (var info in _ParamHistoryList)
                    {
                        string targets = "";
                        targets += $"{(info.Target.SolarPotential ? "1," : "0,")}";
                        targets += $"{(info.Target.Reflection ? "1," : "0,")}";
                        targets += $"{(info.Target.Build ? "1," : "0,")}";
                        targets += $"{(info.Target.Land ? "1" : "0")}";

                        string line = $"{info.Date.ToString("yyyy/MM/dd HH:mm")},{targets},{info.Explanation}";
                        sw.WriteLine(line);
                    }
                }

            }
            catch (Exception ex)
            {
                CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
                LogManager.Instance.ExceptionLog(ex);
                ret = false;
            }

            return ret;
        }

        /// <summary>
        /// 指定したインデックスの履歴を削除
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        public bool DeleteHistory(ParamInfo info)
        {
            if (_ParamHistoryList == null) return false;

            bool ret = false;

            try
            {
                LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

                // 履歴フォルダ内の該当ファイルを削除
                string historyDir = Path.Combine(HistoryDirectory, Enum.GetName(typeof(CommonManager.Process), CommonManager.Instance.CurrentProcess));
                string file = Path.Combine(historyDir, string.Format(NAME_HISTORY_FILE, info.Date.ToString("yyyyMMddHHmm")));
                if (File.Exists(file))  File.Delete(file);

                // リストから削除
                _ParamHistoryList.Remove(info);

                ret = UpdateHistoryFile();

            }
            catch (Exception ex)
            {
                CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
                LogManager.Instance.ExceptionLog(ex);
                ret = false;
            }

            return ret;
        }

        /// <summary>
        /// 履歴を全削除
        /// </summary>
        /// <returns></returns>
        public bool DeleteAllHistory()
        {
            bool ret = false;

            try
            {
                LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

                // 履歴フォルダごと削除
                string historyDir = Path.Combine(HistoryDirectory, Enum.GetName(typeof(CommonManager.Process), CommonManager.Instance.CurrentProcess));
                Directory.Delete(historyDir, true);

                _ParamHistoryList.Clear();

                ret = true;
            }
            catch (Exception ex)
            {
                CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
                LogManager.Instance.ExceptionLog(ex);
                ret = false;
            }

            return ret;
        }

        /// <summary>
        /// 解放処理
        /// </summary>
        public void Dispose()
        {
            _ParamHistoryList.Clear();
            _ParamHistoryList = null;

            _instance = null;
        }

        /// <summary>
        /// 解析パラメータファイル読み込み
        /// </summary>
        /// <param name="file"></param>
        /// <returns></returns>
        public bool ReadAnalyzeParams(string file)
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            if (!File.Exists(file)) return false;

            // 読み込み
            INIFile iniFile = new INIFile(file);

            {   // 共通パラメータ
                string str = iniFile.GetString("AnalyzeParam", "Target", "1,1,1,1");
                string[] split = str.Split(',');
                if (split.Count() == 4)
                {
                    AnalyzeParam.Instance.Target.SolarPotential = split[0] == "1" ? true : false;
                    AnalyzeParam.Instance.Target.Reflection = split[1] == "1" ? true : false;
                    AnalyzeParam.Instance.Target.Build = split[2] == "1" ? true : false;
                    AnalyzeParam.Instance.Target.Land = split[3] == "1" ? true : false;
                }
                else
                {
                    AnalyzeParam.Instance.Target.SolarPotential = true;
                    AnalyzeParam.Instance.Target.Reflection = true;
                    AnalyzeParam.Instance.Target.Build = true;
                    AnalyzeParam.Instance.Target.Land = true;
                }

                AnalyzeParam.Instance.TargetDate = AnalyzeParam.DateType.None;
                str = iniFile.GetString("AnalyzeParam", nameof(AnalyzeParam.Instance.TargetDate), "1,0,0,0,0");
                split = str.Split(',');
                if (split.Count() == 5)
                {
                    if (split[0] == "1") AnalyzeParam.Instance.TargetDate = AnalyzeParam.DateType.OneMonth;
                    if (split[1] == "1") AnalyzeParam.Instance.TargetDate = AnalyzeParam.DateType.OneDay;
                    if (split[2] == "1") AnalyzeParam.Instance.TargetDate |= AnalyzeParam.DateType.Summer;
                    if (split[3] == "1") AnalyzeParam.Instance.TargetDate |= AnalyzeParam.DateType.Winter;
                    if (split[4] == "1") AnalyzeParam.Instance.TargetDate = AnalyzeParam.DateType.Year;
                }
                else
                {
                    AnalyzeParam.Instance.TargetDate = AnalyzeParam.DateType.OneMonth;
                }
                //AnalyzeParam.Instance.TargetDate = (AnalyzeParam.DateType)iniFile.GetInt(nameof(AnalyzeParam), nameof(AnalyzeParam.Instance.TargetDate), 0);
                AnalyzeParam.Instance.Month = iniFile.GetInt(nameof(AnalyzeParam), nameof(AnalyzeParam.Instance.Month), 0);
                AnalyzeParam.Instance.Day = iniFile.GetInt(nameof(AnalyzeParam), nameof(AnalyzeParam.Instance.Day), 0);

                AnalyzeParam.Instance.OutputResultDirectory = iniFile.GetString("AnalyzeParam", "OutputResultDirectory", "");
            }

            {   // 入力データ
                AnalyzeParam.Instance.InputData.CityModel = iniFile.GetString("InputData", "CityModel", "");
                AnalyzeParam.Instance.InputData.KashoData = iniFile.GetString("InputData", "KashoData", "");
                AnalyzeParam.Instance.InputData.NisshoData = iniFile.GetString("InputData", "NisshoData", "");
                AnalyzeParam.Instance.InputData.SnowDepthData = iniFile.GetString("InputData", "SnowDepthData", "");
                AnalyzeParam.Instance.InputData.UseDemData = (iniFile.GetInt("InputData", "UseDemData", 0) == 0) ? false : true;
                AnalyzeParam.Instance.InputData.LandData = iniFile.GetString("InputData", "LandData", "");
                AnalyzeParam.Instance.InputData.ReadLandData = (iniFile.GetInt("InputData", "ReadLandData", 0) == 0) ? false : true;
                AnalyzeParam.Instance.InputData.LandDataDatum = iniFile.GetInt("InputData", "LandDataDatum", 0);
                AnalyzeParam.Instance.InputData.UseRoadData = (iniFile.GetInt(nameof(AnalyzeParam.InputData), nameof(AnalyzeParam.InputData.UseRoadData), 0) == 0) ? false : true;
            }

            {
                AnalyzeParam.Instance.AreaOutputImageRange = iniFile.GetString("AnalyzeParam", "AreaOutputImageRange", "0");

            }

            // 解析エリア
            ReadAreaDataParams(file);

            // 発電ポテンシャル推計設定
            ReadSolarPotentialParams(file);

            // 反射シミュレーション設定
            ReadReflectionParams(file);

            return true;

        }

        /// <summary>
        /// 解析パラメータファイル読み込み(解析エリア)
        /// </summary>
        /// <param name="file"></param>
        /// <returns></returns>
        public bool ReadAreaDataParams(string file)
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            if (!File.Exists(file)) return false;

            // 読み込み
            INIFile iniFile = new INIFile(file);

            string section = nameof(AreaData);

            int AreaCount = iniFile.GetInt(section, "AreaCount", 0);
            AnalyzeParam.Instance.AreaList.Clear();

            for (int n = 0; n < AreaCount; n++)
            {
                AreaData area = new AreaData();
                area.Id = iniFile.GetString(section, $"{nameof(AreaData.Id)}{n}", ""); ;
                area.FeatureId = iniFile.GetInt(section, $"{nameof(AreaData.FeatureId)}{n}", 999);
                area.Name = iniFile.GetString(section, $"{nameof(AreaData.Name)}{n}", ""); ;
                area.Direction = iniFile.GetInt(section, $"{nameof(AreaData.Direction)}{n}", 2);
                area.Degree = iniFile.GetInt(section, $"{nameof(AreaData.Degree)}{n}", 15);
                area.Explanation = iniFile.GetString(section, $"{nameof(AreaData.Explanation)}{n}", ""); ;
                area.Water = (iniFile.GetInt(section, $"{nameof(AreaData.Water)}{n}", 0) == 0) ? false : true;
                area.AnalyzeBuild = (iniFile.GetInt(section, $"{nameof(AreaData.AnalyzeBuild)}{n}", 0) == 0) ? false : true;
                area.AnalyzeLand = (iniFile.GetInt(section, $"{nameof(AreaData.AnalyzeLand)}{n}", 0) == 0) ? false : true;
                area.IsShpData = (iniFile.GetInt(section, $"{nameof(AreaData.IsShpData)}{n}", 0) == 0) ? false : true;
                area.Exclusion_flg = (iniFile.GetInt(section, $"{nameof(AreaData.Exclusion_flg)}{n}", 0) == 0) ? false : true;

                string points =iniFile.GetString(section, $"{nameof(AreaData.Points)}{n}", "");
                string[] split = points.Split(',');
                for (int i = 0; i < split.Count(); i+=2)
                {
                    double.TryParse(split[i], out double lon);
                    double.TryParse(split[i + 1], out double lat);
                    Point2D pt = new Point2D { Lat = lat, Lon = lon };
                    area.Points.Add(pt);
                }

                AnalyzeParam.Instance.AreaList.Add(area);
            }

            AnalyzeParam.Instance.LastAreaIdNum = iniFile.GetInt(section, "LastId", 0);

            return true;
        }

        /// <summary>
        /// 解析パラメータファイル読み込み(発電ポテンシャル推計設定)
        /// </summary>
        /// <param name="file"></param>
        /// <returns></returns>
        public bool ReadSolarPotentialParams(string file)
        {

            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            if (!File.Exists(file)) return false;

            // 読み込み
            INIFile iniFile = new INIFile(file);

            AnalyzeParam.Instance.SolarPotential.Roof.Area = iniFile.GetString("SolarPotential", "Roof_Area", "10");
            AnalyzeParam.Instance.SolarPotential.Roof.SlopeDegree = iniFile.GetString("SolarPotential", "Roof_SlopeDegree", "60");
            AnalyzeParam.Instance.SolarPotential.Roof.DirDeg.Direction = iniFile.GetInt("SolarPotential", "Roof_Direction", 0);
            AnalyzeParam.Instance.SolarPotential.Roof.DirDeg.Degree = iniFile.GetString("SolarPotential", "Roof_Degree", "15");
            AnalyzeParam.Instance.SolarPotential.Roof.Interior = (iniFile.GetInt("SolarPotential", "Roof_Interior", 0) == 0) ? false : true;
            AnalyzeParam.Instance.SolarPotential.Roof.CorrectionCaseDeg = iniFile.GetString("SolarPotential", "Roof_CorrectionCaseDeg", "3");
            AnalyzeParam.Instance.SolarPotential.Roof.CorrectionDirDeg.Direction = iniFile.GetInt("SolarPotential", "Roof_CorrectionDirection", 2);
            AnalyzeParam.Instance.SolarPotential.Roof.CorrectionDirDeg.Degree = iniFile.GetString("SolarPotential", "Roof_CorrectionDegree", "15");

            AnalyzeParam.Instance.SolarPotential.Land.Area = iniFile.GetString("SolarPotential", "Land_Area", "10");
            AnalyzeParam.Instance.SolarPotential.Land.SlopeDegree = iniFile.GetString("SolarPotential", "Land_SlopeDegree", "60");
            AnalyzeParam.Instance.SolarPotential.Land.CorrectionDirDeg.Direction = iniFile.GetInt("SolarPotential", "Land_CorrectionDirection", 2);
            AnalyzeParam.Instance.SolarPotential.Land.CorrectionDirDeg.Degree = iniFile.GetString("SolarPotential", "Land_CorrectionDegree", "15");

            AnalyzeParam.Instance.SolarPotential.PanelMakerSolarPower = iniFile.GetString("SolarPotential", "PanelMakerSolarPower", "0.167");
            AnalyzeParam.Instance.SolarPotential.PanelRatio = iniFile.GetString("SolarPotential", "PanelRatio", "80");

            return true;
        }

        /// <summary>
        /// 解析パラメータファイル読み込み(反射シミュレーション設定)
        /// </summary>
        /// <param name="file"></param>
        /// <returns></returns>
        public bool ReadReflectionParams(string file)
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            if (!File.Exists(file)) return false;

            // 読み込み
            INIFile iniFile = new INIFile(file);

            AnalyzeParam.Instance.Reflection.Roof[ReflectionParams.SlopeConditions.Lower].UseCustomVal = (iniFile.GetInt("Reflection", "Roof_Lower_UseCustomVal", 1) == 0) ? false : true;
            AnalyzeParam.Instance.Reflection.Roof[ReflectionParams.SlopeConditions.Lower].DirDeg.Direction = iniFile.GetInt("Reflection", "Roof_Lower_Direction", 2);
            AnalyzeParam.Instance.Reflection.Roof[ReflectionParams.SlopeConditions.Lower].DirDeg.Degree = iniFile.GetString("Reflection", "Roof_Lower_Degree", "15");
            AnalyzeParam.Instance.Reflection.Roof[ReflectionParams.SlopeConditions.Upper].UseCustomVal = (iniFile.GetInt("Reflection", "Roof_Upper_UseCustomVal", 0) == 0) ? false : true;
            AnalyzeParam.Instance.Reflection.Roof[ReflectionParams.SlopeConditions.Upper].DirDeg.Direction = iniFile.GetInt("Reflection", "Roof_Upper_Direction", -1);
            AnalyzeParam.Instance.Reflection.Roof[ReflectionParams.SlopeConditions.Upper].DirDeg.Degree = iniFile.GetString("Reflection", "Roof_Upper_Degree", "");

            AnalyzeParam.Instance.Reflection.Land[ReflectionParams.SlopeConditions.Lower].UseCustomVal = (iniFile.GetInt("Reflection", "Land_Lower_UseCustomVal", 1) == 0) ? false : true;
            AnalyzeParam.Instance.Reflection.Land[ReflectionParams.SlopeConditions.Lower].DirDeg.Direction = iniFile.GetInt("Reflection", "Land_Lower_Direction", 2);
            AnalyzeParam.Instance.Reflection.Land[ReflectionParams.SlopeConditions.Lower].DirDeg.Degree = iniFile.GetString("Reflection", "Land_Lower_Degree", "15");
            AnalyzeParam.Instance.Reflection.Land[ReflectionParams.SlopeConditions.Upper].UseCustomVal = (iniFile.GetInt("Reflection", "Land_Upper_UseCustomVal", 0) == 0) ? false : true;
            AnalyzeParam.Instance.Reflection.Land[ReflectionParams.SlopeConditions.Upper].DirDeg.Direction = iniFile.GetInt("Reflection", "Land_Upper_Direction", -1);
            AnalyzeParam.Instance.Reflection.Land[ReflectionParams.SlopeConditions.Upper].DirDeg.Degree = iniFile.GetString("Reflection", "Land_Upper_Degree", "");

            AnalyzeParam.Instance.Reflection.ReflectionRange = iniFile.GetString("Reflection", "ReflectionRange", "500");

            return true;
        }

        /// <summary>
        /// 判定・集計パラメータファイル読み込み
        /// </summary>
        /// <param name="file"></param>
        /// <returns></returns>
        public bool ReadAggregateParams(string file)
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            if (!File.Exists(file)) return false;

            // 読み込み
            INIFile iniFile = new INIFile(file);

            {   // 共通パラメータ
                AggregateParam.Instance.SelectArea = (iniFile.GetInt("AggregateParam", "SelectArea", 0) == 0) ? false : true;
                AggregateParam.Instance.OutputResultDirectory = iniFile.GetString("AggregateParam", "OutputResultDirectory", "");
                AggregateParam.Instance.AnalyzeResultPath = iniFile.GetString("AggregateParam", "AnalyzeResultPath", "");
            }

            // 発電ポテンシャル推計設定
            ReadJudgeParams(file);

            return true;
        }

        /// <summary>
        /// 適地判定設定読み込み
        /// </summary>
        /// <param name="file"></param>
        public bool ReadJudgeParams(string file)
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            if (!File.Exists(file)) return false;

            // 読み込み
            INIFile iniFile = new INIFile(file);

            AggregateParam.Instance.Judge.LowerPotential = (iniFile.GetInt("Judge", "LowerPotential", 0) == 0) ? false : true;
            AggregateParam.Instance.Judge.Potential = (iniFile.GetInt("Judge", "Potential", 0) == 0) ? false : true;
            AggregateParam.Instance.Judge.PotentialVal = iniFile.GetString("Judge", "PotentialVal", "");
            AggregateParam.Instance.Judge.PotentialPercent = (iniFile.GetInt("Judge", "PotentialPercent", 0) == 0) ? false : true;
            AggregateParam.Instance.Judge.PotentialPercentVal = iniFile.GetString("Judge", "PotentialPercentVal", "");

            AggregateParam.Instance.Judge.BuildStructure = (iniFile.GetInt("Judge", "BuildStructure", 0) == 0) ? false : true;
            string str = iniFile.GetString("Judge", "BuildStructures", "0,0,0,0,0,0,0,0");
            string[] split = str.Split(',');
            if (split.Count() == 8)
            {
                if (split[0] == "1") AggregateParam.Instance.Judge.BuildStructureFlag |= JudgeParams.BuildStructures.Wood;
                if (split[1] == "1") AggregateParam.Instance.Judge.BuildStructureFlag |= JudgeParams.BuildStructures.SteelReinforcedConcrete;
                if (split[2] == "1") AggregateParam.Instance.Judge.BuildStructureFlag |= JudgeParams.BuildStructures.ReinforcedConcrete;
                if (split[3] == "1") AggregateParam.Instance.Judge.BuildStructureFlag |= JudgeParams.BuildStructures.Steel;
                if (split[4] == "1") AggregateParam.Instance.Judge.BuildStructureFlag |= JudgeParams.BuildStructures.LightGaugeSteel;
                if (split[5] == "1") AggregateParam.Instance.Judge.BuildStructureFlag |= JudgeParams.BuildStructures.MasonryConstruction;
                if (split[6] == "1") AggregateParam.Instance.Judge.BuildStructureFlag |= JudgeParams.BuildStructures.Unknown;
                if (split[7] == "1") AggregateParam.Instance.Judge.BuildStructureFlag |= JudgeParams.BuildStructures.NonWood;
            }
            else
            {
                AggregateParam.Instance.Judge.BuildStructureFlag = JudgeParams.BuildStructures.None;
            }

            AggregateParam.Instance.Judge.BuildFloors = (iniFile.GetInt("Judge", "BuildFloors", 0) == 0) ? false : true;
            AggregateParam.Instance.Judge.FloorsBelowVal = iniFile.GetString("Judge", "FloorsBelowVal", "");
            AggregateParam.Instance.Judge.UpperFloorsVal = iniFile.GetString("Judge", "UpperFloorsVal", "");

            AggregateParam.Instance.Judge.BelowTsunamiHeight = (iniFile.GetInt("Judge", "BelowTsunamiHeight", 0) == 0) ? false : true;
            AggregateParam.Instance.Judge.BelowFloodDepth = (iniFile.GetInt("Judge", "BelowFloodDepth", 0) == 0) ? false : true;
            AggregateParam.Instance.Judge.LandslideWarningArea = (iniFile.GetInt("Judge", "LandslideWarningArea", 0) == 0) ? false : true;

            AggregateParam.Instance.Judge.WeatherData = (iniFile.GetInt("Judge", "WeatherData", 0) == 0) ? false : true;
            AggregateParam.Instance.Judge.WeatherDataPath = iniFile.GetString("Judge", "WeatherDataPath", "");
            AggregateParam.Instance.Judge.UseSnowDepth = (iniFile.GetInt("Judge", "UseSnowDepth", 0) == 0) ? false : true;
            AggregateParam.Instance.Judge.SnowDepthVal = iniFile.GetString("Judge", "SnowDepthVal", "");
            AggregateParam.Instance.Judge.SnowLoadVal = iniFile.GetString("Judge", "SnowLoadVal", "");
            AggregateParam.Instance.Judge.SnowLoadUnitVal = iniFile.GetString("Judge", "SnowLoadUnitVal", "");

            for (int n = 0; n < 3; n++)
            {
                AggregateParam.Instance.Judge.RestrictAreas[n].Enable = (iniFile.GetInt("Judge", $"RestrictArea{n+1}_Enable", 0) == 0) ? false : true;
                AggregateParam.Instance.Judge.RestrictAreas[n].ShapePath = iniFile.GetString("Judge", $"RestrictArea{n+1}_ShapePath", "");
                AggregateParam.Instance.Judge.RestrictAreas[n].Height = iniFile.GetString("Judge", $"RestrictArea{n+1}_Height", "");
                AggregateParam.Instance.Judge.RestrictAreas[n].Direction = iniFile.GetInt("Judge", $"RestrictArea{n+1}_Direction", 0);
                AggregateParam.Instance.Judge.RestrictAreas[n].Datum = iniFile.GetInt("Judge", $"RestrictArea{n+1}_Datum", 0);
            }

            return true;
        }


        /// <summary>
        /// 現在の解析パラメータを保存する
        /// </summary>
        /// <param name="file"></param>
        /// <returns></returns>
        public bool SaveAnalyzeParams(string file)
        {
            try
            {
                LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

                if (!File.Exists(file)) return false;

                // 書き込み
                INIFile iniFile = new INIFile(file);

                {   // 共通パラメータ
                    string str = "";
                    str += AnalyzeParam.Instance.Target.SolarPotential ? "1," : "0,";
                    str += AnalyzeParam.Instance.Target.Reflection ? "1," : "0,";
                    str += AnalyzeParam.Instance.Target.Build ? "1," : "0,";
                    str += AnalyzeParam.Instance.Target.Land ? "1" : "0";
                    iniFile.WriteString(nameof(AnalyzeParam), nameof(AnalyzeParam.Instance.Target), str);

                    str = "";
                    str += AnalyzeParam.Instance.TargetDate.HasFlag(AnalyzeParam.DateType.OneMonth) ? "1," : "0,";
                    str += AnalyzeParam.Instance.TargetDate.HasFlag(AnalyzeParam.DateType.OneDay) ? "1," : "0,";
                    str += AnalyzeParam.Instance.TargetDate.HasFlag(AnalyzeParam.DateType.Summer) ? "1," : "0,";
                    str += AnalyzeParam.Instance.TargetDate.HasFlag(AnalyzeParam.DateType.Winter) ? "1," : "0,";
                    str += AnalyzeParam.Instance.TargetDate.HasFlag(AnalyzeParam.DateType.Year) ? "1" : "0";
                    iniFile.WriteString(nameof(AnalyzeParam), nameof(AnalyzeParam.Instance.TargetDate), str);
                    iniFile.WriteString(nameof(AnalyzeParam), nameof(AnalyzeParam.Instance.Month), $"{(int)AnalyzeParam.Instance.Month}");
                    iniFile.WriteString(nameof(AnalyzeParam), nameof(AnalyzeParam.Instance.Day), $"{(int)AnalyzeParam.Instance.Day}");

                    iniFile.WriteString(nameof(AnalyzeParam), nameof(AnalyzeParam.Instance.OutputResultDirectory), AnalyzeParam.Instance.OutputResultDirectory);

                    iniFile.WriteString(nameof(AnalyzeParam), nameof(AnalyzeParam.Instance.AreaOutputImageRange), AnalyzeParam.Instance.AreaOutputImageRange);
                }

                {   // 入力データ
                    AnalyzeInputData inputData = AnalyzeParam.Instance.InputData;
                    string section = nameof(inputData);
                    iniFile.WriteString(section, nameof(inputData.CityModel), inputData.CityModel);
                    iniFile.WriteString(section, nameof(inputData.KashoData), inputData.KashoData);
                    iniFile.WriteString(section, nameof(inputData.NisshoData), inputData.NisshoData);
                    iniFile.WriteString(section, nameof(inputData.SnowDepthData), inputData.SnowDepthData);
                    iniFile.WriteString(section, nameof(inputData.UseDemData), $"{(inputData.UseDemData ? "1" : "0")}");
                    iniFile.WriteString(section, nameof(inputData.LandData), inputData.LandData);
                    iniFile.WriteString(section, nameof(inputData.ReadLandData), $"{(inputData.ReadLandData ? "1" : "0")}");
                    iniFile.WriteString(section, nameof(inputData.LandDataDatum), $"{inputData.LandDataDatum}");
                    iniFile.WriteString(section, nameof(inputData.UseRoadData), $"{(inputData.UseRoadData ? "1" : "0")}");
                }

                {   // 解析エリア
                    string section = nameof(AreaData);

                    int num = 0;
                    foreach (var area in AnalyzeParam.Instance.AreaList)
                    {
                        iniFile.WriteString(section, $"{nameof(AreaData.Id)}{num}", area.Id);
                        iniFile.WriteString(section, $"{nameof(AreaData.FeatureId)}{num}", $"{area.FeatureId}");
                        iniFile.WriteString(section, $"{nameof(AreaData.Name)}{num}", area.Name);
                        iniFile.WriteString(section, $"{nameof(AreaData.Direction)}{num}", $"{area.Direction}");
                        iniFile.WriteString(section, $"{nameof(AreaData.Degree)}{num}", $"{area.Degree}");
                        iniFile.WriteString(section, $"{nameof(AreaData.Explanation)}{num}", area.Explanation);
                        iniFile.WriteString(section, $"{nameof(AreaData.Water)}{num}", $"{(area.Water ? "1" : "0")}");
                        iniFile.WriteString(section, $"{nameof(AreaData.AnalyzeBuild)}{num}", $"{(area.AnalyzeBuild ? "1" : "0")}");
                        iniFile.WriteString(section, $"{nameof(AreaData.AnalyzeLand)}{num}", $"{(area.AnalyzeLand ? "1" : "0")}");
                        iniFile.WriteString(section, $"{nameof(AreaData.IsShpData)}{num}", $"{(area.IsShpData ? "1" : "0")}");
                        iniFile.WriteString(section, $"{nameof(AreaData.Exclusion_flg)}{num}", $"{(area.Exclusion_flg ? "1" : "0")}");

                        string pointsText = "";
                        foreach (var pt in area.Points)
                        {
                            pointsText += $"{pt.Lon},{pt.Lat},";
                        }
                        pointsText = pointsText.Remove(pointsText.Length - 1);
                        iniFile.WriteString(section, $"{nameof(AreaData.Points)}{num}", $"{pointsText}");

                        num++;
                    }

                    iniFile.WriteString(section, "AreaCount", $"{num}");
                    iniFile.WriteString(section, "LastId", $"{AnalyzeParam.Instance.LastAreaIdNum}");
                }

                // 発電ポテンシャル推計設定
                {
                    SolarPotentialParams param = AnalyzeParam.Instance.SolarPotential;
                    string section = nameof(AnalyzeParam.Instance.SolarPotential);
                    string keyprefix = $"{nameof(param.Roof)}_";
                    iniFile.WriteString(section, $"{keyprefix}{nameof(param.Roof.Area)}", param.Roof.Area);
                    iniFile.WriteString(section, $"{keyprefix}{nameof(param.Roof.SlopeDegree)}", param.Roof.SlopeDegree);
                    iniFile.WriteString(section, $"{keyprefix}{nameof(param.Roof.DirDeg.Direction)}", $"{param.Roof.DirDeg.Direction}");
                    iniFile.WriteString(section, $"{keyprefix}{nameof(param.Roof.DirDeg.Degree)}", param.Roof.DirDeg.Degree);
                    iniFile.WriteString(section, $"{keyprefix}{nameof(param.Roof.Interior)}", $"{(param.Roof.Interior ? "1" : "0")}");
                    iniFile.WriteString(section, $"{keyprefix}{nameof(param.Roof.CorrectionCaseDeg)}", param.Roof.CorrectionCaseDeg);
                    iniFile.WriteString(section, $"{keyprefix}Correction{nameof(param.Roof.CorrectionDirDeg.Direction)}", $"{param.Roof.CorrectionDirDeg.Direction}");
                    iniFile.WriteString(section, $"{keyprefix}Correction{nameof(param.Roof.CorrectionDirDeg.Degree)}", param.Roof.CorrectionDirDeg.Degree);

                    keyprefix = $"{nameof(param.Land)}_";
                    iniFile.WriteString(section, $"{keyprefix}{nameof(param.Land.Area)}", param.Land.Area);
                    iniFile.WriteString(section, $"{keyprefix}{nameof(param.Land.SlopeDegree)}", param.Land.SlopeDegree);
                    iniFile.WriteString(section, $"{keyprefix}Correction{nameof(param.Land.CorrectionDirDeg.Direction)}", $"{param.Land.CorrectionDirDeg.Direction}");
                    iniFile.WriteString(section, $"{keyprefix}Correction{nameof(param.Land.CorrectionDirDeg.Degree)}", param.Land.CorrectionDirDeg.Degree);

                    iniFile.WriteString(section, nameof(param.PanelMakerSolarPower), param.PanelMakerSolarPower);
                    iniFile.WriteString(section, nameof(param.PanelRatio), param.PanelRatio);
                }

                // 反射シミュレーション設定
                {
                    CorrectionParams param = AnalyzeParam.Instance.Reflection.Roof[ReflectionParams.SlopeConditions.Lower];
                    string section = nameof(AnalyzeParam.Instance.Reflection);
                    string keyprefix = $"{nameof(AnalyzeParam.Instance.Reflection.Roof)}_{nameof(ReflectionParams.SlopeConditions.Lower)}_";
                    iniFile.WriteString(section, $"{keyprefix}{nameof(param.UseCustomVal)}", $"{(param.UseCustomVal ? "1" : "0")}");
                    iniFile.WriteString(section, $"{keyprefix}{nameof(param.DirDeg.Direction)}", $"{param.DirDeg.Direction}");
                    iniFile.WriteString(section, $"{keyprefix}{nameof(param.DirDeg.Degree)}", param.DirDeg.Degree);

                    param = AnalyzeParam.Instance.Reflection.Roof[ReflectionParams.SlopeConditions.Upper];
                    keyprefix = $"{nameof(AnalyzeParam.Instance.Reflection.Roof)}_{nameof(ReflectionParams.SlopeConditions.Upper)}_";
                    iniFile.WriteString(section, $"{keyprefix}{nameof(param.UseCustomVal)}", $"{(param.UseCustomVal ? "1" : "0")}");
                    iniFile.WriteString(section, $"{keyprefix}{nameof(param.DirDeg.Direction)}", $"{param.DirDeg.Direction}");
                    iniFile.WriteString(section, $"{keyprefix}{nameof(param.DirDeg.Degree)}", param.DirDeg.Degree);

                    param = AnalyzeParam.Instance.Reflection.Land[ReflectionParams.SlopeConditions.Lower];
                    keyprefix = $"{nameof(AnalyzeParam.Instance.Reflection.Land)}_{nameof(ReflectionParams.SlopeConditions.Lower)}_";
                    iniFile.WriteString(section, $"{keyprefix}{nameof(param.UseCustomVal)}", $"{(param.UseCustomVal ? "1" : "0")}");
                    iniFile.WriteString(section, $"{keyprefix}{nameof(param.DirDeg.Direction)}", $"{param.DirDeg.Direction}");
                    iniFile.WriteString(section, $"{keyprefix}{nameof(param.DirDeg.Degree)}", param.DirDeg.Degree);

                    param = AnalyzeParam.Instance.Reflection.Land[ReflectionParams.SlopeConditions.Upper];
                    keyprefix = $"{nameof(AnalyzeParam.Instance.Reflection.Land)}_{nameof(ReflectionParams.SlopeConditions.Upper)}_";
                    iniFile.WriteString(section, $"{keyprefix}{nameof(param.UseCustomVal)}", $"{(param.UseCustomVal ? "1" : "0")}");
                    iniFile.WriteString(section, $"{keyprefix}{nameof(param.DirDeg.Direction)}", $"{param.DirDeg.Direction}");
                    iniFile.WriteString(section, $"{keyprefix}{nameof(param.DirDeg.Degree)}", param.DirDeg.Degree);

                    iniFile.WriteString(section, nameof(AnalyzeParam.Instance.Reflection.ReflectionRange), AnalyzeParam.Instance.Reflection.ReflectionRange);
                }

                return true;
            }
            catch (Exception ex)
            {
                CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
                LogManager.Instance.ExceptionLog(ex);
                return false;
            }
        }

        /// <summary>
        /// 現在の集計パラメータを保存する
        /// </summary>
        /// <param name="file"></param>
        /// <returns></returns>
        public bool SaveAggregateParams(string file)
        {
            try
            {
                LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

                if (!File.Exists(file)) return false;

                // 書き込み
                INIFile iniFile = new INIFile(file);

                {   // 共通パラメータ
                    iniFile.WriteString(nameof(AggregateParam), nameof(AggregateParam.Instance.SelectArea), $"{(AggregateParam.Instance.SelectArea ? "1" : "0")}");
                    iniFile.WriteString(nameof(AggregateParam), nameof(AggregateParam.Instance.OutputResultDirectory), AggregateParam.Instance.OutputResultDirectory);
                    iniFile.WriteString(nameof(AggregateParam), nameof(AggregateParam.Instance.AnalyzeResultPath), AggregateParam.Instance.AnalyzeResultPath);
                }

                {
                    JudgeParams param = AggregateParam.Instance.Judge;
                    string section = nameof(AggregateParam.Instance.Judge);

                    iniFile.WriteString(section, nameof(param.LowerPotential), $"{(param.LowerPotential ? "1" : "0")}");
                    iniFile.WriteString(section, nameof(param.Potential), $"{(param.Potential ? "1" : "0")}");
                    iniFile.WriteString(section, nameof(param.PotentialVal), param.PotentialVal);
                    iniFile.WriteString(section, nameof(param.PotentialPercent), $"{(param.PotentialPercent ? "1" : "0")}");
                    iniFile.WriteString(section, nameof(param.PotentialPercentVal), param.PotentialPercentVal);

                    iniFile.WriteString(section, nameof(param.BuildStructure), $"{(param.BuildStructure ? "1" : "0")}");
                    string str = "";
                    str += AggregateParam.Instance.Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.Wood) ? "1," : "0,";
                    str += AggregateParam.Instance.Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.SteelReinforcedConcrete) ? "1," : "0,";
                    str += AggregateParam.Instance.Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.ReinforcedConcrete) ? "1," : "0,";
                    str += AggregateParam.Instance.Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.Steel) ? "1," : "0,";
                    str += AggregateParam.Instance.Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.LightGaugeSteel) ? "1," : "0,";
                    str += AggregateParam.Instance.Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.MasonryConstruction) ? "1," : "0,";
                    str += AggregateParam.Instance.Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.Unknown) ? "1," : "0,";
                    str += AggregateParam.Instance.Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.NonWood) ? "1" : "0";
                    iniFile.WriteString(section, nameof(JudgeParams.BuildStructures), str);

                    iniFile.WriteString(section, nameof(param.BuildFloors), $"{(param.BuildFloors ? "1" : "0")}");
                    iniFile.WriteString(section, nameof(param.FloorsBelowVal), param.FloorsBelowVal);
                    iniFile.WriteString(section, nameof(param.UpperFloorsVal), param.UpperFloorsVal);

                    iniFile.WriteString(section, nameof(param.BelowTsunamiHeight), $"{(param.BelowTsunamiHeight ? "1" : "0")}");
                    iniFile.WriteString(section, nameof(param.BelowFloodDepth), $"{(param.BelowFloodDepth ? "1" : "0")}");
                    iniFile.WriteString(section, nameof(param.LandslideWarningArea), $"{(param.LandslideWarningArea ? "1" : "0")}");

                    iniFile.WriteString(section, nameof(param.WeatherData), $"{(param.WeatherData ? "1" : "0")}");
                    iniFile.WriteString(section, nameof(param.WeatherDataPath), param.WeatherDataPath);
                    iniFile.WriteString(section, nameof(param.UseSnowDepth), $"{(param.UseSnowDepth ? "1" : "0")}");
                    iniFile.WriteString(section, nameof(param.SnowDepthVal), param.SnowDepthVal);
                    iniFile.WriteString(section, nameof(param.SnowLoadVal), param.SnowLoadVal);
                    iniFile.WriteString(section, nameof(param.SnowLoadUnitVal), param.SnowLoadUnitVal);

                    for (int n = 0; n < 3; n++)
                    {
                        var area = AggregateParam.Instance.Judge.RestrictAreas[n];
                        string keyprefix = $"{nameof(JudgeParams.RestrictArea)}{n + 1}_";
                        iniFile.WriteString(section, $"{keyprefix}{nameof(area.Enable)}", $"{(area.Enable ? "1" : "0")}");
                        iniFile.WriteString(section, $"{keyprefix}{nameof(area.ShapePath)}", area.ShapePath);
                        iniFile.WriteString(section, $"{keyprefix}{nameof(area.Height)}", area.Height);
                        iniFile.WriteString(section, $"{keyprefix}{nameof(area.Direction)}", $"{area.Direction}");
                        iniFile.WriteString(section, $"{keyprefix}{nameof(area.Datum)}", $"{area.Datum}");
                    }

                }

                return true;
            }
            catch (Exception ex)
            {
                CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
                LogManager.Instance.ExceptionLog(ex);
                return false;
            }
        }


        private static ParamFileManager _instance = null;

        public static ParamFileManager Instance
        {
            get
            {
                if (_instance == null)
                {
                    _instance = new ParamFileManager();

                    if (!Directory.Exists(_instance.HistoryDirectory))
                    {
                        // 履歴フォルダを作成
                        Directory.CreateDirectory(_instance.HistoryDirectory);
                    }

                }
                return _instance;
            }
        }
    }
}
