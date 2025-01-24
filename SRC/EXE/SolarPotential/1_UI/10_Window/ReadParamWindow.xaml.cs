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
using System.Collections.ObjectModel;

namespace SolarPotential
{
    /// <summary>
    /// MainWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class ReadParamWindow : Window
	{
        /// <summary>
        /// 拡張子フィルタ(Paramファイル)
        /// </summary>
        private static readonly string EXT_FILTER_PARAM = "Paramファイル(*.param)|*.param";

		/// <summary>
		/// 履歴リスト
		/// </summary>
		ObservableCollection<ParamFileManager.ParamInfo> FilterList { get; set; }

		public ReadParamWindow()
		{
            this.InitializeComponent();

			// Events
			Loaded += ReadParamWindow_Loaded;
            Closing += ReadParamWindow_Closing;

            ListParamHistory.SelectionChanged += (s, e) =>
            {
                if (ListParamHistory.SelectedItem != null) ReadButton.IsEnabled = true;
            };

            ParamFilePath.LostFocus += (s, e) =>
            { 
                if (!string.IsNullOrEmpty(ParamFilePath.Text)) ReadButton.IsEnabled = true;
            };

			// リストの絞り込み
			CheckBuild.Checked += (s, e) => UpdateList();
			CheckBuild.Unchecked += (s, e) => UpdateList();
			CheckLand.Checked += (s, e) => UpdateList();
			CheckLand.Unchecked += (s, e) => UpdateList();
			CheckPotential.Checked += (s, e) => UpdateList();
			CheckPotential.Unchecked += (s, e) => UpdateList();
			CheckReflection.Checked += (s, e) => UpdateList();
			CheckReflection.Unchecked += (s, e) => UpdateList();
			StartDate.SelectedDateChanged += (s, e) => UpdateList();
			EndDate.SelectedDateChanged += (s, e) => UpdateList();
			TextBoxStartHour.TextChanged += (s, e) => { if (TextBoxStartHour.Text.Length > 0) UpdateList(); };
			TextBoxStartMinute.TextChanged += (s, e) => { if (TextBoxStartMinute.Text.Length > 0) UpdateList(); };
			TextBoxEndHour.TextChanged += (s, e) => { if (TextBoxEndHour.Text.Length > 0) UpdateList(); };
			TextBoxEndMinute.TextChanged += (s, e) => { if (TextBoxEndMinute.Text.Length > 0) UpdateList(); };

			TextBoxStartHour.PreviewTextInput += TextBoxHour_PreviewTextInput;
            TextBoxStartMinute.PreviewTextInput += TextBoxMinute_PreviewTextInput;
            TextBoxEndHour.PreviewTextInput += TextBoxHour_PreviewTextInput;
            TextBoxEndMinute.PreviewTextInput += TextBoxMinute_PreviewTextInput;

        }

        /// <summary>
        /// 履歴リストに表示中リストの変更を反映
        /// </summary>
        private void UpdateHistoryParamList()
        {
            if (ParamFileManager.Instance.ParamHistoryList.Count <= 0) return;

            foreach (var param in FilterList)
            {
                var target = ParamFileManager.Instance.ParamHistoryList.Where(p => p.Date.Equals(param.Date)).FirstOrDefault();
                if (target == null) continue;
                target.Explanation = param.Explanation;
            }

            ParamFileManager.Instance.UpdateHistoryFile();
        }

        /// <summary>
        /// リストの更新
        /// </summary>
        private void UpdateList()
        {
			if (HistoryRadio.IsChecked != true) return;
			if (StartDate.SelectedDate == null) return;
			if (EndDate.SelectedDate == null) return;

            // 更新前に元のリストに説明部分を反映する
            UpdateHistoryParamList();

            FilterList.Clear();

			// 日付を取得
			DateTime dtStart = (DateTime)StartDate.SelectedDate;
			DateTime dtEnd = (DateTime)EndDate.SelectedDate;
			dtEnd = dtEnd.AddHours(23);   // 終了時間を23:59に設定
			dtEnd = dtEnd.AddMinutes(59);
			if (CheckStartTime.IsChecked == true)
			{
				dtStart = new DateTime(dtStart.Year, dtStart.Month, dtStart.Day, int.Parse(TextBoxStartHour.Text), int.Parse(TextBoxStartMinute.Text), 0);
			}
			if (CheckEndTime.IsChecked == true)
			{
				dtEnd = new DateTime(dtEnd.Year, dtEnd.Month, dtEnd.Day, int.Parse(TextBoxEndHour.Text), int.Parse(TextBoxEndMinute.Text), 0);
			}

			foreach (var History in ParamFileManager.Instance.ParamHistoryList)
			{
				// 絞り込み対象
				if (CheckBuild.IsChecked == true && !History.Target.Build) continue;
				if (CheckLand.IsChecked == true && !History.Target.Land) continue;
				if (CheckPotential.IsChecked == true && !History.Target.SolarPotential) continue;
				if (CheckReflection.IsChecked == true && !History.Target.Reflection) continue;

				// 日付
				if (dtStart <= History.Date && History.Date <= dtEnd)
				{
                    FilterList.Add(History);
				}
			}
		}

        /// <summary>
        /// ロードイベント
        /// </summary>
        private void ReadParamWindow_Loaded(object sender, RoutedEventArgs e)
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            FilterList = new ObservableCollection<ParamFileManager.ParamInfo>(ParamFileManager.Instance.ParamHistoryList);
			if (ParamFileManager.Instance.ParamHistoryList.Count > 0)
            {
                // 日付を設定
                DateTime dtStart = DateTime.MaxValue, dtEnd = DateTime.MinValue;
                foreach (var History in ParamFileManager.Instance.ParamHistoryList)
                {
                    dtStart = History.Date < dtStart ? History.Date : dtStart;
                    dtEnd = dtEnd < History.Date ? History.Date : dtEnd;
                }
                StartDate.SelectedDate = dtStart;
                EndDate.SelectedDate = dtEnd;

                HistoryRadio.IsEnabled = true;
            }
            this.DataContext = FilterList;

		}

        private void ReadParamWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            UpdateHistoryParamList();

            // 解放
            ParamFileManager.Instance.Dispose();
        }

        private void ReadButton_Click(object sender, RoutedEventArgs e)
		{
            try
            {
                LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

                // パラメータファイルを取得
                string file = "";
                if (SelectFileRadio.IsChecked == true)
                {
                    file = ParamFilePath.Text;
                }
                else
                {
                    file = ParamFileManager.Instance.GetParamFilePath(ListParamHistory.SelectedIndex);
                }

                // 読み込み処理
                bool ret = false;
                switch (CommonManager.Instance.CurrentProcess)
                {
                    case CommonManager.Process.Analyze:
                        if (ParamFileManager.Instance.ReadAnalyzeParams(file))
                        {
                            ret = true;
						}
                        break;

                    case CommonManager.Process.Aggregate:
                        if (ParamFileManager.Instance.ReadAggregateParams(file))
                        {
                            ret = true;
                        }
                        break;

                    default:
                        break;
                }
                if (!ret)
                {
                    throw new Exception("パラメータファイルの読み込みに失敗しました。");
                }
                else
                {
                    // 読み込んだパラメータの各データパスをチェック
                    ParamFileManager.Instance.DataPathExists();
                }

                this.Close();
            }
            catch (Exception ex)
            {
                CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
                LogManager.Instance.OutputLogMessage(ex.Message);

            }
        }

		private void BackButton_Click(object sender, RoutedEventArgs e)
		{
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
            this.Close();
        }

		private void DelateButton_Click(object sender, RoutedEventArgs e)
		{
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            if (ListParamHistory.SelectedItem == null)
            {
                MessageBox.Show("履歴が選択されていません。", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
			}

            if (ListParamHistory.SelectedItem is ParamFileManager.ParamInfo)
            {
                var result = MessageBox.Show("選択中の履歴を削除しますか？", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.YesNo, MessageBoxImage.Question);
                if (result == MessageBoxResult.No) return;

                // 履歴を削除
                var param = ListParamHistory.SelectedItem as ParamFileManager.ParamInfo;
                var target = ParamFileManager.Instance.ParamHistoryList.Where(x => x == param).FirstOrDefault();
                if (target == null) return;
                ParamFileManager.Instance.DeleteHistory(target);
                UpdateList();
            }

        }

        private void AllDelateButton_Click(object sender, RoutedEventArgs e)
		{
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            var result = MessageBox.Show("すべての履歴を削除しますか？", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.YesNo, MessageBoxImage.Question);
            if (result == MessageBoxResult.No) return;

            // 履歴をすべて削除
            ParamFileManager.Instance.DeleteAllHistory();
            UpdateList();
        }

        private void SelectParamFileButton_Click(object sender, RoutedEventArgs e)
		{
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            string file = ParamFilePath.Text;
            CommonManager.Instance.ShowSelectFileDialog(out string path, "パラメータファイルを選択します。", file, EXT_FILTER_PARAM);
            if (string.IsNullOrEmpty(path)) return;

            ParamFilePath.Text = path;
            LogManager.Instance.OutputLogMessage($"パラメータファイル：{path}", LogManager.LogType.INFO);

            // 読み込みボタンを有効にする
            ReadButton.IsEnabled = true;
        }

        private void TextBoxHour_PreviewTextInput(object sender, TextCompositionEventArgs e)
        {
            TextBox tb = e.Source as TextBox;
            var text = tb.Text + e.Text;

            if (int.TryParse(text, out int num))
            {
                if (num < 0 || 23 < num)
                {
                    e.Handled = true;
                    return;
                }
            }
            else 
            {
                e.Handled = true;
            }
        }

        private void TextBoxMinute_PreviewTextInput(object sender, TextCompositionEventArgs e)
        {
            TextBox tb = e.Source as TextBox;
            var text = tb.Text + e.Text;

            if (int.TryParse(text, out int num))
            {
                if (num < 0 || 59 < num)
                {
                    e.Handled = true;
                    return;
                }
            }
            else
            {
                e.Handled = true;
            }
        }

	}
}
