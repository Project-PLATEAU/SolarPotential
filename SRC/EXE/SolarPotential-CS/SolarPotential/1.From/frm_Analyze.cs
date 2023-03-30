using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using SolarPotential._3.Class;
using System.Threading;

namespace SolarPotential
{
    public partial class frm_Analyze : Form
    {
        // 初期値格納ファイル名（Exe直下に配置予定）
        private string initFileName = "initFile_Analyze.txt";

        private int endFlg = 0;

        // コントロール配列のリスト
        private cls_common clsCommon = new cls_common(); //１行の形でインスタンスを生成
        private List<cls_common.controlObj> listControl = new List<cls_common.controlObj>();

        // 非同期処理をCancelするためのTokenを取得.
        private CancellationTokenSource TaskCanceler;

        //delegateを宣言
        delegate void DelegateProcess(Form form);

        private static frm_Analyze _parentInstance;
        //Form親オブジェクトを取得、設定するためのプロパティ
        public static frm_Analyze ParentInstance
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

        [DllImport("AnalyzeData.dll")]
        internal static extern int AnalizeBldgFiles(string str, string strOut);

        [DllImport("AnalyzeData.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern int AnalizeDemFiles(string str, string strOut);

        [DllImport("AnalyzeData.dll")]
        static extern int LOD2DataOut(string str, string strOut);

        [DllImport("AnalyzeData.dll")]
        static extern void SetJPZone();

        [DllImport("Analyzer.dll")]
        internal static extern bool AnalyzeStart(string str);

        [DllImport("Analyzer.dll")]
        private static extern void SetPossibleSunshineDataPath(string path);
        [DllImport("Analyzer.dll")]
        private static extern void SetAverageSunshineDataPath(string path);
        [DllImport("Analyzer.dll")]
        private static extern void SetMetpvDataPath(string path);
        [DllImport("Analyzer.dll")]
        private static extern void InitializeUIParam();
        [DllImport("Analyzer.dll")]
        private static extern void SetElecPotential(double d1, int s, double d2, double d3);
        [DllImport("Analyzer.dll")]
        private static extern void SetRoofSurfaceCorrect(double d1, double d2);
        [DllImport("Analyzer.dll")]
        private static extern void SetAreaSolarPower(double d);
        [DllImport("Analyzer.dll")]
        private static extern void SetReflectRoofCorrect_Lower(bool b1, bool b2, int e, double d);
        [DllImport("Analyzer.dll")]
        private static extern void SetReflectRoofCorrect_Upper(bool b1, bool b2, int e, double d);
        [DllImport("Analyzer.dll")]
        private static extern void SetEnableDEMData(bool b);
        [DllImport("Analyzer.dll")]
        private static extern void SetExecSolarPotantial(bool b);
        [DllImport("Analyzer.dll")]
        private static extern void SetExecReflection(bool b);


        public frm_Analyze()
        {
            InitializeComponent();
            //初期設定
            Initial();
            chk_Param_CheckedChanged(null, null);
            checkBox1_CheckedChanged(null, null);
        }

        #region 初期設定
        private void Initial()
        {

            // 配列の内容をコンボボックスアイテムに追加する
            string[] items = { "", "北向き", "東向き", "南向き", "西向き" };    // 解析画面は4方位
            cmb_Param_1_2.Items.AddRange(items);
            cmb_Param_4_3.Items.AddRange(items);
            cmb_Param_4_7.Items.AddRange(items);

            rdo_Param_4_1.CheckedChanged += new EventHandler(chk_Param_CheckedChanged);
            rdo_Param_4_2.CheckedChanged += new EventHandler(chk_Param_CheckedChanged);
            rdo_Param_4_5.CheckedChanged += new EventHandler(chk_Param_CheckedChanged);
            rdo_Param_4_6.CheckedChanged += new EventHandler(chk_Param_CheckedChanged);

            // 入力チェックのバリデーションの設定（テキストボックスに設定予定）
            txt_Param_1_1.Validating += textBox_Validated;
            txt_Param_1_3.Validating += textBox_Validated;
            txt_Param_1_4.Validating += textBox_Validated;
            txt_Param_2_1.Validating += textBox_Validated;
            txt_Param_2_2.Validating += textBox_Validated;
            txt_Param_3_1.Validating += textBox_Validated;
            txt_Param_4_4.Validating += textBox_Validated;
            txt_Param_4_8.Validating += textBox_Validated;

            rdo_Param_4_2.CheckedChanged += textBox_Validated;
            rdo_Param_4_6.CheckedChanged += textBox_Validated;

            checkBox1.CheckedChanged += new EventHandler(checkBox1_CheckedChanged);

            // バリデーションエラー時のアイコン設定
            errorProvider.Clear();                                      // 不要？
            errorProvider.BlinkStyle = ErrorBlinkStyle.NeverBlink;

            // コントロールのリストに値を格納　※項目追加時はリストに要追加（同じ項目名は指定しない）
            listControl = new List<cls_common.controlObj> { new cls_common.controlObj{ControlName_JP = "3D都市モデル", ControlName = "txt_Param_1", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP = "月毎の可照時間", ControlName = "txt_Param_2", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP = "毎月の平均日照時間", ControlName = "txt_Param_3", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP = "月毎の積雪深", ControlName = "txt_Param_4", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP = "DEMデータ使用フラグ", ControlName = "checkBox1", Value = "", Type =  cls_common.CON_CHECK},
                                                 new cls_common.controlObj{ControlName_JP = "DEMデータ", ControlName = "txt_Param_5", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP = "解析結果出力フォルダ", ControlName = "txt_Output_Directory", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP = "発電ポテンシャル推計_面積", ControlName = "txt_Param_1_1", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP = "発電ポテンシャル推計_傾き方位", ControlName = "cmb_Param_1_2", Value = "", Type =  cls_common.CON_COMB},
                                                 new cls_common.controlObj{ControlName_JP = "発電ポテンシャル推計_傾き度", ControlName = "txt_Param_1_3", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP = "発電ポテンシャル推計_傾き度以上", ControlName = "txt_Param_1_4", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP = "傾斜が少ない屋根面の補正_傾き", ControlName = "txt_Param_2_1", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP = "傾斜が少ない屋根面の補正_傾き度", ControlName = "txt_Param_2_2", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP = "太陽光パネル単位面積当たりの発電容量", ControlName = "txt_Param_3_1", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP = "シミュレーション補正３度未満_屋根面と同値", ControlName = "rdo_Param_4_1", Value = "", Type =  cls_common.CON_RADIO},
                                                 new cls_common.controlObj{ControlName_JP = "シミュレーション補正３度未満_指定", ControlName = "rdo_Param_4_2", Value = "", Type =  cls_common.CON_RADIO},
                                                 new cls_common.controlObj{ControlName_JP = "シミュレーション補正３度未満_方位", ControlName = "cmb_Param_4_3", Value = "", Type =  cls_common.CON_COMB},
                                                 new cls_common.controlObj{ControlName_JP = "シミュレーション補正３度未満_傾き度", ControlName = "txt_Param_4_4", Value = "", Type =  cls_common.CON_TEXT},
                                                 new cls_common.controlObj{ControlName_JP = "シミュレーション補正３度以上_屋根面と同値", ControlName = "rdo_Param_4_5", Value = "", Type =  cls_common.CON_RADIO},
                                                 new cls_common.controlObj{ControlName_JP = "シミュレーション補正３度以上_指定", ControlName = "rdo_Param_4_6", Value = "", Type =  cls_common.CON_RADIO},
                                                 new cls_common.controlObj{ControlName_JP = "シミュレーション補正３度以上_方位", ControlName = "cmb_Param_4_7", Value = "", Type =  cls_common.CON_COMB},
                                                 new cls_common.controlObj{ControlName_JP = "シミュレーション補正３度以上_傾き度", ControlName = "txt_Param_4_8", Value = "", Type =  cls_common.CON_TEXT}
            };

            // 初期値の読み込み処理
            if (set_controlValue(initFileName) == false)
            {
                // 失敗メッセージ
                cls_common.showMessage(cls_message.general.ERRMSG_DEFAULT_LOADING, cls_common.MessageType.error);
            }
        }
        #endregion

        /// <summary>
        /// 指定ファイルに処理パラメータを出力する
        /// </summary>
        /// <param name="FileName"></param>
        private void outputParam(string FileName, bool append = false)
        {
            Encoding enc = Encoding.GetEncoding("UTF-8");
            using (StreamWriter writer = new StreamWriter(FileName, append, enc))
            {
                // 出力用区切り文字
                var delimiter = ":";
                var comma = ",";

                // 解析処理入力データ選択
                writer.WriteLine($"{cls_common.get_controlNameJP("txt_Param_1", listControl)}{delimiter}{txt_Param_1.Text}{comma}");
                writer.WriteLine($"{cls_common.get_controlNameJP("txt_Param_2", listControl)}{delimiter}{txt_Param_2.Text}{comma}");
                writer.WriteLine($"{cls_common.get_controlNameJP("txt_Param_3", listControl)}{delimiter}{txt_Param_3.Text}{comma}");
                writer.WriteLine($"{cls_common.get_controlNameJP("txt_Param_4", listControl)}{delimiter}{txt_Param_4.Text}{comma}");
                int param_check1 = 0;
                if (checkBox1.Checked == true)
                    param_check1 = 1;
                writer.WriteLine($"{cls_common.get_controlNameJP("checkBox1", listControl)}{delimiter}{param_check1}{comma}");
                writer.WriteLine($"{cls_common.get_controlNameJP("txt_Param_5", listControl)}{delimiter}{txt_Param_5.Text}{comma}");
                writer.WriteLine($"{cls_common.get_controlNameJP("txt_Output_Directory", listControl)}{delimiter}{txt_Output_Directory.Text}{comma}");

                // 解析処理パラメータ入力
                writer.WriteLine($"{cls_common.get_controlNameJP("txt_Param_1_1", listControl)}{delimiter}{txt_Param_1_1.Text}{comma}" +
                    $"{cls_common.get_controlNameJP("cmb_Param_1_2", listControl)}{delimiter}{cmb_Param_1_2.SelectedIndex}{comma}" +
                    $"{cls_common.get_controlNameJP("txt_Param_1_3", listControl)}{delimiter}{txt_Param_1_3.Text}{comma}" +
                    $"{cls_common.get_controlNameJP("txt_Param_1_4", listControl)}{delimiter}{txt_Param_1_4.Text}{comma}");

                writer.WriteLine($"{cls_common.get_controlNameJP("txt_Param_2_1", listControl)}{delimiter}{txt_Param_2_1.Text}{comma}" +
                                 $"{cls_common.get_controlNameJP("txt_Param_2_2", listControl)}{delimiter}{txt_Param_2_2.Text}{comma}");
                writer.WriteLine($"{cls_common.get_controlNameJP("txt_Param_3_1", listControl)}{delimiter}{txt_Param_3_1.Text}{comma}");

                int param_4_1 = 0;
                int param_4_2 = 0;
                int param_4_5 = 0;
                int param_4_6 = 0;
                // 3度未満
                // 屋根面と同値
                if (rdo_Param_4_1.Checked == true)
                    param_4_1 = 1;
                // 指定
                if (rdo_Param_4_2.Checked == true)
                    param_4_2 = 1;
                // 3度以上
                // 屋根面と同値
                if (rdo_Param_4_5.Checked == true)
                    param_4_5 = 1;
                // 指定
                if (rdo_Param_4_6.Checked == true)
                    param_4_6 = 1;

                writer.WriteLine($"{cls_common.get_controlNameJP("rdo_Param_4_1", listControl)}{delimiter}{param_4_1}{comma}" +
                    $"{cls_common.get_controlNameJP("rdo_Param_4_2", listControl)}{delimiter}{param_4_2}{comma}" +
                    $"{cls_common.get_controlNameJP("cmb_Param_4_3", listControl)}{delimiter}{cmb_Param_4_3.SelectedIndex}{comma}" +
                    $"{cls_common.get_controlNameJP("txt_Param_4_4", listControl)}{delimiter}{txt_Param_4_4.Text}{comma}" +
                    $"{cls_common.get_controlNameJP("rdo_Param_4_5", listControl)}{delimiter}{param_4_5}{comma}" +
                    $"{cls_common.get_controlNameJP("rdo_Param_4_6", listControl)}{delimiter}{param_4_6}{comma}" +
                    $"{cls_common.get_controlNameJP("cmb_Param_4_7", listControl)}{delimiter}{cmb_Param_4_7.SelectedIndex}{comma}" +
                    $"{cls_common.get_controlNameJP("txt_Param_4_8", listControl)}{delimiter}{txt_Param_4_8.Text}{comma}");
            }

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
            // タイトル設定
            sfd.Title = "保存先のファイルを選択してください";
            // ダイアログボックスを閉じる前に現在のディレクトリを復元
            sfd.RestoreDirectory = true;
            // ダイアログを表示
            if (sfd.ShowDialog() == DialogResult.OK)
            {
                // OKボタンがクリック時
                // 設定ファイル保存処理
                // 指定ファイルに入力パラメータを出力
                outputParam(sfd.FileName);

                // 入力内容保存完了メッセージ
                cls_common.showMessage(cls_message.general.MSG_SAVE_COMPLETE, cls_common.MessageType.info, "完了");

            }
            sfd.Dispose();
        }

        // ファイル/フォルダ選択
        // ・「3D都市モデル」          tag:10 フォルダ
        // ・「月毎の可照時間」        tag:11 ファイル
        // ・「毎月の平均日照時間」     tag:12 ファイル
        // ・「月毎の積雪深」           tag:13 ファイル
        // ・「DEMデータ」             tag:14 フォルダ
        // ・「解析結果出力フォルダ」    tag:20 フォルダ
        // ・「保存した入力内容の読込」  tag:30 ファイル

        // フォルダ選択ダイアログオープン
        private void btn_Folder_Dialog_Click(object sender, EventArgs e)
        {
            // 押下されたボタンにより分岐
            string description = "";
            string ctlname = "";
            string directory = "";
            switch (((Button)sender).Tag)
            {
                case "10":    // 3D都市モデル
                    description = cls_message.frm_Analyze.MSG_004;
                    ctlname = "txt_Param_1";
                    directory = txt_Param_1.Text;
                    break;
                case "14":    // DEMデータ
                    description = cls_message.frm_Analyze.MSG_005;
                    ctlname = "txt_Param_5";
                    directory = txt_Param_5.Text;
                    break;
                case "20":    // 解析結果出力フォルダ
                    description = cls_message.frm_Analyze.MSG_008;
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
            }
        }
        // ファイル選択ダイアログオープン
        private void btn_File_Dialog_Click(object sender, EventArgs e)
        {
            // 押下されたボタンにより分岐
            string description = "";
            string ctlname = "";
            string filter = "";
            string filePath = "";

            switch (((Button)sender).Tag)
            {
                case "11":    // 月毎の可照時間
                    description = cls_message.frm_Analyze.MSG_001;
                    ctlname = "txt_Param_2";
                    filter = "CSVファイル(*.csv)|*.csv";
                    filePath = txt_Param_2.Text;
                    break;
                case "12":    // 毎月の平均日照時間
                    description = cls_message.frm_Analyze.MSG_002;
                    ctlname = "txt_Param_3";
                    filter = "CSVファイル(*.csv)|*.csv";
                    filePath = txt_Param_3.Text;
                    break;
                case "13":    // 月毎の積雪深
                    description = cls_message.frm_Analyze.MSG_003;
                    ctlname = "txt_Param_4";
                    filter = "CSVファイル(*.csv)|*.csv";
                    filePath = txt_Param_4.Text;
                    break;
                case "30":    // 保存した入力内容の読込
                    description = cls_message.general.MSG_SELECT_INPUT_FILE;
                    ctlname = "";
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
            //指定しないとすべてのファイルが表示される
            ofd.Filter = (String.IsNullOrEmpty(filter)) ? "すべてのファイル(*.*)|*.*" : filter;
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


                }
            }
        }

        /// <summary>
        /// 指定ファイルの内容をコントロールに値をセットする
        /// </summary>
        /// <param name="fileName">入力内容が保存されたファイル</param>
        /// <returns></returns>
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
        /// テキストボックスのバリデーション結果を処理実施前にチェック
        /// </summary>
        /// <param name="type">一部のみ実施する場合は指定</param>
        /// <returns></returns>
        private bool chk_InputText(string type = "")
        {
            bool ret = true;

            if (ret)
                ret = (errorProvider.GetError(txt_Param_1_1).Length > 0) ? false : true;
            if (ret)
                ret = (errorProvider.GetError(txt_Param_1_3).Length > 0) ? false : true;
            if (ret)
                ret = (errorProvider.GetError(txt_Param_1_4).Length > 0) ? false : true;
            if (ret)
                ret = (errorProvider.GetError(txt_Param_2_1).Length > 0) ? false : true;
            if (ret)
                ret = (errorProvider.GetError(txt_Param_2_2).Length > 0) ? false : true;
            if (ret)
                ret = (errorProvider.GetError(txt_Param_3_1).Length > 0) ? false : true;
            if (ret)
                ret = (errorProvider.GetError(txt_Param_4_4).Length > 0) ? false : true;
            if (ret)
                ret = (errorProvider.GetError(txt_Param_4_8).Length > 0) ? false : true;

            return ret;

        }
        /// <summary>
        /// 処理実施前の入力チェック
        /// </summary>
        /// <returns></returns>
        private bool chk_Input()
        {
            List<string> errList = new List<string>();
            //var chkCount = 0;       // チェック数カウント変数

            // 3D都市モデルフォルダチェック
            var extension = cls_common.FILE_GML;
            if (String.IsNullOrEmpty(txt_Param_1.Text))
            {
                // 必須入力エラー
                errList.Add($"{cls_common.get_controlNameJP("txt_Param_1", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
            }
            else if (!cls_common.IsDirectoryExists(txt_Param_1.Text))
            {
                // フォルダ存在チェックエラー
                errList.Add($"{cls_common.get_controlNameJP("txt_Param_1", listControl)}に{cls_message.general.ERRMSG_FOLDER_EXIST}");
            }
            else if (!cls_common.IsExtensionExists_Directory(txt_Param_1.Text, extension))
            {
                // 指定フォルダ、指定拡張子ファイル存在チェックエラー
                errList.Add($"{cls_common.get_controlNameJP("txt_Param_1", listControl)}の指定フォルダに{extension}{cls_message.general.ERRMSG_EXTENSION_EXIST}");
            }

            // 月毎の可照時間
            extension = cls_common.FILE_CSV;
            if (String.IsNullOrEmpty(txt_Param_2.Text))
            {
                // 必須入力エラー
                errList.Add($"{cls_common.get_controlNameJP("txt_Param_2", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
            }
            else if (!cls_common.IsExtensionExists_File(txt_Param_2.Text, extension))
            {
                // ファイル存在チェックエラー(拡張子指定)
                errList.Add($"{cls_common.get_controlNameJP("txt_Param_2", listControl)}に{extension}{cls_message.general.ERRMSG_EXTENSION_EXIST}");
            }

            // 毎月の平均日照時間
            extension = cls_common.FILE_CSV;
            if (String.IsNullOrEmpty(txt_Param_3.Text))
            {
                // 必須入力エラー
                errList.Add($"{cls_common.get_controlNameJP("txt_Param_3", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
            }
            else if(!cls_common.IsExtensionExists_File(txt_Param_3.Text, extension))
            {
                // ファイル存在チェックエラー(拡張子指定)
                errList.Add($"{cls_common.get_controlNameJP("txt_Param_3", listControl)}に{extension}{cls_message.general.ERRMSG_EXTENSION_EXIST}");
            }

            // 月毎の積雪深
            extension = cls_common.FILE_CSV;
            if (String.IsNullOrEmpty(txt_Param_4.Text))
            {
                // 必須入力エラー
                errList.Add($"{cls_common.get_controlNameJP("txt_Param_4", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
            }
            else if (!cls_common.IsExtensionExists_File(txt_Param_4.Text, extension))
            {
                // ファイル存在チェックエラー(拡張子指定)
                errList.Add($"{cls_common.get_controlNameJP("txt_Param_4", listControl)}に{extension}{cls_message.general.ERRMSG_EXTENSION_EXIST}");
            }

            // DEMデータフォルダチェック
            if (checkBox1.Checked == true)   // DEM使用が有効な場合
            {
                extension = cls_common.FILE_GML;
                if (String.IsNullOrEmpty(txt_Param_5.Text))
                {
                    // 必須入力エラー
                    errList.Add($"{cls_common.get_controlNameJP("txt_Param_5", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
                }
                else if (!cls_common.IsDirectoryExists(txt_Param_5.Text))
                {
                    // フォルダ存在チェックエラー
                    errList.Add($"{cls_common.get_controlNameJP("txt_Param_5", listControl)}に{cls_message.general.ERRMSG_FOLDER_EXIST}");
                }
                else if (!cls_common.IsExtensionExists_Directory(txt_Param_1.Text, extension))
                {
                    // 指定フォルダ、指定拡張子ファイル存在チェックエラー
                    errList.Add($"{cls_common.get_controlNameJP("txt_Param_5", listControl)}の指定フォルダに{extension}{cls_message.general.ERRMSG_EXTENSION_EXIST}");
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

            // 発電ポテンシャル推計
            // 面積（m2未満）
            if (String.IsNullOrEmpty(txt_Param_1_1.Text))
            {
                // 必須入力エラー
                errList.Add($"{cls_common.get_controlNameJP("txt_Param_1_1", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
            }

            // 方位　＆　傾き入力チェック
            if (cmb_Param_1_2.SelectedIndex >= 0)
            {
                if (String.IsNullOrEmpty(txt_Param_1_3.Text)) { 
                    // 必須入力エラー
                    errList.Add($"{cls_common.get_controlNameJP("txt_Param_1_3", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
                }
            }
            if (!String.IsNullOrEmpty(txt_Param_1_3.Text))
            {
                if (cmb_Param_1_2.SelectedIndex <= 0)
                {
                    // 必須入力エラー
                    errList.Add($"{cls_common.get_controlNameJP("cmb_Param_1_2", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
                }
            }

            // 傾斜が少ない(水平に近い)屋根面の補正
            // 傾き
            if (String.IsNullOrEmpty(txt_Param_2_1.Text))
            {
                // 必須入力エラー
                errList.Add($"{cls_common.get_controlNameJP("txt_Param_2_1", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
            }
            // 傾き
            if (String.IsNullOrEmpty(txt_Param_2_2.Text))
            {
                // 必須入力エラー
                errList.Add($"{cls_common.get_controlNameJP("txt_Param_2_2", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
            }

            // 太陽光パネル単位面積当たりの発電容量
            if (String.IsNullOrEmpty(txt_Param_3_1.Text))
            {
                // 必須入力エラー
                errList.Add($"{cls_common.get_controlNameJP("txt_Param_3_1", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
            }

            // 屋根面の向き・傾きの補正　３度未満
            if (rdo_Param_4_2.Checked)
            {
                if (cmb_Param_4_3.SelectedIndex <= 0 || String.IsNullOrEmpty(txt_Param_4_4.Text))
                {
                    errList.Add($"{cls_common.get_controlNameJP("rdo_Param_4_2", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
                }
            }
            // 屋根面の向き・傾きの補正　３度以上
            if (rdo_Param_4_6.Checked)
            {
                if (cmb_Param_4_7.SelectedIndex <= 0 || String.IsNullOrEmpty(txt_Param_4_8.Text))
                {
                    errList.Add($"{cls_common.get_controlNameJP("rdo_Param_4_6", listControl)}{cls_message.general.ERRMSG_INPUT_REQUIRED}");
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

            // bldgフォルダ作成（kmlファイル、jpgファイル、csvファイル）
            string bldgDir = outputDir + "\\bldg";
            Directory.CreateDirectory(bldgDir);

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
        private cls_analyzeParam setParam()
        {
            cls_analyzeParam param = new cls_analyzeParam();
            try
            {
                // 解析処理入力データ選択
                // 3D都市モデル
                param.txt_Param_1 = txt_Param_1.Text;
                // 発電ポテンシャル推計
                // 月毎の可照時間
                param.txt_Param_2 = txt_Param_2.Text;
                // 毎月の平均日照時間
                param.txt_Param_3 = txt_Param_3.Text;
                // 月毎の積雪深
                param.txt_Param_4 = txt_Param_4.Text;
                // DEMデータ
                param.txt_Param_5 = txt_Param_5.Text;

                // 解析結果出力フォルダ
                param.txt_Output_Directory = txt_Output_Directory.Text;

                // 発電ポテンシャル推移
                // 面積
                param.txt_Param_1_1 = txt_Param_1_1.Text;
                // 向き
                param.cmb_Param_1_2 = cmb_Param_1_2.SelectedIndex;
                // 傾き1
                param.txt_Param_1_3 = txt_Param_1_3.Text;
                // 傾き2
                param.txt_Param_1_4 = txt_Param_1_4.Text;

                // 傾斜が少ない(水平に近い)屋根面の補正
                // 傾き1
                param.txt_Param_2_1 = txt_Param_2_1.Text;
                // 傾き2
                param.txt_Param_2_2 = txt_Param_2_2.Text;

                // 太陽光パネル単位面積当たりの発電容量
                // 発電容量
                param.txt_Param_3_1 = txt_Param_3_1.Text;
                // 屋根の傾きによる反射シミュレーション時の屋根面の向き・傾きの補正
                // 3度未満
                // 屋根面と同値
                if (rdo_Param_4_1.Checked == true)
                {
                    param.rdo_Param_4_1 = 1;
                }
                else
                {
                    param.rdo_Param_4_1 = 0;
                }
                // 指定
                if (rdo_Param_4_2.Checked == true)
                {
                    param.rdo_Param_4_2 = 1;
                }
                else
                {
                    param.rdo_Param_4_2 = 0;
                }
                // 向き
                param.cmb_Param_4_3 = cmb_Param_4_3.SelectedIndex;
                // 傾き
                param.txt_Param_4_4 = txt_Param_4_4.Text;

                // 3度以上
                // 屋根面と同値
                if (rdo_Param_4_5.Checked == true)
                {
                    param.rdo_Param_4_5 = 1;
                }
                else
                {
                    param.rdo_Param_4_5 = 0;
                }
                // 指定
                if (rdo_Param_4_6.Checked == true)
                {
                    param.rdo_Param_4_6 = 1;
                }
                else
                {
                    param.rdo_Param_4_6 = 0;
                }
                // 向き
                param.cmb_Param_4_7 = cmb_Param_4_7.SelectedIndex;
                // 傾き
                param.txt_Param_4_8 = txt_Param_4_8.Text;

                return param;
            }
            catch (Exception)
            {
                // パラメータの設定失敗メッセージ
                cls_common.showMessage(cls_message.general.ERRMSG_SET_PARAMETER, cls_common.MessageType.error);
                return param;
            }
        }

        // 解析開始
        private void btn_Start_Analyze_Click(object sender, EventArgs e)
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

            dlg_StartAnalyzeConfirm dlgStartAnalyzeConfirm = new dlg_StartAnalyzeConfirm();
            dlgStartAnalyzeConfirm.ShowDialog();

            // 処理開始時間格納用
            DateTime dt;

            if (dlgStartAnalyzeConfirm.DialogResult == System.Windows.Forms.DialogResult.OK)
            {
                // システム日付
                dt = DateTime.Now;
                String now = dt.ToString($"{dt:yyyyMMddHHmm}");

                // 処理パラメータを構造体に設定
                cls_analyzeParam param = setParam();

                // 出力フォルダ名 
                param.dir_Output_Result = txt_Output_Directory.Text + "\\" + cls_common.OUTPUT_DIR_ANALYSIS + "_" + now;

                // 出力フォルダ作成
                createOutputDir(param.dir_Output_Result);

                // ログファイルパス
                string pathLog = param.dir_Output_Result + "\\" + "log" + "\\" + cls_common.FILE_PARAMETER_LOG;


                // 開始ログ（集計処理）
                cls_common.outputLogMessage(pathLog, "解析処理", cls_common.LogType.start);
                cls_common.outputLogMessage(pathLog, "処理パラメータ");
                cls_common.outputLogMessage(pathLog, "------------------------");
                // 入力パラメータ出力処理
                outputParam(pathLog, true);
                cls_common.outputLogMessage(pathLog, "------------------------");

                // タスクオブジェがnullなら生成
                if (TaskCanceler == null) TaskCanceler = new CancellationTokenSource();

                Task tmpTask = Task.Run(() => FunctionExecute(pathLog, param.dir_Output_Result, param)).ContinueWith(t => FunctionCompleteTask());
                // 処理中ダイアログ表示
                dlg_Analyzing dlgAnalyzing = new dlg_Analyzing();
                dlgAnalyzing.ShowDialog(this);

                // ダイアログ表示
                while (true)
                {
                    if (endFlg == 1)
                    {
                        // 処理終了ダイアログ表示
                        dlg_ResultAnalyze dlgResultAnalyze = new dlg_ResultAnalyze();
                        dlgResultAnalyze.label1.Text = "解析処理が終了しました。";
                        dlgResultAnalyze.ShowDialog();
                        endFlg = 0;
                        break;
                    }
                    else if (endFlg == 2)
                    {
                        // タスク終了を待機
                        Thread.Sleep(1000);

                        // キャンセルダイアログ表示
                        dlg_Cancel dlgCancel = new dlg_Cancel();
                        dlgCancel.label1.Text = "解析処理がキャンセルされました。";
                        dlgCancel.ShowDialog();
                        endFlg = 0;
                        break;
                    }
                }

            }
        }

        // TOP画面
        private void btn_Top_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        #region プライベートメソッド
        /// <summary>
        /// フォームロード
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void frm_Analyze_Load(object sender, EventArgs e)
        {
            // 初期値

            // 処理対象から除外する屋根の条件
            // 北向き
            cmb_Param_1_2.SelectedIndex = 1;

            // 屋根の傾きによる反射シミュレーション時の屋根面の向き・傾きの補正
            // 南向き
            cmb_Param_4_3.SelectedIndex = 3;

            //親Instanceに代入
            frm_Analyze.ParentInstance = this;
        }

        private void chk_Param_CheckedChanged(object sender, EventArgs e)
        {
            if (rdo_Param_4_1.Checked)
            {
                cmb_Param_4_3.Enabled = false;
                txt_Param_4_4.Enabled = false;
                errorProvider.SetError(txt_Param_4_4, string.Empty);
            }
            if (rdo_Param_4_2.Checked)
            {
                cmb_Param_4_3.Enabled = true;
                txt_Param_4_4.Enabled = true;
            }

            if (rdo_Param_4_5.Checked)
            {
                cmb_Param_4_7.Enabled = false;
                txt_Param_4_8.Enabled = false;
                errorProvider.SetError(txt_Param_4_8, string.Empty);
            }
            if (rdo_Param_4_6.Checked)
            {
                cmb_Param_4_7.Enabled = true;
                txt_Param_4_8.Enabled = true;
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
                else
                {
                    RadioButton chk = (RadioButton)sender;
                    cName = chk.Name;
                }
                var error = string.Empty;

                switch (cName)
                {
                    // 発電ポテンシャル推計入力チェック
                    case "txt_Param_1_1":
                        if (!String.IsNullOrEmpty(txt_Param_1_1.Text))
                        {
                            // 符号なし数値（整数値）チェック
                            error = (!cls_common.IsInteger(txt_Param_1_1.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                            errorProvider.SetError(txt_Param_1_1, error);
                            errorProvider.SetIconAlignment(txt_Param_1_1, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        else
                        {
                            errorProvider.SetError(txt_Param_1_1, string.Empty);
                        }
                        break;

                    case "txt_Param_1_3":
                        if (!String.IsNullOrEmpty(txt_Param_1_3.Text))
                        {
                            // 符号なし数値（整数値）チェック
                            error = (!cls_common.IsInteger(txt_Param_1_3.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                            errorProvider.SetError(txt_Param_1_3, error);
                            errorProvider.SetIconAlignment(txt_Param_1_3, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        else
                        {
                            errorProvider.SetError(txt_Param_1_3, string.Empty);
                        }
                        break;

                    case "txt_Param_1_4":
                        if (!String.IsNullOrEmpty(txt_Param_1_4.Text))
                        {
                            // 符号なし数値（整数値）チェック
                            error = (!cls_common.IsInteger(txt_Param_1_4.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                            errorProvider.SetError(txt_Param_1_4, error);
                            errorProvider.SetIconAlignment(txt_Param_1_4, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        else
                        {
                            errorProvider.SetError(txt_Param_1_4, string.Empty);
                        }
                        break;

                    case "txt_Param_2_1":
                        if (!String.IsNullOrEmpty(txt_Param_2_1.Text))
                        {
                            // 符号なし数値（整数値）チェック
                            error = (!cls_common.IsInteger(txt_Param_2_1.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                            errorProvider.SetError(txt_Param_2_1, error);
                            errorProvider.SetIconAlignment(txt_Param_2_1, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        else
                        {
                            errorProvider.SetError(txt_Param_2_1, string.Empty);
                        }
                        break;

                    case "txt_Param_2_2":
                        if (!String.IsNullOrEmpty(txt_Param_2_2.Text))
                        {
                            // 符号なし数値（整数値）チェック
                            error = (!cls_common.IsInteger(txt_Param_2_2.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                            errorProvider.SetError(txt_Param_2_2, error);
                            errorProvider.SetIconAlignment(txt_Param_2_2, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        else
                        {
                            errorProvider.SetError(txt_Param_2_2, string.Empty);
                        }
                        break;

                    // 範囲選択入力チェック
                    case "txt_Param_3_1":
                        if (!String.IsNullOrEmpty(txt_Param_3_1.Text))
                        {
                            // 符号なし数値（小数を含む）チェック
                            error = (!cls_common.IsDecimal(txt_Param_3_1.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                            errorProvider.SetError(txt_Param_3_1, error);
                            errorProvider.SetIconAlignment(txt_Param_3_1, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        else
                        {
                            errorProvider.SetError(txt_Param_3_1, string.Empty);
                        }
                        break;

                    case "txt_Param_4_4":
                        if (!String.IsNullOrEmpty(txt_Param_4_4.Text))
                        {
                            // 符号なし数値（整数値）チェック
                            error = (!cls_common.IsInteger(txt_Param_4_4.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                            errorProvider.SetError(txt_Param_4_4, error);
                            errorProvider.SetIconAlignment(txt_Param_4_4, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        else
                        {
                            errorProvider.SetError(txt_Param_4_4, string.Empty);
                        }
                        break;

                    case "txt_Param_4_8":
                        if (!String.IsNullOrEmpty(txt_Param_4_8.Text))
                        {
                            // 符号なし数値（整数値）チェック
                            error = (!cls_common.IsInteger(txt_Param_4_8.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                            errorProvider.SetError(txt_Param_4_8, error);
                            errorProvider.SetIconAlignment(txt_Param_4_8, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                        }
                        else
                        {
                            errorProvider.SetError(txt_Param_4_8, string.Empty);
                        }
                        break;

                    case "rdo_Param_4_2":
                        if (rdo_Param_4_2.Checked)
                        {
                            if (!String.IsNullOrEmpty(txt_Param_4_4.Text))
                            {
                                // 符号なし数値（整数値）チェック
                                error = (!cls_common.IsInteger(txt_Param_4_4.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                                errorProvider.SetError(txt_Param_4_4, error);
                                errorProvider.SetIconAlignment(txt_Param_4_4, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                            }
                            else
                            {
                                errorProvider.SetError(txt_Param_4_4, string.Empty);
                            }
                        }
                        break;

                    case "rdo_Param_4_6":
                        if (rdo_Param_4_6.Checked)
                        {
                            if (!String.IsNullOrEmpty(txt_Param_4_8.Text))
                            {
                                // 符号なし数値（整数値）チェック
                                error = (!cls_common.IsInteger(txt_Param_4_8.Text)) ? cls_message.general.ERRMSG_NUMBER : string.Empty;
                                errorProvider.SetError(txt_Param_4_8, error);
                                errorProvider.SetIconAlignment(txt_Param_4_8, ErrorIconAlignment.MiddleLeft);    // アイコン:右側中央に表示
                            }
                            else
                            {
                                errorProvider.SetError(txt_Param_4_8, string.Empty);
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

        #endregion

        #region Task処理メソッド
        private void FunctionExecute(string path, string param, cls_analyzeParam analyzeParam)
        {
            Console.WriteLine("処理開始");
            Task tmpTask = Task.Run(() => processFunction(path, param, analyzeParam));
            while (true)
            {
                if (TaskCanceler.Token.IsCancellationRequested)
                {
                    // キャンセルファイルパス
                    string pathCancel = param + "\\" + cls_common.FILE_CANCEL;
                    // ファイル作成
                    using (FileStream fs = File.Create(pathCancel)) {}
                    // キャンセルログ（解析処理）
                    cls_common.outputLogMessage(path, "解析処理がキャンセルされました", cls_common.LogType.end);
                    endFlg = 2;
                    break;
                }
                else if (tmpTask.Status == TaskStatus.RanToCompletion)
                {
                    for (int i = 0; i < Application.OpenForms.Count; i++)
                    {
                        Form f = Application.OpenForms[i];
                        if (f.Name == "dlg_Analyzing")
                        {
                            Invoke(new DelegateProcess(FormClose), f);
                        }
                    }
                    endFlg = 1;
                    break;
                }
            }

            Console.WriteLine("処理終了");
        }
        private void processFunction(string path, string param, cls_analyzeParam analyzeParam)
        {
            int ret = 0;

            // 座標系を取得
            SetJPZone();


            // C++呼出し
            cls_common.outputLogMessage(path, "AnalizeBldgFiles", cls_common.LogType.start);
            ret = AnalizeBldgFiles(txt_Param_1.Text, param + "\\output");
            cls_common.outputLogMessage(path, "AnalizeBldgFiles", cls_common.LogType.end);

            if (ret == 1)
            {
                // ファイルなし
                MessageBox.Show("ファイルが存在しません。", "終了", MessageBoxButtons.OK, MessageBoxIcon.Information);
                cls_common.outputLogMessage(path, "処理結果：エラー");
                return;
            }
            if (ret == 2)
            {
                // キャンセル
                return;
            }

            // C++呼出し
            if (checkBox1.Checked == true)  // DEM使用が有効な場合
            {
                cls_common.outputLogMessage(path, "AnalizeDemFiles", cls_common.LogType.start);
                ret = AnalizeDemFiles(txt_Param_5.Text, param + "\\output");
                cls_common.outputLogMessage(path, "AnalizeDemFiles", cls_common.LogType.end);

                if (ret == 1)
                {
                    // ファイルなし
                    MessageBox.Show("ファイルが存在しません。", "終了", MessageBoxButtons.OK, MessageBoxIcon.Information);
                    cls_common.outputLogMessage(path, "処理結果：エラー");
                    return;
                }
                if (ret == 2)
                {
                    // キャンセル
                    return;
                }
            }

            // 入力データパス設定
            SetPossibleSunshineDataPath(analyzeParam.txt_Param_2);
            SetAverageSunshineDataPath(analyzeParam.txt_Param_3);
            SetMetpvDataPath(analyzeParam.txt_Param_4);

            // パラメータ引き渡し  
            InitializeUIParam();
            // 発電ポテンシャル推移
            SetElecPotential(double.Parse(analyzeParam.txt_Param_1_1), analyzeParam.cmb_Param_1_2, double.Parse(analyzeParam.txt_Param_1_3), double.Parse(analyzeParam.txt_Param_1_4));
            // 傾斜が少ない(水平に近い)屋根面の補正
            SetRoofSurfaceCorrect(double.Parse(analyzeParam.txt_Param_2_1), double.Parse(analyzeParam.txt_Param_2_2));
            // 太陽光パネル単位面積当たりの発電容量
            SetAreaSolarPower(double.Parse(analyzeParam.txt_Param_3_1));
            // 屋根の傾きによる反射シミュレーション時の屋根面の向き・傾きの補正
            if (Convert.ToBoolean(analyzeParam.rdo_Param_4_1))
            {
                analyzeParam.cmb_Param_4_3 = 0;
                analyzeParam.txt_Param_4_4 = "0";
            }
            if (Convert.ToBoolean(analyzeParam.rdo_Param_4_5))
            {
                analyzeParam.cmb_Param_4_7 = 0;
                analyzeParam.txt_Param_4_8 = "0";
            }
            SetReflectRoofCorrect_Lower(Convert.ToBoolean(analyzeParam.rdo_Param_4_1), Convert.ToBoolean(analyzeParam.rdo_Param_4_2), analyzeParam.cmb_Param_4_3, double.Parse(analyzeParam.txt_Param_4_4));    // 3度未満
            SetReflectRoofCorrect_Upper(Convert.ToBoolean(analyzeParam.rdo_Param_4_5), Convert.ToBoolean(analyzeParam.rdo_Param_4_6), analyzeParam.cmb_Param_4_7, double.Parse(analyzeParam.txt_Param_4_8));    // 3度以上

            SetEnableDEMData(checkBox1.Checked);

            // 各解析処理の実行フラグ
            if (checkBox2.Checked)
            {
                SetExecSolarPotantial(true);
                SetExecReflection(false);
            }
            else if(checkBox3.Checked)
            {
                SetExecSolarPotantial(false);
                SetExecReflection(true);
            }
            else
            {
                SetExecSolarPotantial(true);
                SetExecReflection(true);
            }

            // 解析
            cls_common.outputLogMessage(path, "AnalyzeStart", cls_common.LogType.start);
            bool bret = AnalyzeStart(analyzeParam.dir_Output_Result + "\\data");
            cls_common.outputLogMessage(path, "AnalyzeStart", cls_common.LogType.end);
            if (!bret)  return;

            // C++呼出し
            cls_common.outputLogMessage(path, "LOD2DataOut", cls_common.LogType.start);
            ret = LOD2DataOut(txt_Param_1.Text, param);
            cls_common.outputLogMessage(path, "LOD2DataOut", cls_common.LogType.end);

            if (ret == 1)
            {
                // ファイルなし
                MessageBox.Show("ファイルが存在しません。", "終了", MessageBoxButtons.OK, MessageBoxIcon.Information);
                cls_common.outputLogMessage(path, "処理結果：エラー");
                return;
            }
            if (ret == 2)
            {
                // キャンセル
                return;
            }

            // 後ほど処理結果を出力するように要修正
            cls_common.outputLogMessage(path, "処理結果：完了");

            // 終了ログ（解析処理）
            cls_common.outputLogMessage(path, "解析処理", cls_common.LogType.end);

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

        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {
            // DEMデータ関連のコントロールON/OFF
            bool bEnabled = checkBox1.Checked;
            txt_Param_5.Enabled = bEnabled;
            btn_Select_Param_5.Enabled = bEnabled;
        }

        private void checkBox2_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox2.Checked && checkBox3.Checked)
            {
                checkBox3.Checked = false;
            }
        }

        private void checkBox3_CheckedChanged(object sender, EventArgs e)
        {
            if (checkBox2.Checked && checkBox3.Checked)
            {
                checkBox2.Checked = false;
            }
        }
    }
}
