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
	public partial class AnalyzeWindow : Window
	{
		#region Pages
		/// <summary>
		/// 解析-データ入力画面
		/// </summary>
		Page pageAnalyzeDataIO = null;

		/// <summary>
		/// 解析-エリア選択画面
		/// </summary>
		Page pageAnalyzeSelectArea = null;

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

		public AnalyzeWindow()
		{
			this.InitializeComponent();

#if DEBUG
			MainContentFrame.NavigationUIVisibility = NavigationUIVisibility.Visible;
#endif

			// Events
			Loaded += AnalyzeWindow_Loaded;
			Closing += AnalyzeWindow_Closing;
			MainContentFrame.Navigating += MainContentFrame_Navigating;
			MainContentFrame.LoadCompleted += (sender, args) => { UpdateControls(); };

		}

		/// <summary>
		/// 画面を更新する
		/// </summary>
		private void UpdateControls()
		{
			try
			{
				bool buttonEnable = true;
				IsRunning = false;

				var pageType = MainContentFrame.Content as Page;
				if (pageType == pageAnalyzeDataIO)
				{
					NextPageButton.Visibility = Visibility.Visible;
					NextPageButton.DataContext = new { NextButtonText = "解析エリア設定 >>" };
					BackPageButton.Visibility = Visibility.Hidden;

					// 色変更
					HighlightButton(DataIOButton);
					NormalButton(SelectAreaButton);
					NormalButton(ExecButton);
				}
				else if (pageType == pageAnalyzeSelectArea)
				{
					NextPageButton.Visibility = Visibility.Visible;
					NextPageButton.DataContext = new { NextButtonText = "確認画面 >>" };
					BackPageButton.Visibility = Visibility.Visible;
					BackPageButton.DataContext = new { BackButtonText = "<< 入出力データ設定" };

					// 色変更
					NormalButton(DataIOButton);
					HighlightButton(SelectAreaButton);
					NormalButton(ExecButton);
				}
				else if (pageType == pageExecute)
				{
					NextPageButton.Visibility = Visibility.Visible;
					NextPageButton.DataContext = new { NextButtonText = "実行開始" };
					BackPageButton.Visibility = Visibility.Visible;
					BackPageButton.DataContext = new { BackButtonText = "<< 解析エリア設定" };

					// 色変更
					NormalButton(DataIOButton);
					NormalButton(SelectAreaButton);
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
					NormalButton(SelectAreaButton);
					HighlightButton(ExecButton);
				}
				else
				{
					NextPageButton.Visibility = Visibility.Hidden;
					BackPageButton.Visibility = Visibility.Hidden;

					// 色変更
					NormalButton(DataIOButton);
					NormalButton(SelectAreaButton);
					HighlightButton(ExecButton);
				}

				// ボタン活性制御
				foreach(var btn in buttons)
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
		private void HighlightButton(Button button)
		{
			button.Background = SystemColors.HighlightBrush;
			button.Foreground = SystemColors.HighlightTextBrush;
		}

		/// <summary>
		/// 通常ボタン
		/// </summary>
		private void NormalButton(Button button)
		{
			button.Background = SystemColors.ControlBrush;
			button.Foreground = SystemColors.ControlTextBrush;
		}


		#region Events

		/// <summary>
		/// ロードイベント
		/// </summary>
		private void AnalyzeWindow_Loaded(object sender, RoutedEventArgs e)
		{
			try
			{
				LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

				// パラメータ読み込み
				if (!CommonManager.Instance.ReadPreParam || !ParamFileManager.Instance.ReadLastParamFile())
				{
					AnalyzeParam.Instance.InitParams();
				}

				pageAnalyzeDataIO = new PageAnalyzeDataIO();
				MainContentFrame.Navigate(pageAnalyzeDataIO);

				// 画面上のボタンを取得しておく
				Func<DependencyObject, IEnumerable<Button>> func = null;
				func = b =>
				{
					if (b is Button) { return new[] { (Button)b }; }
					return Enumerable.Range(0, VisualTreeHelper.GetChildrenCount(b)).Select(i => func(VisualTreeHelper.GetChild(b, i))).SelectMany(a => a);
				};
				buttons = func(this);
			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}
		}

		/// <summary>
		/// クローズ時イベント
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void AnalyzeWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			// 解析実行中チェック
			if (pageProgress != null && IsRunning)
			{
				MessageBox.Show("処理が実行中です。\n終了するには、キャンセルボタンからキャンセルしてください。", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Warning);
				e.Cancel = true;
			}

			if (!e.Cancel)
			{
				pageAnalyzeDataIO = null;
				pageAnalyzeSelectArea = null;
				pageExecute = null;
				pageProgress = null;

				AnalyzeParam.Instance.Dispose();
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
				if (pageAnalyzeDataIO == null) pageAnalyzeDataIO = new PageAnalyzeDataIO();
				MainContentFrame.Navigate(pageAnalyzeDataIO);
			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void SelectAreaButton_Click(object sender, RoutedEventArgs e)
		{
			try
			{
				LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
				if (pageAnalyzeSelectArea == null) pageAnalyzeSelectArea = new PageAnalyzeSelectArea();
				MainContentFrame.Navigate(pageAnalyzeSelectArea);
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
				MainContentFrame.Navigate(pageExecute, MainContentFrame);
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

				// 更新
				(pageAnalyzeDataIO as PageAnalyzeDataIO).UpdateParams();
			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}
		}

		private void CustomSettingsButton_Click(object sender, RoutedEventArgs e)
		{
			try
			{
				LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
				var window = new CustomSettingsWindow();
				window.ShowDialog();
			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}
		}

		private void NextPageButton_Click(object sender, RoutedEventArgs e)
		{
			try
			{
				LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

				var pageType = MainContentFrame.Content as Page;
				if (pageType == pageAnalyzeDataIO)
				{
					if (pageAnalyzeSelectArea == null) pageAnalyzeSelectArea = new PageAnalyzeSelectArea();
					MainContentFrame.Navigate(pageAnalyzeSelectArea);
				}
				else if (pageType == pageAnalyzeSelectArea)
				{
					if (pageExecute == null) pageExecute = new PageCommonExecute();
					MainContentFrame.Navigate(pageExecute);
				}
				else if (pageType == pageExecute)
				{
					if (pageProgress == null) pageProgress = new PageCommonProgress();
					IsRunning = true;
					MainContentFrame.Navigate(pageProgress);
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
				if (pageType == pageAnalyzeDataIO)
				{
					// 何もしない
				}
				else if (pageType == pageAnalyzeSelectArea)
				{
					if (pageAnalyzeDataIO == null) pageAnalyzeDataIO = new PageAnalyzeDataIO();
					MainContentFrame.Navigate(pageAnalyzeDataIO);
				}
				else if (pageType == pageExecute)
				{
					if (pageAnalyzeSelectArea == null) pageAnalyzeSelectArea = new PageAnalyzeSelectArea();
					MainContentFrame.Navigate(pageAnalyzeSelectArea);
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

				if (pageType == pageAnalyzeDataIO)
				{
					if (!(pageType as PageAnalyzeDataIO).CheckParams())
					{
						e.Cancel = true;
					}
					else
					{
						// 3D都市モデル範囲を読み込む
						(pageType as PageAnalyzeDataIO).InitMapRangeFromMetadata();

						// 土地範囲指定データを読み込む
						(pageType as PageAnalyzeDataIO).ReadLandShp();
					}
				}
				else if (pageType == pageAnalyzeSelectArea)
				{
					if (!(pageType as PageAnalyzeSelectArea).CheckParams())
					{
						e.Cancel = true;
					}
					else
					{
						// 表示中の地図画像を保存する
						(pageType as PageAnalyzeSelectArea).SaveMapImage();
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
		#endregion

	}
}

