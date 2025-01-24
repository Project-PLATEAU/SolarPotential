using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace SolarPotential
{
	/// <summary>
	/// MainWindow.xaml の相互作用ロジック
	/// </summary>
	public partial class MainWindow : Window
	{
        /// <summary>
        /// 解析・シミュレーション画面
        /// </summary>
        public static AnalyzeWindow analyzeWindow { get; set; }

        /// <summary>
        /// 適地判定・集計画面
        /// </summary>
        public static AggregateWindow aggregateWindow { get; set; }

        public MainWindow()
		{
            this.InitializeComponent();

            this.Title = CommonManager.TEXT_SYSTEM_CAPTION;

            JpZoneLink.NavigateUri = new Uri("https://www.gsi.go.jp/sokuchikijun/jpc.html");

            // Events
            Loaded += MainWindow_Loaded;
            Closing += MainWindow_Closing;

            JPZoneComboBox.SelectionChanged += (s, e) => {
                GeoUtil.Instance.JPZone = JPZoneComboBox.SelectedIndex + 1;
                GeoUtil.Instance.UpdateJPZone();
            };

            PreParamCheckBox.Checked += (s, e) => { CommonManager.Instance.ReadPreParam = true; };
            PreParamCheckBox.Unchecked += (s, e) => { CommonManager.Instance.ReadPreParam = false; };
        }

        /// <summary>
        /// 解析完了後→集計画面を表示する
        /// </summary>
        public static void ShowAggregateWindow(string dir)
        {
            analyzeWindow?.Hide();

            // 集計画面を開く
            if (aggregateWindow == null)
            {
                aggregateWindow = new AggregateWindow();
                aggregateWindow.Closing += (sender, e) => { aggregateWindow = null; };
            }
            CommonManager.Instance.CurrentProcess = CommonManager.Process.Aggregate;
            aggregateWindow.AnalyzeResultDir = dir;
            aggregateWindow.ShowDialog();
        }


        /// <summary>
        /// ロードイベント
        /// </summary>
        private void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
            LogManager.Instance.OutputLogMessage($"{CommonManager.TEXT_SYSTEM_CAPTION} システム開始");

            // 座標系設定
            List<string> JPZoneList = new List<string>();
            for (int i = 1; i <= 19; i++)
            {
                JPZoneList.Add($"{i}系");
            }
            JPZoneComboBox.ItemsSource = JPZoneList;

            // 設定ファイルの系番号をデフォルトに設定
            JPZoneComboBox.SelectedIndex = GeoUtil.Instance.JPZone - 1;

            // 前回パラメータ読み込みフラグ
            PreParamCheckBox.IsChecked = CommonManager.Instance.ReadPreParam;
        }

        private void MainWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
            LogManager.Instance.OutputLogMessage($"{CommonManager.TEXT_SYSTEM_CAPTION} システム終了");

            analyzeWindow?.Close();
            aggregateWindow?.Close();
        }

        private void ButtonAnalyze_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

                analyzeWindow = new AnalyzeWindow();
                analyzeWindow.Closing += (_sender, _e) => { analyzeWindow = null; };
                CommonManager.Instance.CurrentProcess = CommonManager.Process.Analyze;
                LogManager.Instance.OutputLogMessage($"{CommonManager.TitleText}を実行します。");

                analyzeWindow.Title = $"{CommonManager.TEXT_SYSTEM_CAPTION} - {CommonManager.TitleText}";
                analyzeWindow.ShowDialog();
            }
            catch (Exception ex)
            {
                CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
                LogManager.Instance.ExceptionLog(ex);
            }
        }

        private void ButtonAggregate_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

                aggregateWindow = new AggregateWindow();
                aggregateWindow.Closing += (_sender, _e) => { aggregateWindow = null; };
                CommonManager.Instance.CurrentProcess = CommonManager.Process.Aggregate;
                LogManager.Instance.OutputLogMessage($"{CommonManager.TitleText}を実行します。");

                aggregateWindow.Title = $"{CommonManager.TEXT_SYSTEM_CAPTION} - {CommonManager.TitleText}";
                aggregateWindow.ShowDialog();
            }
            catch (Exception ex)
            {
                CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
                LogManager.Instance.ExceptionLog(ex);
            }
        }

        private void ButtonGetInputData_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            }
            catch (Exception ex)
            {
                CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
                LogManager.Instance.ExceptionLog(ex);
            }
        }

        private void ButtonExit_Click(object sender, RoutedEventArgs e)
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            var result = MessageBox.Show("システムを終了します。よろしいですか？", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OKCancel, MessageBoxImage.Question);
            if (result == MessageBoxResult.Cancel)
            {
                return;
			}

            this.Close();
        }

		private void Hyperlink_RequestNavigate(object sender, RequestNavigateEventArgs e)
		{
            try
            {
                System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo(e.Uri.AbsoluteUri));
                e.Handled = true;
            }
            catch (Exception ex)
            {
                LogManager.Instance.ExceptionLog(ex);
            }
        }
    }
}
