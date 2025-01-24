using System;
using System.IO;
using System.Collections.Generic;
using System.Collections.ObjectModel;

namespace SolarPotential
{
    /// <summary>
    /// 解析・シミュレーション処理パラメータ
    /// </summary>
    class AnalyzeParam : BindableBase
    {
        /// <summary>
        /// 解析対象
        /// </summary>
        public AnalyzeTargets Target { get; set; } = new AnalyzeTargets();

        /// <summary>
        /// 解析対象日
        /// </summary>
        [Flags]
        public enum DateType
        {
            None = 0,

            /// <summary>
            /// 指定月
            /// </summary>
            OneMonth = 1 << 0,

            /// <summary>
            /// 指定日
            /// </summary>
            OneDay = 1 << 1,

            /// <summary>
            /// 夏至
            /// </summary>
            Summer = 1 << 2,

            /// <summary>
            /// 冬至
            /// </summary>
            Winter = 1 << 3,

            /// <summary>
            /// 年間
            /// </summary>
            Year = 1 << 4
        }

        public DateType TargetDate { get; set; } = DateType.None;

        /// <summary>
        /// 解析対象月
        /// </summary>
        public int Month { get; set; }

        /// <summary>
        /// 解析対象日
        /// </summary>
        public int Day { get; set; }

        /// <summary>
        /// 入力データ
        /// </summary>
        public AnalyzeInputData InputData { get; set; } = new AnalyzeInputData();

        /// <summary>
        /// 解析エリアデータ
        /// </summary>
        public ObservableCollection<AreaData> AreaList { get; set; } = new ObservableCollection<AreaData>();

        /// <summary>
        /// 最後に追加したエリアID番号
        /// </summary>
        public int LastAreaIdNum { get; set; } = 0;

        private string _AreaOutputImageRange;
        /// <summary>
        /// エリア拡張範囲
        /// </summary>
        public string AreaOutputImageRange
        {
            get => this._AreaOutputImageRange;
            set => base.SetProperty(ref _AreaOutputImageRange, value);
        }

        /// <summary>
        /// 発電ポテンシャル推計設定
        /// </summary>
        public SolarPotentialParams SolarPotential { get; set; } = new SolarPotentialParams();

        /// <summary>
        /// 反射シミュレーション設定
        /// </summary>
        public ReflectionParams Reflection { get; set; } = new ReflectionParams();

        private string _OutputResultDirectory;
        /// <summary>
        /// 解析結果出力フォルダ
        /// </summary>
        public string OutputResultDirectory
        {
            get => this._OutputResultDirectory;
            set => base.SetProperty(ref _OutputResultDirectory, value);
        }

        /// <summary>
        /// 全域解析時のエリアID
        /// </summary>
        public static readonly string ALLAREA_ID = "A000";

        /// <summary>
        /// 全域解析時のエリア名称
        /// </summary>
        public static readonly string ALLAREA_NAME = "CityGML全域";

        /// <summary>
        /// 解析パラメータ初期値格納ファイル
        /// </summary>
        private readonly string FILE_INIT_ANALYZE = "initFile_Analyze.ini";

        /// <summary>
        /// 解析パラメータ前回値格納ファイル
        /// </summary>
        public static readonly string FILE_PRE_ANALYZE = "preFile_Analyze.param";


        AnalyzeParam()
        {
        }

        /// <summary>
        /// 解析パラメータ文字列を取得
        /// </summary>
        public void GetParamsText(out string inputParams, out string customParams)
        {
            // 入力パラメータ
            inputParams = $"【解析対象】\n";
            inputParams += $"　日射量・発電量推計：{(Target.SolarPotential ? "○" : "×")}" +
                         $"　反射シミュレーション：{(Target.Reflection ? "○" : "×")}\n";
            inputParams += $"　建物：{(Target.Build ? "○" : "×")}" +
                         $"　空地：{(Target.Land ? "○" : "×")}\n";
            inputParams += $"【解析対象期間】\n";
            inputParams += $"　期間：";
            if (TargetDate == DateType.OneMonth) { inputParams += $"　{Month}月\n"; }
            else if (TargetDate == DateType.OneDay) { inputParams += $"　{Month}月 {Day}日\n"; }
            else if (TargetDate == DateType.Year) { inputParams += $"　年間\n"; }
            else {
                string str = "";
                if (TargetDate.HasFlag(DateType.Summer)) str += "　夏至,";
                if (TargetDate.HasFlag(DateType.Winter)) str += "　冬至,";
                str = str.Remove(str.Length - 1);
                str += "\n";
                inputParams += $"{str}";

            }
            inputParams += $"\n";
            inputParams += $"【入力データ】\n";
            inputParams += $"3D都市モデル：{InputData.CityModel}\n";
            inputParams += $"可照時間：{InputData.KashoData}\n";
            inputParams += $"平均日照時間：{InputData.NisshoData}\n";
            if(!string.IsNullOrEmpty(InputData.SnowDepthData))
            {
                inputParams += $"積雪深：{InputData.SnowDepthData}\n";
            }
            if (InputData.UseDemData)
            {
                inputParams += $"地形を考慮したシミュレーションを行う：○\n";
            }
            if (Target.Land)
            {
                inputParams += $"空地データ：{InputData.LandData}\n";

                if (InputData.UseRoadData)
                {
                    inputParams += $"道路を除外したシミュレーションを行う：○\n";
                }
            }
            inputParams += $"\n";
            inputParams += $"出力フォルダ：{OutputResultDirectory}\n";

            // 解析エリア
            inputParams += $"\n";
            inputParams += $"【解析エリア】\n";
            foreach (var area in AreaList)
            {
                inputParams += $"{area.Id}, {area.Name}, 方位( {CommonManager.Directions4[area.Direction]} ), 傾き( {area.Degree} )度, " +
                               $"水面( {(area.Water ? "○" : "×")} ), 説明( {area.Explanation} )\n";
			}

            // 詳細設定パラメータ
            customParams = $"【発電ポテンシャル推計条件】\n";
            if (Target.Build)
            {
                customParams += $"屋根面の解析対象外とする条件：\n";
                customParams += $"　面積( {SolarPotential.Roof.Area} )m2未満, 傾き( {SolarPotential.Roof.SlopeDegree} )度以上\n";
                customParams += $"　方位が( {CommonManager.Directions4[SolarPotential.Roof.DirDeg.Direction]} )かつ傾き( {SolarPotential.Roof.DirDeg.Degree} )度以上\n";
                customParams += $"　インテリア面の除外：( {(SolarPotential.Roof.Interior ? "○" : "×")} )\n";
                customParams += $"屋根面の傾斜補正：\n";
                customParams += $"　傾き( {SolarPotential.Roof.Area} )度未満の場合、" +
                             $"方位を( {CommonManager.Directions4[SolarPotential.Roof.CorrectionDirDeg.Direction]} )" +
                             $"かつ( {SolarPotential.Roof.CorrectionDirDeg.Degree} )度に補正\n";
            }
            if (Target.Land)
            {
                customParams += $"土地面の解析対象外とする条件：\n";
                customParams += $"　面積( {SolarPotential.Land.Area} )m2未満, 傾き( {SolarPotential.Land.SlopeDegree} )度以上\n";
                customParams += $"土地面の傾斜補正：\n";
                customParams += $"　方位を( {CommonManager.Directions4[SolarPotential.Land.CorrectionDirDeg.Direction]} )" +
                             $"かつ( {SolarPotential.Land.CorrectionDirDeg.Degree} )度に補正\n";
            }
            customParams += $"メーカー別設置係数( {SolarPotential.PanelMakerSolarPower} )\n";
            customParams += $"パネル設置割合( {SolarPotential.PanelRatio} )%\n";
            customParams += $"\n";
            customParams += $"【反射シミュレーション条件】\n";
            if (Target.Build)
            {
                customParams += $"太陽光パネル面の向き・傾きの補正：\n";
                if (Reflection.Roof[ReflectionParams.SlopeConditions.Lower].UseCustomVal)
                {
                    customParams += $"　3度未満：( 方位を( {CommonManager.Directions4[Reflection.Roof[ReflectionParams.SlopeConditions.Lower].DirDeg.Direction]} ), " +
                                    $"( {Reflection.Roof[ReflectionParams.SlopeConditions.Lower].DirDeg.Degree} )度に補正 )\n";
                }
                else
                {
                    customParams += $"　3度未満：( 屋根面と同値 )\n";
                }
                if (Reflection.Roof[ReflectionParams.SlopeConditions.Upper].UseCustomVal)
                {
                    customParams += $"　3度以上：( 方位を( {CommonManager.Directions4[Reflection.Roof[ReflectionParams.SlopeConditions.Upper].DirDeg.Direction]} ), " +
                                    $"( {Reflection.Roof[ReflectionParams.SlopeConditions.Upper].DirDeg.Degree} )度に補正 )\n";
                }
                else
                {
                    customParams += $"　3度以上：( 屋根面と同値 )\n";
                }
            }
            if (Target.Land)
            {
                customParams += $"太陽光パネル面の向き・傾きの補正：\n";
                if (Reflection.Land[ReflectionParams.SlopeConditions.Lower].UseCustomVal)
                {
                    customParams += $"　3度未満：( 方位を( {CommonManager.Directions4[Reflection.Land[ReflectionParams.SlopeConditions.Lower].DirDeg.Direction]} ), " +
                                    $"( {Reflection.Land[ReflectionParams.SlopeConditions.Lower].DirDeg.Degree} )度に補正 )\n";
                }
                else
                {
                    customParams += $"　3度未満：( 土地面と同値 )\n";
                }
                if (Reflection.Land[ReflectionParams.SlopeConditions.Upper].UseCustomVal)
                {
                    customParams += $"　3度未満：( 方位を( {CommonManager.Directions4[Reflection.Land[ReflectionParams.SlopeConditions.Upper].DirDeg.Direction]} ), " +
                                    $"( {Reflection.Land[ReflectionParams.SlopeConditions.Upper].DirDeg.Degree} )度に補正 )\n";
                }
                else
                {
                    customParams += $"　3度以上：( 屋根面と同値 )\n";
                }
            }
            customParams += $"反射有効範囲( {Reflection.ReflectionRange} )m\n";

        }

        /// <summary>
        /// 解析パラメータ初期化
        /// </summary>
        /// <param name="file"></param>
        /// <returns></returns>
        public bool InitParams()
        {
            try
            {
                LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

                string file = Path.Combine(Directory.GetCurrentDirectory(), "Assets", FILE_INIT_ANALYZE);
                LogManager.Instance.OutputLogMessage($"InitParameter File : {file}", LogManager.LogType.INFO);

                if (!File.Exists(file))
                {
                    throw new Exception("パラメータ初期値格納ファイルが存在しません。");
                }

                ParamFileManager.Instance.ReadAnalyzeParams(file);

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
        /// 発電ポテンシャル推計パラメータ初期化
        /// </summary>
        /// <param name="file"></param>
        /// <returns></returns>
        public bool InitSolarPotentialParam()
        {
            try
            {
                LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
                string file = Path.Combine(Directory.GetCurrentDirectory() + "/Assets/" + FILE_INIT_ANALYZE);
                LogManager.Instance.OutputLogMessage($"InitParameter File : {file}", LogManager.LogType.INFO);

                if (!File.Exists(file))
                {
                    throw new Exception("パラメータ初期値格納ファイルが存在しません。");
                }

                ParamFileManager.Instance.ReadSolarPotentialParams(file);

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
        /// 反射シミュレーションパラメータ初期化
        /// </summary>
        /// <param name="file"></param>
        /// <returns></returns>
        public bool InitReflectionParam()
        {
            try
            {
                LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
                string file = Path.Combine(Directory.GetCurrentDirectory() + "/Assets/" + FILE_INIT_ANALYZE);
                LogManager.Instance.OutputLogMessage($"InitParameter File : {file}", LogManager.LogType.INFO);

                if (!File.Exists(file))
                {
                    throw new Exception("パラメータ初期値格納ファイルが存在しません。");
                }

                ParamFileManager.Instance.ReadReflectionParams(file);

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
        /// 解析パラメータを保存
        /// </summary>
        /// <param name="outDir">出力フォルダ</param>
        /// <param name="date">実行日時</param>
        /// <returns></returns>
        public bool SaveParams(string outDir, DateTime date)
        {
            try
            {
                LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

                // 出力フォルダ作成
                CommonManager.Instance.CreateDirectory(outDir);

                // 初期値格納ファイルをコピー
                string srcfile = Path.Combine(Directory.GetCurrentDirectory(), "Assets", FILE_INIT_ANALYZE);
                string paramfile = Path.Combine(outDir, ResultData.ParamData.ANALYZE_PARAM_FILENAME);
                LogManager.Instance.OutputLogMessage($"InitParameter File : {paramfile}", LogManager.LogType.INFO);
                File.Copy(srcfile, paramfile, true);

                // 現在のパラメータを保存
                ParamFileManager.Instance.SaveAnalyzeParams(paramfile);

                // 前回の履歴ファイルを上書き
                string prefile = Path.Combine(ParamFileManager.Instance.HistoryDirectory, FILE_PRE_ANALYZE);
                File.Copy(paramfile, prefile, true);

                // 履歴に追加
                ParamFileManager.Instance.AddHistory(paramfile, date);

                // パラメータ文字列を生成
                GetParamsText(out string inputparams, out string customparams);
                string paramtext = $"{inputparams}\n{customparams}";

                // パラメータ確認用のテキストファイルを出力
                string textfile = Path.Combine(Directory.GetParent(outDir).FullName, ResultData.ParamData.PARAM_LOG_FILENAME);
                using (StreamWriter sw = new StreamWriter(textfile, true, System.Text.Encoding.GetEncoding("Shift_JIS")))
                {
                    // UIから入力したパラメータを出力
                    sw.WriteLine(paramtext);

                    // iniファイルの設定を出力
                    INIFile iniFile = new INIFile(CommonManager.SystemIniFilePath);
                    sw.WriteLine("【SolarPotential.iniファイルの設定条件】");
                    double dist = 0.0;
                    dist = iniFile.GetDouble("SolarRadiation", "NeighborBuildDist", 0.0);
                    sw.WriteLine($"発電ポテンシャル推計時の近隣建物の検索範囲：{string.Format("{0:F1}", dist)}");
                    dist = iniFile.GetDouble("ReflectionSimulator", "NeighborBuildDist", 0.0);
                    sw.WriteLine($"反射シミュレーション時の近隣建物の検索範囲：{string.Format("{0:F1}", dist)}");

                }

                LogManager.Instance.OutputLogMessage(paramtext, LogManager.LogType.LOG);

                return true;
            }
            catch (Exception ex)
            {
                CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
                LogManager.Instance.ExceptionLog(ex);
                return false;
            }
        }

        public void Dispose()
        {
            _instance = null;
        }

        private static AnalyzeParam _instance = null;

        public static AnalyzeParam Instance
        {
            get
            {
                if (_instance == null)
                {
                    _instance = new AnalyzeParam();
                }
                return _instance;
            }
        }

    }

    /// <summary>
    /// 解析・シミュレーション入力データ
    /// </summary>
    class AnalyzeInputData : BindableBase
    {
        /// <summary>
        /// 3D都市モデル
        /// </summary>
        public string CityModel
        {
            get => this._CityModel;
            set => base.SetProperty(ref _CityModel, value);
        }

        /// <summary>
        /// 入力3D都市モデルの最大座標
        /// </summary>
        public Point2D MaxPos { get; set; } = new Point2D();

        /// <summary>
        /// 入力3D都市モデルの最小座標
        /// </summary>
        public Point2D MinPos { get; set; } = new Point2D();

        /// <summary>
        /// 可照時間
        /// </summary>
        public string KashoData
        {
            get => this._KashoData;
            set => base.SetProperty(ref _KashoData, value);
        }

        /// <summary>
        /// 平均日照時間
        /// </summary>
        public string NisshoData
        {
            get => this._NisshoData;
            set => base.SetProperty(ref _NisshoData, value);
        }

        /// <summary>
        /// 積雪深
        /// </summary>
        public string SnowDepthData
        {
            get => this._SnowDepthData;
            set => base.SetProperty(ref _SnowDepthData, value);
        }

        /// <summary>
        /// DEMデータ使用フラグ
        /// </summary>
        public bool UseDemData
        {
            get => this._UseDemData;
            set => base.SetProperty(ref _UseDemData, value);
        }

        /// <summary>
        /// 空地データ
        /// </summary>
        public string LandData
        {
            get => this._LandData;
            set => base.SetProperty(ref _LandData, value);
        }

        /// <summary>
        /// 空地データ読み込み済フラグ
        /// </summary>
        public bool ReadLandData { get; set; } = false;

        /// <summary>
        /// 空地データの座標系
        /// </summary>
        public int LandDataDatum { get; set; } = 0;

        /// <summary>
        /// DEMデータ使用フラグ
        /// </summary>
        public bool UseRoadData
        {
            get => this._UseRoadData;
            set => base.SetProperty(ref _UseRoadData, value);
        }

        public AnalyzeInputData()
        {
            // 日本の東西南北端点の緯度経度(国土地理院)より緯度経度の初期値を設定
            MinPos.Lat = 20.4253; MinPos.Lon = 122.9325;
            MaxPos.Lat = 45.5572; MaxPos.Lon = 152.9867;

        }

        #region プロパティ
        private string _CityModel;
        private string _KashoData;
        private string _NisshoData;
        private string _SnowDepthData;
        private bool _UseDemData;
        private string _LandData;
        private bool _UseRoadData;
        #endregion
    }

	/// <summary>
	/// 発電ポテンシャル推計設定
	/// </summary>
	class SolarPotentialParams : BindableBase
    {
        /// <summary>
        /// 屋根面設定
        /// </summary>
        public SolarPotentialParams_Roof Roof { get; set; } = new SolarPotentialParams_Roof();

        /// <summary>
        /// 土地面設定
        /// </summary>
        public SolarPotentialParams_Land Land { get; set; } = new SolarPotentialParams_Land();

        /// <summary>
        /// 発電容量
        /// </summary>
        public string PanelMakerSolarPower
        {
            get => this._PanelMakerSolarPower;
            set => base.SetProperty(ref _PanelMakerSolarPower, value);
        }

        /// <summary>
        /// パネル設置割合
        /// </summary>
        public string PanelRatio
        {
            get => this._PanelRatio;
            set => base.SetProperty(ref _PanelRatio, value);
        }

        #region プロパティ
        private string _PanelMakerSolarPower;
        private string _PanelRatio;
        #endregion
    }

    /// <summary>
    /// 発電ポテンシャル推計設定(屋根面)
    /// </summary>
    class SolarPotentialParams_Roof : BindableBase
    {
        /// <summary>
        /// 除外する面積
        /// </summary>
        public string Area
        {
            get => this._Area;
            set => base.SetProperty(ref _Area, value);
        }

        /// <summary>
        /// 除外する屋根面の傾き
        /// </summary>
        public string SlopeDegree
        {
            get => this._SlopeDegree;
            set => base.SetProperty(ref _SlopeDegree, value);
        }

        /// <summary>
        /// 除外する方位＋傾き
        /// </summary>
        public DirectionDegree DirDeg { get; set; } = new DirectionDegree();

        /// <summary>
        /// interior面の除外(除外:true)
        /// </summary>
        public bool Interior
        {
            get => this._Interior;
            set => base.SetProperty(ref _Interior, value);
        }

        /// <summary>
        /// 補正対象となる傾き
        /// </summary>
        public string CorrectionCaseDeg
        {
            get => this._CorrectionCaseDeg;
            set => base.SetProperty(ref _CorrectionCaseDeg, value);
        }

        /// <summary>
        /// 補正する方位＋傾き
        /// </summary>
        public DirectionDegree CorrectionDirDeg { get; set; } = new DirectionDegree();

        #region プロパティ
        private string _Area;
        private string _SlopeDegree;
        private string _CorrectionCaseDeg;
        private bool _Interior;
        #endregion
    }

    /// <summary>
    /// 発電ポテンシャル推計設定(土地面)
    /// </summary>
    class SolarPotentialParams_Land : BindableBase
    {
        /// <summary>
        /// 除外する面積
        /// </summary>
        public string Area
        {
            get => this._Area;
            set => base.SetProperty(ref _Area, value);
        }

        /// <summary>
        /// 除外する土地面の傾き
        /// </summary>
        public string SlopeDegree
        {
            get => this._SlopeDegree;
            set => base.SetProperty(ref _SlopeDegree, value);
        }

        /// <summary>
        /// 補正する方位＋傾き
        /// </summary>
        public DirectionDegree CorrectionDirDeg { get; set; } = new DirectionDegree();

        #region プロパティ
        private string _Area;
        private string _SlopeDegree;
        #endregion
    }

    /// <summary>
    /// 反射シミュレーション設定
    /// </summary>
    class ReflectionParams : BindableBase
    {
        /// <summary>
        /// 傾きの条件
        /// </summary>
        public enum SlopeConditions
        {
            /// <summary>
            /// 3度未満
            /// </summary>
            Lower,

            /// <summary>
            /// 3度以上
            /// </summary>
            Upper
		}

        /// <summary>
        /// 屋根面の補正条件
        /// </summary>
        public Dictionary<SlopeConditions, CorrectionParams> Roof
        {
            get => this._Roof;
            set => base.SetProperty(ref _Roof, value);
        }

        /// <summary>
        /// 土地面の補正条件
        /// </summary>
        public Dictionary<SlopeConditions, CorrectionParams> Land
        {
            get => this._Land;
            set => base.SetProperty(ref _Land, value);
        }

        /// <summary>
        /// 反射有効範囲
        /// </summary>
        public string ReflectionRange
        {
            get => this._ReflectionRange;
            set => base.SetProperty(ref _ReflectionRange, value);
        }

        public ReflectionParams()
        {
            this.Roof = new Dictionary<SlopeConditions, CorrectionParams>();
            this.Roof.Add(SlopeConditions.Lower, new CorrectionParams());
            this.Roof.Add(SlopeConditions.Upper, new CorrectionParams());
            this.Land = new Dictionary<SlopeConditions, CorrectionParams>();
            this.Land.Add(SlopeConditions.Lower, new CorrectionParams());
            this.Land.Add(SlopeConditions.Upper, new CorrectionParams());
        }


        #region プロパティ
        private Dictionary<SlopeConditions, CorrectionParams> _Roof;
        private Dictionary<SlopeConditions, CorrectionParams> _Land;
        private string _ReflectionRange;
        #endregion
    }

    /// <summary>
    /// 補正設定
    /// </summary>
    class CorrectionParams : BindableBase
    {
        /// <summary>
        /// 指定値を使用する
        /// </summary>
        public bool UseCustomVal { get; set; }

        /// <summary>
        /// 補正する方位＋傾き
        /// </summary>
        public DirectionDegree DirDeg { get; set; } = new DirectionDegree();
    }
}
