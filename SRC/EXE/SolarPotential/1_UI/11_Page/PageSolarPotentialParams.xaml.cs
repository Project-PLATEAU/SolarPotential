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
using System.IO;

namespace SolarPotential
{
	/// <summary>
	/// Page1.xaml の相互作用ロジック
	/// </summary>
	public partial class PageSolarPotentialParams : Page
	{
		/// <summary>
		/// 発電ポテンシャル推計パラメータ
		/// </summary>
		SolarPotentialParams Params = AnalyzeParam.Instance.SolarPotential;

		/// <summary>
		/// メーカー別発電容量情報
		/// </summary>
		public class PanelMakerInfo
		{
			/// <summary>
			/// メーカー名
			/// </summary>
			public string MakerName { get; set; } = "";

			/// <summary>
			/// 単位面積当たりの発電容量
			/// </summary>
			public string PotentialVal { get; set; } = "";

		}

		/// <summary>
		/// 発電容量の平均値情報
		/// </summary>
		PanelMakerInfo DefaultPanelMakerInfo { get; } = new PanelMakerInfo { MakerName = "デフォルト（平均値）", PotentialVal = "0.167" };

		/// <summary>
		/// メーカー別発電容量リスト
		/// </summary>
		List<PanelMakerInfo> PanelMakerList { get; set; } = new List<PanelMakerInfo>();

		/// <summary>
		/// 発電容量情報ファイルパス
		/// </summary>
		static string PanelMakerInfoFilePath { get; } = CommonManager.AssetsDirectory + "/PanelMakerInfo.csv";

		/// <summary>
		/// 発電容量情報ファイルの列数
		/// </summary>
		const int NUM_MAKERINFO_COLUMN = 5;


		public PageSolarPotentialParams()
		{
			InitializeComponent();

			SetBindings();

			RoofDirectionCombo.ItemsSource = CommonManager.Directions4;
			RoofCorrectDirectionCombo.ItemsSource = CommonManager.Directions4;
			LandCorrectDirectionCombo.ItemsSource = CommonManager.Directions4;

			Loaded += PageSolarPotentialParams_Loaded;
		}

		/// <summary>
		/// 画面の初期化
		/// </summary>
		public void InitControls()
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
			LogManager.Instance.OutputLogMessage("パラメータを初期化します。");

			// パラメータの初期化
			AnalyzeParam.Instance.InitSolarPotentialParam();

		}

		/// <summary>
		/// バインディング定義
		/// </summary>
		private void SetBindings()
		{
			try
			{
				LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

				var binding = new Binding(nameof(AnalyzeParam.Instance.SolarPotential.Roof.Area))
				{
					Source = AnalyzeParam.Instance.SolarPotential.Roof,
					Mode = BindingMode.TwoWay,
				};
				RoofArea.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.Instance.SolarPotential.Roof.SlopeDegree))
				{
					Source = AnalyzeParam.Instance.SolarPotential.Roof,
					Mode = BindingMode.TwoWay,
				};
				RoofDegree.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.Instance.SolarPotential.Roof.DirDeg.Direction))
				{
					Source = AnalyzeParam.Instance.SolarPotential.Roof.DirDeg,
					Mode = BindingMode.TwoWay,
				};
				RoofDirectionCombo.SetBinding(ComboBox.SelectedIndexProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.Instance.SolarPotential.Roof.DirDeg.Degree))
				{
					Source = AnalyzeParam.Instance.SolarPotential.Roof.DirDeg,
					Mode = BindingMode.TwoWay,
				};
				RoofDirectionDegree.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.Instance.SolarPotential.Roof.Interior))
				{
					Source = AnalyzeParam.Instance.SolarPotential.Roof,
					Mode = BindingMode.TwoWay,
				};
				RoofInteriorCheck.SetBinding(CheckBox.IsCheckedProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.Instance.SolarPotential.Roof.CorrectionCaseDeg))
				{
					Source = AnalyzeParam.Instance.SolarPotential.Roof,
					Mode = BindingMode.TwoWay,
				};
				RoofCorrectionCaseDegree.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.Instance.SolarPotential.Roof.CorrectionDirDeg.Direction))
				{
					Source = AnalyzeParam.Instance.SolarPotential.Roof.CorrectionDirDeg,
					Mode = BindingMode.TwoWay,
				};
				RoofCorrectDirectionCombo.SetBinding(ComboBox.SelectedIndexProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.Instance.SolarPotential.Roof.CorrectionDirDeg.Degree))
				{
					Source = AnalyzeParam.Instance.SolarPotential.Roof.CorrectionDirDeg,
					Mode = BindingMode.TwoWay,
				};
				RoofCorrectDegree.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.Instance.SolarPotential.Land.Area))
				{
					Source = AnalyzeParam.Instance.SolarPotential.Land,
					Mode = BindingMode.TwoWay,
				};
				LandArea.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.Instance.SolarPotential.Land.SlopeDegree))
				{
					Source = AnalyzeParam.Instance.SolarPotential.Land,
					Mode = BindingMode.TwoWay,
				};
				LandDegree.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.Instance.SolarPotential.Land.CorrectionDirDeg.Direction))
				{
					Source = AnalyzeParam.Instance.SolarPotential.Land.CorrectionDirDeg,
					Mode = BindingMode.TwoWay,
				};
				LandCorrectDirectionCombo.SetBinding(ComboBox.SelectedIndexProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.Instance.SolarPotential.Land.CorrectionDirDeg.Degree))
				{
					Source = AnalyzeParam.Instance.SolarPotential.Land.CorrectionDirDeg,
					Mode = BindingMode.TwoWay,
				};
				LandCorrectDegree.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.Instance.SolarPotential.PanelMakerSolarPower))
				{
					Source = AnalyzeParam.Instance.SolarPotential,
					Mode = BindingMode.TwoWay,
				};
				PanelMaker.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.Instance.SolarPotential.PanelRatio))
				{
					Source = AnalyzeParam.Instance.SolarPotential,
					Mode = BindingMode.TwoWay,
				};
				PanelRatio.SetBinding(TextBox.TextProperty, binding);

			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}
		}

		private void PageSolarPotentialParams_Loaded(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

		}

	}
}
