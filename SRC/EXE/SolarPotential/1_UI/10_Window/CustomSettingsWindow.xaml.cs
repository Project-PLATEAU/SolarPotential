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
	public partial class CustomSettingsWindow : Window
	{
		#region Pages
		/// <summary>
		/// 解析-発電ポテンシャル設定画面
		/// </summary>
		Page pageSolarPotentialParams = null;

		/// <summary>
		/// 解析-反射シミュレーション設定画面
		/// </summary>
		Page pageReflectionParams = null;

		///// <summary>
		///// 判定-適地判定設定画面
		///// </summary>
		//Page pageJudgeParams = null;
		#endregion

		public CustomSettingsWindow()
		{
			this.InitializeComponent();

#if DEBUG
			SettingFrame.NavigationUIVisibility = NavigationUIVisibility.Visible;
#endif

			// Events
			Loaded += CustomSettingsWindow_Loaded;
			Closing += CustomSettingsWindow_Closing;

			SettingFrame.LoadCompleted += (sender, args) => { UpdateControls(); };
		}

		/// <summary>
		/// 画面を更新する
		/// </summary>
		private void UpdateControls()
		{
			try
			{
				var pageType = SettingFrame.Content as Page;
				if (pageType == pageSolarPotentialParams)
				{
					// 色変更
					HighlightButton(SolarPotentialButton);
					NormalButton(ReflectionButton);
				}
				else if (pageType == pageReflectionParams)
				{
					// 色変更
					NormalButton(SolarPotentialButton);
					HighlightButton(ReflectionButton);
				}
				//else if (pageType == pageJudgeParams)
				//{
				//	// 色変更
				//	HighlightButton(JudgeButton);
				//}

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
		private void CustomSettingsWindow_Loaded(object sender, RoutedEventArgs e)
        {
			try
			{
				LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

				// タイトル表示
				TextHeaderTitle.Text = $"{CommonManager.TitleText} 詳細設定";

				switch (CommonManager.Instance.CurrentProcess)
				{
					case CommonManager.Process.Analyze:
						pageSolarPotentialParams = new PageSolarPotentialParams();
						pageReflectionParams = new PageReflectionParams();
						// ボタン表示
						SolarPotentialButton.Visibility = Visibility.Visible;
						ReflectionButton.Visibility = Visibility.Visible;
						//JudgeButton.Visibility = Visibility.Collapsed;
						BackButton.Content = $"元の画面に戻る";
						SettingFrame.Navigate(pageSolarPotentialParams);
						break;

					default:
						TextHeaderTitle.Text = "";
						throw new Exception("Initialize Error.\nTarget Process None.");
				}
			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}
		}

		private void CustomSettingsWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			pageSolarPotentialParams = null;
			pageReflectionParams = null;
			//pageJudgeParams = null;
		}

		private void BackButton_Click(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			// 条件を保存

			this.Close();
		}

		private void SolarPotentialButton_Click(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			if (pageSolarPotentialParams == null)
			{
				pageSolarPotentialParams = new PageSolarPotentialParams();
			}
			SettingFrame.Navigate(pageSolarPotentialParams);
		}

		private void ReflectionButton_Click(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			if (pageReflectionParams == null)
			{
				pageReflectionParams = new PageReflectionParams();
			}
			SettingFrame.Navigate(pageReflectionParams);
		}

		//private void JudgeButton_Click(object sender, RoutedEventArgs e)
		//{
		//	LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

		//	if (pageJudgeParams == null)
		//	{
		//		pageJudgeParams = new PageJudgeParams();
		//	}
		//	SettingFrame.Navigate(pageJudgeParams);
		//}

		/// <summary>
		/// 初期値に戻す
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void ParamInitializeButton_Click(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			if (MessageBox.Show("設定を初期値に戻します。\nよろしいですか？", CommonManager.TEXT_SYSTEM_CAPTION,
									MessageBoxButton.YesNo, MessageBoxImage.Question) == MessageBoxResult.Yes)
			{
				// 初期値に戻す
				var pageType = SettingFrame.Content as Page;
				if (pageType == pageSolarPotentialParams)
				{
					(pageType as PageSolarPotentialParams).InitControls();
				}
				else if (pageType == pageReflectionParams)
				{
					(pageType as PageReflectionParams).InitControls();
				}
				//else if (pageType == pageJudgeParams)
				//{
				//	(pageType as PageJudgeParams).InitControls();
				//}
			}
		}

		#endregion
	}
}
