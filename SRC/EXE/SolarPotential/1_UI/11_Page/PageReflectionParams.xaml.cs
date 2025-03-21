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
	public partial class PageReflectionParams : Page
	{
		/// <summary>
		/// 発電ポテンシャル推計パラメータ
		/// </summary>
		ReflectionParams Params = AnalyzeParam.Instance.Reflection;

		public PageReflectionParams()
		{
			InitializeComponent();

			SetBindings();

			RoofLowerDirectionCombo.ItemsSource = CommonManager.Directions4;
			RoofUpperDirectionCombo.ItemsSource = CommonManager.Directions4;
			LandLowerDirectionCombo.ItemsSource = CommonManager.Directions4;
			LandUpperDirectionCombo.ItemsSource = CommonManager.Directions4;

			// Events
			Loaded += PageReflectionParams_Loaded;

			RoofLowerCustomRadio.Checked += (sender, e) => {
				Params.Roof[ReflectionParams.SlopeConditions.Lower].UseCustomVal = true;
			};
			RoofLowerCustomRadio.Unchecked += (sender, e) => {
				Params.Roof[ReflectionParams.SlopeConditions.Lower].UseCustomVal = false;
			};

			RoofUpperCustomRadio.Checked += (sender, e) => {
				Params.Roof[ReflectionParams.SlopeConditions.Upper].UseCustomVal = true;
			};
			RoofUpperCustomRadio.Unchecked += (sender, e) => {
				Params.Roof[ReflectionParams.SlopeConditions.Upper].UseCustomVal = false;
			};

			LandLowerCustomRadio.Checked += (sender, e) => {
				Params.Land[ReflectionParams.SlopeConditions.Lower].UseCustomVal = true;
			};
			LandLowerCustomRadio.Unchecked += (sender, e) => {
				Params.Land[ReflectionParams.SlopeConditions.Lower].UseCustomVal = false;
			};

			LandUpperCustomRadio.Checked += (sender, e) => {
				Params.Land[ReflectionParams.SlopeConditions.Upper].UseCustomVal = true;
			};
			LandUpperCustomRadio.Unchecked += (sender, e) => {
				Params.Land[ReflectionParams.SlopeConditions.Upper].UseCustomVal = false;
			};

		}

		/// <summary>
		/// 画面の初期化
		/// </summary>
		public void InitControls()
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
			LogManager.Instance.OutputLogMessage("パラメータを初期化します。");

			if (!AnalyzeParam.Instance.InitReflectionParam())
			{

			}
		}

		/// <summary>
		/// バインディング定義
		/// </summary>
		private void SetBindings()
		{
			try
			{
				LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

				var lowerRoofParam = Params.Roof[ReflectionParams.SlopeConditions.Lower];
				var upperRoofParam = Params.Roof[ReflectionParams.SlopeConditions.Upper];
				var lowerLandParam = Params.Land[ReflectionParams.SlopeConditions.Lower];
				var upperLandParam = Params.Land[ReflectionParams.SlopeConditions.Upper];

				var binding = new Binding(nameof(lowerRoofParam.DirDeg.Direction))
				{
					Source = lowerRoofParam.DirDeg,
					Mode = BindingMode.TwoWay,
				};
				RoofLowerDirectionCombo.SetBinding(ComboBox.SelectedIndexProperty, binding);

				binding = new Binding(nameof(lowerRoofParam.DirDeg.Degree))
				{
					Source = lowerRoofParam.DirDeg,
					Mode = BindingMode.TwoWay,
				};
				RoofLowerDegree.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(upperRoofParam.DirDeg.Direction))
				{
					Source = upperRoofParam.DirDeg,
					Mode = BindingMode.TwoWay,
				};
				RoofUpperDirectionCombo.SetBinding(ComboBox.SelectedIndexProperty, binding);

				binding = new Binding(nameof(upperRoofParam.DirDeg.Degree))
				{
					Source = upperRoofParam.DirDeg,
					Mode = BindingMode.TwoWay,
				};
				RoofUpperDegree.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(lowerLandParam.DirDeg.Direction))
				{
					Source = lowerLandParam.DirDeg,
					Mode = BindingMode.TwoWay,
				};
				LandLowerDirectionCombo.SetBinding(ComboBox.SelectedIndexProperty, binding);

				binding = new Binding(nameof(lowerLandParam.DirDeg.Degree))
				{
					Source = lowerLandParam.DirDeg,
					Mode = BindingMode.TwoWay,
				};
				LandLowerDegree.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(upperLandParam.DirDeg.Direction))
				{
					Source = upperLandParam.DirDeg,
					Mode = BindingMode.TwoWay,
				};
				LandUpperDirectionCombo.SetBinding(ComboBox.SelectedIndexProperty, binding);

				binding = new Binding(nameof(upperLandParam.DirDeg.Degree))
				{
					Source = upperLandParam.DirDeg,
					Mode = BindingMode.TwoWay,
				};
				LandUpperDegree.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(Params.ReflectionRange))
				{
					Source = Params,
					Mode = BindingMode.TwoWay,
				};
				ReflectionRange.SetBinding(TextBox.TextProperty, binding);

			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}
		}

		private void PageReflectionParams_Loaded(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			RoofLowerRadio.IsChecked = !Params.Roof[ReflectionParams.SlopeConditions.Lower].UseCustomVal;
			RoofUpperRadio.IsChecked = !Params.Roof[ReflectionParams.SlopeConditions.Upper].UseCustomVal;
			LandLowerRadio.IsChecked = !Params.Land[ReflectionParams.SlopeConditions.Lower].UseCustomVal;
			LandUpperRadio.IsChecked = !Params.Land[ReflectionParams.SlopeConditions.Upper].UseCustomVal;
			RoofLowerCustomRadio.IsChecked = Params.Roof[ReflectionParams.SlopeConditions.Lower].UseCustomVal;
			RoofUpperCustomRadio.IsChecked = Params.Roof[ReflectionParams.SlopeConditions.Upper].UseCustomVal;
			LandLowerCustomRadio.IsChecked = Params.Land[ReflectionParams.SlopeConditions.Lower].UseCustomVal;
			LandUpperCustomRadio.IsChecked = Params.Land[ReflectionParams.SlopeConditions.Upper].UseCustomVal;

		}

	}
}
