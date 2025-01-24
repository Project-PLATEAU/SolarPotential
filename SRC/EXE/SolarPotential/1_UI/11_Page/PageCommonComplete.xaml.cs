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
	/// Page1.xaml の相互作用ロジック
	/// </summary>
	public partial class PageCommonComplete : Page
	{
		/// <summary>
		/// 出力フォルダ
		/// </summary>
		public string OutputDirectory { get; set; } = "";

		public PageCommonComplete()
		{
			InitializeComponent();

			Loaded += PageCommonComplete_Loaded;
		}

		private void PageCommonComplete_Loaded(object sender, RoutedEventArgs e)
		{
			try
			{
				LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

				switch (CommonManager.Instance.CurrentProcess)
				{
					case CommonManager.Process.Analyze:
						TextMessage.Text = "解析処理が完了しました。";

						// 日射量推計実行時は適地判定画面の遷移ボタンを表示する
						AggregateButton.Visibility = AnalyzeParam.Instance.Target.SolarPotential ? Visibility.Visible : Visibility.Collapsed;

						break;

					case CommonManager.Process.Aggregate:
						TextMessage.Text = "集計処理が完了しました。";
						AggregateButton.Visibility = Visibility.Collapsed;
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

		private void OpenResultButton_Click(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			// 結果フォルダを開く
			if (System.IO.Directory.Exists(OutputDirectory))
			{
				System.Diagnostics.Process.Start(OutputDirectory);
			}
			else
			{
				MessageBox.Show("出力結果フォルダが見つかりませんでした。\n" +
								"ログを確認してください。", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Error);
			}
		}

		private void AggregateButton_Click(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			// 解放処理？

			MainWindow.ShowAggregateWindow(OutputDirectory);
		}
	}
}
