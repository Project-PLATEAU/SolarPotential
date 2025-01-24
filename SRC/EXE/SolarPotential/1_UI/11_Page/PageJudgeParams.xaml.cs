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
	/// PageJudgeParams.xaml の相互作用ロジック
	/// </summary>
	public partial class PageJudgeParams : Page
	{
		/// <summary>
		/// 適地判定パラメータ
		/// </summary>
		JudgeParams Params = AggregateParam.Instance.Judge;

		public PageJudgeParams()
		{
			InitializeComponent();

			RestrictAreaDirectionCombo1.ItemsSource = CommonManager.Directions16;
			RestrictAreaDirectionCombo2.ItemsSource = CommonManager.Directions16;
			RestrictAreaDirectionCombo3.ItemsSource = CommonManager.Directions16;

			ShpDatumCombo1.ItemsSource = CommonManager.DatumTypeList;
			ShpDatumCombo2.ItemsSource = CommonManager.DatumTypeList;
			ShpDatumCombo3.ItemsSource = CommonManager.DatumTypeList;

			SetBindings();

			// Events
			Loaded += PageJudgeParams_Loaded;

			WoodCheckBox.Checked += (sender, e) => { Params.BuildStructureFlag |= JudgeParams.BuildStructures.Wood; };
			WoodCheckBox.Unchecked += (sender, e) => { Params.BuildStructureFlag &= ~JudgeParams.BuildStructures.Wood; };
			SteelReinforcedConcreteCheckBox.Checked += (sender, e) => { Params.BuildStructureFlag |= JudgeParams.BuildStructures.SteelReinforcedConcrete; };
			SteelReinforcedConcreteCheckBox.Unchecked += (sender, e) => { Params.BuildStructureFlag &= ~JudgeParams.BuildStructures.SteelReinforcedConcrete; };
			ReinforcedConcreteCheckBox.Checked += (sender, e) => { Params.BuildStructureFlag |= JudgeParams.BuildStructures.ReinforcedConcrete; };
			ReinforcedConcreteCheckBox.Unchecked += (sender, e) => { Params.BuildStructureFlag &= ~JudgeParams.BuildStructures.ReinforcedConcrete; };
			SteelCheckBox.Checked += (sender, e) => { Params.BuildStructureFlag |= JudgeParams.BuildStructures.Steel; };
			SteelCheckBox.Unchecked += (sender, e) => { Params.BuildStructureFlag &= ~JudgeParams.BuildStructures.Steel; };
			LightGaugeSteelCheckBox.Checked += (sender, e) => { Params.BuildStructureFlag |= JudgeParams.BuildStructures.LightGaugeSteel; };
			LightGaugeSteelCheckBox.Unchecked += (sender, e) => { Params.BuildStructureFlag &= ~JudgeParams.BuildStructures.LightGaugeSteel; };
			MasonryConstructionCheckBox.Checked += (sender, e) => { Params.BuildStructureFlag |= JudgeParams.BuildStructures.MasonryConstruction; };
			MasonryConstructionCheckBox.Unchecked += (sender, e) => { Params.BuildStructureFlag &= ~JudgeParams.BuildStructures.MasonryConstruction; };
			UnknownCheckBox.Checked += (sender, e) => { Params.BuildStructureFlag |= JudgeParams.BuildStructures.Unknown; };
			UnknownCheckBox.Unchecked += (sender, e) => { Params.BuildStructureFlag &= ~JudgeParams.BuildStructures.Unknown; };
			NonWoodCheckBox.Checked += (sender, e) => { Params.BuildStructureFlag |= JudgeParams.BuildStructures.NonWood; };
			NonWoodCheckBox.Unchecked += (sender, e) => { Params.BuildStructureFlag &= ~JudgeParams.BuildStructures.NonWood; };

			if (Params.RestrictAreas.Count() == 3)
			{
				RestrictAreaCheckBox1.Checked += (sender, e) => { Params.RestrictAreas[0].Enable = true; };
				RestrictAreaCheckBox1.Unchecked += (sender, e) => { Params.RestrictAreas[0].Enable = false; };
				RestrictAreaPath1.LostFocus += (sender, e) => { Params.RestrictAreas[0].ShapePath = RestrictAreaPath1.Text; };
				RestrictHeight1.LostFocus += (sender, e) => { Params.RestrictAreas[0].Height = RestrictHeight1.Text; };
				RestrictAreaDirectionCombo1.SelectionChanged += (sender, e) => { Params.RestrictAreas[0].Direction = RestrictAreaDirectionCombo1.SelectedIndex; };
				ShpDatumCombo1.SelectionChanged += (sender, e) => { Params.RestrictAreas[0].Datum = ShpDatumCombo1.SelectedIndex; };

				RestrictAreaCheckBox2.Checked += (sender, e) => { Params.RestrictAreas[1].Enable = true; };
				RestrictAreaCheckBox2.Unchecked += (sender, e) => { Params.RestrictAreas[1].Enable = false; };
				RestrictAreaPath2.LostFocus += (sender, e) => { Params.RestrictAreas[1].ShapePath = RestrictAreaPath2.Text; };
				RestrictHeight2.LostFocus += (sender, e) => { Params.RestrictAreas[1].Height = RestrictHeight2.Text; };
				RestrictAreaDirectionCombo2.SelectionChanged += (sender, e) => { Params.RestrictAreas[1].Direction = RestrictAreaDirectionCombo2.SelectedIndex; };
				ShpDatumCombo2.SelectionChanged += (sender, e) => { Params.RestrictAreas[1].Datum = ShpDatumCombo2.SelectedIndex; };

				RestrictAreaCheckBox3.Checked += (sender, e) => { Params.RestrictAreas[2].Enable = true; };
				RestrictAreaCheckBox3.Unchecked += (sender, e) => { Params.RestrictAreas[2].Enable = false; };
				RestrictAreaPath3.LostFocus += (sender, e) => { Params.RestrictAreas[2].ShapePath = RestrictAreaPath3.Text; };
				RestrictHeight3.LostFocus += (sender, e) => { Params.RestrictAreas[2].Height = RestrictHeight3.Text; };
				RestrictAreaDirectionCombo3.SelectionChanged += (sender, e) => { Params.RestrictAreas[2].Direction = RestrictAreaDirectionCombo3.SelectedIndex; };
				ShpDatumCombo3.SelectionChanged += (sender, e) => { Params.RestrictAreas[2].Datum = ShpDatumCombo3.SelectedIndex; };
			}
		}

		/// <summary>
		/// 画面の初期化
		/// </summary>
		public void InitControls()
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
			LogManager.Instance.OutputLogMessage("パラメータを初期化します。");

			if (AggregateParam.Instance.InitJudgeParam())
			{
				ParamToControls();
			}
		}

		/// <summary>
		/// パラメータをコントロールに反映する(バインディング定義外)
		/// </summary>
		private void ParamToControls()
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			if (Params.BuildStructure)
			{
				WoodCheckBox.IsChecked = Params.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.Wood);
				SteelReinforcedConcreteCheckBox.IsChecked = Params.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.SteelReinforcedConcrete);
				ReinforcedConcreteCheckBox.IsChecked = Params.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.ReinforcedConcrete);
				SteelCheckBox.IsChecked = Params.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.Steel);
				LightGaugeSteelCheckBox.IsChecked = Params.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.LightGaugeSteel);
				MasonryConstructionCheckBox.IsChecked = Params.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.MasonryConstruction);
				UnknownCheckBox.IsChecked = Params.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.Unknown);
				NonWoodCheckBox.IsChecked = Params.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.NonWood);
			}

			for (int n = 0; n < 3; n++)
			{
				var cb = FindName($"RestrictAreaCheckBox{n + 1}") as CheckBox;
				cb.IsChecked = Params.RestrictAreas[n].Enable;
				if (Params.RestrictAreas[n].Enable)
				{
					var tb = FindName($"RestrictAreaPath{n + 1}") as TextBox;
					tb.Text = Params.RestrictAreas[n].ShapePath;
					tb = FindName($"RestrictHeight{n + 1}") as TextBox;
					tb.Text = Params.RestrictAreas[n].Height;
					var cmb1 = FindName($"RestrictAreaDirectionCombo{n + 1}") as ComboBox;
					cmb1.SelectedIndex = Params.RestrictAreas[n].Direction;
					var cmb2 = FindName($"ShpDatumCombo{n + 1}") as ComboBox;
					cmb2.SelectedIndex = Params.RestrictAreas[n].Datum;
				}
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

				var binding = new Binding(nameof(AggregateParam.Instance.Judge.LowerPotential))
				{
					Source = AggregateParam.Instance.Judge,
					Mode = BindingMode.TwoWay,
				};
				LowerPotentialCheckBox.SetBinding(CheckBox.IsCheckedProperty, binding);

				binding = new Binding(nameof(AggregateParam.Instance.Judge.Potential))
				{
					Source = AggregateParam.Instance.Judge,
					Mode = BindingMode.TwoWay,
				};
				PotentialCheckBox.SetBinding(CheckBox.IsCheckedProperty, binding);

				binding = new Binding(nameof(AggregateParam.Instance.Judge.PotentialVal))
				{
					Source = AggregateParam.Instance.Judge,
					Mode = BindingMode.TwoWay,
				};
				PotentialVal.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AggregateParam.Instance.Judge.PotentialPercent))
				{
					Source = AggregateParam.Instance.Judge,
					Mode = BindingMode.TwoWay,
				};
				LowerPercentCheckBox.SetBinding(CheckBox.IsCheckedProperty, binding);

				binding = new Binding(nameof(AggregateParam.Instance.Judge.PotentialPercentVal))
				{
					Source = AggregateParam.Instance.Judge,
					Mode = BindingMode.TwoWay,
				};
				PotentialPercentVal.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AggregateParam.Instance.Judge.BuildStructure))
				{
					Source = AggregateParam.Instance.Judge,
					Mode = BindingMode.TwoWay,
				};
				BuildStructuresCheckBox.SetBinding(CheckBox.IsCheckedProperty, binding);

				binding = new Binding(nameof(AggregateParam.Instance.Judge.BuildFloors))
				{
					Source = AggregateParam.Instance.Judge,
					Mode = BindingMode.TwoWay,
				};
				BuildFloorCheckBox.SetBinding(CheckBox.IsCheckedProperty, binding);

				binding = new Binding(nameof(AggregateParam.Instance.Judge.FloorsBelowVal))
				{
					Source = AggregateParam.Instance.Judge,
					Mode = BindingMode.TwoWay,
				};
				FloorsBelowVal.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AggregateParam.Instance.Judge.UpperFloorsVal))
				{
					Source = AggregateParam.Instance.Judge,
					Mode = BindingMode.TwoWay,
				};
				UpperFloorsVal.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AggregateParam.Instance.Judge.BelowTsunamiHeight))
				{
					Source = AggregateParam.Instance.Judge,
					Mode = BindingMode.TwoWay,
				};
				BelowTsunamiHeightCheckBox.SetBinding(CheckBox.IsCheckedProperty, binding);

				binding = new Binding(nameof(AggregateParam.Instance.Judge.BelowFloodDepth))
				{
					Source = AggregateParam.Instance.Judge,
					Mode = BindingMode.TwoWay,
				};
				BelowFloodDepthCheckBox.SetBinding(CheckBox.IsCheckedProperty, binding);

				binding = new Binding(nameof(AggregateParam.Instance.Judge.LandslideWarningArea))
				{
					Source = AggregateParam.Instance.Judge,
					Mode = BindingMode.TwoWay,
				};
				LandslideWarningAreaCheckBox.SetBinding(CheckBox.IsCheckedProperty, binding);

				binding = new Binding(nameof(AggregateParam.Instance.Judge.WeatherData))
				{
					Source = AggregateParam.Instance.Judge,
					Mode = BindingMode.TwoWay,
				};
				WeatherDataCheckBox.SetBinding(CheckBox.IsCheckedProperty, binding);

				binding = new Binding(nameof(AggregateParam.Instance.Judge.WeatherDataPath))
				{
					Source = AggregateParam.Instance.Judge,
					Mode = BindingMode.TwoWay,
				};
				WeatherDataPath.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AggregateParam.Instance.Judge.UseSnowDepth))
				{
					Source = AggregateParam.Instance.Judge,
					Mode = BindingMode.TwoWay,
				};
				SnowDepthRadio.SetBinding(CheckBox.IsCheckedProperty, binding);

				binding = new Binding(nameof(AggregateParam.Instance.Judge.SnowDepthVal))
				{
					Source = AggregateParam.Instance.Judge,
					Mode = BindingMode.TwoWay,
				};
				SnowDepthVal.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AggregateParam.Instance.Judge.SnowLoadVal))
				{
					Source = AggregateParam.Instance.Judge,
					Mode = BindingMode.TwoWay,
				};
				SnowLoadVal.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AggregateParam.Instance.Judge.SnowLoadUnitVal))
				{
					Source = AggregateParam.Instance.Judge,
					Mode = BindingMode.TwoWay,
				};
				SnowLoadUnitVal.SetBinding(TextBox.TextProperty, binding);

			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}
		}

		/// <summary>
		/// 入力値チェック
		/// </summary>
		/// <returns></returns>
		public bool CheckParams()
		{
			List<string> ErrorParams = new List<string>();

			// 変換確認用
			int n;
			double d;

			if (Params.Potential && !double.TryParse(Params.PotentialVal, out d) ||
				(Params.PotentialPercent && !int.TryParse(Params.PotentialPercentVal, out n)))
			{
				LogManager.Instance.OutputLogMessage($"!!入力エラー!! 日射量のしきい値が未入力", LogManager.LogType.INFO);
				ErrorParams.Add("日射量の数値が入力されていません。");
			}
			if (Params.BuildStructure && Params.BuildStructureFlag == JudgeParams.BuildStructures.None)
			{
				LogManager.Instance.OutputLogMessage($"!!入力エラー!! 建物構造の選択エラー", LogManager.LogType.INFO);
				ErrorParams.Add("建物構造が選択されていません。");
			}
			if (Params.BuildFloors && !int.TryParse(Params.FloorsBelowVal, out n) && !int.TryParse(Params.UpperFloorsVal, out n))
			{
				LogManager.Instance.OutputLogMessage($"!!入力エラー!! 階数が未入力", LogManager.LogType.INFO);
				ErrorParams.Add("除外する階数が入力されていません。");
			}
			if (Params.WeatherData)
			{
				if (string.IsNullOrEmpty(Params.WeatherDataPath) || !System.IO.File.Exists(Params.WeatherDataPath))
				{
					LogManager.Instance.OutputLogMessage($"!!入力エラー!! 気象データが未入力", LogManager.LogType.INFO);
					ErrorParams.Add("気象データのシェープファイルが入力されていません。");
				}
				if ((Params.UseSnowDepth && !double.TryParse(Params.SnowDepthVal, out d)) ||
					(!Params.UseSnowDepth && (!double.TryParse(Params.SnowLoadVal, out d) || !double.TryParse(Params.SnowLoadUnitVal, out d))))
				{
					LogManager.Instance.OutputLogMessage($"!!入力エラー!! 積雪のしきい値が未入力", LogManager.LogType.INFO);
					ErrorParams.Add("積雪のしきい値入力に不備があります。");
				}
			}
			for (int i = 0; i < 3; i++)
			{
				if (Params.RestrictAreas[i].Enable)
				{
					if (string.IsNullOrEmpty(Params.RestrictAreas[i].ShapePath) ||
						!System.IO.File.Exists(Params.RestrictAreas[i].ShapePath) ||
						Params.RestrictAreas[i].Direction < 0 ||
						!double.TryParse(Params.RestrictAreas[i].Height, out d) ||
						Params.RestrictAreas[i].Datum < 0)
					{
						LogManager.Instance.OutputLogMessage($"!!入力エラー!! 制限範囲シェープファイル入力エラー", LogManager.LogType.INFO);
						ErrorParams.Add("制限範囲の設定に不備があります。");
						break;
					}
				}
			}

			if (ErrorParams.Count() > 0)
			{
				string msg = "入力内容に不備があります。\n";
				foreach(var str in ErrorParams)
				{
					msg += $"・{str}\n";
				}
				MessageBox.Show(msg, CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Warning);
				return false;
			}

			return true;

		}

		private void PageJudgeParams_Loaded(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			if (Params == null)
			{
				InitControls();
			}
			else
			{
				ParamToControls();
			}
		}

		private void RestrictAreaButton1_Click(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			string file = RestrictAreaPath1.Text;
			CommonManager.Instance.ShowSelectFileDialog(out string path, "制限を設ける範囲のシェープファイルを選択します。", file, CommonManager.EXT_FILTER_SHP);
			if (string.IsNullOrEmpty(path)) return;
			RestrictAreaPath1.Text = path;
			Params.RestrictAreas[0].ShapePath = path;
			LogManager.Instance.OutputLogMessage($"制限を設ける範囲のシェープファイル(1)：{path}", LogManager.LogType.INFO);

		}

		private void RestrictAreaButton2_Click(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			string file = RestrictAreaPath2.Text;
			CommonManager.Instance.ShowSelectFileDialog(out string path, "制限を設ける範囲のシェープファイルを選択します。", file, CommonManager.EXT_FILTER_SHP);
			if (string.IsNullOrEmpty(path)) return;
			RestrictAreaPath2.Text = path;
			Params.RestrictAreas[1].ShapePath = path;
			LogManager.Instance.OutputLogMessage($"制限を設ける範囲のシェープファイル(2)：{path}", LogManager.LogType.INFO);

		}

		private void RestrictAreaButton3_Click(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			string file = RestrictAreaPath3.Text;
			CommonManager.Instance.ShowSelectFileDialog(out string path, "制限を設ける範囲のシェープファイルを選択します。", file, CommonManager.EXT_FILTER_SHP);
			if (string.IsNullOrEmpty(path)) return;
			RestrictAreaPath3.Text = path;
			Params.RestrictAreas[2].ShapePath = path;
			LogManager.Instance.OutputLogMessage($"制限を設ける範囲のシェープファイル(3)：{path}", LogManager.LogType.INFO);

		}

		private void WeatherDataPathButton_Click(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			string file = WeatherDataPath.Text;
			CommonManager.Instance.ShowSelectFileDialog(out string path, "気象データのシェープファイルを選択します。", file, CommonManager.EXT_FILTER_SHP);
			if (string.IsNullOrEmpty(path)) return;
			Params.WeatherDataPath = path;
			LogManager.Instance.OutputLogMessage($"気象データのシェープファイル：{path}", LogManager.LogType.INFO);

		}
	}
}
