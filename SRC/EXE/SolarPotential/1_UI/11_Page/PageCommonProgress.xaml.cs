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
using System.Threading;

namespace SolarPotential
{
	/// <summary>
	/// Page1.xaml の相互作用ロジック
	/// </summary>
	public partial class PageCommonProgress : Page
	{
		/// <summary>
		/// メインフレーム
		/// </summary>
		public Frame mainContentFrame { get; set; } = null;

		/// <summary>
		/// 解析処理実行クラス
		/// </summary>
		_3_Class.ExecuteAnalyze Analyze;

		/// <summary>
		/// 集計・判定処理実行クラス
		/// </summary>
		_3_Class.ExecuteAggregate Aggregate;

		public PageCommonProgress()
		{
			InitializeComponent();

			Loaded += PageCommonProgress_Loaded;
			Unloaded += (sender, e) => { Dispose(); };

		}

		/// <summary>
		/// プロセス終了処理
		/// </summary>
		private void ProcessEnd(bool complete)
		{
			PageCommonComplete page = new PageCommonComplete();
			switch (CommonManager.Instance.CurrentProcess)
			{
				case CommonManager.Process.Analyze:
					page.OutputDirectory = Analyze.OutputDirectory;
					break;

				case CommonManager.Process.Aggregate:
					page.OutputDirectory = Aggregate.OutputDirectory;
					break;

				default:
					throw new Exception("Initialize Error.\nTarget Process None.");
			}

			if (complete)
			{
				mainContentFrame?.Navigate(page, mainContentFrame);
			}
			else
			{
				mainContentFrame?.GoBack();
			}
			
		}

		/// <summary>
		/// キャンセル処理
		/// </summary>
		public void ProcessCancel()
		{
			var result = MessageBox.Show("実行中の処理をキャンセルしますか？", "キャンセル確認", MessageBoxButton.YesNo, MessageBoxImage.Question);
			if (result == MessageBoxResult.Yes)
			{
				progressBar.IsIndeterminate = true;

				// キャンセル処理
				switch (CommonManager.Instance.CurrentProcess)
				{
					case CommonManager.Process.Analyze:
						Analyze.TaskCanceler?.Cancel();
						Analyze.NoticeCancelComplete += () =>
						{
							progressBar.IsIndeterminate = false;
							mainContentFrame?.GoBack();
						};
						break;

					case CommonManager.Process.Aggregate:
						Aggregate.TaskCanceler?.Cancel();
						Aggregate.NoticeCancelComplete += () =>
						{
							progressBar.IsIndeterminate = false;
							mainContentFrame?.GoBack();
						};
						break;

					default:
						throw new Exception("Initialize Error.\nTarget Process None.");
				}

			}
		}

		/// <summary>
		/// 解放処理
		/// </summary>
		public void Dispose()
		{
			if (Analyze != null)
			{
				Analyze.Dispose();
				Analyze = null;
			}

			if (Aggregate != null)
			{
				Aggregate.Dispose();
				Aggregate = null;
			}

		}

		private void PageCommonProgress_Loaded(object sender, RoutedEventArgs e)
		{
			try
			{
				LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

				// 初期化
				progressBar.Minimum = 0; 
				progressBar.Maximum = 100;

				switch (CommonManager.Instance.CurrentProcess)
				{
					case CommonManager.Process.Analyze:
						Analyze = new _3_Class.ExecuteAnalyze();
						Analyze.NoticeUpdateProgressBar += (val) => { progressBar.Value = val; };
						Analyze.NoticeUpdateProgressText += (text) => { textProgress.Text = text; };
						_ = Task.Run(() => Analyze.ProcessStart(Dispatcher));
						Analyze.NoticeAnalyzeComplete += (flag) => ProcessEnd(flag);
						break;

					case CommonManager.Process.Aggregate:
						Aggregate = new _3_Class.ExecuteAggregate();
						Aggregate.NoticeUpdateProgressBar += (val) => { progressBar.Value = val; };
						Aggregate.NoticeUpdateProgressText += (str) => { textProgress.Text = str; };
						_ = Task.Run(() => Aggregate.ProcessStart(Dispatcher));
						Aggregate.NoticeAggregateComplete += (flag) => ProcessEnd(flag);
						break;

					default:
						throw new Exception("Initialize Error.\nTarget Process None.");
				}
			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}
		}

		/// <summary>
		/// キャンセルボタン押下
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void CancelButton_Click(object sender, RoutedEventArgs e)
		{
			try
			{
				LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
				ProcessCancel();
			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}
		}
	}
}
