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
	public partial class AggregateWindow : Window
	{
		#region Pages
		/// <summary>
		/// 判定・集計-データ入力画面
		/// </summary>
		Page pageAggregateDataIO = null;

		/// <summary>
		/// 判定-適地判定設定画面
		/// </summary>
		Page pageJudgeParams = null;

		/// <summary>
		/// 実行画面
		/// </summary>
		Page pageExecute = null;

		/// <summary>
		/// 実行中画面
		/// </summary>
		Page pageProgress = null;
		#endregion

		/// <summary>
		/// 画面内のボタンリスト
		/// </summary>
		IEnumerable<Button> buttons;

		/// <summary>
		/// プロセス実行中フラグ
		/// </summary>
		bool IsRunning = false;

		/// <summary>
		/// 解析結果フォルダ
		/// </summary>
		public string AnalyzeResultDir { get; set; } = "";

		public AggregateWindow()
		{
			this.InitializeComponent();

#if DEBUG
			MainContentFrame.NavigationUIVisibility = NavigationUIVisibility.Visible;
#endif

			// Events
			Loaded += AggregateWindow_Loaded;
			Closing += AggregateWindow_Closing;
			MainContentFrame.Navigating += MainContentFrame_Navigating;
			MainContentFrame.LoadCompleted += (sender, args) => { UpdateControls(); };
			MainContentFrame.ContentRendered += MainContentFrame_ContentRendered;
		}


		/// <summary>
		/// 画面を更新する
		/// </summary>
		private void UpdateControls()
		{
			try
			{
				LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

				bool buttonEnable = true;
				IsRunning = false;

				var pageType = MainContentFrame.Content as Page;
				if (pageType == pageAggregateDataIO)
				{
					NextPageButton.Visibility = Visibility.Visible;
					NextPageButton.Content = "判定条件設定 >>";
					BackPageButton.Visibility = Visibility.Hidden;

					// 色変更
					HighlightButton(DataIOButton);
					NormalButton(JudgeButton);
					NormalButton(ExecButton);
				}
				else if (pageType == pageJudgeParams)
				{
					NextPageButton.Visibility = Visibility.Visible;
					NextPageButton.Content = "確認画面 >>";
					BackPageButton.Visibility = Visibility.Visible;
					BackPageButton.DataContext = new { BackButtonText = "<< 入出力データ設定" };

					// 色変更
					NormalButton(DataIOButton);
					HighlightButton(JudgeButton);
					NormalButton(ExecButton);
				}
				else if (pageType == pageExecute)
				{
					NextPageButton.Visibility = Visibility.Visible;
					NextPageButton.Content = "実行開始";
					BackPageButton.Visibility = Visibility.Visible;
					BackPageButton.DataContext = new { BackButtonText = "<< 判定条件設定" };

					// 色変更
					NormalButton(DataIOButton);
					NormalButton(JudgeButton);
					HighlightButton(ExecButton);

				}
				else if (pageType == pageProgress)
				{
					NextPageButton.Visibility = Visibility.Hidden;
					BackPageButton.Visibility = Visibility.Hidden;
					buttonEnable = false;
					IsRunning = true;

					// 色変更
					NormalButton(DataIOButton);
					NormalButton(JudgeButton);
					HighlightButton(ExecButton);
				}
				else
				{
					NextPageButton.Visibility = Visibility.Hidden;
					BackPageButton.Visibility = Visibility.Hidden;

					// 色変更
					NormalButton(DataIOButton);
					NormalButton(JudgeButton);
					HighlightButton(ExecButton);
				}

				// ボタン活性制御
				foreach (var btn in buttons)
				{
					btn.IsEnabled = buttonEnable;
				}
			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}
		}


		/// <summary>
		/// ハイライトボタン
		/// </summary>
		/// <param name="button"></param>
		private void HighlightButton(Button button)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			button.Background = SystemColors.HighlightBrush;
			button.Foreground = SystemColors.HighlightTextBrush;
		}

		/// <summary>
		/// 通常ボタン
		/// </summary>
		/// <param name="button"></param>
		private void NormalButton(Button button)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
			LogManager.Instance.OutputLogMessage($"In Button Name:{button.Name}", LogManager.LogType.DEBUG);

			button.Background = SystemColors.ControlBrush;
			button.Foreground = SystemColors.ControlTextBrush;
		}
		#region Events

		/// <summary>
		/// ロードイベント
		/// </summary>
		private void AggregateWindow_Loaded(object sender, RoutedEventArgs e)
        {
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			if (!string.IsNullOrEmpty(AnalyzeResultDir))
			{
				// 解析フォルダが指定されている
				AggregateParam.Instance.InitParams();
				AggregateParam.Instance.AnalyzeResultPath = AnalyzeResultDir;
			}
			else
			{
				if (!CommonManager.Instance.ReadPreParam || !ParamFileManager.Instance.ReadLastParamFile())
				{
					AggregateParam.Instance.InitParams();
				}
			}

			pageAggregateDataIO = new PageAggregateDataIO();
			MainContentFrame.Navigate(pageAggregateDataIO);

			// 画面上のボタンを取得しておく
			Func<DependencyObject, IEnumerable<Button>> func = null;
			func = b =>
			{
				if (b is Button) { return new[] { (Button)b }; }
				return Enumerable.Range(0, VisualTreeHelper.GetChildrenCount(b)).Select(i => func(VisualTreeHelper.GetChild(b, i))).SelectMany(a => a);
			};
			buttons = func(this);
		}

		/// <summary>
		/// クローズ時イベント
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void AggregateWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			// 実行中チェック
			if (pageProgress != null && IsRunning)
			{
				MessageBox.Show("処理が実行中です。\n終了するには、キャンセルボタンからキャンセルしてください。", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Warning);
				e.Cancel = true;
			}

			if (!e.Cancel)
			{
				pageAggregateDataIO = null;
				pageJudgeParams = null;
				pageExecute = null;
				pageProgress = null;

				AggregateParam.Instance.Dispose();
			}

		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void DataIOButton_Click(object sender, RoutedEventArgs e)
		{
			try
			{
				LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
				if (pageAggregateDataIO == null) pageAggregateDataIO = new PageAggregateDataIO();
				MainContentFrame.Navigate(pageAggregateDataIO);
			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}
		}

		private void JudgeButton_Click(object sender, RoutedEventArgs e)
		{
			try
			{
				LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
				if (pageJudgeParams == null) pageJudgeParams = new PageJudgeParams();
				MainContentFrame.Navigate(pageJudgeParams);
			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}
		}

		private void ExecButton_Click(object sender, RoutedEventArgs e)
		{
			try
			{
				LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
				if (pageExecute == null) pageExecute = new PageCommonExecute();
				MainContentFrame.Navigate(pageExecute);
			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}
		}

		private void BackButton_Click(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
			this.Close();
		}

		private void ReadParamButton_Click(object sender, RoutedEventArgs e)
		{
			try
			{
				LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
				var window = new ReadParamWindow();
				window.ShowDialog();
			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}
		}

		//private void CustomSettingsButton_Click(object sender, RoutedEventArgs e)
		//{
		//	try
		//	{
		//		LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
		//		var window = new CustomSettingsWindow();
		//		window.ShowDialog();
		//	}
		//	catch (Exception ex)
		//	{
		//		CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
		//		LogManager.Instance.ExceptionLog(ex);
		//	}
		//}

		private void NextPageButton_Click(object sender, RoutedEventArgs e)
		{
			try
			{
				LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
				var pageType = MainContentFrame.Content as Page;
				if (pageType == pageAggregateDataIO)
				{
					if (pageJudgeParams == null) pageJudgeParams = new PageJudgeParams();
					MainContentFrame.Navigate(pageJudgeParams);
				}
				else if (pageType == pageJudgeParams)
				{
					if (pageExecute == null) pageExecute = new PageCommonExecute();
					MainContentFrame.Navigate(pageExecute);
				}
				else if (pageType == pageExecute)
				{
					if (pageProgress == null) pageProgress = new PageCommonProgress();
					MainContentFrame.Navigate(pageProgress);
				}
				else
				{
					// 何もしない
				}
			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}
		}

		private void BackPageButton_Click(object sender, RoutedEventArgs e)
		{
			try
			{
				LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

				var pageType = MainContentFrame.Content as Page;
				if (pageType == pageAggregateDataIO)
				{
					// 何もしない
				}
				else if (pageType == pageJudgeParams)
				{
					if (pageAggregateDataIO == null) pageAggregateDataIO = new PageAggregateDataIO();
					MainContentFrame.Navigate(pageAggregateDataIO);
				}
				else if (pageType == pageExecute)
				{
					if (pageJudgeParams == null) pageJudgeParams = new PageJudgeParams();
					MainContentFrame.Navigate(pageJudgeParams);
				}
			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}
		}

		private void MainContentFrame_Navigating(object sender, NavigatingCancelEventArgs e)
		{
			try
			{
				LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

				// 入力チェック
				var pageType = MainContentFrame.Content as Page;
				if (pageType == null) return;

				if (pageType == pageAggregateDataIO)
				{
					if (!(pageType as PageAggregateDataIO).CheckParams())
					{
						e.Cancel = true;
					}
					else
					{
						// 表示中の地図画像を保存する
						(pageType as PageAggregateDataIO).SaveRange();
					}

				}
				else if (pageType == pageJudgeParams)
				{
					if (!(pageType as PageJudgeParams).CheckParams())
					{
						e.Cancel = true;
					}
				}
				else if (pageType == pageExecute)
				{
					if (pageProgress != null)
					{
						(pageProgress as PageCommonProgress).mainContentFrame = MainContentFrame;
					}
				}
			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}
		}

		private void MainContentFrame_ContentRendered(object sender, EventArgs e)
		{
			try
			{
				LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

				// 入力チェック
				var pageType = MainContentFrame.Content as Page;
				if (pageType == null) return;

				if (pageType == pageAggregateDataIO)
				{
					// 地図を表示する
					(pageType as PageAggregateDataIO).MapDataSelection();
				}
			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}
		}
		#endregion

	}
}
