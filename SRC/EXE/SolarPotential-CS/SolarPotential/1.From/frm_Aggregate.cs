using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using SolarPotential._3.Class;
using System.Runtime.InteropServices;
using System.Threading;

namespace SolarPotential
{
    public partial class frm_Aggregate : Form
    {
        // 初期値格納ファイル名（Exe直下に配置予定）
        private string initFileName = "initFile_Aggregate.txt";

        private int endFlg = 0;

        // コントロール配列のリスト
        private List<cls_common.controlObj> listControl = new List<cls_common.controlObj>();

        // デフォルト：座標情報＆ズームレベル格納用
        private double dbl_topMax = 0;
        private double dbl_bottomMax = 0;
        private double dbl_leftMax = 0;
        private double dbl_rightMax = 0;

        // 前回入力値
        private string txt_Param_1_Path = "";

        // 非同期処理をCancelするためのTokenを取得.
        private CancellationTokenSource TaskCanceler;

        //delegateを宣言
        delegate void DelegateProcess(Form form);

        private static frm_Aggregate _parentInstance;
        //Form親オブジェクトを取得、設定するためのプロパティ
        public static frm_Aggregate ParentInstance
        {
            get
            {
                return _parentInstance;
            }
            set
            {
                _parentInstance = value;
            }
        }

        public CancellationTokenSource taskCanceler
        {
            get
            {
                return TaskCanceler;
            }
            set
            {
                //空白 ←子から書き換えられないようにする場合。
            }

        }
        [DllImport("AggregateData.dll")]
        static extern int AggregateBldgFiles(string str, string outstr);
        [DllImport("AggregateData.dll")]
        static extern int AggregateAllData(string str, string outstr);
        // キャプチャ用
        [System.Runtime.InteropServices.DllImport("User32.dll")]
        private extern static bool PrintWindow(IntPtr hwnd, IntPtr hDC, uint nFlags);


        [DllImport("AggregateData.dll")]
        static extern void SetJPZone();
        [DllImport("JudgeSuitablePlace.dll")]
        internal static extern void InitializeUIParam();
        [DllImport("JudgeSuitablePlace.dll")]
        internal static extern void SetAggregateParam(IntPtr param);
        [DllImport("JudgeSuitablePlace.dll")]
        internal static extern bool SetOutputPath(string aggregatePath);
        [DllImport("JudgeSuitablePlace.dll")]
        internal static extern bool SetBldgResultPath(string analyzePath);
        [DllImport("JudgeSuitablePlace.dll")]
        internal static extern int JadgeStart();

        public const int MAX_PATH = 260;

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
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = MAX_PATH)]
            public string strParam_2_4;        // 気象データ(積雪)フォルダパス
            public double dParam_2_4_1;        // 気象データ(積雪)_cm以上
            public double dParam_2_4_2;        // 積雪荷重(kgf/m2)
            public double dParam_2_4_3;        // 単位荷重(N/m2)

            // 太陽光パネルの設置に制限がある施設の判定
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = MAX_PATH)]
            public string strParam_3_1;       // 制限を設ける範囲のシェープファイル_フォルダパス１
            public double dParam_3_1_1;       // 制限を設ける範囲のシェープファイル_高さ１
            public int iParam_3_1_2;          // 制限を設ける範囲のシェープファイル_方位１
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = MAX_PATH)]
            public string strParam_3_2;       // 制限を設ける範囲のシェープファイル_フォルダパス２
            public double dParam_3_2_1;       // 制限を設ける範囲のシェープファイル_高さ２
            public int iParam_3_2_2;          // 制限を設ける範囲のシェープファイル_方位２
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = MAX_PATH)]
            public string strParam_3_3;       // 制限を設ける範囲のシェープファイル_フォルダパス３
            public double dParam_3_3_1;       // 制限を設ける範囲のシェープファイル_高さ３
            public int iParam_3_3_2;          // 制限を設ける範囲のシェープファイル_方位３
        }

        public frm_Aggregate()
        {
            InitializeComponent();

            map_disp();

            //初期設定
            Initial();
            chk_Param_CheckedChanged(null,null);

            //親Instanceに代入
            frm_Aggregate.ParentInstance = this;

            web_Map_Area.NewWindow += CancelNewWindow;
        }


        #region 初期設定
        private void Initial()
        {

            // 配列の内容をコンボボックスアイテムに追加する
            string[] items = { "", "北向き", "北東向き", "東向き", "南東向き", "南向き", "南西向き", "西向き", "北西向き" };
            cmb_Param_3_1_2.Items.AddRange(items);
            cmb_Param_3_2_2.Items.AddRange(items);
            cmb_Param_3_3_2.Items.AddRange(items);


            // チェック時切り替えイベント
            chk_Param_1_1.CheckedChanged += new EventHandler(chk_Param_CheckedChanged);
            chk_Param_1_1_1.CheckedChanged += new EventHandler(chk_Param_CheckedChanged);
            chk_Param_1_1_2.CheckedChanged += new EventHandler(chk_Param_CheckedChanged);
            chk_Param_1_2.CheckedChanged += new EventHandler(chk_Param_CheckedChanged);
            chk_Param_1_3.CheckedChanged += new EventHandler(chk_Param_CheckedChanged);
            chk_Param_2_4.CheckedChanged += new EventHandler(chk_Param_CheckedChanged);
            rdo_Param_2_4_1.CheckedChanged += new EventHandler(chk_Param_CheckedChanged);
            rdo_Param_2_4_2.CheckedChanged += new EventHandler(chk_Param_CheckedChanged);
            chk_Param_3_1.CheckedChanged += new EventHandler(chk_Param_CheckedChanged);
            chk_Param_3_2.CheckedChanged += new EventHandler(chk_Param_CheckedChanged);
            chk_Param_3_3.CheckedChanged += new EventHandler(chk_Param_CheckedChanged);

            // 入力チェックのバリデーションの設定（テキストボックスに設定予定）
            txt_Max_Lat.Validating += textBox_Validated;
            txt_Min_Lon.Validating += textBox_Validated;
            txt_Max_Lon.Validating += textBox_Validated;
            txt_Min_Lat.Validating += textBox_Validated;
            txt_Param_1_1_1.Validating += textBox_Validated;
            txt_Param_1_1_2.Validating += textBox_Validated;
            txt_Param_1_3_1.Validating += textBox_Validated;
            txt_Param_1_3_2.Validating += textBox_Validated;
            txt_Param_2_4_1.Validating += textBox_Validated;
            txt_Param_2_4_2.Validating += textBox_Validated;
            txt_Param_2_4_3.Validating += textBox_Validated;

            // 追加したバリデーション
            txt_Param_3_1_1.Validating += textBox_Validated;
            txt_Param_3_2_1.Validating += textBox_Validated;
            txt_Param_3_3_1.Validating += textBox_Validated;

            chk_Param_1_1.CheckedChanged += textBox_Validated;
            chk_Param_1_1_1.CheckedChanged += textBox_Validated;
            chk_Param_1_1_2.CheckedChanged += textBox_Validated;
            chk_Param_1_3.CheckedChanged += textBox_Validated;
            chk_Param_2_4.CheckedChanged += textBox_Validated;
            rdo_Param_2_4_1.CheckedChanged += textBox_Validated;
            rdo_Param_2_4_2.CheckedChanged += textBox_Validated;
            chk_Param_3_1.CheckedChanged += textBox_Validated;
            chk_Param_3_2.CheckedChanged += textBox_Validated;
            chk_Param_3_3.CheckedChanged += textBox_Validated;

            // バリデーションエラー時のアイコン設定
            errorProvider.Clear();                                      // 不要？
            errorProvider.BlinkStyle = ErrorBlinkStyle.NeverBlink;      // アイコンを点滅なしに設定する


            // コントロールのリストに値を格納　※項目追加時はリストに要追加（同じ項目名は指定しない）
            listControl = new List<cls_common.controlObj> { new cls_common.controlObj{ControlName_JP =  "解析結果データ", ControlName = "txt_Param_1", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP =  "全範囲で集計", ControlName = "rdo_Param_2", Value = "", Type =  cls_common.CON_RADIO},
                                                 new cls_common.controlObj{ControlName_JP =  "集計範囲を選択", ControlName = "rdo_Param_3", Value = "", Type =  cls_common.CON_RADIO},
                                                 new cls_common.controlObj{ControlName_JP =  "最大緯度", ControlName = "txt_Max_Lat", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP =  "最小経度", ControlName = "txt_Min_Lon", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP =  "最大経度", ControlName = "txt_Max_Lon", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP =  "最小緯度", ControlName = "txt_Min_Lat", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP =  "集計結果出力フォルダ", ControlName = "txt_Output_Directory", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP =  "日射量が少ない施設を除外", ControlName = "chk_Param_1_1", Value = "", Type =  cls_common.CON_CHECK},
                                                 new cls_common.controlObj{ControlName_JP =  "日射量_除外範囲", ControlName = "chk_Param_1_1_1", Value = "", Type =  cls_common.CON_CHECK},
                                                 new cls_common.controlObj{ControlName_JP =  "日射量_除外範囲kWh/m", ControlName = "txt_Param_1_1_1", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP =  "日射量_除外下位", ControlName = "chk_Param_1_1_2", Value = "", Type =  cls_common.CON_CHECK},
                                                 new cls_common.controlObj{ControlName_JP =  "日射量_除外下位％", ControlName = "txt_Param_1_1_2", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP =  "建物構造による除外", ControlName = "chk_Param_1_2", Value = "", Type =  cls_common.CON_CHECK},
                                                 new cls_common.controlObj{ControlName_JP =  "木造・土蔵造", ControlName = "chk_Param_1_2_1", Value = "", Type =  cls_common.CON_CHECK},
                                                 new cls_common.controlObj{ControlName_JP =  "鉄骨鉄筋コンクリート造", ControlName = "chk_Param_1_2_2", Value = "", Type =  cls_common.CON_CHECK},
                                                 new cls_common.controlObj{ControlName_JP =  "鉄筋コンクリート造", ControlName = "chk_Param_1_2_3", Value = "", Type =  cls_common.CON_CHECK},
                                                 new cls_common.controlObj{ControlName_JP =  "鉄骨造", ControlName = "chk_Param_1_2_4", Value = "", Type =  cls_common.CON_CHECK},
                                                 new cls_common.controlObj{ControlName_JP =  "軽量鉄骨造", ControlName = "chk_Param_1_2_5", Value = "", Type =  cls_common.CON_CHECK},
                                                 new cls_common.controlObj{ControlName_JP =  "レンガ造・コンクリートブロック造・石造", ControlName = "chk_Param_1_2_6", Value = "", Type =  cls_common.CON_CHECK},
                                                 new cls_common.controlObj{ControlName_JP =  "不明", ControlName = "chk_Param_1_2_7", Value = "", Type =  cls_common.CON_CHECK},
                                                 new cls_common.controlObj{ControlName_JP =  "非木造", ControlName = "chk_Param_1_2_8", Value = "", Type =  cls_common.CON_CHECK},
                                                 new cls_common.controlObj{ControlName_JP =  "特定の階層の施設を除外", ControlName = "chk_Param_1_3", Value = "", Type =  cls_common.CON_CHECK},
                                                 new cls_common.controlObj{ControlName_JP =  "階以下", ControlName = "txt_Param_1_3_1", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP =  "階以上", ControlName = "txt_Param_1_3_2", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP =  "最大津波高さを下回る建物を除外", ControlName = "chk_Param_2_1", Value = "", Type =  cls_common.CON_CHECK},
                                                 new cls_common.controlObj{ControlName_JP =  "想定される河川浸水想定浸水深を下回る建物を除外", ControlName = "chk_Param_2_2", Value = "", Type =  cls_common.CON_CHECK},
                                                 new cls_common.controlObj{ControlName_JP =  "土砂災害警戒区域内に存在する建物を除外", ControlName = "chk_Param_2_3", Value = "", Type =  cls_common.CON_CHECK},
                                                 new cls_common.controlObj{ControlName_JP =  "気象データ(積雪)", ControlName = "chk_Param_2_4", Value = "", Type =  cls_common.CON_CHECK},
                                                 new cls_common.controlObj{ControlName_JP =  "気象データ(積雪)_パス", ControlName = "txt_Param_2_4", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP =  "積雪が多い地域の建物を除外", ControlName = "rdo_Param_2_4_1", Value = "", Type =  cls_common.CON_RADIO},
                                                 new cls_common.controlObj{ControlName_JP =  "積雪が多い地域の建物を除外_cm以上", ControlName = "txt_Param_2_4_1", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP =  "積雪荷重(kgf/m3)年最深積雪量", ControlName = "rdo_Param_2_4_2", Value = "", Type =  cls_common.CON_RADIO},
                                                 new cls_common.controlObj{ControlName_JP =  "積雪荷重(kgf/m3)", ControlName = "txt_Param_2_4_2", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP =  "年最深積雪量(N/m3)", ControlName = "txt_Param_2_4_3", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP =  "制限を設ける範囲のシェープファイル１", ControlName = "chk_Param_3_1", Value = "", Type =  cls_common.CON_CHECK},
                                                 new cls_common.controlObj{ControlName_JP =  "制限を設ける範囲のシェープファイル１_パス", ControlName = "txt_Param_3_1", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP =  "制限を設ける範囲のシェープファイル１_高さ", ControlName = "txt_Param_3_1_1", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP =  "制限を設ける範囲のシェープファイル１_方位", ControlName = "cmb_Param_3_1_2", Value = "", Type =  cls_common.CON_COMB},
                                                 new cls_common.controlObj{ControlName_JP =  "制限を設ける範囲のシェープファイル２", ControlName = "chk_Param_3_2", Value = "", Type =  cls_common.CON_CHECK},
                                                 new cls_common.controlObj{ControlName_JP =  "制限を設ける範囲のシェープファイル２_パス", ControlName = "txt_Param_3_2", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP =  "制限を設ける範囲のシェープファイル２_高さ", ControlName = "txt_Param_3_2_1", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP =  "制限を設ける範囲のシェープファイル２_方位", ControlName = "cmb_Param_3_2_2", Value = "", Type =  cls_common.CON_COMB},
                                                 new cls_common.controlObj{ControlName_JP =  "制限を設ける範囲のシェープファイル３", ControlName = "chk_Param_3_3", Value = "", Type =  cls_common.CON_CHECK},
                                                 new cls_common.controlObj{ControlName_JP =  "制限を設ける範囲のシェープファイル３_パス", ControlName = "txt_Param_3_3", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP =  "制限を設ける範囲のシェープファイル３_高さ", ControlName = "txt_Param_3_3_1", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP =  "制限を設ける範囲のシェープファイル３_方位", ControlName = "cmb_Param_3_3_2", Value = "", Type =  cls_common.CON_COMB},
            };

            // 初期値の読み込み処理
            if (set_controlValue(initFileName) == false)
            {
                // 失敗メッセージ
                cls_common.showMessage(cls_message.general.ERRMSG_DEFAULT_LOADING, cls_common.MessageType.error);
            }


        }
        #endregion

        // 地図表示
        private void map_disp()
        {
            cls_ExternalMapObj extMapObj = new cls_ExternalMapObj();
            web_Map_Area.ObjectForScripting = extMapObj;

            extMapObj.MoveCenter += new cls_ExternalMapObj.MoveCenterEventHandler(this.MoveCenter);
            extMapObj.PickStart += new cls_ExternalMapObj.PickStartEventHandler(this.PickStart);

            var cls_hml = new cls_HTML();
            setDefault_Coordinates(txt_Param_1.Text);
            web_Map_Area.DocumentText = cls_hml.GetHTMLText();

        }

        // ファイル保存ダイアログオープン
        // ・「入力内容の保存」
        private void open_SaveDialog(object sender, EventArgs e)
        {
            SaveFileDialog sfd = new SaveFileDialog();
            // 初期表示フォルダ
            sfd.InitialDirectory = @"C:\";
            // [ファイルの種類]にテキストファイルを表示
            sfd.Filter = "Textファイル(*.txt)|*.txt";
            // [ファイルの種類]でテキストファイルを初期選択
            sfd.FilterIndex = 1;
            // タイトル設定（ファイル選択）
            sfd.Title = cls_message.general.MSG_SELECT_SAVE_FILE;
            // ダイアログボックスを閉じる前に現在のディレクトリを復元
            sfd.RestoreDirectory = true;
            // ダイアログを表示
            if (sfd.ShowDialog() == DialogResult.OK)
            {
                // OKボタンがクリック時
                // 設定ファイル保存処理

                // 指定ファイルに入力パラメータを出力
                outputParam(sfd.FileName);

                cls_common.showMessage(cls_message.general.MSG_SAVE_COMPLETE, cls_common.MessageType.info,"完了");
            }
            sfd.Dispose();
        }

        /// <summary>
        /// 指定ファイルに処理パラメータを出力する
        /// </summary>
        /// <param name="FileName"></param>
        private void outputParam(string FileName , bool append = false)
        {
            Encoding enc = Encoding.GetEncoding("UTF-8");
            using (StreamWriter writer = new StreamWriter(FileName, append, enc))
            {

                // 入力値の判定（ラジオボタン）
                string rdoParam_2 = rdo_Param_2.Checked ? "1" : "0";
                string rdoParam_3 = rdo_Param_3.Checked ? "1" : "0";
                string rdoParam_2_4_1 = rdo_Param_2_4_1.Checked ? "1" : "0";
                string rdoParam_2_4_2 = rdo_Param_2_4_2.Checked ? "1" : "0";

                // 入力値の判定（チェックボックス）
                string chkParam_1_1 = chk_Param_1_1.Checked ? "1" : "0";
                string chkParam_1_1_1 = chk_Param_1_1_1.Checked ? "1" : "0";
                string chkParam_1_1_2 = chk_Param_1_1_2.Checked ? "1" : "0";
                string chkParam_1_2 = chk_Param_1_2.Checked ? "1" : "0";
                string chkParam_1_2_1 = chk_Param_1_2_1.Checked ? "1" : "0";
                string chkParam_1_2_2 = chk_Param_1_2_2.Checked ? "1" : "0";
                string chkParam_1_2_3 = chk_Param_1_2_3.Checked ? "1" : "0";
                string chkParam_1_2_4 = chk_Param_1_2_4.Checked ? "1" : "0";
                string chkParam_1_2_5 = chk_Param_1_2_5.Checked ? "1" : "0";
                string chkParam_1_2_6 = chk_Param_1_2_6.Checked ? "1" : "0";
                string chkParam_1_2_7 = chk_Param_1_2_7.Checked ? "1" : "0";
                string chkParam_1_2_8 = chk_Param_1_2_8.Checked ? "1" : "0";
                string chkParam_1_3 = chk_Param_1_3.Checked ? "1" : "0";

                string chkParam_2_1 = chk_Param_2_1.Checked ? "1" : "0";
                string chkParam_2_2 = chk_Param_2_2.Checked ? "1" : "0";
                string chkParam_2_3 = chk_Param_2_3.Checked ? "1" : "0";
                string chkParam_2_4 = chk_Param_2_4.Checked ? "1" : "0";

                string chkParam_3_1 = chk_Param_3_1.Checked ? "1" : "0";
                string chkParam_3_2 = chk_Param_3_2.Checked ? "1" : "0";
                string chkParam_3_3 = chk_Param_3_3.Checked ? "1" : "0";

                // 各グループ毎に1行カンマ区切りで入力値を出力していく
                // 区切り文字　※保存する入力文字と値の出力用
                var delimiter = ":";
                var comma = ",";

                // ・解析処理入力データ選択
                List<string> list_AnalyzingInputSelection = new List<string>();
                list_AnalyzingInputSelection.Add($"{cls_common.get_controlNameJP("txt_Param_1", listControl)}{delimiter}{txt_Param_1.Text}");// パス項目には前後に改行
                list_AnalyzingInputSelection.Add($"{ Environment.NewLine}{cls_common.get_controlNameJP("rdo_Param_2", listControl)}{delimiter}{rdoParam_2}");
                list_AnalyzingInputSelection.Add($"{cls_common.get_controlNameJP("rdo_Param_3", listControl)}{delimiter}{rdoParam_3}");

                // 入力をカンマ区切りの文字列で出力
                Console.WriteLine(string.Join(comma, list_AnalyzingInputSelection));
                writer.WriteLine(string.Join(comma, list_AnalyzingInputSelection));

                // 選択範囲
                List<string> list_SelectedRange = new List<string>();
                list_SelectedRange.Add($"{cls_common.get_controlNameJP("txt_Max_Lat", listControl)}{delimiter}{txt_Max_Lat.Text}");
                list_SelectedRange.Add($"{cls_common.get_controlNameJP("txt_Min_Lon", listControl)}{delimiter}{txt_Min_Lon.Text}");
                list_SelectedRange.Add($"{cls_common.get_controlNameJP("txt_Max_Lon", listControl)}{ delimiter}{txt_Max_Lon.Text}");
                list_SelectedRange.Add($"{cls_common.get_controlNameJP("txt_Min_Lat", listControl)}{ delimiter}{txt_Min_Lat.Text}");

                // 入力をカンマ区切りの文字列で出力
                Console.WriteLine(string.Join(comma, list_SelectedRange));
                writer.WriteLine(string.Join(comma, list_SelectedRange));

                // 集計結果出力フォルダ
                writer.WriteLine($"{cls_common.get_controlNameJP("txt_Output_Directory", listControl)}{delimiter}{ txt_Output_Directory.Text}");

                // 太陽光パネルの設置に関して優先度が低い施設の判定
                List<string> list_DecisionLowByPriority = new List<string>();
                list_DecisionLowByPriority.Add($"{cls_common.get_controlNameJP("chk_Param_1_1", listControl)}{delimiter}{chkParam_1_1}");
                list_DecisionLowByPriority.Add($"{cls_common.get_controlNameJP("chk_Param_1_1_1", listControl)}{delimiter}{chkParam_1_1_1}");
                list_DecisionLowByPriority.Add($"{cls_common.get_controlNameJP("txt_Param_1_1_1", listControl)}{delimiter}{txt_Param_1_1_1.Text}");
                list_DecisionLowByPriority.Add($"{cls_common.get_controlNameJP("chk_Param_1_1_2", listControl)}{delimiter}{chkParam_1_1_2}");
                list_DecisionLowByPriority.Add($"{cls_common.get_controlNameJP("txt_Param_1_1_2", listControl)}{delimiter}{txt_Param_1_1_2.Text}");
                list_DecisionLowByPriority.Add($"{cls_common.get_controlNameJP("chk_Param_1_2", listControl)}{delimiter}{chkParam_1_2}");
                list_DecisionLowByPriority.Add($"{cls_common.get_controlNameJP("chk_Param_1_2_1", listControl)}{delimiter}{chkParam_1_2_1}");
                list_DecisionLowByPriority.Add($"{cls_common.get_controlNameJP("chk_Param_1_2_2", listControl)}{delimiter}{chkParam_1_2_2}");
                list_DecisionLowByPriority.Add($"{cls_common.get_controlNameJP("chk_Param_1_2_3", listControl)}{delimiter}{chkParam_1_2_3}");
                list_DecisionLowByPriority.Add($"{cls_common.get_controlNameJP("chk_Param_1_2_4", listControl)}{delimiter}{chkParam_1_2_4}");
                list_DecisionLowByPriority.Add($"{cls_common.get_controlNameJP("chk_Param_1_2_5", listControl)}{delimiter}{chkParam_1_2_5}");
                list_DecisionLowByPriority.Add($"{cls_common.get_controlNameJP("chk_Param_1_2_6", listControl)}{delimiter}{chkParam_1_2_6}");
                list_DecisionLowByPriority.Add($"{cls_common.get_controlNameJP("chk_Param_1_2_7", listControl)}{delimiter}{chkParam_1_2_7}");
                list_DecisionLowByPriority.Add($"{cls_common.get_controlNameJP("chk_Param_1_2_8", listControl)}{delimiter}{chkParam_1_2_8}");
                list_DecisionLowByPriority.Add($"{cls_common.get_controlNameJP("chk_Param_1_3", listControl)}{delimiter}{chkParam_1_3}");
                list_DecisionLowByPriority.Add($"{cls_common.get_controlNameJP("txt_Param_1_3_1", listControl)}{delimiter}{txt_Param_1_3_1.Text}");
                list_DecisionLowByPriority.Add($"{cls_common.get_controlNameJP("txt_Param_1_3_2", listControl)}{delimiter}{txt_Param_1_3_2.Text}");

                // 入力をカンマ区切りの文字列で出力
                Console.WriteLine(string.Join(comma, list_DecisionLowByPriority));
                writer.WriteLine(string.Join(comma, list_DecisionLowByPriority));

                // 災害時に太陽光パネルが破損、消失する危険性の判定
                List<string> list_DecisionDangerousness = new List<string>();
                list_DecisionDangerousness.Add($"{cls_common.get_controlNameJP("chk_Param_2_1", listControl)}{delimiter}{chkParam_2_1}");
                list_DecisionDangerousness.Add($"{cls_common.get_controlNameJP("chk_Param_2_2", listControl)}{delimiter}{chkParam_2_2}");
                list_DecisionDangerousness.Add($"{cls_common.get_controlNameJP("chk_Param_2_3", listControl)}{delimiter}{chkParam_2_3}");
                list_DecisionDangerousness.Add($"{cls_common.get_controlNameJP("chk_Param_2_4", listControl)}{delimiter}{chkParam_2_4}");
                list_DecisionDangerousness.Add($"{cls_common.get_controlNameJP("txt_Param_2_4", listControl)}{delimiter}{txt_Param_2_4.Text}");// パス項目には前後に改行
                list_DecisionDangerousness.Add($"{cls_common.get_controlNameJP("rdo_Param_2_4_1", listControl)}{delimiter}{rdoParam_2_4_1}");
                list_DecisionDangerousness.Add($"{cls_common.get_controlNameJP("txt_Param_2_4_1", listControl)}{delimiter}{txt_Param_2_4_1.Text}");
                list_DecisionDangerousness.Add($"{cls_common.get_controlNameJP("rdo_Param_2_4_2", listControl)}{delimiter}{rdoParam_2_4_2}");
                list_DecisionDangerousness.Add($"{cls_common.get_controlNameJP("txt_Param_2_4_3", listControl)}{delimiter}{txt_Param_2_4_3.Text}");
                // 入力をカンマ区切りの文字列で出力
                Console.WriteLine(string.Join(comma, list_DecisionDangerousness));
                writer.WriteLine(string.Join(comma, list_DecisionDangerousness));


                // 太陽光パネルの設置に制限がある施設の判定
                List<string> list_DecisionInstallationLimit = new List<string>();
                list_DecisionInstallationLimit.Add($"{cls_common.get_controlNameJP("chk_Param_3_1", listControl)}{delimiter}{chkParam_3_1}");
                list_DecisionInstallationLimit.Add($"{ Environment.NewLine}{cls_common.get_controlNameJP("txt_Param_3_1", listControl)}{delimiter}{txt_Param_3_1.Text}"); //  パス項目には前後に改行                   
                list_DecisionInstallationLimit.Add($"{ Environment.NewLine}{cls_common.get_controlNameJP("txt_Param_3_1_1", listControl)}{delimiter}{txt_Param_3_1_1.Text}");
                list_DecisionInstallationLimit.Add($"{cls_common.get_controlNameJP("cmb_Param_3_1_2", listControl)}{delimiter}{cmb_Param_3_1_2.SelectedIndex.ToString()}");
                list_DecisionInstallationLimit.Add($"{ Environment.NewLine}{cls_common.get_controlNameJP("chk_Param_3_2", listControl)}{delimiter}{chkParam_3_2}");
                list_DecisionInstallationLimit.Add($"{ Environment.NewLine}{cls_common.get_controlNameJP("txt_Param_3_2", listControl)}{delimiter}{txt_Param_3_2.Text}"); //  パス項目には前後に改行       
                list_DecisionInstallationLimit.Add($"{ Environment.NewLine}{cls_common.get_controlNameJP("txt_Param_3_2_1", listControl)}{delimiter}{txt_Param_3_2_1.Text}");
                list_DecisionInstallationLimit.Add($"{cls_common.get_controlNameJP("cmb_Param_3_2_2", listControl)}{delimiter}{cmb_Param_3_2_2.SelectedIndex.ToString()}");
                list_DecisionInstallationLimit.Add($"{ Environment.NewLine}{cls_common.get_controlNameJP("chk_Param_3_3", listControl)}{delimiter}{chkParam_3_3}");
                list_DecisionInstallationLimit.Add($"{ Environment.NewLine}{cls_common.get_controlNameJP("txt_Param_3_3", listControl)}{delimiter}{txt_Param_3_3.Text}"); //  パス項目には前後に改行       
                list_DecisionInstallationLimit.Add($"{ Environment.NewLine}{cls_common.get_controlNameJP("txt_Param_3_3_1", listControl)}{delimiter}{txt_Param_3_3_1.Text}");
                list_DecisionInstallationLimit.Add($"{cls_common.get_controlNameJP("cmb_Param_3_3_2", listControl)}{delimiter}{cmb_Param_3_3_2.SelectedIndex.ToString()}");

                // 入力をカンマ区切りの文字列で出力
                Console.WriteLine(string.Join(comma, list_DecisionInstallationLimit));
                writer.WriteLine(string.Join(comma, list_DecisionInstallationLimit));

            }
        }

        // フォルダ選択
        // ・「解析結果データ」          tag:10 フォルダ
        // ・「集計結果出力フォルダ」    tag:20 フォルダ
        // ・「制限を設ける範囲のシェープファイル」  tag:40 フォルダ
        // ・「制限を設ける範囲のシェープファイル」  tag:41 フォルダ
        // ・「制限を設ける範囲のシェープファイル」  tag:42 フォルダ
        // ・「保存した入力内容の読込」  tag:XX ファイル ※後ほど追加予定        

        // フォルダ選択ダイアログオープン
        private void btn_Folder_Dialog_Click(object sender, EventArgs e)
        {
            // 押下されたボタンにより分岐
            string description = "";
            string ctlname = "";
            string directory = "";
            switch (((Button)sender).Tag)
            {
                case "10":    // 解析結果データ
                    description = cls_message.frm_Aggregate.MSG_001;
                    ctlname = "txt_Param_1";
                    directory = txt_Param_1.Text;
                    break;
                case "20":    // 集計結果出力フォルダ
                    description = cls_message.frm_Aggregate.MSG_002;
                    ctlname = "txt_Output_Directory";
                    directory = txt_Output_Directory.Text;
                    break;

            }
            // 指定パス存在チェック
            if (!System.IO.Directory.Exists(directory))
            {
                // 存在しない場合はデフォルトを設定
                directory = @"C:\";
            }
            FolderBrowserDialog fbd = new FolderBrowserDialog();
            // 説明
            fbd.Description = description;
            // ルートフォルダを指定(デフォルトはDesktop)
            fbd.RootFolder = Environment.SpecialFolder.Desktop;
            // 最初に選択するフォルダを指定する
            fbd.SelectedPath = directory;
            // ユーザーが新しいフォルダを作成できるようにする
            fbd.ShowNewFolderButton = true;
            // ダイアログを表示する
            if (fbd.ShowDialog(this) == DialogResult.OK)
            {
                // 選択したパスをテキストボックスに表示
                Control[] controls = this.Controls.Find(ctlname, true);
                if (controls.Length > 0)
                {
                    ((TextBox)controls[0]).Text = fbd.SelectedPath;
                }
                switch (((Button)sender).Tag)
                {
                    case "10":    // 解析結果データ

                        mapData_selection();

                        break;
                }
            }
        }

        // コントロールに値をセットする
        private bool set_controlValue(string fileName)
        {
            // ファイル読み込み
            try
            {
                // ファイル保存した入力内容の読み込み、配列に保持（日本語名と値のみ）
                var saveControls = new List<cls_common.controlObj>();
                saveControls = cls_common.read_SaveFile(fileName);

                // 取得した保存内容をコントロールにセット
                for (int i = 0; i < listControl.Count; i++)
                {
                    // 値のセット
                    var test = saveControls.Find(x => x.ControlName_JP == listControl[i].ControlName_JP).Value;

                    // 名前からコントロールを検索
                    Control[] cList = this.Controls.Find(listControl[i].ControlName, true);
                    for (int n = 0; n < cList.Length; n++)
                    {

                        if (cList[n].Name == "rdo_Param_2")
                        {
                            // 値のセット
                            test = saveControls.Find(x => x.ControlName_JP == listControl[i].ControlName_JP).Value;


                        }
                        // コントロールのタイプによって分岐、コントロール名と合致する値を検索しセットする
                        switch (listControl[i].Type)
                        {
                            case cls_common.CON_TEXT:
                                if (cList[n] is TextBox)
                                {
                                    ((TextBox)cList[n]).Text = saveControls.Find(x => x.ControlName_JP == listControl[i].ControlName_JP).Value;
                                }
                                break;

                            case cls_common.CON_COMB:
                                ((ComboBox)cList[n]).SelectedIndex = int.Parse(saveControls.Find(x => x.ControlName_JP == listControl[i].ControlName_JP).Value);
                                break;

                            case cls_common.CON_RADIO:
                                var radio = int.Parse(saveControls.Find(x => x.ControlName_JP == listControl[i].ControlName_JP).Value);
                                if (radio == 1)
                                {
                                    ((RadioButton)cList[n]).Checked = true;
                                }
                                else
                                {
                                    ((RadioButton)cList[n]).Checked = false;
                                }
                                break;

                            case cls_common.CON_CHECK:
                                var chk = int.Parse(saveControls.Find(x => x.ControlName_JP == listControl[i].ControlName_JP).Value);
                                if (chk == 1)
                                {
                                    ((CheckBox)cList[n]).Checked = true;
                                }
                                else
                                {
                                    ((CheckBox)cList[n]).Checked = false;
                                }
                                break;

                        }
                    }
                }
                // 処理結果を返す
                return true;
            }
            catch (Exception)
            {
                // 処理結果を返す
                return false;
            }

        }

        /// <summary>
        /// 解析結果フォルダ選択時
        /// </summary>
        /// <param name="entire">全体表示ボタン押下</param>
        private void mapData_selection(bool entire = false)
        {
            if (!string.IsNullOrEmpty(txt_Param_1.Text))
            {
                // 解析結果データより得られる予定の座標を指定（指定フォルダを引数にセット）
                if (setDefault_Coordinates(txt_Param_1.Text)) {
                    object[] array = new object[4];
                    array[0] = dbl_topMax;          // 最大緯度（top）
                    array[1] = dbl_bottomMax;       // 最小経度（bottom）
                    array[2] = dbl_leftMax;         // 最小経度（left）
                    array[3] = dbl_rightMax;        // 最大経度（right）

                    if (checkWeb())
                    {
                        rdo_Param_3.Enabled = true;
                        label26.Enabled = true;
                        web_Map_Area.Document.InvokeScript("MapDisp", array);
                    }
                    else
                    {
                        rdo_Param_3.Enabled = false;
                    }

                    // 全体表示の場合は以下の処理は実施しない
                    if (entire) return;

                    // 初期状態：全範囲選択時は最大値をセットする
                    if (rdo_Param_2.Checked)
                    {
                        txt_Max_Lat.Text = dbl_topMax.ToString();
                        txt_Min_Lat.Text = dbl_bottomMax.ToString();
                        txt_Max_Lon.Text = dbl_rightMax.ToString();
                        txt_Min_Lon.Text = dbl_leftMax.ToString();
                    }
                    else
                    {
                        btn_Reflect_Coordinate.PerformClick();

                    }
                    // 入力エラー削除
                    errorProvider.SetError(txt_Max_Lat, string.Empty);
                    errorProvider.SetError(txt_Min_Lat, string.Empty);
                    errorProvider.SetError(txt_Max_Lon, string.Empty);
                    errorProvider.SetError(txt_Min_Lon, string.Empty);

                }
                else
                {
                    // 地図を初期化
                    //web_Map_Area.Navigate("about:blank");
                    var cls_hml = new cls_HTML();
                    web_Map_Area.DocumentText = cls_hml.GetHTMLText();
                    // 全範囲で集計を選択させる
                    rdo_Param_2.Checked = true;
                    rdo_Param_3.Checked = false;
                    rdo_Param_3.Enabled = false;
                    label26.Enabled = false;

                    // 初期状態：全範囲選択時は最大値をセットする
                    txt_Max_Lat.Text = "";
                    txt_Min_Lat.Text = "";
                    txt_Max_Lon.Text = "";
                    txt_Min_Lon.Text = "";
                    // 入力エラー削除
                    errorProvider.SetError(txt_Max_Lat, string.Empty);
                    errorProvider.SetError(txt_Min_Lat, string.Empty);
                    errorProvider.SetError(txt_Max_Lon, string.Empty);
                    errorProvider.SetError(txt_Min_Lon, string.Empty);
                    // 保存されている座標データを作雄
                    dbl_topMax = 0;
                    dbl_bottomMax = 0;
                    dbl_rightMax = 0;
                    dbl_leftMax = 0;
                }

            }
            else
            {
                // フォルダ指定がない場合は範囲指定は無効
                rdo_Param_3.Enabled = false;
                label26.Enabled = false;
            }
        }

        /// <summary>
        /// 指定の解析結果フォルダから最大緯度経度の座標値を取得
        /// </summary>
        /// <param name="dir">解析結果データフォルダ</param>
        public bool setDefault_Coordinates(string dir)
        {
            // 対象地域の座標とズーム最大値を取得しておく（ファイル読み込み）
            var fileName = $"{dir}{cls_common.FILE_RANGE_COORDINATES}";

            if (string.IsNullOrEmpty(dir))
            {
                // 指定なしの場合は後続処理しない
                return false;
            }

            //読み込むテキストを保存する変数
            var maxs = new List<string>();

            try
            {
                // 最大最小緯度経度、ズームファイルをオープンする
                using (StreamReader sr = new StreamReader(fileName, Encoding.GetEncoding("Shift_JIS")))
                {
                    while (0 <= sr.Peek())
                    {
                        maxs.Add(sr.ReadLine());
                    }
                }

                // 取得した保存内容をコントロールにセット
                for (int i = 0; i < maxs.Count; i++)
                {
                    if (maxs[i].Contains("top"))
                    {
                        //「:」で分割して値セット
                        var items = maxs[i].Split(':');
                        dbl_topMax = double.Parse(items[1]);

                    }
                    else if (maxs[i].Contains("bottom"))
                    {
                        //「:」で分割して値セット
                        var items = maxs[i].Split(':');
                        dbl_bottomMax = double.Parse(items[1]);

                    }
                    else if (maxs[i].Contains("left"))
                    {
                        //「:」で分割して値セット
                        var items = maxs[i].Split(':');
                        dbl_leftMax = double.Parse(items[1]);
                    }
                    else if (maxs[i].Contains("right"))
                    {
                        //「:」で分割して値セット
                        var items = maxs[i].Split(':');
                        dbl_rightMax = double.Parse(items[1]);
                    }
                }
                return true;

            }
            catch (Exception ex)
            {
                cls_common.showMessage(ex.Message, cls_common.MessageType.error);
                return false;

            }

        }

        // ファイル選択
        // ・「保存した入力内容の読込」              tag:30 ファイル
        // ・「制限を設ける範囲のシェープファイル」  tag:40 フォルダ
        // ・「制限を設ける範囲のシェープファイル」  tag:41 フォルダ
        // ・「制限を設ける範囲のシェープファイル」  tag:42 フォルダ
        // ・「気象データ(積雪)シェープファイル」    tag:50 フォルダ
        private void btn_File_Dialog_Click(object sender, EventArgs e)
        {
            // 押下されたボタンにより分岐
            string description = "";
            string ctlname = "";
            string filter = "";
            string filePath = "";

            switch (((Button)sender).Tag)
            {
                case "30":    // 保存した入力内容の読込
                    description = cls_message.general.MSG_SELECT_INPUT_FILE;
                    ctlname = "";
                    break;
                case "40":    // 制限を設ける範囲のシェープファイル
                    description = cls_message.frm_Aggregate.MSG_004;
                    ctlname = "txt_Param_3_1";
                    filter = "シェープファイル(*.shp)|*.shp";
                    filePath = txt_Param_3_1.Text;
                    break;
                case "41":    // 制限を設ける範囲のシェープファイル
                    description = cls_message.frm_Aggregate.MSG_004;
                    ctlname = "txt_Param_3_2";
                    filter = "シェープファイル(*.shp)|*.shp";
                    filePath = txt_Param_3_2.Text;
                    break;
                case "42":    // 制限を設ける範囲のシェープファイル
                    description = cls_message.frm_Aggregate.MSG_004;
                    ctlname = "txt_Param_3_3";
                    filter = "シェープファイル(*.shp)|*.shp";
                    filePath = txt_Param_3_3.Text;
                    break;
                case "50":    // 気象データ(積雪)シェープファイル
                    description = cls_message.frm_Aggregate.MSG_003;
                    ctlname = "txt_Param_2_4";
                    filter = "シェープファイル(*.shp)|*.shp";
                    filePath = txt_Param_2_4.Text;
                    break;

            }

            string directory;
            string name;
            // 指定パス存在チェック
            if (System.IO.File.Exists(filePath))
            {
                // 存在する場合はフォルダ設定
                directory = Path.GetDirectoryName(filePath);
                name = Path.GetFileName(filePath);
            }
            else
            {
                // 存在しない場合はデフォルトを設定
                directory = @"C:\";
                name = "";
            }
            OpenFileDialog ofd = new OpenFileDialog();
            // 最初に選択するフォルダを指定する
            ofd.InitialDirectory = directory;
            // 最初に選択するファイルを指定する
            ofd.FileName = name;
            // [ファイルの種類]に表示される選択肢を指定
            //指定なしの場合はすべてのファイルを表示
            ofd.Filter = (String.IsNullOrEmpty(filter))? "すべてのファイル(*.*)|*.*": filter;
            // [ファイルの種類]で初期選択を指定
            ofd.FilterIndex = 1;
            // タイトルを設定
            ofd.Title = description;
            // ダイアログボックスを閉じる前に現在のディレクトリを復元するようにする
            ofd.RestoreDirectory = true;
            // ダイアログを表示する
            if (ofd.ShowDialog() == DialogResult.OK)
            {
                if (ctlname != "")
                {
                    // 保存した入力内容の読み込み以外
                    // 選択したパスをテキストボックスに表示
                    Control[] controls = this.Controls.Find(ctlname, true);
                    if (controls.Length > 0)
                    {
                        ((TextBox)controls[0]).Text = ofd.FileName;
                    }
                }
                else
                {
                    // 保存した入力内容の読み込み時の処理
                    if (set_controlValue(ofd.FileName) == false)
                    {
                        // 失敗メッセージ
                        cls_common.showMessage(cls_message.general.ERRMSG_INPUT_LOADING, cls_common.MessageType.error);
                    }
                    else
                    {
                        // 解析結果データの更新
                        if (!string.IsNullOrEmpty(txt_Param_1.Text)) mapData_selection();
                    }
                }
            }
        }

        /// <summary>
        /// 指定コントロールをキャプチャ
        /// </summary>
        /// <param name="ctrl">指定コントロール</param>
        /// <returns>Bitmapを返却</returns>
        public Bitmap CaptureControl(Control ctrl)
        {
            Bitmap img = new Bitmap(ctrl.Width, ctrl.Height);
            Graphics memg = Graphics.FromImage(img);
            IntPtr dc = memg.GetHdc();
            PrintWindow(ctrl.Handle, dc, 0);
            memg.ReleaseHdc(dc);
            memg.Dispose();
            return img;
        }

        /// <summary>
        /// 選択範囲の座標をKMLファイルに出力
        /// </summary>
        /// <param name="top">最大緯度</param>
        /// <param name="bottom">最小緯度</param>
        /// <param name="left">最小経度</param>
        /// <param name="right">最大経度</param>
        /// <param name="dir">出力先フォルダ</param>
        /// <param name="date">実施日時</param>
        private void WriteKMLFile(string top, string bottom, string left, string right,string dir){
            
            var fileName = $"{"aggregate_range"}{cls_common.FILE_KML}";               // KMLファイル原紙
            var fileNameNew = $"{dir}\\{cls_common.FILE_AGGREGATE_RANGE}{cls_common.FILE_KML}";     // 出力先

            // ファイルコピー
            File.Copy(fileName, fileNameNew, true);

            try
            {
                // 置換文字列の指定
                string bf_p1x = "p1x";
                string af_p1x = left.ToString();
                string bf_p1y = "p1y";
                string af_p1y = top.ToString();
                string bf_p2x = "p2x";
                string af_p2x = right.ToString();
                string bf_p2y = "p2y";
                string af_p2y = top.ToString();
                string bf_p3x = "p3x";
                string af_p3x = right.ToString();
                string bf_p3y = "p3y";
                string af_p3y = bottom.ToString();
                string bf_p4x = "p4x";
                string af_p4x = left.ToString();
                string bf_p4y = "p4y";
                string af_p4y = bottom.ToString();

                StreamReader sr = new StreamReader(fileNameNew);
                string s = sr.ReadToEnd();
                sr.Close();

                // 緯度、経度を選択範囲に置換する
                s = s.Replace(bf_p1x, af_p1x);
                s = s.Replace(bf_p1y, af_p1y);
                s = s.Replace(bf_p2x, af_p2x);
                s = s.Replace(bf_p2y, af_p2y);
                s = s.Replace(bf_p3x, af_p3x);
                s = s.Replace(bf_p3y, af_p3y);
                s = s.Replace(bf_p4x, af_p4x);
                s = s.Replace(bf_p4y, af_p4y);

                StreamWriter sw = new StreamWriter(
                    fileNameNew,
                    false
                );

                sw.Write(s);
                sw.Close();

            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        /// <summary>
        /// 出力用フォルダ作成
        /// </summary>
        /// <param name="dir">指定配下以下のディレクトリにフォルダを作成</param>
        private void createOutputDir(string resultDir)
        {
            // 出力用フォルダの作成
            Directory.CreateDirectory(resultDir);

            // outputフォルダ作成
            string outputDir = resultDir + "\\output";
            Directory.CreateDirectory(outputDir);

            // 出力フォルダ作成
            string dataDir = resultDir + "\\data";
            Directory.CreateDirectory(dataDir);

            // パラメータ出力フォルダ作成（parameter.log）
            string logDir = resultDir + "\\log";
            Directory.CreateDirectory(logDir);

        }

        /// <summary>
        /// 処理パラメータの設定
        /// </summary>
        /// <returns>必要なパラメータをセットした構造体</returns>
        private cls_aggregateParam setParam()
        {
            cls_aggregateParam param = new cls_aggregateParam();

            try
            {
                // 解析処理入力データ選択
                // 解析結果データフォルダパス
                param.txt_Param_1 = txt_Param_1.Text;

                // 集計範囲を選択時のみ、選択範囲の値をセット 
                if (rdo_Param_3.Checked)
                {
                    // 選択範囲
                    // 最大緯度
                    param.txt_Max_Lat = txt_Max_Lat.Text;

                    // 最小経度
                    param.txt_Min_Lon = txt_Min_Lon.Text;

                    // 最大経度
                    param.txt_Max_Lon = txt_Max_Lon.Text;

                    // 最小緯度
                    param.txt_Min_Lat = txt_Min_Lat.Text;
                }

                // 太陽光パネルの設置に関して優先度が低い施設の判定
                // 日射量が少ない施設を除外チェック時のみ、選択値をセット
                if (chk_Param_1_1.Checked)
                {
                    if (chk_Param_1_1_1.Checked)
                        // kWh/m2未満
                        param.txt_Param_1_1_1 = txt_Param_1_1_1.Text;

                    if (chk_Param_1_1_2.Checked)
                        // 下位％
                        param.txt_Param_1_1_2 = txt_Param_1_1_2.Text;
                }

                // 建物構造による除外チェック時のみ、選択値をセット
                if (chk_Param_1_2.Checked)
                {
                    // 木造・土蔵造
                    param.chk_Param_1_2_1 = chk_Param_1_2_1.Checked ? 1 : 0;

                    // 鉄骨鉄筋コンクリート造
                    param.chk_Param_1_2_2 = chk_Param_1_2_2.Checked ? 1 : 0;

                    // 鉄筋コンクリート造 ※
                    param.chk_Param_1_2_3 = chk_Param_1_2_3.Checked ? 1 : 0;

                    // 鉄骨造
                    param.chk_Param_1_2_4 = chk_Param_1_2_4.Checked ? 1 : 0;

                    // 軽量鉄骨造
                    param.chk_Param_1_2_5 = chk_Param_1_2_5.Checked ? 1 : 0;

                    // レンガ造・コンクリートブロック造・石造
                    param.chk_Param_1_2_6 = chk_Param_1_2_6.Checked ? 1 : 0;
                    // 不明
                    param.chk_Param_1_2_7 = chk_Param_1_2_7.Checked ? 1 : 0;

                    // 非木造
                    param.chk_Param_1_2_8 = chk_Param_1_2_8.Checked ? 1 : 0;
                }
                // 特定の階層の施設を除外チェック時のみ、選択値をセット
                if (chk_Param_1_3.Checked)
                {
                    // 〇階以下 未入力の場合、"-1"
                    param.txt_Param_1_3_1 = (String.IsNullOrEmpty(txt_Param_1_3_1.Text)) ? "-1" : txt_Param_1_3_1.Text;

                    // 〇階以上 未入力の場合、"9999"を設定
                    param.txt_Param_1_3_2 = (String.IsNullOrEmpty(txt_Param_1_3_2.Text)) ? "9999" : txt_Param_1_3_2.Text;

                }

                // 災害時に太陽光パネルが破損、消失する危険性の判定
                // 高さが想定される最大津波高さを下回る建物を除外
                param.chk_Param_2_1 = chk_Param_2_1.Checked ? 1 : 0;

                // 建物高さが想定される河川浸水想定浸水深を下回る建物を除外
                param.chk_Param_2_2 = chk_Param_2_2.Checked ? 1 : 0;

                // 土砂災害警戒区域内に存在する建物を除外
                param.chk_Param_2_3 = chk_Param_2_3.Checked ? 1 : 0;

                // 気象データ(積雪)チェック時のみ、選択値をセット
                if (chk_Param_2_4.Checked)
                {

                    // 気象データ(積雪)フォルダパス
                    param.txt_Param_2_4 = txt_Param_2_4.Text;

                    // 気象データ(積雪)_積雪が多い地域の建物を除外チェック時のみ、値を格納
                    if (rdo_Param_2_4_1.Checked)
                    {
                        // 気象データ(積雪)_cm以上
                        param.txt_Param_2_4_1 = txt_Param_2_4_1.Text;
                    }

                    // 気象データ(積雪)_積雪荷重(kgf/m3) = 年最深積雪量チェック時のみ、値を格納
                    if (rdo_Param_2_4_2.Checked)
                    {
                        // 積雪荷重(kgf/m3)
                        param.txt_Param_2_4_2 = txt_Param_2_4_2.Text;
                        // 年最深積雪量  ×　N/m3
                        param.txt_Param_2_4_3 = txt_Param_2_4_3.Text;
                    }
                }

                // 太陽光パネルの設置に制限がある施設の判定
                // チェック時のみ、値を格納
                if (chk_Param_3_1.Checked)
                {
                    // 制限を設ける範囲のシェープファイル_フォルダパス１
                    param.txt_Param_3_1 = txt_Param_3_1.Text;

                    // 制限を設ける範囲のシェープファイル_高さ１
                    param.txt_Param_3_1_1 = txt_Param_3_1_1.Text;

                    // 制限を設ける範囲のシェープファイル_方位１
                    param.cmb_Param_3_1_2 = cmb_Param_3_1_2.SelectedIndex;
                }
                // チェック時のみ、値を格納
                if (chk_Param_3_2.Checked)
                {
                    // 制限を設ける範囲のシェープファイル_フォルダパス２
                    param.txt_Param_3_2 = txt_Param_3_2.Text;

                    // 制限を設ける範囲のシェープファイル_高さ２
                    param.txt_Param_3_2_1 = txt_Param_3_2_1.Text;

                    // 制限を設ける範囲のシェープファイル_方位２
                    param.cmb_Param_3_2_2 = cmb_Param_3_2_2.SelectedIndex;
                }
                // チェック時のみ、値を格納
                if (chk_Param_3_3.Checked)
                {
                    // 制限を設ける範囲のシェープファイル_フォルダパス３
                    param.txt_Param_3_3 = txt_Param_3_3.Text;

                    // 制限を設ける範囲のシェープファイル_高さ３
                    param.txt_Param_3_3_1 = txt_Param_3_3_1.Text;

                    // 制限を設ける範囲のシェープファイル_方位３
                    param.cmb_Param_3_3_2 = cmb_Param_3_3_2.SelectedIndex;
                }



                return param;
            }
            catch (Exception)
            {
                // パラメータの設定失敗メッセージ
                cls_common.showMessage(cls_message.general.ERRMSG_SET_PARAMETER, cls_common.MessageType.error);
                return param;
            }

        }

        // 集計開始
        private void btn_Start_Aggregate_Click(object sender, EventArgs e)
        {
            if (!chk_InputText())
            {
                // テキスト入力エラー有無チェック
                cls_common.showMessage(cls_message.general.ERRMSG_INPUT_DATA, cls_common.MessageType.error);
                return;
            }
            if (!chk_Input())
            {
                // エラーの場合は後続処理は実施しない
                return;
            }

            // 処理開始時間格納用
            DateTime dt;
            // エラーチェックOKの場合は後続処理を実施
            dlg_StartAggregateConfirm dlgStartAggregateConfirm = new dlg_StartAggregateConfirm();
            dlgStartAggregateConfirm.ShowDialog();

            if (dlgStartAggregateConfirm.DialogResult == System.Windows.Forms.DialogResult.OK)
            {
                // システム日付
                dt = DateTime.Now;
                String now = dt.ToString($"{dt:yyyyMMddHHmm}");

                // 処理パラメータを構造体に設定
                cls_aggregateParam param = setParam();

                // 出力フォルダ名
                param.dir_Output_Result = txt_Output_Directory.Text + "\\" + cls_common.OUTPUT_DIR_AGGREGATE + "_" + now;

                // 出力フォルダ作成
                createOutputDir(param.dir_Output_Result);

                // ログファイルパス
                string pathLog = param.dir_Output_Result + "\\" + "log" + "\\" + cls_common.FILE_PARAMETER_LOG;

                // 開始ログ（集計処理）
                cls_common.outputLogMessage(pathLog, "集計処理", cls_common.LogType.start);
                cls_common.outputLogMessage(pathLog, "処理パラメータ");
                cls_common.outputLogMessage(pathLog, "------------------------");
                // 入力パラメータ出力処理
                outputParam(pathLog,true);   
                cls_common.outputLogMessage(pathLog, "------------------------");
                                
                // gmlファイル格納フォルダパス
                var bldgdir = $"{txt_Param_1.Text}\\output\\bldg";
                var outputdir = $"{param.dir_Output_Result}\\output";

                //-------------------------------------------------
                // 選択範囲の画像出力
                //-------------------------------------------------
                // キャプチャ前に不要なコントロールを削除する
                web_Map_Area.Document.InvokeScript("MapCaptureBefore");

                Bitmap img = CaptureControl(this.web_Map_Area);
                img.Save($"{outputdir}\\集計範囲.jpg");
                img.Dispose();

                // キャプチャ後に必要なコントロールを元に戻す
                web_Map_Area.Document.InvokeScript("MapAddControl");

                //-------------------------------------------------
                // KMLファイル出力
                //-------------------------------------------------
                WriteKMLFile(txt_Max_Lat.Text, txt_Min_Lat.Text, txt_Min_Lon.Text, txt_Max_Lon.Text, outputdir);

                // タスクオブジェがnullなら生成
                if (TaskCanceler == null) TaskCanceler = new CancellationTokenSource();

                Task tmpTask = Task.Run(() => FunctionExecute(pathLog, param.dir_Output_Result, bldgdir, outputdir, now, param)).ContinueWith(t => FunctionCompleteTask());

                // 処理中ダイアログ表示
                dlg_Aggregating dlgAggregating = new dlg_Aggregating();
                dlgAggregating.ShowDialog(this);

                // ダイアログ表示
                while (true)
                {
                    if (endFlg == 1)
                    {
                        // 処理終了ダイアログ表示
                        dlg_ResultAggregate dlgResultAggregate = new dlg_ResultAggregate();
                        dlgResultAggregate.label1.Text = "集計処理が終了しました。";
                        dlgResultAggregate.ShowDialog();
                        endFlg = 0;
                        break;
                    }
                    else if(endFlg == 2)
                    {
                        // キャンセルダイアログ表示
                        dlg_Cancel dlgCancel = new dlg_Cancel();
                        dlgCancel.label1.Text = "集計処理がキャンセルされました。";
                        dlgCancel.ShowDialog();
                        endFlg = 0;
                        break;
                    }
                }
            }
        }

        /// <summary>
        /// テキストボックスのバリデーション結果を処理実施前にチェック
        /// </summary>
        /// <param name="type">一部のみ実施する場合は指定</param>
        /// <returns></returns>
        private bool chk_InputText(string type = "")
        {
            bool ret = true;

            if (ret)
                ret = (errorProvider.GetError(txt_Max_Lat).Length > 0) ? false : true;
            if (ret)
                ret = (errorProvider.GetError(txt_Min_Lat).Length > 0) ? false : true;
            if (ret)
                ret = (errorProvider.GetError(txt_Max_Lon).Length > 0) ? false : true;
            if (ret)
                ret = (errorProvider.GetError(txt_Min_Lon).Length > 0) ? false : true;

            // 選択範囲のみチェックして
            if (type == "selectRange") {
                // 選択範囲のチェック指定がある場合は他の項目のチェックはせずに結果を返す
                return ret;
            }

            if (ret)
                ret = (errorProvider.GetError(txt_Param_1_1_1).Length > 0) ? false : true;
            if (ret)
                ret = (errorProvider.GetError(txt_Param_1_1_2).Length > 0) ? false : true;
            if (ret)
                ret = (errorProvider.GetError(txt_Param_1_3_1).Length > 0) ? false : true;
            if (ret)
                ret = (errorProvider.GetError(txt_Param_1_3_2).Length > 0) ? false : true;
            if (ret)
                ret = (errorProvider.GetError(txt_Param_2_4_1).Length > 0) ? false : true;
            if (ret)
                ret = (errorProvider.GetError(txt_Param_2_4_2).Length > 0) ? false : true;
            if (ret)
                ret = (errorProvider.GetError(txt_Param_2_4_3).Length > 0) ? false : true;
            if (ret)
                ret = (errorProvider.GetError(txt_Param_3_1_1).Length > 0) ? false : true;
            if (ret)
                ret = (errorProvider.GetError(txt_Param_3_2_1).Length > 0) ? false : true;
            if (ret)
                ret = (errorProvider.GetError(txt_Param_3_3_1).Length > 0) ? false : true;

            return ret;

        }

        /// <summary>
        /// 座標入力チェック
        /// </summary>
        /// <returns></returns>
        private bool chk_InputCoordinate() {

            List<string> errList = new List<string>();

            if (String.IsNullOrEmpty(txt_Max_Lat.Text))
            {
                errList.Add($"{cls_common.get_controlNameJP("txt_Max_Lat", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
            }
            if (String.IsNullOrEmpty(txt_Max_Lon.Text))
            {
                errList.Add($"{cls_common.get_controlNameJP("txt_Max_Lon", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
            }
            if (String.IsNullOrEmpty(txt_Min_Lat.Text))
            {
                errList.Add($"{cls_common.get_controlNameJP("txt_Min_Lat", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
            }
            if (String.IsNullOrEmpty(txt_Min_Lon.Text))
            {
                errList.Add($"{cls_common.get_controlNameJP("txt_Min_Lon", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
            }

            // エラーがある場合はまとめてエラーメッセージ出力
            if (errList.Count > 0)
            {
                MessageBox.Show(string.Join("\n", errList), "入力データ、またはパラメータに不備があります。", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return false;
            }
            else
            {
                return true;
            }
        }

        /// <summary>
        /// 処理実施前の入力チェック
        /// </summary>
        /// <returns></returns>
        private bool chk_Input()
        {
            List<string> errList = new List<string>();
            var chkCount = 0;       // チェック数カウント変数

            // フォルダ存在チェック＋指定拡張子ファイル存在チェック
            // 解析結果フォルダチェック
            var extension = cls_common.FILE_GML;
            var bldgdir = $"{txt_Param_1.Text}/output/bldg";
            if (String.IsNullOrEmpty(txt_Param_1.Text))
            {
                // 必須入力エラー
                errList.Add($"{cls_common.get_controlNameJP("txt_Param_1", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
            }
            else if (!cls_common.IsDirectoryExists(bldgdir))
            {
                // 解析結果フォルダ存在チェックエラー
                errList.Add($"{cls_common.get_controlNameJP("txt_Param_1", listControl)}に{cls_message.general.ERRMSG_FOLDER_EXIST}");
            }
            else if (!cls_common.IsExtensionExists_Directory(bldgdir, extension))
            {
                // 指定フォルダ、指定拡張子ファイル存在チェックエラー
                errList.Add($"{cls_common.get_controlNameJP("txt_Param_1", listControl)}の指定フォルダに{extension}{cls_message.general.ERRMSG_EXTENSION_EXIST}");
            }

            // 選択範囲入力チェック
            if (rdo_Param_3.Checked)
            {
                if (String.IsNullOrEmpty(txt_Max_Lat.Text))
                {
                    errList.Add($"{cls_common.get_controlNameJP("txt_Max_Lat", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
                }
                if (String.IsNullOrEmpty(txt_Max_Lon.Text))
                {
                    errList.Add($"{cls_common.get_controlNameJP("txt_Max_Lon", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
                }
                if (String.IsNullOrEmpty(txt_Min_Lat.Text))
                {
                    errList.Add($"{cls_common.get_controlNameJP("txt_Min_Lat", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
                }
                if (String.IsNullOrEmpty(txt_Min_Lon.Text))
                {
                    errList.Add($"{cls_common.get_controlNameJP("txt_Min_Lon", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
                }
            }

            // 解析結果格納フォルダ存在チェック
            if (String.IsNullOrEmpty(txt_Output_Directory.Text))
            {
                // 必須入力エラー
                errList.Add($"{cls_common.get_controlNameJP("txt_Output_Directory", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
            }
            else if (!cls_common.IsDirectoryExists(txt_Output_Directory.Text))
            {
                // フォルダ存在チェックエラー
                errList.Add($"{cls_common.get_controlNameJP("txt_Output_Directory", listControl)}に{cls_message.general.ERRMSG_FOLDER_EXIST}");
            }

            // 日射量が少ない施設を除外
            // 太陽光パネルの設置に関して優先度が低い施設の判定
            // 日射量が少ない施設を除外チェック時のみ、選択値をセット
            if (chk_Param_1_1.Checked)
            {
                // チェックボックスチェック数カウント
                chkCount = 0;
                chkCount = chk_Param_1_1_1.Checked ? chkCount + 1 : chkCount;
                chkCount = chk_Param_1_1_2.Checked ? chkCount + 1 : chkCount;
                if (chkCount < 1)
                {
                    // 必須入力エラー
                    errList.Add($"{cls_common.get_controlNameJP("chk_Param_1_1", listControl)}{cls_message.general.ERRMSG_CHECK_REQUIRED}");
                }
                if (chk_Param_1_1_1.Checked)
                {
                    if (String.IsNullOrEmpty(txt_Param_1_1_1.Text))
                    {
                        errList.Add($"{cls_common.get_controlNameJP("txt_Param_1_1_1", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
                    }
                }
                if (chk_Param_1_1_2.Checked)
                {
                    if (String.IsNullOrEmpty(txt_Param_1_1_2.Text))
                    {
                        errList.Add($"{cls_common.get_controlNameJP("txt_Param_1_1_2", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
                    }
                }
            }
            // 建物構造による除外
            if (chk_Param_1_2.Checked)
            {
                // チェックボックスチェック数カウント
                chkCount = 0;
                chkCount = chk_Param_1_2_1.Checked ? chkCount + 1 : chkCount;
                chkCount = chk_Param_1_2_2.Checked ? chkCount + 1 : chkCount;
                chkCount = chk_Param_1_2_3.Checked ? chkCount + 1 : chkCount;
                chkCount = chk_Param_1_2_4.Checked ? chkCount + 1 : chkCount;
                chkCount = chk_Param_1_2_5.Checked ? chkCount + 1 : chkCount;
                chkCount = chk_Param_1_2_6.Checked ? chkCount + 1 : chkCount;
                chkCount = chk_Param_1_2_7.Checked ? chkCount + 1 : chkCount;
                chkCount = chk_Param_1_2_8.Checked ? chkCount + 1 : chkCount;
                if (chkCount < 1)
                {
                    // 必須入力エラー
                    errList.Add($"{cls_common.get_controlNameJP("chk_Param_1_2", listControl)}{cls_message.general.ERRMSG_CHECK_REQUIRED}");
                }
            }

            // 特定の階層の施設を除外
            if (chk_Param_1_3.Checked)
            {
                if (String.IsNullOrEmpty(txt_Param_1_3_1.Text) && String.IsNullOrEmpty(txt_Param_1_3_2.Text))
                {
                    errList.Add($"{cls_common.get_controlNameJP("chk_Param_1_3", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
                }
            }

            // 気象データ（積雪）チェック
            if (chk_Param_2_4.Checked)
            {
                if (String.IsNullOrEmpty(txt_Param_2_4.Text))
                {
                    // 必須入力エラー
                    errList.Add($"{cls_common.get_controlNameJP("txt_Param_2_4", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
                }
                else
                {
                    // ファイル拡張子チェック
                    extension = cls_common.FILE_SHP;
                    if (!cls_common.IsExtensionExists_File(txt_Param_2_4.Text, extension))
                    {
                        // ファイル存在チェックエラー(拡張子指定)
                        errList.Add($"{cls_common.get_controlNameJP("txt_Param_2_4", listControl)}に{extension}{cls_message.general.ERRMSG_EXTENSION_EXIST}");
                    }
                    else
                    {
                        // 「.shp」と同じ階層に必要ファイルが存在するかチェック
                        string dirName = Path.GetDirectoryName(txt_Param_2_4.Text);
                        string fileName = Path.GetFileNameWithoutExtension(txt_Param_2_4.Text);

                        //「.shx」ファイルの存在チェック
                        // ファイル拡張子チェック
                        extension = cls_common.FILE_SHX;
                        if (!cls_common.IsFileExists(Path.Combine(dirName, fileName + extension)))
                        {

                            // 指定フォルダ、指定拡張子ファイル存在チェックエラー
                            errList.Add($"{cls_common.get_controlNameJP("txt_Param_2_4", listControl)}と同一フォルダに{extension}{cls_message.general.ERRMSG_EXTENSION_EXIST}");

                        }

                        //「.dbf」ファイルの存在チェック
                        // ファイル拡張子チェック
                        extension = cls_common.FILE_DBF;
                        if (!cls_common.IsFileExists(Path.Combine(dirName, fileName + extension)))
                        {

                            // 指定フォルダ、指定拡張子ファイル存在チェックエラー
                            errList.Add($"{cls_common.get_controlNameJP("txt_Param_2_4", listControl)}と同一フォルダに{extension}{cls_message.general.ERRMSG_EXTENSION_EXIST}");

                        }

                    }

                }

                // 積雪が多い地域の建物を除外
                if (rdo_Param_2_4_1.Checked && String.IsNullOrEmpty(txt_Param_2_4_1.Text))
                {
                    // 必須入力エラー
                    errList.Add($"{cls_common.get_controlNameJP("rdo_Param_2_4_1", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
                }
                // 積雪荷重(kgf / m3) = 年最深積雪量  ×　N/m3
                if (rdo_Param_2_4_2.Checked)
                {
                    if (String.IsNullOrEmpty(txt_Param_2_4_2.Text) && String.IsNullOrEmpty(txt_Param_2_4_3.Text))
                        // 必須入力エラー
                        errList.Add($"{cls_common.get_controlNameJP("rdo_Param_2_4_2", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
                }
            }

            // 制限を設ける範囲のシェープファイル
            if (chk_Param_3_1.Checked)
            {
                if (String.IsNullOrEmpty(txt_Param_3_1.Text))
                {
                    // 必須入力エラー
                    errList.Add($"{cls_common.get_controlNameJP("chk_Param_3_1", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
                }
                else
                {
                    // ファイル拡張子チェック
                    extension = cls_common.FILE_SHP;
                    if (!cls_common.IsExtensionExists_File(txt_Param_3_1.Text, extension))
                    {
                        // ファイル存在チェックエラー(拡張子指定)
                        errList.Add($"{cls_common.get_controlNameJP("txt_Param_3_1", listControl)}に{extension}{cls_message.general.ERRMSG_EXTENSION_EXIST}");
                    }
                    else
                    {
                        // 「.shp」と同じ階層に必要ファイルが存在するかチェック
                        string dirName = Path.GetDirectoryName(txt_Param_3_1.Text);
                        string fileName = Path.GetFileNameWithoutExtension(txt_Param_3_1.Text);

                        //「.shx」ファイルの存在チェック
                        // ファイル拡張子チェック
                        extension = cls_common.FILE_SHX;
                        if (!cls_common.IsFileExists(Path.Combine(dirName, fileName + extension)))
                        {
                         
                            // 指定フォルダ、指定拡張子ファイル存在チェックエラー
                            errList.Add($"{cls_common.get_controlNameJP("txt_Param_3_1", listControl)}と同一フォルダに{extension}{cls_message.general.ERRMSG_EXTENSION_EXIST}");

                        }

                        //「.dbf」ファイルの存在チェック
                        // ファイル拡張子チェック
                        extension = cls_common.FILE_DBF;
                        if (!cls_common.IsFileExists(Path.Combine(dirName, fileName + extension)))
                        {

                            // 指定フォルダ、指定拡張子ファイル存在チェックエラー
                            errList.Add($"{cls_common.get_controlNameJP("txt_Param_3_1", listControl)}と同一フォルダに{extension}{cls_message.general.ERRMSG_EXTENSION_EXIST}");

                        }

                    }

                }

                // 未選択チェック
                if (String.IsNullOrEmpty(txt_Param_3_1_1.Text))
                {
                    // 必須入力エラー
                    errList.Add($"{cls_common.get_controlNameJP("txt_Param_3_1_1", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
                }
            }
            if (chk_Param_3_2.Checked)
            {
                if (String.IsNullOrEmpty(txt_Param_3_2.Text))
                {
                    // 必須入力エラー
                    errList.Add($"{cls_common.get_controlNameJP("txt_Param_3_2", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
                }
                else
                {
                    // ファイル拡張子チェック
                    extension = cls_common.FILE_SHP;
                    if (!cls_common.IsExtensionExists_File(txt_Param_3_2.Text, extension))
                    {
                        // ファイル存在チェックエラー(拡張子指定)
                        errList.Add($"{cls_common.get_controlNameJP("txt_Param_3_2", listControl)}に{extension}{cls_message.general.ERRMSG_EXTENSION_EXIST}");
                    }
                    else
                    {
                        // 「.shp」と同じ階層に必要ファイルが存在するかチェック
                        string dirName = Path.GetDirectoryName(txt_Param_3_2.Text);

                        //「.shx」ファイルの存在チェック
                        // ファイル拡張子チェック
                        extension = cls_common.FILE_SHX;
                        if (!cls_common.IsExtensionExists_Directory(dirName, extension))
                        {

                            // 指定フォルダ、指定拡張子ファイル存在チェックエラー
                            errList.Add($"{cls_common.get_controlNameJP("txt_Param_3_2", listControl)}と同一フォルダに{extension}{cls_message.general.ERRMSG_EXTENSION_EXIST}");

                        }

                        //「.dbf」ファイルの存在チェック
                        // ファイル拡張子チェック
                        extension = cls_common.FILE_DBF;
                        if (!cls_common.IsExtensionExists_Directory(dirName, extension))
                        {

                            // 指定フォルダ、指定拡張子ファイル存在チェックエラー
                            errList.Add($"{cls_common.get_controlNameJP("txt_Param_3_2", listControl)}と同一フォルダに{extension}{cls_message.general.ERRMSG_EXTENSION_EXIST}");

                        }

                    }

                }
                // 未選択チェック
                if (String.IsNullOrEmpty(txt_Param_3_2_1.Text))
                {
                    // 必須入力エラー
                    errList.Add($"{cls_common.get_controlNameJP("txt_Param_3_2_1", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
                }

            }
            if (chk_Param_3_3.Checked)
            {
                if (String.IsNullOrEmpty(txt_Param_3_3.Text))
                {
                    // 必須入力エラー
                    errList.Add($"{cls_common.get_controlNameJP("txt_Param_3_3", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
                }
                else
                {
                    // ファイル拡張子チェック
                    extension = cls_common.FILE_SHP;
                    if (!cls_common.IsExtensionExists_File(txt_Param_3_3.Text, extension))
                    {
                        // ファイル存在チェックエラー(拡張子指定)
                        errList.Add($"{cls_common.get_controlNameJP("txt_Param_3_3", listControl)}に{extension}{cls_message.general.ERRMSG_EXTENSION_EXIST}");
                    }
                    else
                    {
                        // 「.shp」と同じ階層に必要ファイルが存在するかチェック
                        string dirName = Path.GetDirectoryName(txt_Param_3_3.Text);

                        //「.shx」ファイルの存在チェック
                        // ファイル拡張子チェック
                        extension = cls_common.FILE_SHX;
                        if (!cls_common.IsExtensionExists_Directory(dirName, extension))
                        {

                            // 指定フォルダ、指定拡張子ファイル存在チェックエラー
                            errList.Add($"{cls_common.get_controlNameJP("txt_Param_3_3", listControl)}と同一フォルダに{extension}{cls_message.general.ERRMSG_EXTENSION_EXIST}");

                        }

                        //「.dbf」ファイルの存在チェック
                        // ファイル拡張子チェック
                        extension = cls_common.FILE_DBF;
                        if (!cls_common.IsExtensionExists_Directory(dirName, extension))
                        {

                            // 指定フォルダ、指定拡張子ファイル存在チェックエラー
                            errList.Add($"{cls_common.get_controlNameJP("txt_Param_3_3", listControl)}と同一フォルダに{extension}{cls_message.general.ERRMSG_EXTENSION_EXIST}");

                        }

                    }

                }



                // 未選択チェック
                if (String.IsNullOrEmpty(txt_Param_3_3_1.Text))
                {
                    // 必須入力エラー
                    errList.Add($"{cls_common.get_controlNameJP("txt_Param_3_3_1", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
                }
            }

            // エラーがある場合はまとめてエラーメッセージ出力
            if (errList.Count > 0)
            {
                MessageBox.Show(string.Join("\n", errList), "入力データ、またはパラメータに不備があります。", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return false;
            }
            else
            {
                return true;
            }

        }

        // TOP画面
        private void btn_Top_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        // 集計範囲ラジオボタン変更
        private void rdo_Param_CheckedChanged(object sender, EventArgs e)
        {
            int PickExtent = Convert.ToInt32(((RadioButton)sender).Tag);
            // 座標表示設定
            switch (PickExtent)
            {
                case 0:    // 全範囲で集計
                    if (rdo_Param_2.Checked)
                    {
                        if (!String.IsNullOrEmpty(txt_Param_1.Text))
                        {
                            txt_Max_Lat.Text = dbl_topMax.ToString();
                            txt_Min_Lat.Text = dbl_bottomMax.ToString();
                            txt_Max_Lon.Text = dbl_rightMax.ToString();
                            txt_Min_Lon.Text = dbl_leftMax.ToString();
                            // 入力エラー削除
                            errorProvider.SetError(txt_Max_Lat, string.Empty);
                            errorProvider.SetError(txt_Min_Lat, string.Empty);
                            errorProvider.SetError(txt_Max_Lon, string.Empty);
                            errorProvider.SetError(txt_Min_Lon, string.Empty);
                        }

                        txt_Max_Lat.Enabled = false;
                        txt_Min_Lat.Enabled = false;
                        txt_Max_Lon.Enabled = false;
                        txt_Min_Lon.Enabled = false;
                        btn_Reflect_Coordinate.Enabled = false;
                    }
                    break;
                case 1:    // 選択範囲で集計
                    if (rdo_Param_3.Checked)
                    {
                        txt_Max_Lat.Enabled = true;
                        txt_Min_Lat.Enabled = true;
                        txt_Max_Lon.Enabled = true;
                        txt_Min_Lon.Enabled = true;
                        btn_Reflect_Coordinate.Enabled = true;
                    }
                    break;
            }
            object[] array = new object[1];
            array[0] = PickExtent;
            web_Map_Area.Document.InvokeScript("ChangeExtent", array);
        }

        // 全体表示ボタン
        private void btn_view_entire_Click(object sender, EventArgs e)
        {
            // 全体表示処理
            // …
            // …
            // …

            // 最大範囲をセット
            mapData_selection(true);

        }

        // 更新ボタン
        private void btn_Reflect_Coordinate_Click(object sender, EventArgs e)
        {

            // チェック処理
            // 入力エラー有無チェック
            if (!chk_InputText("selectRange"))
            {
                // エラーの場合はメッセージ表示＆後続処理は実施しない
                // 選択範囲入力エラー
                cls_common.showMessage(cls_message.general.ERRMSG_INPUT_DATA, cls_common.MessageType.error);
                return;

            }
            if (!chk_InputCoordinate())
            {
                // エラーの場合は後続処理は実施しない
                return;
            }


            // 入力範囲チェック（4点の座標位置を全てチェックしてOKな場合は描画）
            double dbl_Max_Lat = double.Parse(txt_Max_Lat.Text); // top
            double dbl_Min_Lat = double.Parse(txt_Min_Lat.Text); // bottom
            double dbl_Min_Lon = double.Parse(txt_Min_Lon.Text); // left
            double dbl_Max_Lon = double.Parse(txt_Max_Lon.Text); // right

            // 選択範囲判定（4点の座標位置を全てチェック）
            if (dbl_topMax < dbl_Max_Lat ||
                dbl_bottomMax > dbl_Min_Lat ||
                dbl_leftMax > dbl_Min_Lon || 
                dbl_rightMax < dbl_Max_Lon) {

                // NGの場合：選択範囲外エラー
                cls_common.showMessage(cls_message.general.ERRMSG_SELECTED_RANGE, cls_common.MessageType.error);
            }
            else
            {
                object[] array = new object[4];
                array[0] = txt_Max_Lat.Text;    // 最大緯度（top）
                array[1] = txt_Min_Lat.Text;    // 最小経度（bottom）
                array[2] = txt_Min_Lon.Text;    // 最小経度（left）
                array[3] = txt_Max_Lon.Text;    // 最大経度（right）

                // OKの場合：入力された座標値で描画を更新
                web_Map_Area.Document.InvokeScript("feature_update", array);
            }
            
        }


        public void MoveCenter(double lat, double lon, int zoom) {
            object[] array = new object[3];
            array[0] = lat;
            array[1] = lon;
            array[2] = zoom;
            web_Map_Area.Document.InvokeScript("MoveCenter", array);
        }
        public void PickStart(double top, double bottom, double left, double right)
        {
            txt_Max_Lat.Text = top.ToString();
            txt_Min_Lat.Text = bottom.ToString();
            txt_Min_Lon.Text = left.ToString();
            txt_Max_Lon.Text = right.ToString();
        }

        // (仮メソッド)座標取得処理作成用
        private void button1_Click(object sender, EventArgs e)
        {
            // 選択範囲の座標取得処理作成のための仮メソッド
            // 【ToDo】実際は矩形選択時にイベントを発生させる必要がある
            int mode = -1;
            if (rdo_Param_2.Checked == true)
            {
                mode = 0;
            }
            else if (rdo_Param_3.Checked == true)
            {
                mode = 1;
            }
            object[] array = new object[1];
            array[0] = mode;
            web_Map_Area.Document.InvokeScript("GetExtent", array);

        }

        //チェックボックス切り替えイベント(子項目の有効／無効の切り替え）
        private void chk_Param_CheckedChanged(object sender, EventArgs e)
        {
            // 日射量が少ない施設を除外
            if (chk_Param_1_1.Checked) { 
                chk_Param_1_1_1.Enabled = true;
                chk_Param_1_1_2.Enabled = true;
                label14.Enabled = true;
                label17.Enabled = true;
                label19.Enabled = true;
                // チェック有の場合は入力許可
                if (chk_Param_1_1_1.Checked)
                {
                    txt_Param_1_1_1.Enabled = true;
                }
                else
                {
                    txt_Param_1_1_1.Enabled = false;
                    errorProvider.SetError(txt_Param_1_1_1, string.Empty);
                }
                // チェック有の場合は入力許可
                if (chk_Param_1_1_2.Checked)
                    txt_Param_1_1_2.Enabled = true;
                else
                {
                    txt_Param_1_1_2.Enabled = false;
                    errorProvider.SetError(txt_Param_1_1_2, string.Empty);
                }
            }
            else {
                // 親項目にチェック無の場合は入力不許可
                chk_Param_1_1_1.Enabled = false;
                chk_Param_1_1_2.Enabled = false;
                chk_Param_1_1_1.Checked = false;
                chk_Param_1_1_2.Checked = false;
                txt_Param_1_1_1.Enabled = false;
                txt_Param_1_1_2.Enabled = false;
                label14.Enabled = false;
                label17.Enabled = false;
                label19.Enabled = false;
                errorProvider.SetError(txt_Param_1_1_1, string.Empty);
                errorProvider.SetError(txt_Param_1_1_2, string.Empty);
            }

            // 建物構造による除外
            if (chk_Param_1_2.Checked)
                panel10.Enabled = true;
            else
                panel10.Enabled = false;


            // 特定の階層の施設を除外
            if (chk_Param_1_3.Checked){
                txt_Param_1_3_1.Enabled = true;
                txt_Param_1_3_2.Enabled = true;
                label15.Enabled = true;
                label18.Enabled = true;
            }
            else
            {
                txt_Param_1_3_1.Enabled = false;
                txt_Param_1_3_2.Enabled = false;
                label15.Enabled = false;
                label18.Enabled = false;
                errorProvider.SetError(txt_Param_1_3_1, string.Empty);
                errorProvider.SetError(txt_Param_1_3_2, string.Empty);
            }

            // 気象データ(積雪)
            if (chk_Param_2_4.Checked)
            {
                txt_Param_2_4.Enabled = true;
                btn_Select_Param_6.Enabled = true;
                rdo_Param_2_4_1.Enabled = true;
                rdo_Param_2_4_2.Enabled = true;
                panel5.Enabled = true;
                // 親項目にチェック無の場合は入力不許可
                if (rdo_Param_2_4_1.Checked) {
                    txt_Param_2_4_1.Enabled = true;
                    label23.Enabled = false;
                    label30.Enabled = false;
                    label31.Enabled = false;
                    label32.Enabled = false;
                }
                else { 
                    txt_Param_2_4_1.Enabled = false;
                    label23.Enabled = true;
                    label30.Enabled = true;
                    label31.Enabled = true;
                    label32.Enabled = true;
                    errorProvider.SetError(txt_Param_2_4_1, string.Empty);
                }
                // 親項目にチェック無の場合は入力不許可
                if (rdo_Param_2_4_2.Checked) {
                    txt_Param_2_4_2.Enabled = true;
                    txt_Param_2_4_3.Enabled = true;
                    label16.Enabled = false;
                }
                else {
                    txt_Param_2_4_2.Enabled = false;
                    txt_Param_2_4_3.Enabled = false;
                    label16.Enabled = true;
                    errorProvider.SetError(txt_Param_2_4_2, string.Empty);
                    errorProvider.SetError(txt_Param_2_4_3, string.Empty);
                }
            }
            else {
                txt_Param_2_4.Enabled = false;
                panel5.Enabled = false;
                btn_Select_Param_6.Enabled = false;
                rdo_Param_2_4_1.Enabled = false;
                rdo_Param_2_4_2.Enabled = false;
                txt_Param_2_4_1.Enabled = false;
                txt_Param_2_4_2.Enabled = false;
                txt_Param_2_4_3.Enabled = false;
                errorProvider.SetError(txt_Param_2_4_1, string.Empty);
                errorProvider.SetError(txt_Param_2_4_2, string.Empty);
                errorProvider.SetError(txt_Param_2_4_3, string.Empty);
            }


            // 太陽光パネルの設置に制限がある施設の判定
            if (chk_Param_3_1.Checked) {
                txt_Param_3_1.Enabled=true;
                btn_Select_Param_3_1.Enabled = true;
                txt_Param_3_1_1.Enabled =true;
                cmb_Param_3_1_2.Enabled = true;
                label27.Enabled = true;
            }
            else {
                txt_Param_3_1.Enabled = false;
                btn_Select_Param_3_1.Enabled = false;
                txt_Param_3_1_1.Enabled = false;
                cmb_Param_3_1_2.Enabled = false;
                cmb_Param_3_1_2.SelectedIndex = -1;
                label27.Enabled = false;
                errorProvider.SetError(txt_Param_3_1_1, string.Empty);
            }
            if (chk_Param_3_2.Checked)
            {
                txt_Param_3_2.Enabled = true;
                btn_Select_Param_3_2.Enabled = true;
                txt_Param_3_2_1.Enabled = true;
                cmb_Param_3_2_2.Enabled = true;
                label28.Enabled = true;
            }
            else
            {
                txt_Param_3_2.Enabled = false;
                btn_Select_Param_3_2.Enabled = false;
                txt_Param_3_2_1.Enabled = false;
                cmb_Param_3_2_2.Enabled = false;
                cmb_Param_3_2_2.SelectedIndex = -1;
                label28.Enabled = false;
                errorProvider.SetError(txt_Param_3_2_1, string.Empty);

            }
            if (chk_Param_3_3.Checked)
            {
                txt_Param_3_3.Enabled = true;
                btn_Select_Param_3_3.Enabled = true;
                txt_Param_3_3_1.Enabled = true;
                cmb_Param_3_3_2.Enabled = true;
                label29.Enabled = true;
            }
            else
            {
                txt_Param_3_3.Enabled = false;
                btn_Select_Param_3_3.Enabled = false;
                txt_Param_3_3_1.Enabled = false;
                cmb_Param_3_3_2.Enabled = false;
                cmb_Param_3_3_2.SelectedIndex = -1;
                label29.Enabled = false;
                errorProvider.SetError(txt_Param_3_3_1, string.Empty);

            }
        }

        /// <summary>
        /// 解析結果データのLeaveイベント（直接入力時を考慮）
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void txt_Param_1_Leave(object sender, EventArgs e)
        {
            string activeName = this.ActiveControl.Name;

            // 「トップ画面」、「入力内容の保存」、「保存した内容の読込」を押下した場合は更新をしない
            if (activeName != "btn_Top" && activeName != "btn_Save_Config" && activeName != "btn_Load_Config" && activeName != "btn_Select_Param_1")
            {
                // 前回と同じ場合は更新をしない
                if (txt_Param_1_Path != txt_Param_1.Text)
                {
                    // 解析結果データの更新
                    if (!string.IsNullOrEmpty(txt_Param_1.Text)) mapData_selection();
                    // 前回値保存
                    txt_Param_1_Path = txt_Param_1.Text;
                }
            }
        }

        /// <summary>
        /// テキストボックス用のバリデーション
        /// </summary>
        /// <param name="sender">イベント発生元のコントロール</param>
        /// <param name="e"></param>
        private void textBox_Validated(object sender, System.EventArgs e)
        {
            try
            {
                string cName;
                if (sender.GetType().Equals(typeof(TextBox)))
                {
                    TextBox chk = (TextBox)sender;
                    cName = chk.Name;
                }
                else if(sender.GetType().Equals(typeof(CheckBox)))
                {
                    CheckBox chk = (CheckBox)sender;
                    cName = chk.Name;
                }
                else
                {
                    RadioButton chk = (RadioButton)sender;
                    cName = chk.Name;
                }

                var error = string.Empty;

                switch (cName)
                {
                    // 範囲選択入力チェック
                    case "txt_Max_Lat":
                        // 符号なし数値（小数を含む）チェック
                        if (!String.IsNullOrEmpty(txt_Max_Lat.Text)) { 
                            error = (!cls_common.IsDecimal(txt_Max_Lat.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                            errorProvider.SetError(txt_Max_Lat, error);
                            errorProvider.SetIconAlignment(txt_Max_Lat, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        else
                        {
                            errorProvider.SetError(txt_Max_Lat, cls_message.general.ERRMSG_NUMBER);
                            errorProvider.SetIconAlignment(txt_Max_Lat, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }

                        break;
                    case "txt_Min_Lon":
                        // 符号なし数値（小数を含む）チェック
                        if (!String.IsNullOrEmpty(txt_Min_Lon.Text))
                        {
                            error = (!cls_common.IsDecimal(txt_Min_Lon.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                            errorProvider.SetError(txt_Min_Lon, error);
                            errorProvider.SetIconAlignment(txt_Min_Lon, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        else
                        {
                            errorProvider.SetError(txt_Min_Lon, cls_message.general.ERRMSG_NUMBER);
                            errorProvider.SetIconAlignment(txt_Min_Lon, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }

                        break;

                    case "txt_Min_Lat":
                        // 符号なし数値（小数を含む）チェック
                        if (!String.IsNullOrEmpty(txt_Min_Lat.Text))
                        {
                            error = (!cls_common.IsDecimal(txt_Min_Lat.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                            errorProvider.SetError(txt_Min_Lat, error);
                            errorProvider.SetIconAlignment(txt_Min_Lat, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        else
                        {
                            errorProvider.SetError(txt_Min_Lat, cls_message.general.ERRMSG_NUMBER);
                            errorProvider.SetIconAlignment(txt_Min_Lat, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        break;

                    case "txt_Max_Lon":
                        // 符号なし数値（小数を含む）チェック
                        if (!String.IsNullOrEmpty(txt_Max_Lon.Text))
                        {
                            error = (!cls_common.IsDecimal(txt_Max_Lon.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                            errorProvider.SetError(txt_Max_Lon, error);
                            errorProvider.SetIconAlignment(txt_Max_Lon, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        else
                        {
                            errorProvider.SetError(txt_Max_Lon, cls_message.general.ERRMSG_NUMBER);
                            errorProvider.SetIconAlignment(txt_Max_Lon, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        break;


                    case "txt_Param_1_1_1":
                        // 符号なし数値（整数値）チェック
                        if (!String.IsNullOrEmpty(txt_Param_1_1_1.Text))
                        {
                            error = (!cls_common.IsInteger(txt_Param_1_1_1.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                            errorProvider.SetError(txt_Param_1_1_1, error);
                            errorProvider.SetIconAlignment(txt_Param_1_1_1, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        else
                        {
                            errorProvider.SetError(txt_Param_1_1_1, string.Empty);
                        }
                        break;

                    case "txt_Param_1_1_2":
                        // 符号なし数値（整数値）チェック
                        if (!String.IsNullOrEmpty(txt_Param_1_1_2.Text))
                        {
                            error = (!cls_common.IsInteger(txt_Param_1_1_2.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                            errorProvider.SetError(txt_Param_1_1_2, error);
                            errorProvider.SetIconAlignment(txt_Param_1_1_2, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        else
                        {
                            errorProvider.SetError(txt_Param_1_1_2, string.Empty);
                        }
                        break;

                    case "txt_Param_1_3_1":
                        if (!String.IsNullOrEmpty(txt_Param_1_3_1.Text))
                        {
                            // 符号なし数値（整数値）チェック
                            error = (!cls_common.IsInteger(txt_Param_1_3_1.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                            errorProvider.SetError(txt_Param_1_3_1, error);
                            errorProvider.SetIconAlignment(txt_Param_1_3_1, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        else
                        {
                            errorProvider.SetError(txt_Param_1_3_1, string.Empty);
                        }
                        break;

                    case "txt_Param_1_3_2":
                        // 符号なし数値（整数値）チェック
                        if (!String.IsNullOrEmpty(txt_Param_1_3_2.Text))
                        {
                            error = (!cls_common.IsInteger(txt_Param_1_3_2.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                            errorProvider.SetError(txt_Param_1_3_2, error);
                            errorProvider.SetIconAlignment(txt_Param_1_3_2, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        else
                        {
                            errorProvider.SetError(txt_Param_1_3_2, string.Empty);
                        }
                        break;

                    case "txt_Param_2_4_1":
                        // 符号なし数値（整数値）チェック
                        if (!String.IsNullOrEmpty(txt_Param_2_4_1.Text))
                        {
                            error = (!cls_common.IsInteger(txt_Param_2_4_1.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                            errorProvider.SetError(txt_Param_2_4_1, error);
                            errorProvider.SetIconAlignment(txt_Param_2_4_1, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        else
                        {
                            errorProvider.SetError(txt_Param_2_4_1, string.Empty);
                        }
                        break;

                    case "txt_Param_2_4_2":
                        // 符号なし数値（整数値）チェック
                        if (!String.IsNullOrEmpty(txt_Param_2_4_2.Text))
                        {
                            error = (!cls_common.IsInteger(txt_Param_2_4_2.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                            errorProvider.SetError(txt_Param_2_4_2, error);
                            errorProvider.SetIconAlignment(txt_Param_2_4_2, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        else
                        {
                            errorProvider.SetError(txt_Param_2_4_2, string.Empty);
                        }
                        break;

                    case "txt_Param_2_4_3":
                        // 符号なし数値（整数値）チェック
                        if (!String.IsNullOrEmpty(txt_Param_2_4_3.Text))
                        {
                            error = (!cls_common.IsInteger(txt_Param_2_4_3.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                            errorProvider.SetError(txt_Param_2_4_3, error);
                            errorProvider.SetIconAlignment(txt_Param_2_4_3, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        else
                        {
                            errorProvider.SetError(txt_Param_2_4_3, string.Empty);
                        }
                        break;

                    case "txt_Param_3_1_1":
                        // 符号なし数値（小数を含む）チェック
                        if (!String.IsNullOrEmpty(txt_Param_3_1_1.Text))
                        {
                            error = (!cls_common.IsDecimal(txt_Param_3_1_1.Text,1)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                            errorProvider.SetError(txt_Param_3_1_1, error);
                            errorProvider.SetIconAlignment(txt_Param_3_1_1, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        else
                        {
                            errorProvider.SetError(txt_Param_3_1_1, string.Empty);
                        }
                        break;

                    case "txt_Param_3_2_1":
                        // 符号なし数値（小数を含む）チェック
                        if (!String.IsNullOrEmpty(txt_Param_3_2_1.Text))
                        {
                            error = (!cls_common.IsDecimal(txt_Param_3_2_1.Text, 1)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                            errorProvider.SetError(txt_Param_3_2_1, error);
                            errorProvider.SetIconAlignment(txt_Param_3_2_1, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        else
                        {
                            errorProvider.SetError(txt_Param_3_2_1, string.Empty);
                        }
                        break;


                    case "txt_Param_3_3_1":
                        // 符号なし数値（小数を含む）チェック
                        if (!String.IsNullOrEmpty(txt_Param_3_3_1.Text))
                        {
                            error = (!cls_common.IsDecimal(txt_Param_3_3_1.Text, 1)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                            errorProvider.SetError(txt_Param_3_3_1, error);
                            errorProvider.SetIconAlignment(txt_Param_3_3_1, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        else
                        {
                            errorProvider.SetError(txt_Param_3_3_1, string.Empty);
                        }
                        break;

                    case "chk_Param_1_1":
                        // チェックがあれば確認
                        if (chk_Param_1_1.Checked)
                        {
                            if (chk_Param_1_1_1.Checked)
                            {
                                // 符号なし数値（整数値）チェック
                                if (!String.IsNullOrEmpty(txt_Param_1_1_1.Text))
                                {
                                    error = (!cls_common.IsInteger(txt_Param_1_1_1.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                                    errorProvider.SetError(txt_Param_1_1_1, error);
                                    errorProvider.SetIconAlignment(txt_Param_1_1_1, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                                }
                                else
                                {
                                    errorProvider.SetError(txt_Param_1_1_1, string.Empty);
                                }
                            }
                            if (chk_Param_1_1_2.Checked)
                            {
                                // 符号なし数値（整数値）チェック
                                if (!String.IsNullOrEmpty(txt_Param_1_1_2.Text))
                                {
                                    error = (!cls_common.IsInteger(txt_Param_1_1_2.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                                    errorProvider.SetError(txt_Param_1_1_2, error);
                                    errorProvider.SetIconAlignment(txt_Param_1_1_2, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                                }
                                else
                                {
                                    errorProvider.SetError(txt_Param_1_1_2, string.Empty);
                                }
                            }
                        }

                        break;

                    case "chk_Param_1_1_1":
                        // チェックがあれば確認
                        if (chk_Param_1_1_1.Checked)
                        {
                            // 符号なし数値（整数値）チェック
                            if (!String.IsNullOrEmpty(txt_Param_1_1_1.Text))
                            {
                                error = (!cls_common.IsInteger(txt_Param_1_1_1.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                                errorProvider.SetError(txt_Param_1_1_1, error);
                                errorProvider.SetIconAlignment(txt_Param_1_1_1, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                            }
                            else
                            {
                                errorProvider.SetError(txt_Param_1_1_1, string.Empty);
                            }
                        }
                        break;

                    case "chk_Param_1_1_2":
                        // チェックがあれば確認
                        if (chk_Param_1_1_2.Checked)
                        {
                            // 符号なし数値（整数値）チェック
                            if (!String.IsNullOrEmpty(txt_Param_1_1_2.Text))
                            {
                                error = (!cls_common.IsInteger(txt_Param_1_1_2.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                                errorProvider.SetError(txt_Param_1_1_2, error);
                                errorProvider.SetIconAlignment(txt_Param_1_1_2, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                            }
                            else
                            {
                                errorProvider.SetError(txt_Param_1_1_2, string.Empty);
                            }
                        }
                        break;

                    case "chk_Param_1_3":
                        // チェックがあれば確認
                        if (chk_Param_1_3.Checked)
                        {
                            if (!String.IsNullOrEmpty(txt_Param_1_3_1.Text))
                            {
                                // 符号なし数値（整数値）チェック
                                error = (!cls_common.IsInteger(txt_Param_1_3_1.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                                errorProvider.SetError(txt_Param_1_3_1, error);
                                errorProvider.SetIconAlignment(txt_Param_1_3_1, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                            }
                            else
                            {
                                errorProvider.SetError(txt_Param_1_3_1, string.Empty);
                            }
                            // 符号なし数値（整数値）チェック
                            if (!String.IsNullOrEmpty(txt_Param_1_3_2.Text))
                            {
                                error = (!cls_common.IsInteger(txt_Param_1_3_2.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                                errorProvider.SetError(txt_Param_1_3_2, error);
                                errorProvider.SetIconAlignment(txt_Param_1_3_2, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                            }
                            else
                            {
                                errorProvider.SetError(txt_Param_1_3_2, string.Empty);
                            }
                        }
                        break;

                    case "chk_Param_2_4":
                        // チェックがあれば確認
                        if (chk_Param_2_4.Checked)
                        {
                            if (rdo_Param_2_4_1.Checked)
                            {
                                // 符号なし数値（整数値）チェック
                                if (!String.IsNullOrEmpty(txt_Param_2_4_1.Text))
                                {
                                    error = (!cls_common.IsInteger(txt_Param_2_4_1.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                                    errorProvider.SetError(txt_Param_2_4_1, error);
                                    errorProvider.SetIconAlignment(txt_Param_2_4_1, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                                }
                                else
                                {
                                    errorProvider.SetError(txt_Param_2_4_1, string.Empty);
                                }
                            }
                            if (rdo_Param_2_4_2.Checked)
                            {
                                // 符号なし数値（整数値）チェック
                                if (!String.IsNullOrEmpty(txt_Param_2_4_2.Text))
                                {
                                    error = (!cls_common.IsInteger(txt_Param_2_4_2.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                                    errorProvider.SetError(txt_Param_2_4_2, error);
                                    errorProvider.SetIconAlignment(txt_Param_2_4_2, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                                }
                                else
                                {
                                    errorProvider.SetError(txt_Param_2_4_2, string.Empty);
                                }
                                // 符号なし数値（整数値）チェック
                                if (!String.IsNullOrEmpty(txt_Param_2_4_3.Text))
                                {
                                    error = (!cls_common.IsInteger(txt_Param_2_4_3.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                                    errorProvider.SetError(txt_Param_2_4_3, error);
                                    errorProvider.SetIconAlignment(txt_Param_2_4_3, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                                }
                                else
                                {
                                    errorProvider.SetError(txt_Param_2_4_3, string.Empty);
                                }
                            }
                        }
                        break;

                    case "rdo_Param_2_4_1":
                        // チェックがあれば確認
                        if (rdo_Param_2_4_1.Checked)
                        {
                            // 符号なし数値（整数値）チェック
                            if (!String.IsNullOrEmpty(txt_Param_2_4_1.Text))
                            {
                                error = (!cls_common.IsInteger(txt_Param_2_4_1.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                                errorProvider.SetError(txt_Param_2_4_1, error);
                                errorProvider.SetIconAlignment(txt_Param_2_4_1, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                            }
                            else
                            {
                                errorProvider.SetError(txt_Param_2_4_1, string.Empty);
                            }
                        }
                        break;

                    case "rdo_Param_2_4_2":
                        // チェックがあれば確認
                        if (rdo_Param_2_4_2.Checked)
                        {
                            // 符号なし数値（整数値）チェック
                            if (!String.IsNullOrEmpty(txt_Param_2_4_2.Text))
                            {
                                error = (!cls_common.IsInteger(txt_Param_2_4_2.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                                errorProvider.SetError(txt_Param_2_4_2, error);
                                errorProvider.SetIconAlignment(txt_Param_2_4_2, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                            }
                            else
                            {
                                errorProvider.SetError(txt_Param_2_4_2, string.Empty);
                            }
                            // 符号なし数値（整数値）チェック
                            if (!String.IsNullOrEmpty(txt_Param_2_4_3.Text))
                            {
                                error = (!cls_common.IsInteger(txt_Param_2_4_3.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                                errorProvider.SetError(txt_Param_2_4_3, error);
                                errorProvider.SetIconAlignment(txt_Param_2_4_3, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                            }
                            else
                            {
                                errorProvider.SetError(txt_Param_2_4_3, string.Empty);
                            }
                        }
                        break;

                    case "chk_Param_3_1":
                        // チェックがあれば確認
                        if (chk_Param_3_1.Checked)
                        {
                            // 符号なし数値（小数を含む）チェック
                            if (!String.IsNullOrEmpty(txt_Param_3_1_1.Text))
                            {
                                error = (!cls_common.IsDecimal(txt_Param_3_1_1.Text, 1)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                                errorProvider.SetError(txt_Param_3_1_1, error);
                                errorProvider.SetIconAlignment(txt_Param_3_1_1, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                            }
                            else
                            {
                                errorProvider.SetError(txt_Param_3_1_1, string.Empty);
                            }
                        }
                        break;

                    case "chk_Param_3_2":
                        // チェックがあれば確認
                        if (chk_Param_3_2.Checked)
                        {
                            // 符号なし数値（小数を含む）チェック
                            if (!String.IsNullOrEmpty(txt_Param_3_2_1.Text))
                            {
                                error = (!cls_common.IsDecimal(txt_Param_3_2_1.Text, 1)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                                errorProvider.SetError(txt_Param_3_2_1, error);
                                errorProvider.SetIconAlignment(txt_Param_3_2_1, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                            }
                            else
                            {
                                errorProvider.SetError(txt_Param_3_2_1, string.Empty);
                            }
                        }
                        break;

                    case "chk_Param_3_3":
                        // チェックがあれば確認
                        if (chk_Param_3_3.Checked)
                        {
                            // 符号なし数値（小数を含む）チェック
                            if (!String.IsNullOrEmpty(txt_Param_3_3_1.Text))
                            {
                                error = (!cls_common.IsDecimal(txt_Param_3_3_1.Text, 1)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                                errorProvider.SetError(txt_Param_3_3_1, error);
                                errorProvider.SetIconAlignment(txt_Param_3_3_1, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                            }
                            else
                            {
                                errorProvider.SetError(txt_Param_3_3_1, string.Empty);
                            }
                        }
                        break;

                    default:                         
                        break;  
                }

            }
            catch (Exception)
            {
                // エラーチェック失敗メッセージ
                cls_common.showMessage(cls_message.general.ERRMSG_ERROR_CHACK, cls_common.MessageType.error);
            }
        }

        /// <summary>
        /// 地図表示可能か確認
        /// </summary>
        /// <returns></returns>
        private static bool checkWeb()
        {
            bool result = true;
            //要求するURL
            string url = "http://cyberjapandata.gsi.go.jp/xyz/std/{z}/{x}/{y}.png";

            //WebRequestの作成
            System.Net.HttpWebRequest webreq =
                (System.Net.HttpWebRequest)System.Net.WebRequest.Create(url);

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
                    Console.WriteLine(ex.Message);
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
        /// リンクをキャンセルして既定のブラウザで表示
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void CancelNewWindow(object sender, System.ComponentModel.CancelEventArgs e)
        {
            // 新しいウィンドウを開かせない
            e.Cancel = true;

            System.Diagnostics.Process.Start("https://maps.gsi.go.jp/development/ichiran.html");

        }

        #region Task処理メソッド
        private void FunctionExecute(string logPath, string param, string readPath, string outPath, string now, cls_aggregateParam aggregateParam)
        {
            Console.WriteLine("処理開始");
            Task tmpTask = Task.Run(() => processFunction(logPath, param, readPath, outPath, now, aggregateParam));
            while (true)
            {
                if (TaskCanceler.Token.IsCancellationRequested)
                {
                    // キャンセルファイルパス
                    string pathCancel = param + "\\" + cls_common.FILE_CANCEL;
                    // ファイル作成
                    using (FileStream fs = File.Create(pathCancel)) { }
                    // キャンセルログ（集計処理）
                    cls_common.outputLogMessage(logPath, "集計処理がキャンセルされました", cls_common.LogType.end);
                    endFlg = 2;
                    break;
                }
                else if (tmpTask.Status == TaskStatus.RanToCompletion)
                {
                    for (int i = 0; i < Application.OpenForms.Count; i++)
                    {
                        Form f = Application.OpenForms[i];
                        if (f.Name == "dlg_Aggregating")
                        {
                            Invoke(new DelegateProcess(FormClose), f);
                            break;
                        }
                    }
                    // 処理終了ダイアログ表示
                    cls_common.outputLogMessage(logPath, "処理結果：完了");
                    endFlg = 1;
                    break;
                }
            }

            Console.WriteLine("処理終了");
        }
        private void processFunction(string path, string param, string readPath, string outPath, string now, cls_aggregateParam aggregateParam)
        {
            int ret = 0;
            // 座標系を取得
            SetJPZone();

            // C++呼出し
            cls_common.outputLogMessage(path, "AggregateBldgFiles", cls_common.LogType.start);
            ret =AggregateBldgFiles(readPath, param);
            cls_common.outputLogMessage(path, "AggregateBldgFiles", cls_common.LogType.end);
            if (ret == 1)
            {
                // ファイルなし
                for (int i = 0; i < Application.OpenForms.Count; i++)
                {
                    Form f = Application.OpenForms[i];
                    if (f.Name == "dlg_Aggregating")
                    {
                        Invoke(new DelegateProcess(FormClose), f);
                    }
                }
                MessageBox.Show("集計に失敗しました。集計範囲や入力データを確認してください。", "終了", MessageBoxButtons.OK, MessageBoxIcon.Information);
                cls_common.outputLogMessage(path, "処理結果：エラー");
                return;
            }
            if (ret == 2)
            {
                // キャンセル
                return;
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
                strParam_3_2 = "",
                dParam_3_2_1 = -1.0,
                iParam_3_2_2 = 0,
                strParam_3_3 = "",
                dParam_3_3_1 = -1.0,
                iParam_3_3_2 = 0
            };
            if (rdo_Param_3.Checked)
            {
                parameter.bAggregateRange = true;
                parameter.dMaxLat = double.Parse(aggregateParam.txt_Max_Lat);
                parameter.dMinLon = double.Parse(aggregateParam.txt_Min_Lon);
                parameter.dMaxLon = double.Parse(aggregateParam.txt_Max_Lon);
                parameter.dMinLat = double.Parse(aggregateParam.txt_Min_Lat);
            }
            if (chk_Param_1_1.Checked)
            {
                if (chk_Param_1_1_1.Checked)
                    parameter.dParam_1_1_1 = double.Parse(aggregateParam.txt_Param_1_1_1);
                if (chk_Param_1_1_2.Checked)
                    parameter.iParam_1_1_2 = int.Parse(aggregateParam.txt_Param_1_1_2);
            }
            if (chk_Param_1_2.Checked)
            {
                parameter.bParam_1_2_1 = aggregateParam.chk_Param_1_2_1 == 1 ? true : false;
                parameter.bParam_1_2_2 = aggregateParam.chk_Param_1_2_2 == 1 ? true : false;
                parameter.bParam_1_2_3 = aggregateParam.chk_Param_1_2_3 == 1 ? true : false;
                parameter.bParam_1_2_4 = aggregateParam.chk_Param_1_2_4 == 1 ? true : false;
                parameter.bParam_1_2_5 = aggregateParam.chk_Param_1_2_5 == 1 ? true : false;
                parameter.bParam_1_2_6 = aggregateParam.chk_Param_1_2_6 == 1 ? true : false;
                parameter.bParam_1_2_7 = aggregateParam.chk_Param_1_2_7 == 1 ? true : false;
                parameter.bParam_1_2_8 = aggregateParam.chk_Param_1_2_8 == 1 ? true : false;
            }
            if (chk_Param_1_3.Checked)
            {
                parameter.iParam_1_3_1 = int.Parse(aggregateParam.txt_Param_1_3_1);
                parameter.iParam_1_3_2 = int.Parse(aggregateParam.txt_Param_1_3_2);

            }
            parameter.bParam_2_1 = aggregateParam.chk_Param_2_1 == 1 ? true : false;
            parameter.bParam_2_2 = aggregateParam.chk_Param_2_2 == 1 ? true : false;
            parameter.bParam_2_3 = aggregateParam.chk_Param_2_3 == 1 ? true : false;
            if (chk_Param_2_4.Checked)
            {
                parameter.strParam_2_4 = aggregateParam.txt_Param_2_4;
                if (rdo_Param_2_4_1.Checked)
                {
                    parameter.dParam_2_4_1 = double.Parse(aggregateParam.txt_Param_2_4_1);
                }
                if (rdo_Param_2_4_2.Checked)
                {
                    parameter.dParam_2_4_2 = double.Parse(aggregateParam.txt_Param_2_4_2);
                    parameter.dParam_2_4_3 = double.Parse(aggregateParam.txt_Param_2_4_3);
                }
            }
            if (chk_Param_3_1.Checked)
            {
                parameter.strParam_3_1 = aggregateParam.txt_Param_3_1;
                parameter.dParam_3_1_1 = double.Parse(aggregateParam.txt_Param_3_1_1);
                parameter.iParam_3_1_2 = aggregateParam.cmb_Param_3_1_2;
            }
            if (chk_Param_3_2.Checked)
            {
                parameter.strParam_3_2 = aggregateParam.txt_Param_3_2;
                parameter.dParam_3_2_1 = double.Parse(aggregateParam.txt_Param_3_2_1);
                parameter.iParam_3_2_2 = aggregateParam.cmb_Param_3_2_2;
            }
            if (chk_Param_3_3.Checked)
            {
                parameter.strParam_3_3 = aggregateParam.txt_Param_3_3;
                parameter.dParam_3_3_1 = double.Parse(aggregateParam.txt_Param_3_3_1);
                parameter.iParam_3_3_2 = aggregateParam.cmb_Param_3_3_2;
            }

            // C++JudgeSuitablePlace.dll用パラメータをセット
            InitializeUIParam();

            IntPtr p = Marshal.AllocCoTaskMem(Marshal.SizeOf(parameter));
            Marshal.StructureToPtr(parameter, p, false);
            SetAggregateParam(p);
            Marshal.FreeCoTaskMem(p);

            // C++適地判定出力フォルダ設定
            SetOutputPath(param);

            // 解析結果フォルダパス設定
            SetBldgResultPath(readPath);

            // C++適地判定処理呼出し
            cls_common.outputLogMessage(path, "JadgeStart", cls_common.LogType.start);
            ret = JadgeStart();
            cls_common.outputLogMessage(path, "JadgeStart", cls_common.LogType.end);
            if (ret == 1)
            {
                // 失敗
                MessageBox.Show("適地判定を失敗しました。", "終了", MessageBoxButtons.OK, MessageBoxIcon.Information);
                cls_common.outputLogMessage(path, "処理結果：エラー");
                return;
            }
            if (ret == 2)
            {
                // キャンセル
                return;
            }

            // 集計処理
            cls_common.outputLogMessage(path, "AggregateAllData", cls_common.LogType.start);
            ret = AggregateAllData(txt_Param_1.Text, outPath);
            cls_common.outputLogMessage(path, "AggregateAllData", cls_common.LogType.end);
            if (ret == 1)
            {
                // ファイルなし
                for (int i = 0; i < Application.OpenForms.Count; i++)
                {
                    Form f = Application.OpenForms[i];
                    if (f.Name == "dlg_Aggregating")
                    {
                        Invoke(new DelegateProcess(FormClose), f);
                    }
                }
                MessageBox.Show("集計に失敗しました。集計範囲や入力データを確認してください。", "終了", MessageBoxButtons.OK, MessageBoxIcon.Information);
                cls_common.outputLogMessage(path, "処理結果：エラー");
                return;
            }
            if (ret == 2)
            {
                // キャンセル
                return;
            }

            // 終了ログ（集計処理）
            cls_common.outputLogMessage(path, "集計処理", cls_common.LogType.end);

        }

        //UIを変更する処理を関数にする
        private void FormClose(Form form)
        {
            form.Close();
            return;
        }

        private void FunctionCompleteTask()
        {
            Console.WriteLine("終了処理開始");

            //--- 終了処理 ---
            // キャンセルクラスを解放
            if (TaskCanceler != null)
            {
                TaskCanceler.Dispose();
                TaskCanceler = null;
            }

            Console.WriteLine("終了処理終了");
        }

        #endregion

    }
}
