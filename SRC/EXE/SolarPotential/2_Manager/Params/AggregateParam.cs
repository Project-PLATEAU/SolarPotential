using System;
using System.IO;
using System.Collections.Generic;

namespace SolarPotential
{
    /// <summary>
    /// 適地判定・集計処理パラメータ
    /// </summary>
    class AggregateParam : BindableBase
    {
        ///// <summary>
        ///// 集計範囲の出力ファイル名
        ///// </summary>
        //public static readonly string FILE_AGGREGATE_RANGE = "集計範囲";

        /// <summary>
        /// 解析結果データフォルダパス
        /// </summary>
        public string AnalyzeResultPath
        {
            get => this._AnalyzeResultPath;
            set => base.SetProperty(ref _AnalyzeResultPath, value);
        }

        /// <summary>
        /// 解析時に使用した3D都市モデルデータフォルダのパス
        /// </summary>
        public string CityModelPath { get; set; }

        /// <summary>
        /// 集計範囲を選択する
        /// </summary>
        public bool SelectArea { get; set; } = false;

        /// <summary>
        /// 集計範囲(最大緯度)
        /// </summary>
        public string MaxLat
        {
            get => this._MaxLat;
            set => base.SetProperty(ref _MaxLat, value);
        }

        /// <summary>
        /// 集計範囲(最小緯度)
        /// </summary>
        public string MinLat
        {
            get => this._MinLon;
            set => base.SetProperty(ref _MinLon, value);
        }

        /// <summary>
        /// 集計範囲(最大経度)
        /// </summary>
        public string MaxLon
        {
            get => this._MaxLon;
            set => base.SetProperty(ref _MaxLon, value);
        }

        /// <summary>
        /// 集計範囲(最小緯度)
        /// </summary>
        public string MinLon
        {
            get => this._MinLat;
            set => base.SetProperty(ref _MinLat, value);
        }

        /// <summary>
        /// 解析エリアデータ
        /// </summary>
        public List<AreaData> AnalyzeAreaList { get; set; } = new List<AreaData>();

        /// <summary>
        /// 発電ポテンシャル推計設定
        /// </summary>
        public JudgeParams Judge { get; set; } = new JudgeParams();

        /// <summary>
        /// 結果出力フォルダ
        /// </summary>
        public string OutputResultDirectory
        {
            get => this._OutputResultDirectory;
            set => base.SetProperty(ref _OutputResultDirectory, value);
        }

        /// <summary>
        /// 解析パラメータ初期値格納ファイル
        /// </summary>
        private readonly string FILE_INIT_AGGREGATE = "initFile_Aggregate.ini";

        /// <summary>
        /// 解析パラメータ前回値格納ファイル
        /// </summary>
        public static readonly string FILE_PRE_AGGREGATE = "preFile_Aggregate.param";


        AggregateParam()
        {
        }

        /// <summary>
        /// 判定・集計パラメータ文字列を取得
        /// </summary>
        public void GetParamsText(out string inputParams, out string customParams)
        {
            // 入力パラメータ
            inputParams = $"解析結果フォルダ：{AnalyzeResultPath}\n";
            inputParams += $"\n";
            inputParams += $"【集計範囲】\n";
            if (SelectArea)
            {
                inputParams += string.Format($"　集計範囲を選択：最大緯度{0},最大経度{1},最小緯度{2},最小経度{3}\n",
                                             "",
                                             "",
                                             "",
                                             "");
            }
            else
            {
                inputParams += $"　全範囲で集計\n";
            }
            inputParams += $"\n";
            inputParams += $"出力フォルダ：{OutputResultDirectory}\n";

            // 詳細設定パラメータ
            customParams = $"【優先度が低い建物の除外条件】\n";
            if (Judge.LowerPotential)
            {
                if (Judge.Potential) customParams += $"　日射量が( {Judge.PotentialVal} )未満\n";
                if (Judge.PotentialPercent) customParams += $"　日射量が下位( {Judge.PotentialPercentVal} )％\n";
            }
            if (Judge.BuildStructure)
            {
                string str = "";
                if (Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.Wood)) str += $"{Judge.BuildStructureNames[JudgeParams.BuildStructures.Wood]},";
                if (Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.SteelReinforcedConcrete)) str += $"{Judge.BuildStructureNames[JudgeParams.BuildStructures.SteelReinforcedConcrete]},";
                if (Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.ReinforcedConcrete)) str += $"{Judge.BuildStructureNames[JudgeParams.BuildStructures.ReinforcedConcrete]},";
                if (Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.Steel)) str += $"{Judge.BuildStructureNames[JudgeParams.BuildStructures.Steel]},";
                if (Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.LightGaugeSteel)) str += $"{Judge.BuildStructureNames[JudgeParams.BuildStructures.LightGaugeSteel]},";
                if (Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.MasonryConstruction)) str += $"{Judge.BuildStructureNames[JudgeParams.BuildStructures.MasonryConstruction]},";
                if (Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.Unknown)) str += $"{Judge.BuildStructureNames[JudgeParams.BuildStructures.Unknown]},";
                if (Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.NonWood)) str += $"{Judge.BuildStructureNames[JudgeParams.BuildStructures.NonWood]},";
                str = str.Remove(str.Length - 1);

                customParams += $"　除外する建物構造：\n";
                customParams += $"　　{str}\n";
            }
            if (Judge.BuildFloors)
            {
                customParams += $"　( {Judge.FloorsBelowVal} )階以下( {Judge.UpperFloorsVal} )階以上の建物\n";
            }
            customParams += $"\n";
            customParams += $"【災害リスクによる除外】\n";
            customParams += $"　津波：{(Judge.BelowTsunamiHeight ? "○" : "×")}" +
                            $"　河川浸水：{(Judge.BelowFloodDepth ? "○" : "×")}" +
                            $"　土砂災害：{(Judge.LandslideWarningArea ? "○" : "×")}" +
                            $"　積雪：{(Judge.WeatherData ? "○" : "×")}\n";
            if (Judge.WeatherData)
            {
                customParams += $"　気象データ：{Judge.WeatherDataPath}\n";
                if (Judge.UseSnowDepth)
                {
                    customParams += $"　積雪が( {Judge.SnowDepthVal} ) cm以上\n";
                }
                else
                {
                    customParams += $"　積雪荷重が( {Judge.SnowLoadVal} ) kgf/m2以上, 単位荷重( {Judge.SnowLoadUnitVal} )N/m2\n";
                }
            }
            customParams += $"\n";
            customParams += $"【制限範囲による除外】\n";
            for (int n = 0; n < 3; n++)
            {
                var data = Judge.RestrictAreas[n];
                customParams += $"　データ{n + 1}：";
                if (!data.Enable)
                {
                    customParams += $"指定なし\n";
                }
                else
                {
                    customParams += $"\n";
                    customParams += $"　　シェープファイル：{data.ShapePath}";
                    customParams += $"　　制限する建物の高さ：{data.Height}";
                    customParams += $"　　制限する方位：{CommonManager.Directions16[data.Direction]}";
                    customParams += $"　　座標：{CommonManager.DatumTypeList[data.Datum]}";
                }
            }
        }

        /// <summary>
        /// 判定・集計パラメータ初期化
        /// </summary>
        /// <param name="file"></param>
        /// <returns></returns>
        public bool InitParams()
        {
            try
            {
                LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
                string file = Path.Combine(Directory.GetCurrentDirectory() + "/Assets/" + FILE_INIT_AGGREGATE);
                LogManager.Instance.OutputLogMessage($"InitParameter File : {file}", LogManager.LogType.INFO);

                if (!File.Exists(file))
                {
                    throw new Exception("パラメータ初期値格納ファイルが存在しません。");
                }

                ParamFileManager.Instance.ReadAggregateParams(file);

                return true;
            }
            catch (Exception ex)
            {
                CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
                LogManager.Instance.ExceptionLog(ex);
                return false;
            }
        }

        public bool InitJudgeParam()
        {

            try
            {
                LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
                string file = Path.Combine(Directory.GetCurrentDirectory() + "/Assets/" + FILE_INIT_AGGREGATE);
                LogManager.Instance.OutputLogMessage($"InitParameter File : {file}", LogManager.LogType.INFO);

                if (!File.Exists(file))
                {
                    throw new Exception("パラメータ初期値格納ファイルが存在しません。");
                }

                ParamFileManager.Instance.ReadJudgeParams(file);

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
        /// パラメータを保存
        /// </summary>
        /// <param name="outDir">出力フォルダ</param>
        /// <returns></returns>
        public bool SaveParams(string outDir, DateTime date)
        {
            try
            {
                LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

                // 出力フォルダ作成
                CommonManager.Instance.CreateDirectory(outDir);

                // 初期値格納ファイルをコピー
                string srcfile = Path.Combine(Directory.GetCurrentDirectory(), "Assets", FILE_INIT_AGGREGATE);
                string paramfile = Path.Combine(outDir, ResultData.ParamData.AGGREGATE_PARAM_FILENAME);
                LogManager.Instance.OutputLogMessage($"InitParameter File : {paramfile}", LogManager.LogType.INFO);
                File.Copy(srcfile, paramfile, true);

                ParamFileManager.Instance.SaveAggregateParams(paramfile);

                // 前回の履歴ファイルを上書き
                string prefile = Path.Combine(ParamFileManager.Instance.HistoryDirectory, FILE_PRE_AGGREGATE);
                File.Copy(paramfile, prefile, true);

                // 履歴に追加
                ParamFileManager.Instance.AddHistory(paramfile, date);

                // パラメータ確認用のテキストファイルを出力
                GetParamsText(out string inputparams, out string customparams);
                string paramtext = $"{inputparams}\n{customparams}";
                string textfile = Path.Combine(Directory.GetParent(outDir).FullName, ResultData.ParamData.PARAM_LOG_FILENAME);
                using (StreamWriter sw = new StreamWriter(textfile, true, System.Text.Encoding.GetEncoding("Shift_JIS")))
                {
                    sw.WriteLine(paramtext);
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

        private static AggregateParam _instance = null;

        public static AggregateParam Instance
        {
            get
            {
                if (_instance == null)
                {
                    _instance = new AggregateParam();
                }
                return _instance;
            }
        }

        #region プロパティ
        private string _AnalyzeResultPath;
        private string _MaxLat;
        private string _MaxLon;
        private string _MinLat;
        private string _MinLon;
        private string _OutputResultDirectory;
        #endregion
    }

    class JudgeParams : BindableBase
    {
        /// <summary>
        /// 日射量が少ない建物を除外
        /// </summary>
        public bool LowerPotential
        {
            get => this._LowerPotential;
            set => base.SetProperty(ref _LowerPotential, value);
        }

        /// <summary>
        /// 日射量(kWh/m2)を除外
        /// </summary>
        public bool Potential
        {
            get => this._Potential;
            set => base.SetProperty(ref _Potential, value);
        }

        /// <summary>
        /// 除外する日射量(kWh/m2)
        /// </summary>
        public string PotentialVal
        {
            get => this._PotentialVal;
            set => base.SetProperty(ref _PotentialVal, value);
        }

        /// <summary>
        /// 日射量(下位％)を除外
        /// </summary>
        public bool PotentialPercent
        {
            get => this._PotentialPercent;
            set => base.SetProperty(ref _PotentialPercent, value);
        }

        /// <summary>
        /// 除外する日射量(下位％)
        /// </summary>
        public string PotentialPercentVal
        {
            get => this._PotentialPercentVal;
            set => base.SetProperty(ref _PotentialPercentVal, value);
        }

        /// <summary>
        /// 建物構造による除外
        /// </summary>
        public bool BuildStructure
        {
            get => this._BuildStructure;
            set => base.SetProperty(ref _BuildStructure, value);
        }

        /// <summary>
        /// 建物構造
        /// </summary>
        [Flags]
        public enum BuildStructures
        {
            None                    = 0,
            Wood                    = 1 << 0,
            SteelReinforcedConcrete = 1 << 1,
            ReinforcedConcrete      = 1 << 2,
            Steel                   = 1 << 3,
            LightGaugeSteel         = 1 << 4,
            MasonryConstruction     = 1 << 5,
            Unknown                 = 1 << 6,
            NonWood                 = 1 << 7,
        }

        /// <summary>
        /// 建物構造名称
        /// </summary>
        public Dictionary<BuildStructures, string> BuildStructureNames { get; } = new Dictionary<BuildStructures, string>()
        {
            { BuildStructures.None, ""},
            { BuildStructures.Wood, "木造・土蔵造"},
            { BuildStructures.SteelReinforcedConcrete, "鉄骨鉄筋コンクリート造"},
            { BuildStructures.ReinforcedConcrete, "鉄筋コンクリート造"},
            { BuildStructures.Steel, "鉄骨造"},
            { BuildStructures.LightGaugeSteel, "軽量鉄骨造"},
            { BuildStructures.MasonryConstruction, "レンガ造・コンクリートブロック造・石造"},
            { BuildStructures.Unknown, "不明"},
            { BuildStructures.NonWood, "非木造"},
        };

        /// <summary>
        /// 除外する建物構造
        /// </summary>
        public BuildStructures BuildStructureFlag { get; set; } = BuildStructures.None;

        /// <summary>
        /// 特定の階数の建物を除外
        /// </summary>
        public bool BuildFloors
        {
            get => this._BuildFloors;
            set => base.SetProperty(ref _BuildFloors, value);
        }

        /// <summary>
        /// 除外する階層n階以下
        /// </summary>
        public string FloorsBelowVal
        {
            get => this._FloorsBelowVal;
            set => base.SetProperty(ref _FloorsBelowVal, value);
        }

        /// <summary>
        /// 除外する階層n階以上
        /// </summary>
        public string UpperFloorsVal
        {
            get => this._UpperFloorsVal;
            set => base.SetProperty(ref _UpperFloorsVal, value);
        }

        /// <summary>
        /// 高さが想定される最大津波高さを下回る建物を除外
        /// </summary>
        public bool BelowTsunamiHeight
        {
            get => this._BelowTsunamiHeight;
            set => base.SetProperty(ref _BelowTsunamiHeight, value);
        }

        /// <summary>
        /// 建物高さが想定される河川浸水想定浸水深を下回る建物を除外
        /// </summary>
        public bool BelowFloodDepth
        {
            get => this._BelowFloodDepth;
            set => base.SetProperty(ref _BelowFloodDepth, value);
        }

        /// <summary>
        /// 土砂災害警戒区域内に存在する建物を除外
        /// </summary>
        public bool LandslideWarningArea
        {
            get => this._LandslideWarningArea;
            set => base.SetProperty(ref _LandslideWarningArea, value);
        }

        /// <summary>
        /// 気象データ(積雪)で除外する
        /// </summary>
        public bool WeatherData
        {
            get => this._WeatherData;
            set => base.SetProperty(ref _WeatherData, value);
        }

        /// <summary>
        /// 気象データ(積雪)フォルダパス
        /// </summary>
        public string WeatherDataPath
        {
            get => this._WeatherDataPath;
            set => base.SetProperty(ref _WeatherDataPath, value);
        }

        /// <summary>
        /// 積雪が多い地域の建物を除外
        /// </summary>
        public bool UseSnowDepth
        {
            get => this._UseSnowDepth;
            set => base.SetProperty(ref _UseSnowDepth, value);
        }

        /// <summary>
        /// 気象データ(積雪)_cm以上
        /// </summary>
        public string SnowDepthVal
        {
            get => this._SnowDepthVal;
            set => base.SetProperty(ref _SnowDepthVal, value);
        }

        /// <summary>
        /// 積雪荷重(kgf/m3)
        /// </summary>
        public string SnowLoadVal
        {
            get => this._SnowLoadVal;
            set => base.SetProperty(ref _SnowLoadVal, value);
        }

        /// <summary>
        /// 単位荷重(N/m3)
        /// </summary>
        public string SnowLoadUnitVal
        {
            get => this._SnowLoadUnitVal;
            set => base.SetProperty(ref _SnowLoadUnitVal, value);
        }

        /// <summary>
        /// 制限を設ける範囲のシェープファイル
        /// </summary>
        public struct RestrictArea
        {
            /// <summary>
            /// 使用フラグ
            /// </summary>
            public bool Enable;    

            /// <summary>
            /// シェープファイル(フォルダ)パス
            /// </summary>
            public string ShapePath;

            /// <summary>
            /// 高さ
            /// </summary>
            public string Height;

            /// <summary>
            /// 方位
            /// </summary>
            public int Direction;

            /// <summary>
            /// 座標系
            /// </summary>
            public int Datum;

            public RestrictArea(bool b, string str1, string str2, int n1, int n2)
            {
                Enable = b; ShapePath = str1; Height = str2; Direction = n1; Datum = n2;

            }
        }

        /// <summary>
        /// 制限を設ける範囲データ
        /// </summary>
        public RestrictArea[] RestrictAreas { get; set; } = new RestrictArea[3];


        #region プロパティ
        private bool _LowerPotential;
        private bool _Potential;
        private string _PotentialVal;
        private bool _PotentialPercent;
        private string _PotentialPercentVal;
        private bool _BuildStructure;
        private bool _BuildFloors;
        private string _FloorsBelowVal;
        private string _UpperFloorsVal;
        private bool _BelowTsunamiHeight;
        private bool _BelowFloodDepth;
        private bool _LandslideWarningArea;
        private bool _WeatherData;
        private string _WeatherDataPath;
        private bool _UseSnowDepth;
        private string _SnowDepthVal;
        private string _SnowLoadVal;
        private string _SnowLoadUnitVal;
        #endregion
    }
}
