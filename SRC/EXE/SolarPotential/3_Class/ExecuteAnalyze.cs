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
    class ExecuteAnalyze : IDisposable
    {
        #region C++受け渡し
        [DllImport("AnalyzeData.dll")]
        internal static extern int AnalizeBldgFiles(string str, string strOut, bool analyzeInterior);
        [DllImport("AnalyzeData.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int AnalizeDemFiles(string str, string strOut, bool useDem);
        [DllImport("AnalyzeData.dll")]
        internal static extern int AnalizeTranFiles(string str, string strOut);
        [DllImport("AnalyzeData.dll")]
        internal static extern int ExtractLandMesh(bool exclusionRoad, string strOut);
        [DllImport("AnalyzeData.dll")]
        static extern int LOD2DataOut(string str, string strOut);
        [DllImport("AnalyzeData.dll")]
        static extern void Initialize();
        [DllImport("AnalyzeData.dll")]
        internal static extern void AddAnalyzeAreaData(IntPtr param);
        [DllImport("AnalyzeData.dll")]
        static extern void DllDispose();
        [DllImport("AnalyzeData.dll")]
        internal static extern void OutputCoordinatesFile(string strOut);

        [DllImport("Analyzer.dll")]
        internal static extern int AnalyzeStart(string str);
        [DllImport("Analyzer.dll")]
        private static extern void InitializeUIParam();
        [DllImport("Analyzer.dll")]
        internal static extern void SetAnalyzeParam(IntPtr param);
        [DllImport("Analyzer.dll")]
        internal static extern void SetAnalyzeInputData(IntPtr param);
        [DllImport("Analyzer.dll")]
        internal static extern void SetSolarPotentialParam(IntPtr param);
        [DllImport("Analyzer.dll")]
        internal static extern void SetReflectionParam(IntPtr param);
        [DllImport("Analyzer.dll")]
        internal static extern void SetAnalyzeDateParam(IntPtr param);

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        struct AnalyzeInputData_t
        {
            [MarshalAs(UnmanagedType.LPStr)]
            public string strKashoData;
            [MarshalAs(UnmanagedType.LPStr)]
            public string strNisshoData;
            [MarshalAs(UnmanagedType.LPStr)]
            public string strSnowDepthData;
            [MarshalAs(UnmanagedType.LPStr)]
            public string strLandData;
            [MarshalAs(UnmanagedType.U1)]
            public bool bUseDemData;
        };

        [StructLayout(LayoutKind.Sequential)]
        struct LatLon_t
        {
            public double lat;
            public double lon;
        };

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
            public int eDirection;
            public double dDegree;
            [MarshalAs(UnmanagedType.LPStr)]
			public string strExplanation;
			[MarshalAs(UnmanagedType.U1)]
			public bool isWater;
			[MarshalAs(UnmanagedType.U1)]
            public bool analyzeBuild;
            [MarshalAs(UnmanagedType.U1)]
            public bool analyzeLand;
        };

        struct SolarPotentialParam_t
        {
            public double dArea2D_Roof;
            public int eDirection_Roof;
            public double dDirectionDegree_Roof;
            public double dSlopeDegree_Roof;
            public double dCorrectionCaseDeg_Roof;
            public int eCorrectionDirection_Roof;
            public double dCorrectionDirectionDegree_Roof;

            [MarshalAs(UnmanagedType.U1)]
            public bool bExclusionInterior_Roof;

            public double dArea2D_Land;
            public double dSlopeDegree_Land;
            public int eCorrectionDirection_Land;
            public double dCorrectionDirectionDegree_Land;

            public double dPanelMakerSolarPower;
            public double dPanelRatio;
        };

        // 反射シミュレーション条件
        struct ReflectionParam_t
        {
            [MarshalAs(UnmanagedType.U1)]
            public bool bCustom_Roof_Lower;
            public int eAzimuth_Roof_Lower;
            public double dSlopeAngle_Roof_Lower;

            [MarshalAs(UnmanagedType.U1)]
            public bool bCustom_Roof_Upper;
            public int eAzimuth_Roof_Upper;
            public double dSlopeAngle_Roof_Upper;

            [MarshalAs(UnmanagedType.U1)]
            public bool bCustom_Land_Lower;
            public int eAzimuth_Land_Lower;
            public double dSlopeAngle_Land_Lower;

            [MarshalAs(UnmanagedType.U1)]
            public bool bCustom_Land_Upper;
            public int eAzimuth_Land_Upper;
            public double dSlopeAngle_Land_Upper;

            public double dReflectionRange;
        };

        struct AnalyzeParam_t
        {
            [MarshalAs(UnmanagedType.U1)]
            public bool bExecSolarPotantial;
            [MarshalAs(UnmanagedType.U1)]
            public bool bExecReflection;
            [MarshalAs(UnmanagedType.U1)]
            public bool bExecBuild;
            [MarshalAs(UnmanagedType.U1)]
            public bool bExecLand;

        };

        struct AnalyzeDateParam_t
        {
			public int iTargetDate;
			public int iMonth;
			public int iDay;

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
        public event Action<bool> NoticeAnalyzeComplete = delegate { };

        /// <summary>
        /// 実行中画面Dispatcher
        /// </summary>
        private Dispatcher parentDispatcher;

        /// <summary>
        /// 出力フォルダ(ex 解析_yyyyMMddHHmm)
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
            AnalizeBldgFiles,
            AnalizeDemFiles,
            AnalizeTranFiles,
            ExtractLandMesh,
            AnalyzeStart,
            LOD2DataOut,
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
            { eProgress.AnalizeBldgFiles,   new ProcessInfo(20, "3D都市モデル(建物)読み込み中...") },
            { eProgress.AnalizeDemFiles,    new ProcessInfo(25, "3D都市モデル(地形)読み込み中...") },
            { eProgress.AnalizeTranFiles,   new ProcessInfo(30, "3D都市モデル(道路)読み込み中...") },
            { eProgress.ExtractLandMesh,    new ProcessInfo(40, "土地面の抽出中...") },
            { eProgress.AnalyzeStart,       new ProcessInfo(80, "解析処理中...") },
            { eProgress.LOD2DataOut,        new ProcessInfo(90, "3D都市モデルデータに書き込み中...") },
            { eProgress.Complete,           new ProcessInfo(100, "処理完了") },
            { eProgress.Cancel,             new ProcessInfo(90, "処理をキャンセルしています...") },
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
        /// 解析・シミュレーション処理実行
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
                parentDispatcher.Invoke(() => NoticeAnalyzeComplete.Invoke(false));
                return;
            }
            parentDispatcher.Invoke(() => NoticeUpdateProgressBar.Invoke(ProgressDict[eProgress.CheckParam].Val));

            // 出力フォルダ名 
            StartDate = DateTime.Now;
            OutputDirectory = Path.Combine(AnalyzeParam.Instance.OutputResultDirectory, $"{CommonManager.OutputDirectoryName}_{StartDate:yyyyMMddHHmm}");
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
                    parentDispatcher.Invoke(() => NoticeAnalyzeComplete.Invoke(!hasError));
                    break;
                }
            }
        }

        private bool CheckParams()
        {
            if (AnalyzeParam.Instance.Target.Land)
            {
                var areaList = AnalyzeParam.Instance.AreaList.Where(x => x.Id != AnalyzeParam.ALLAREA_ID).ToList();
                if (areaList == null || areaList.Count() == 0)
                {
                    if (string.IsNullOrEmpty(AnalyzeParam.Instance.InputData.LandData))
                    {
                        // 土地の解析が有効＋土地データ選択なし
                        MessageBox.Show("解析する土地の範囲が選択されていません。", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Warning);
                        return false;
                    }
                }
            }

            if (AnalyzeParam.Instance.Target.Build)
            {
                var areaList = AnalyzeParam.Instance.AreaList.Where(x => !x.IsShpData).ToList();
                if (areaList == null || areaList.Count() == 0)
                {
                    var result = MessageBox.Show("入力した3D都市モデルのLOD2整備範囲全域を対象に解析します。\n" +
                        "処理に数日かかる可能性がありますが、続けてよろしいですか？", "確認", MessageBoxButton.YesNo, MessageBoxImage.Question);
                    if (result == MessageBoxResult.No)
                    {
                        return false;
                    }
                    else
                    {
						// 全域を解析エリアとして追加
						AreaData area = new AreaData();
                        area.Id = AnalyzeParam.ALLAREA_ID;
                        area.Name = AnalyzeParam.ALLAREA_NAME;
                        area.Directions = CommonManager.Directions4;
                        area.Direction = 2; // 南向き
                        area.Water = false;
                        area.AnalyzeBuild = true;
                        area.AnalyzeLand = false;   // 全域は土地解析しない
                        area.Degree = 15;
                        var MinPos = AnalyzeParam.Instance.InputData.MinPos;
                        var MaxPos = AnalyzeParam.Instance.InputData.MaxPos;
                        area.Points.Add(new Point2D { Lat = MinPos.Lat, Lon = MinPos.Lon });  // 左下
                        area.Points.Add(new Point2D { Lat = MinPos.Lat, Lon = MaxPos.Lon });  // 右下
                        area.Points.Add(new Point2D { Lat = MaxPos.Lat, Lon = MaxPos.Lon });  // 右上
                        area.Points.Add(new Point2D { Lat = MaxPos.Lat, Lon = MinPos.Lon });  // 左上
                        AnalyzeParam.Instance.AreaList.Add(area);
                    }
                }
            }

            // 範囲が重なっていないかチェック
            var targetList = AnalyzeParam.Instance.AreaList.Where(x => !x.Exclusion_flg && !x.Id.Equals(AnalyzeParam.ALLAREA_ID)).ToList();
            var tmpList = targetList.ToList();
            foreach (var targetArea in targetList)
            {
                tmpList.Remove(targetArea);

                foreach (var area in tmpList)
                {
                    foreach (var pt in area.Points)
                    {
                        if (GeoUtil.Instance.IsPointInPolygon(pt, targetArea.Points))
                        {
                            // 範囲が重なっている
                            MessageBox.Show("指定したエリアが重なっている箇所があります。", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Warning);
                            return false;
                        }
                    }
                }
            }

            return true;
        }

        /// <summary>
        /// C++DLLにパラメータを引き渡す
        /// </summary>
        private void SetDllParams(AnalyzeParam.DateType dateType)
        {
            // パラメータ引き渡し
            InitializeUIParam();

            // 入力データ
            var inputData_T = new AnalyzeInputData_t
            {
                strKashoData = AnalyzeParam.Instance.InputData.KashoData,
                strNisshoData = AnalyzeParam.Instance.InputData.NisshoData,
                strSnowDepthData = AnalyzeParam.Instance.InputData.SnowDepthData,
                strLandData = AnalyzeParam.Instance.InputData.LandData,
                bUseDemData = AnalyzeParam.Instance.InputData.UseDemData
            };
            IntPtr p = Marshal.AllocCoTaskMem(Marshal.SizeOf(inputData_T));
            Marshal.StructureToPtr(inputData_T, p, false);
            SetAnalyzeInputData(p);
            Marshal.FreeCoTaskMem(p);

            // 発電ポテンシャル推移
            SolarPotentialParams solarPotentialParams = AnalyzeParam.Instance.SolarPotential;
            var solarPotentialParam_T = new SolarPotentialParam_t
            {
                dArea2D_Roof = double.Parse(solarPotentialParams.Roof.Area),
                eDirection_Roof = solarPotentialParams.Roof.DirDeg.Direction,
                dDirectionDegree_Roof = double.Parse(solarPotentialParams.Roof.DirDeg.Degree),
                dSlopeDegree_Roof = double.Parse(solarPotentialParams.Roof.SlopeDegree),
                dCorrectionCaseDeg_Roof = double.Parse(solarPotentialParams.Roof.CorrectionCaseDeg),
                eCorrectionDirection_Roof = solarPotentialParams.Roof.CorrectionDirDeg.Direction,
                dCorrectionDirectionDegree_Roof = double.Parse(solarPotentialParams.Roof.CorrectionDirDeg.Degree),
                bExclusionInterior_Roof = solarPotentialParams.Roof.Interior,
                dArea2D_Land = double.Parse(solarPotentialParams.Land.Area),
                dSlopeDegree_Land = double.Parse(solarPotentialParams.Land.SlopeDegree),
                eCorrectionDirection_Land = solarPotentialParams.Land.CorrectionDirDeg.Direction,
                dCorrectionDirectionDegree_Land = double.Parse(solarPotentialParams.Land.CorrectionDirDeg.Degree),
                dPanelMakerSolarPower = double.Parse(solarPotentialParams.PanelMakerSolarPower),
                dPanelRatio = double.Parse(solarPotentialParams.PanelRatio) * 0.01
            };
            p = Marshal.AllocCoTaskMem(Marshal.SizeOf(solarPotentialParam_T));
            Marshal.StructureToPtr(solarPotentialParam_T, p, false);
            SetSolarPotentialParam(p);
            Marshal.FreeCoTaskMem(p);

            // 反射シミュレーション
            CorrectionParams Roof_Lower = AnalyzeParam.Instance.Reflection.Roof[ReflectionParams.SlopeConditions.Lower];
            CorrectionParams Roof_Upper = AnalyzeParam.Instance.Reflection.Roof[ReflectionParams.SlopeConditions.Upper];
            CorrectionParams Land_Lower = AnalyzeParam.Instance.Reflection.Land[ReflectionParams.SlopeConditions.Lower];
            CorrectionParams Land_Upper = AnalyzeParam.Instance.Reflection.Land[ReflectionParams.SlopeConditions.Upper];
            var reflectionParam_T = new ReflectionParam_t
            {
                bCustom_Roof_Lower = Roof_Lower.UseCustomVal,
                eAzimuth_Roof_Lower = Roof_Lower.DirDeg.Direction,
                dSlopeAngle_Roof_Lower = double.Parse(Roof_Lower.DirDeg.Degree),
                bCustom_Roof_Upper = Roof_Upper.UseCustomVal,
                eAzimuth_Roof_Upper = Roof_Upper.DirDeg.Direction,
                dSlopeAngle_Roof_Upper = double.Parse(Roof_Upper.DirDeg.Degree),
                bCustom_Land_Lower = Land_Lower.UseCustomVal,
                eAzimuth_Land_Lower = Land_Lower.DirDeg.Direction,
                dSlopeAngle_Land_Lower = double.Parse(Land_Lower.DirDeg.Degree),
                bCustom_Land_Upper = Land_Upper.UseCustomVal,
                eAzimuth_Land_Upper = Land_Upper.DirDeg.Direction,
                dSlopeAngle_Land_Upper = double.Parse(Land_Upper.DirDeg.Degree),
                dReflectionRange = double.Parse(AnalyzeParam.Instance.Reflection.ReflectionRange)
            };
            p = Marshal.AllocCoTaskMem(Marshal.SizeOf(reflectionParam_T));
            Marshal.StructureToPtr(reflectionParam_T, p, false);
            SetReflectionParam(p);
            Marshal.FreeCoTaskMem(p);

            // その他の解析パラメータ
            var analyzeParam_T = new AnalyzeParam_t
            {
                bExecSolarPotantial = AnalyzeParam.Instance.Target.SolarPotential,
                bExecReflection = AnalyzeParam.Instance.Target.Reflection,
                bExecBuild = AnalyzeParam.Instance.Target.Build,
                bExecLand = AnalyzeParam.Instance.Target.Land,
            };
            p = Marshal.AllocCoTaskMem(Marshal.SizeOf(analyzeParam_T));
            Marshal.StructureToPtr(analyzeParam_T, p, false);
            SetAnalyzeParam(p);
            Marshal.FreeCoTaskMem(p);

            int iDate = 1;  // OneMonth
            switch (dateType)
            {
                case AnalyzeParam.DateType.OneMonth:
                    iDate = 0;
                    break;
                case AnalyzeParam.DateType.OneDay:
                    iDate = 1;
                    break;
                case AnalyzeParam.DateType.Summer:
                    iDate = 2;
                    break;
                case AnalyzeParam.DateType.Winter:
                    iDate = 3;
                    break;
                case AnalyzeParam.DateType.Year:
                    iDate = 4;
                    break;
                default:
                    break;

            }

            // 期間パラメータ
            var analyzeDateParam_T = new AnalyzeDateParam_t
            {
                iTargetDate = iDate,
                iMonth = AnalyzeParam.Instance.Month,
                iDay = AnalyzeParam.Instance.Day,
            };
            p = Marshal.AllocCoTaskMem(Marshal.SizeOf(analyzeDateParam_T));
            Marshal.StructureToPtr(analyzeDateParam_T, p, false);
            SetAnalyzeDateParam(p);
            Marshal.FreeCoTaskMem(p);
		}

        /// <summary>
        /// 実行
        /// </summary>
        private void Exec(string outDir)
        {
            try
            {
                int ret = 0;

                // 開始ログ（解析処理）
                LogManager.Instance.OutputLogMessage("解析処理", LogManager.LogType.START);

                // パラメータ出力
                AnalyzeParam.Instance.SaveParams(Path.Combine(outDir, ResultData.ParamData.PARAM_DIRNAME), StartDate);

                // DLLを初期化
                Initialize();

				// 解析エリア
				int areaCount = AnalyzeParam.Instance.AreaList.Count;
                for (int i = 0; i < areaCount; i++)
                {
                    var area = AnalyzeParam.Instance.AreaList[i];
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
                        eDirection = area.Direction,
                        dDegree = area.Degree,
                        strExplanation = area.Explanation,
                        isWater = area.Water,
                        analyzeBuild = AnalyzeParam.Instance.Target.Build && area.AnalyzeBuild && !area.Water,
                        analyzeLand = AnalyzeParam.Instance.Target.Land && area.AnalyzeLand
                    };

                    var ptr = Marshal.AllocCoTaskMem(Marshal.SizeOf(areaData_T));
                    Marshal.StructureToPtr(areaData_T, ptr, false);
                    AddAnalyzeAreaData(ptr);
                    Marshal.FreeCoTaskMem(ptr);
                }

                // 解析エリアを出力(出力済の一時ファイルをコピー)
                string areaRangeDir = Path.Combine(outDir, ResultData.AnalyzeResult.AREADATA_NAME);
                if (File.Exists(CommonManager.AreaImageTempPath))
                {   // jpg
                    string fileName = Path.GetFileName(CommonManager.AreaImageTempPath);
                    string path = Path.Combine(areaRangeDir, $"{fileName}");
                    CommonManager.Instance.CreateDirectory(areaRangeDir);
                    File.Move(CommonManager.AreaImageTempPath, path);
                    LogManager.Instance.OutputLogMessage($"解析エリア画像：{path}", LogManager.LogType.LOG);
                }
                if (File.Exists(CommonManager.AreaKmlTempPath))
                {   // kml
                    string fileName = Path.GetFileName(CommonManager.AreaKmlTempPath);
                    string path = Path.Combine(areaRangeDir, $"{fileName}");
                    CommonManager.Instance.CreateDirectory(areaRangeDir);
                    File.Move(CommonManager.AreaKmlTempPath, path);
                    LogManager.Instance.OutputLogMessage($"解析エリアKML：{path}", LogManager.LogType.LOG);
                }

                // 3D都市モデルデータフォルダ
                string udxDir = Path.Combine(AnalyzeParam.Instance.InputData.CityModel, "udx");

                // システムデータの出力フォルダ
                string sysDir = Path.Combine(outDir, ResultData.SystemData.SYSTEM_DIRNAME);
                CommonManager.Instance.CreateDirectory(sysDir);

                // 建物モデル読み込み
                string bldgDir = Path.Combine(udxDir, "bldg");
                parentDispatcher.Invoke(() => NoticeUpdateProgressText.Invoke(ProgressDict[eProgress.AnalizeBldgFiles].Text));
                LogManager.Instance.OutputLogMessage("AnalizeBldgFiles", LogManager.LogType.START);
                ret = AnalizeBldgFiles(bldgDir, sysDir, !AnalyzeParam.Instance.SolarPotential.Roof.Interior);
                LogManager.Instance.OutputLogMessage("AnalizeBldgFiles", LogManager.LogType.END);

                if (ret == 1)
                {
                    // ファイルなし
                    throw new Exception("3D都市モデル(建物モデル)の読み込みに失敗しました。\nファイルが存在しません。");
                }
                if (ret == 2)
                {
                    // キャンセル
                    DllDispose();
                    return;
                }
                if (ret == 13 && AnalyzeParam.Instance.Target.Build)
                {
                    // 対象データなし
                    throw new Exception("3D都市モデル(建物モデル)の読み込みに失敗しました。\n範囲内にLOD2の建物が存在しないか、CityGMLのバージョンが異なります。");
                }

                parentDispatcher.Invoke(() => NoticeUpdateProgressBar.Invoke(ProgressDict[eProgress.AnalizeBldgFiles].Val));

                // 地形モデル読み込み
                if (AnalyzeParam.Instance.Target.Land
                    || AnalyzeParam.Instance.InputData.UseDemData)
                {
                    string demDir = Path.Combine(udxDir, "dem");
                    parentDispatcher.Invoke(() => NoticeUpdateProgressText.Invoke(ProgressDict[eProgress.AnalizeDemFiles].Text));
                    LogManager.Instance.OutputLogMessage("AnalizeDemFiles", LogManager.LogType.START);
                    ret = AnalizeDemFiles(demDir, sysDir, AnalyzeParam.Instance.InputData.UseDemData);
                    LogManager.Instance.OutputLogMessage("AnalizeDemFiles", LogManager.LogType.END);

                    if (ret == 1)
                    {
                        throw new Exception("3D都市モデル(地形モデル)の読み込みに失敗しました。\nファイルが存在しません。");
                    }
                    if (ret == 2)
                    {
                        // キャンセル
                        DllDispose();
                        return;
                    }

                    parentDispatcher.Invoke(() => NoticeUpdateProgressBar.Invoke(ProgressDict[eProgress.AnalizeDemFiles].Val));
                }

				// 道路モデル読み込み
				if (AnalyzeParam.Instance.Target.Land
					&& AnalyzeParam.Instance.InputData.UseRoadData)  // 道路モデル使用が有効な場合
				{
					string tranDir = Path.Combine(udxDir, "tran");
					parentDispatcher.Invoke(() => NoticeUpdateProgressText.Invoke(ProgressDict[eProgress.AnalizeTranFiles].Text));
					LogManager.Instance.OutputLogMessage("AnalizeTranFiles", LogManager.LogType.START);
					ret = AnalizeTranFiles(tranDir, sysDir);
					LogManager.Instance.OutputLogMessage("AnalizeTranFiles", LogManager.LogType.END);

					if (ret == 1)
					{
						throw new Exception("3D都市モデル(道路モデル)の読み込みに失敗しました。\nファイルが存在しません。");
					}
					if (ret == 2)
					{
                        // キャンセル
                        DllDispose();
                        return;
					}

					parentDispatcher.Invoke(() => NoticeUpdateProgressBar.Invoke(ProgressDict[eProgress.AnalizeTranFiles].Val));
				}

                // 土地面の抽出
                if (AnalyzeParam.Instance.Target.Land)
                {
                    parentDispatcher.Invoke(() => NoticeUpdateProgressText.Invoke(ProgressDict[eProgress.ExtractLandMesh].Text));
                    LogManager.Instance.OutputLogMessage("ExtractLandMesh", LogManager.LogType.START);
                    ret = ExtractLandMesh(AnalyzeParam.Instance.InputData.UseRoadData, sysDir);
                    LogManager.Instance.OutputLogMessage("ExtractLandMesh", LogManager.LogType.END);

                    if (ret == 1)
                    {
                        throw new Exception("土地面の抽出処理に失敗しました。ファイルが存在しません。");
                    }
                    if (ret == 2)
                    {
                        // キャンセル
                        DllDispose();
                        return;
                    }

                    parentDispatcher.Invoke(() => NoticeUpdateProgressBar.Invoke(ProgressDict[eProgress.ExtractLandMesh].Val));
                }

                // 期間ごとに解析実行
                foreach (var t in Enum.GetValues(typeof(AnalyzeParam.DateType)))
                {
                    AnalyzeParam.DateType dateType = (AnalyzeParam.DateType)t;
                    if (dateType is AnalyzeParam.DateType.None) continue;
                    if (!AnalyzeParam.Instance.TargetDate.HasFlag(dateType)) continue;

                    // 期間ごとのフォルダを作成
                    string resultDir = Path.Combine(outDir, ResultData.AnalyzeResult.GetResultDateDirName(dateType));

                    // シミュレーション結果出力フォルダ作成
                    string dataDir = Path.Combine(resultDir, ResultData.AnalyzeResult.SIM_DIRNAME);
                    CommonManager.Instance.CreateDirectory(dataDir);

                    // パラメータ設定
                    SetDllParams(dateType);

                    // 解析開始
                    parentDispatcher.Invoke(() => NoticeUpdateProgressText.Invoke(ProgressDict[eProgress.AnalyzeStart].Text));
                    LogManager.Instance.OutputLogMessage("AnalyzeStart", LogManager.LogType.START);
                    ret = AnalyzeStart(dataDir);
                    LogManager.Instance.OutputLogMessage("AnalyzeStart", LogManager.LogType.END);
                    if (ret == 1)
                    {
                        throw new Exception("解析処理に失敗しました。");
                    }
                    if (ret == 2)
                    {
                        // キャンセル
                        DllDispose();
                        return;
                    }
                    if (ret == 3)
                    {
                        throw new Exception("解析処理に失敗しました。\n入力データの読み込みに失敗しました。");
                    }
                    // 可照時間データ不正エラー
                    if (ret == 10)
                    {
                        throw new Exception("解析処理に失敗しました。\n可照時間データの読み込みに失敗しました。");
                    }
                    // 日照時間データ不正エラー
                    if (ret == 11)
                    {
                        throw new Exception("解析処理に失敗しました。\n日照時間データの読み込みに失敗しました。");
                    }
                    // 積雪深データ不正エラー
                    if (ret == 12)
                    {
                        throw new Exception("解析処理に失敗しました。\n積雪深データの読み込みに失敗しました。");
                    }
                    parentDispatcher.Invoke(() => NoticeUpdateProgressBar.Invoke(ProgressDict[eProgress.AnalyzeStart].Val));

                    if (AnalyzeParam.Instance.Target.Build)
                    {
                        // CityGML出力フォルダ作成
                        string gmlDir = Path.Combine(resultDir, ResultData.AnalyzeResult.CITYGML_DIRNAME);
                        CommonManager.Instance.CreateDirectory(gmlDir);

                        // CityGMLの属性に付与して出力
                        parentDispatcher.Invoke(() => NoticeUpdateProgressText.Invoke(ProgressDict[eProgress.LOD2DataOut].Text));
                        LogManager.Instance.OutputLogMessage("LOD2DataOut", LogManager.LogType.START);
                        ret = LOD2DataOut(bldgDir, resultDir);
                        LogManager.Instance.OutputLogMessage("LOD2DataOut", LogManager.LogType.END);

                        if (ret == 1)
                        {
                            // ファイルなし
                            throw new Exception("3D都市モデル(建物モデル)の出力に失敗しました。\n");
                        }
                        if (ret == 2)
                        {
                            // キャンセル
                            DllDispose();
                            return;
                        }
                        parentDispatcher.Invoke(() => NoticeUpdateProgressBar.Invoke(ProgressDict[eProgress.LOD2DataOut].Val));
                    }

                }

                // 適地判定用のファイルを作成
                OutputCoordinatesFile(sysDir);

                // C++側の解放処理
                DllDispose();

                // 終了ログ（解析処理）
                LogManager.Instance.OutputLogMessage("解析処理", LogManager.LogType.END);
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
