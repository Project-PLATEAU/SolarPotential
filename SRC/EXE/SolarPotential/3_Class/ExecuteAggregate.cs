using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Threading;
using System.Threading;

namespace SolarPotential._3_Class
{
    class ExecuteAggregate : IDisposable
    {
        #region DllImport
        [DllImport("AggregateData.dll")]
        static extern int AggregateBldgFiles(string str, string outstr);
        [DllImport("AggregateData.dll")]
        static extern int AggregateLandFiles(string str, string outstr);
        [DllImport("AggregateData.dll")]
        static extern int AggregateAllData(string str, string outstr);
        [DllImport("AggregateData.dll")]
        internal static extern void AddAnalyzeAreaData(IntPtr param);
        [DllImport("AggregateData.dll")]
        static extern int AnalyzeHazardRisk(string str, string outstr, bool fldrisk, bool tnmrisk, bool lsldrisk);
        // キャプチャ用
        [System.Runtime.InteropServices.DllImport("User32.dll")]
        private extern static bool PrintWindow(IntPtr hwnd, IntPtr hDC, uint nFlags);

        [DllImport("AggregateData.dll")]
        static extern void Initialize();
        [DllImport("AggregateData.dll")]
        static extern void DllDispose();
        [DllImport("JudgeSuitablePlace.dll")]
        internal static extern void InitializeUIParam();
        [DllImport("JudgeSuitablePlace.dll")]
        internal static extern void SetAggregateParam(IntPtr param);
        [DllImport("JudgeSuitablePlace.dll")]
        internal static extern void SetAggregateTarget(IntPtr param);
        [DllImport("JudgeSuitablePlace.dll")]
        internal static extern bool SetOutputPath(string aggregatePath);
        [DllImport("JudgeSuitablePlace.dll")]
        internal static extern bool SetAnalyzeResultPath(string analyzePath);
        [DllImport("JudgeSuitablePlace.dll")]
        internal static extern int JadgeStart();

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        struct AnalyzeAreaData_t
        {
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = AreaData.MAX_AREA_ID_LEN)]
            public string strAreaId;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = AreaData.MAX_AREA_NAME_LEN)]
            public string strAreaName;
            public int nPointCount;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = AreaData.MAX_POINT_LEN)]
            public double[] pPointArray;
        };
        #endregion

        /// <summary>
        /// 非同期処理をCancelするためのTokenを取得
        /// </summary>
        public CancellationTokenSource TaskCanceler { get; set; }

        /// <summary>
        /// プログレスバー(%)更新用イベント
        /// </summary>
        public event Action<double> NoticeUpdateProgressBar = delegate { };

        /// <summary>
        /// 進捗状況テキスト更新用イベント
        /// </summary>
        public event Action<string> NoticeUpdateProgressText = delegate { };

        /// <summary>
        /// キャンセル完了通知イベント
        /// </summary>
        public event Action NoticeCancelComplete = delegate { };

        /// <summary>
        /// 完了通知イベント
        /// </summary>
        public event Action<bool> NoticeAggregateComplete = delegate { };

        /// <summary>
        /// 実行中画面Dispatcher
        /// </summary>
        private Dispatcher parentDispatcher;

        /// <summary>
        /// 出力フォルダ(ex 適地判定_yyyyMMddHHmm)
        /// </summary>
        public string OutputDirectory { get; private set; } = "";

        /// <summary>
        /// 処理の開始日時
        /// </summary>
        DateTime StartDate { get; set; }

        /// <summary>
        /// エラー発生フラグ
        /// </summary>
        bool hasError { get; set; } = false;

        /// <summary>
        /// 進捗
        /// </summary>
        enum eProgress
        {
            Start,
            CheckParam,
            AnalyzeHazardRisk,
            AggregateBldgFiles,
            AggregateLandFiles,
            JadgeStart,
            AggregateAllData,
            Complete,
            Cancel,
        }

        struct ProcessInfo
        {
            /// <summary>
            /// 進捗%(対象処理完了時)
            /// </summary>
            public int Val;

            /// <summary>
            /// 進捗表示テキスト
            /// </summary>
            public string Text;

            public ProcessInfo(int n, string str)
            {
                Val = n; Text = str;
            }
        }

        /// <summary>
        /// 進捗表示用
        /// </summary>
        Dictionary<eProgress, ProcessInfo> ProgressDict = new Dictionary<eProgress, ProcessInfo>()
        {
            { eProgress.Start,              new ProcessInfo(0, "処理開始") },
            { eProgress.CheckParam,         new ProcessInfo(10, "入力値チェック中...") },
            { eProgress.AnalyzeHazardRisk,  new ProcessInfo(30, "3D都市モデル(災害リスク)読み込み中...") },
            { eProgress.AggregateBldgFiles, new ProcessInfo(40, "建物解析結果の読み込み中...") },
            { eProgress.AggregateLandFiles, new ProcessInfo(50, "土地解析結果の読み込み中...") },
            { eProgress.JadgeStart,         new ProcessInfo(60, "適地判定処理中...") },
            { eProgress.AggregateAllData,   new ProcessInfo(70, "集計処理中...") },
            { eProgress.Complete,           new ProcessInfo(100, "処理完了") },
            { eProgress.Cancel,             new ProcessInfo(90, "処理をキャンセルしています...") },
        };

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        public struct AggregateParam_t
        {
            // 選択範囲
            [MarshalAs(UnmanagedType.U1)]
            public bool bAggregateRange;// 集計範囲
            public double dMaxLat;      // 最大緯度
            public double dMinLon;      // 最小経度
            public double dMaxLon;      // 最大経度
            public double dMinLat;      // 最小緯度
            // 太陽光パネルの設置に関して優先度が低い施設の判定
            // 日射量が少ない施設を除外
            public double dParam_1_1_1;   // kWh/m2未満
            public int iParam_1_1_2;      // 下位％
            // 建物構造による除外
            [MarshalAs(UnmanagedType.U1)]
            public bool bParam_1_2_1;      // 木造・土蔵造
            [MarshalAs(UnmanagedType.U1)]
            public bool bParam_1_2_2;      // 鉄骨鉄筋コンクリート造
            [MarshalAs(UnmanagedType.U1)]
            public bool bParam_1_2_3;      // 鉄筋コンクリート造
            [MarshalAs(UnmanagedType.U1)]
            public bool bParam_1_2_4;      // 鉄骨造
            [MarshalAs(UnmanagedType.U1)]
            public bool bParam_1_2_5;      // 軽量鉄骨造
            [MarshalAs(UnmanagedType.U1)]
            public bool bParam_1_2_6;      // レンガ造・コンクリートブロック造・石造
            [MarshalAs(UnmanagedType.U1)]
            public bool bParam_1_2_7;      // 不明
            [MarshalAs(UnmanagedType.U1)]
            public bool bParam_1_2_8;      // 非木造
            // 特定の階層の施設を除外
            public int iParam_1_3_1;      // 〇階以下
            public int iParam_1_3_2;      // 〇階以上
                                          // 災害時に太陽光パネルが破損、消失する危険性の判定
            [MarshalAs(UnmanagedType.U1)]
            public bool bParam_2_1;            // 高さが想定される最大津波高さを下回る建物を除外
            [MarshalAs(UnmanagedType.U1)]
            public bool bParam_2_2;            // 建物高さが想定される河川浸水想定浸水深を下回る建物を除外
            [MarshalAs(UnmanagedType.U1)]
            public bool bParam_2_3;            // 土砂災害警戒区域内に存在する建物を除外
            [MarshalAs(UnmanagedType.LPStr)]
            public string strParam_2_4;        // 気象データ(積雪)フォルダパス
            public double dParam_2_4_1;        // 気象データ(積雪)_cm以上
            public double dParam_2_4_2;        // 積雪荷重(kgf/m2)
            public double dParam_2_4_3;        // 単位荷重(N/m2)

            // 太陽光パネルの設置に制限がある施設の判定
            [MarshalAs(UnmanagedType.LPStr)]
            public string strParam_3_1;       // 制限を設ける範囲のシェープファイル_フォルダパス１
            public double dParam_3_1_1;       // 制限を設ける範囲のシェープファイル_高さ１
            public int iParam_3_1_2;          // 制限を設ける範囲のシェープファイル_方位１
            public int iParam_3_1_3;          // 制限を設ける範囲のシェープファイル_座標系１
            [MarshalAs(UnmanagedType.LPStr)]
            public string strParam_3_2;       // 制限を設ける範囲のシェープファイル_フォルダパス２
            public double dParam_3_2_1;       // 制限を設ける範囲のシェープファイル_高さ２
            public int iParam_3_2_2;          // 制限を設ける範囲のシェープファイル_方位２
            public int iParam_3_2_3;          // 制限を設ける範囲のシェープファイル_座標系２
            [MarshalAs(UnmanagedType.LPStr)]
            public string strParam_3_3;       // 制限を設ける範囲のシェープファイル_フォルダパス３
            public double dParam_3_3_1;       // 制限を設ける範囲のシェープファイル_高さ３
            public int iParam_3_3_2;          // 制限を設ける範囲のシェープファイル_方位３
            public int iParam_3_3_3;          // 制限を設ける範囲のシェープファイル_座標系３
        }

        public struct AggregateTarget_t
        {
            // 集計対象
            [MarshalAs(UnmanagedType.U1)]
            public bool bExecBuild;     // 建物実行フラグ
            [MarshalAs(UnmanagedType.U1)]
            public bool bExecLand;      // 土地実行フラグ

        };

        public void Dispose()
        {
            // キャンセルクラスを解放
            if (TaskCanceler != null)
            {
                TaskCanceler.Dispose();
                TaskCanceler = null;
            }
        }

        /// <summary>
        /// 適地判定・集計処理実行
        /// </summary>
        public async void ProcessStart(Dispatcher dispatcher)
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            parentDispatcher = dispatcher;
            
            hasError = false;

            // 入力データチェック
            parentDispatcher.Invoke(() => NoticeUpdateProgressText.Invoke(ProgressDict[eProgress.CheckParam].Text));
            if (!CheckParams())
            {
                parentDispatcher.Invoke(() => NoticeAggregateComplete.Invoke(false));
                return;
            }
            parentDispatcher.Invoke(() => NoticeUpdateProgressBar.Invoke(ProgressDict[eProgress.CheckParam].Val));

            // 出力フォルダ名 
            StartDate = DateTime.Now;
            OutputDirectory = Path.Combine(AggregateParam.Instance.OutputResultDirectory, $"{CommonManager.OutputDirectoryName}_{StartDate:yyyyMMddHHmm}");
            // 出力フォルダ作成
            CommonManager.Instance.CreateDirectory(OutputDirectory);

            if (TaskCanceler == null) TaskCanceler = new CancellationTokenSource();

            var tmpTask = Task.Run(() => Exec(OutputDirectory));

            while (true)
            {
                if (TaskCanceler.Token.IsCancellationRequested)
                {
                    // キャンセルファイルパス
                    string pathCancel = OutputDirectory + "\\" + CommonManager.FILE_CANCEL;
                    // ファイル作成
                    using (FileStream fs = File.Create(pathCancel)) { }

                    parentDispatcher.Invoke(() => NoticeUpdateProgressText.Invoke(ProgressDict[eProgress.Cancel].Text));

                    while (tmpTask.Status == TaskStatus.Running)
                    {
                        await Task.Delay(1000);
                    }

                    MessageBox.Show("処理をキャンセルしました", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Information);
                    LogManager.Instance.OutputLogMessage("解析処理がキャンセルされました。", LogManager.LogType.LOG);
                    parentDispatcher.Invoke(() => NoticeCancelComplete.Invoke());

                    break;
                }
                else if (tmpTask.Status == TaskStatus.RanToCompletion)
                {
                    parentDispatcher.Invoke(() => NoticeAggregateComplete.Invoke(!hasError));
                    break;
                }
            }
        }

        private bool CheckParams()
        {
            // パラメータチェック

            return true;
        }

        /// <summary>
        /// 実行
        /// </summary>
        private void Exec(string outDir)
        {
            try
            {
                int ret = 0;

                // 開始ログ（集計処理）
                LogManager.Instance.OutputLogMessage("集計処理", LogManager.LogType.START);

                // パラメータ出力
                AggregateParam.Instance.SaveParams(Path.Combine(outDir, ResultData.ParamData.PARAM_DIRNAME), StartDate);

                // 集計範囲を出力(出力済の一時ファイルをコピー)
                string areaRangeDir = Path.Combine(outDir, ResultData.AggregateResult.RANGEDATA_NAME);
                if (File.Exists(CommonManager.AreaImageTempPath))
                {   // jpg
                    string fileName = Path.GetFileName(CommonManager.AreaImageTempPath);
                    string path = Path.Combine(areaRangeDir, $"{fileName}");
                    CommonManager.Instance.CreateDirectory(areaRangeDir);
                    File.Move(CommonManager.AreaImageTempPath, path);
                    LogManager.Instance.OutputLogMessage($"範囲画像：{path}", LogManager.LogType.LOG);
                }
                if (File.Exists(CommonManager.AreaKmlTempPath))
                {   // kml
                    string fileName = Path.GetFileName(CommonManager.AreaKmlTempPath);
                    string path = Path.Combine(areaRangeDir, $"{fileName}");
                    CommonManager.Instance.CreateDirectory(areaRangeDir);
                    File.Move(CommonManager.AreaKmlTempPath, path);
                    LogManager.Instance.OutputLogMessage($"範囲KML：{path}", LogManager.LogType.LOG);
                }

                // DLLを初期化
                Initialize();

                // 解析エリア
                int areaCount = AggregateParam.Instance.AnalyzeAreaList.Count;
                for (int i = 0; i < areaCount; i++)
                {
                    var area = AggregateParam.Instance.AnalyzeAreaList[i];
                    if (area.Exclusion_flg)
                    {
                        continue;
                    }
                    int ptCount = area.Points.Count;
                    double[] points = new double[AreaData.MAX_POINT_LEN];
                    for (int j = 0; j < ptCount; j++)
                    {
                        int index = j * 2;
                        points[index] = area.Points[j].Lat;
                        points[index + 1] = area.Points[j].Lon;
                    }
                    var areaData_T = new AnalyzeAreaData_t
                    {
                        strAreaId = area.Id,
                        strAreaName = area.Name,
                        nPointCount = ptCount,
                        pPointArray = points,
                    };

                    var ptr = Marshal.AllocCoTaskMem(Marshal.SizeOf(areaData_T));
                    Marshal.StructureToPtr(areaData_T, ptr, false);
                    AddAnalyzeAreaData(ptr);
                    Marshal.FreeCoTaskMem(ptr);
                }

                // C++受け渡し構造体格納
                var parameter = new AggregateParam_t
                {
                    // 初期化
                    bAggregateRange = false,
                    dMaxLat = 0.0,
                    dMinLon = 0.0,
                    dMaxLon = 0.0,
                    dMinLat = 0.0,
                    dParam_1_1_1 = 0.0,
                    iParam_1_1_2 = 0,
                    bParam_1_2_1 = false,
                    bParam_1_2_2 = false,
                    bParam_1_2_3 = false,
                    bParam_1_2_4 = false,
                    bParam_1_2_5 = false,
                    bParam_1_2_6 = false,
                    bParam_1_2_7 = false,
                    bParam_1_2_8 = false,
                    iParam_1_3_1 = -1,
                    iParam_1_3_2 = 9999,
                    bParam_2_1 = false,
                    bParam_2_2 = false,
                    bParam_2_3 = false,
                    strParam_2_4 = "",
                    dParam_2_4_1 = 0.0,
                    dParam_2_4_2 = 0.0,
                    dParam_2_4_3 = 0.0,
                    strParam_3_1 = "",
                    dParam_3_1_1 = -1.0,
                    iParam_3_1_2 = 0,
                    iParam_3_1_3 = 0,
                    strParam_3_2 = "",
                    dParam_3_2_1 = -1.0,
                    iParam_3_2_2 = 0,
                    iParam_3_2_3 = 0,
                    strParam_3_3 = "",
                    dParam_3_3_1 = -1.0,
                    iParam_3_3_2 = 0,
                    iParam_3_3_3 = 0
                };
                if (AggregateParam.Instance.SelectArea)
                {
                    parameter.bAggregateRange = true;
                    parameter.dMaxLat = double.Parse(AggregateParam.Instance.MaxLat);
                    parameter.dMinLon = double.Parse(AggregateParam.Instance.MinLon);
                    parameter.dMaxLon = double.Parse(AggregateParam.Instance.MaxLon);
                    parameter.dMinLat = double.Parse(AggregateParam.Instance.MinLat);
                }
                if (AggregateParam.Instance.Judge.LowerPotential)
                {
                    if (AggregateParam.Instance.Judge.Potential)
                        parameter.dParam_1_1_1 = double.Parse(AggregateParam.Instance.Judge.PotentialVal);
                    if (AggregateParam.Instance.Judge.PotentialPercent)
                        parameter.iParam_1_1_2 = int.Parse(AggregateParam.Instance.Judge.PotentialPercentVal);
                }
                if (AggregateParam.Instance.Judge.BuildStructure)
                {
                    JudgeParams.BuildStructures flag = AggregateParam.Instance.Judge.BuildStructureFlag;
                    parameter.bParam_1_2_1 = flag.HasFlag(JudgeParams.BuildStructures.Wood);
                    parameter.bParam_1_2_2 = flag.HasFlag(JudgeParams.BuildStructures.SteelReinforcedConcrete);
                    parameter.bParam_1_2_3 = flag.HasFlag(JudgeParams.BuildStructures.ReinforcedConcrete);
                    parameter.bParam_1_2_4 = flag.HasFlag(JudgeParams.BuildStructures.Steel);
                    parameter.bParam_1_2_5 = flag.HasFlag(JudgeParams.BuildStructures.LightGaugeSteel);
                    parameter.bParam_1_2_6 = flag.HasFlag(JudgeParams.BuildStructures.MasonryConstruction);
                    parameter.bParam_1_2_7 = flag.HasFlag(JudgeParams.BuildStructures.Unknown);
                    parameter.bParam_1_2_8 = flag.HasFlag(JudgeParams.BuildStructures.NonWood);
                }
                if (AggregateParam.Instance.Judge.BuildFloors)
                {
                    int val = 0;
                    parameter.iParam_1_3_1 = int.TryParse(AggregateParam.Instance.Judge.FloorsBelowVal, out val) ? val : -1;
                    parameter.iParam_1_3_2 = int.TryParse(AggregateParam.Instance.Judge.UpperFloorsVal, out val) ? val : 9999;
                }
                parameter.bParam_2_1 = AggregateParam.Instance.Judge.BelowTsunamiHeight;
                parameter.bParam_2_2 = AggregateParam.Instance.Judge.BelowFloodDepth;
                parameter.bParam_2_3 = AggregateParam.Instance.Judge.LandslideWarningArea;
                if (AggregateParam.Instance.Judge.WeatherData)
                {
                    parameter.strParam_2_4 = AggregateParam.Instance.Judge.WeatherDataPath;
                    if (AggregateParam.Instance.Judge.UseSnowDepth)
                    {
                        parameter.dParam_2_4_1 = double.Parse(AggregateParam.Instance.Judge.SnowDepthVal);
                    }
                    else
                    {
                        parameter.dParam_2_4_2 = double.Parse(AggregateParam.Instance.Judge.SnowLoadVal);
                        parameter.dParam_2_4_3 = double.Parse(AggregateParam.Instance.Judge.SnowLoadUnitVal);
                    }
                }
                var areas = AggregateParam.Instance.Judge.RestrictAreas;
                if (areas[0].Enable)
                {
                    parameter.strParam_3_1 = areas[0].ShapePath;
                    parameter.dParam_3_1_1 = double.Parse(areas[0].Height);
                    parameter.iParam_3_1_2 = areas[0].Direction + 1;
                    parameter.iParam_3_1_3 = areas[0].Datum + 1;
                }
                if (areas[1].Enable)
                {
                    parameter.strParam_3_2 = areas[1].ShapePath;
                    parameter.dParam_3_2_1 = double.Parse(areas[1].Height);
                    parameter.iParam_3_2_2 = areas[1].Direction + 1;
                    parameter.iParam_3_2_3 = areas[1].Datum + 1;
                }
                if (areas[2].Enable)
                {
                    parameter.strParam_3_3 = areas[2].ShapePath;
                    parameter.dParam_3_3_1 = double.Parse(areas[2].Height);
                    parameter.iParam_3_3_2 = areas[2].Direction + 1;
                    parameter.iParam_3_3_3 = areas[2].Datum + 1;
                }

                // 災害リスクの読み込みフラグ
                bool bReadHazard = false;
                bReadHazard |= AggregateParam.Instance.Judge.BelowFloodDepth;
                bReadHazard |= AggregateParam.Instance.Judge.BelowTsunamiHeight;
                bReadHazard |= AggregateParam.Instance.Judge.LandslideWarningArea;

                // 解析結果の期間ごとに集計
                string[] directories = Directory.GetDirectories(AggregateParam.Instance.AnalyzeResultPath, $"{ResultData.ResultDataManager.RESULT_DIRNAME}*");
                foreach (var dir in directories)
                {
                    // 建物gmlファイル格納フォルダパス
                    string bldgDir = Path.Combine(dir, ResultData.AnalyzeResult.CITYGML_DIRNAME);
                    // 土地結果フォルダパス
                    string landDir = Path.Combine(dir, ResultData.AnalyzeResult.LAND_DIRNAME);

                    // パラメータ引き渡し
                    InitializeUIParam();

                    IntPtr p = Marshal.AllocCoTaskMem(Marshal.SizeOf(parameter));
                    Marshal.StructureToPtr(parameter, p, false);
                    SetAggregateParam(p);
                    Marshal.FreeCoTaskMem(p);

                    // 集計対象を設定
                    var target = new AggregateTarget_t
                    {
                        bExecBuild = Directory.Exists(bldgDir),
                        bExecLand = Directory.Exists(landDir)
                    };
                    p = Marshal.AllocCoTaskMem(Marshal.SizeOf(target));
                    Marshal.StructureToPtr(target, p, false);
                    SetAggregateTarget(p);
                    Marshal.FreeCoTaskMem(p);

                    if (bReadHazard)
                    {
                        // 災害リスクの読み込み
                        string udxDir = Path.Combine(AggregateParam.Instance.CityModelPath, "udx");
                        if (Directory.Exists(udxDir))
                        {
                            parentDispatcher.Invoke(() => NoticeUpdateProgressText.Invoke(ProgressDict[eProgress.AnalyzeHazardRisk].Text));
                            LogManager.Instance.OutputLogMessage("AnalyzeHazardRisk", LogManager.LogType.START);
                            ret = AnalyzeHazardRisk(udxDir, outDir, AggregateParam.Instance.Judge.BelowFloodDepth, AggregateParam.Instance.Judge.BelowTsunamiHeight, AggregateParam.Instance.Judge.LandslideWarningArea);
                            LogManager.Instance.OutputLogMessage("AnalyzeHazardRisk", LogManager.LogType.END);
                            if (ret == 1)
                            {
                                // ファイルなし
                                throw new Exception("3D都市モデル(災害リスク)の読み込み中にエラーが発生しました。");
                            }
                            if (ret == 2)
                            {
                                // キャンセル
                                DllDispose();
                                return;
                            }
                            parentDispatcher.Invoke(() => NoticeUpdateProgressBar.Invoke(ProgressDict[eProgress.AnalyzeHazardRisk].Val));

                            bReadHazard = false;    // 初回だけ読み込む
                        }
                    }

                    if (target.bExecBuild)
                    {
                        parentDispatcher.Invoke(() => NoticeUpdateProgressText.Invoke(ProgressDict[eProgress.AggregateBldgFiles].Text));
                        LogManager.Instance.OutputLogMessage("AggregateBldgFiles", LogManager.LogType.START);
                        ret = AggregateBldgFiles(bldgDir, outDir);
                        LogManager.Instance.OutputLogMessage("AggregateBldgFiles", LogManager.LogType.END);
                        if (ret == 1)
                        {
                            // ファイルなし
                            throw new Exception("集計に失敗しました。集計範囲や入力データを確認してください。");
                        }
                        if (ret == 2)
                        {
                            // キャンセル
                            DllDispose();
                            return;
                        }
                        parentDispatcher.Invoke(() => NoticeUpdateProgressBar.Invoke(ProgressDict[eProgress.AggregateBldgFiles].Val));
                    }

                    if (target.bExecLand)
                    {
                        parentDispatcher.Invoke(() => NoticeUpdateProgressText.Invoke(ProgressDict[eProgress.AggregateLandFiles].Text));
                        LogManager.Instance.OutputLogMessage("AggregateLandFiles", LogManager.LogType.START);
                        ret = AggregateLandFiles(landDir, outDir);
                        LogManager.Instance.OutputLogMessage("AggregateLandFiles", LogManager.LogType.END);
                        if (ret == 1)
                        {
                            // ファイルなし
                            throw new Exception("集計に失敗しました。集計範囲や入力データを確認してください。");
                        }
                        if (ret == 2)
                        {
                            // キャンセル
                            DllDispose();
                            return;
                        }
                        parentDispatcher.Invoke(() => NoticeUpdateProgressBar.Invoke(ProgressDict[eProgress.AggregateLandFiles].Val));
                    }

                    // 期間ごとのフォルダを作成
                    string dirName = Path.GetFileName(dir);
                    string date = dirName.Split('_')[1];
                    var resultDir = Path.Combine(outDir, $"{ResultData.ResultDataManager.RESULT_DIRNAME}_{date}");

                    // 適地判定結果出力フォルダ作成
                    string dataDir = Path.Combine(resultDir, ResultData.AggregateResult.JUDGE_DIRNAME);
                    CommonManager.Instance.CreateDirectory(dataDir);

                    // 出力フォルダ設定
                    SetOutputPath(dataDir);

                    // 解析結果のシステムデータフォルダパス設定
                    string sysDir = Path.Combine(AggregateParam.Instance.AnalyzeResultPath, ResultData.SystemData.SYSTEM_DIRNAME);
                    SetAnalyzeResultPath(sysDir);

                    // C++適地判定処理呼出し
                    parentDispatcher.Invoke(() => NoticeUpdateProgressText.Invoke(ProgressDict[eProgress.JadgeStart].Text));
                    LogManager.Instance.OutputLogMessage("JadgeStart", LogManager.LogType.START);
                    ret = JadgeStart();
                    LogManager.Instance.OutputLogMessage("JadgeStart", LogManager.LogType.END);
                    if (ret == 1)
                    {
                        // 失敗
                        throw new Exception("適地判定に失敗しました。");
                    }
                    if (ret == 2)
                    {
                        // キャンセル
                        DllDispose();
                        return;
                    }
                    parentDispatcher.Invoke(() => NoticeUpdateProgressBar.Invoke(ProgressDict[eProgress.JadgeStart].Val));

                    // 集計処理
                    string aggregateDir = Path.Combine(resultDir, ResultData.AggregateResult.AGGREGATE_DIRNAME);
                    CommonManager.Instance.CreateDirectory(aggregateDir);
                    parentDispatcher.Invoke(() => NoticeUpdateProgressText.Invoke(ProgressDict[eProgress.AggregateAllData].Text));
                    LogManager.Instance.OutputLogMessage("AggregateAllData", LogManager.LogType.START);
                    ret = AggregateAllData(dir, resultDir);
                    LogManager.Instance.OutputLogMessage("AggregateAllData", LogManager.LogType.END);
                    if (ret == 1)
                    {
                        throw new Exception("集計に失敗しました。集計範囲や入力データを確認してください。");
                    }
                    if (ret == 2)
                    {
                        // キャンセル
                        DllDispose();
                        return;
                    }
                    parentDispatcher.Invoke(() => NoticeUpdateProgressBar.Invoke(ProgressDict[eProgress.AggregateAllData].Val));

                }

                // C++側の解放処理
                DllDispose();

                // 終了ログ（集計処理）
                LogManager.Instance.OutputLogMessage("集計処理", LogManager.LogType.END);
                parentDispatcher.Invoke(() => NoticeUpdateProgressText.Invoke(ProgressDict[eProgress.Complete].Text));
                parentDispatcher.Invoke(() => NoticeUpdateProgressBar.Invoke(ProgressDict[eProgress.Complete].Val));

            }
            catch (Exception ex)
            {
                CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
                LogManager.Instance.ExceptionLog(ex);
                hasError = true;

                DllDispose();
            }

        }
    }
}
