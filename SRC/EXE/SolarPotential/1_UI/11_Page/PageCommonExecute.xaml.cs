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
	public partial class PageCommonExecute : Page
	{
		public PageCommonExecute()
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
						MessageText.Text = "以下の条件で解析・シミュレーションを実行します。";
						ShowAnalyzeParams();
						break;

					case CommonManager.Process.Aggregate:
						MessageText.Text = "以下の条件で適地判定・集計を実行します。";
						ShowAggregateParams();
						break;

					default:
						throw new Exception("Initialize Error.\nTarget Process None.");
				}
			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);

			}
		}

		/// <summary>
		/// 解析パラメータの表示
		/// </summary>
		private void ShowAnalyzeParams()
		{
            var p = AnalyzeParam.Instance;

			string str = "";     // 一時格納用

			// 解析対象
			AnalyzeSettings.Visibility = Visibility.Visible;
			{
				str = p.Target.SolarPotential ? "日射量・発電量推計、" : "";
				str += p.Target.Reflection ? "反射シミュレーション、" : "";
				str = str.Remove(str.Length - 1);
				AnalyzeType.Text = str;
				str = p.Target.Build ? "建物、" : "";
				str += p.Target.Land ? "土地、" : "";
				str = str.Remove(str.Length - 1);
				AnalyzeTarget.Text = str;
				if (p.TargetDate == AnalyzeParam.DateType.OneMonth) { str = $"{p.Month}月"; }
				else if (p.TargetDate == AnalyzeParam.DateType.OneDay) { str = $"{p.Month}月 {p.Day}日"; }
				else if (p.TargetDate == AnalyzeParam.DateType.Year) { str = $"年間"; }
				else {
					str = "";
					if (p.TargetDate.HasFlag(AnalyzeParam.DateType.Summer)) str += "夏至、";
					if (p.TargetDate.HasFlag(AnalyzeParam.DateType.Winter)) str += "冬至、";
					str = str.Remove(str.Length - 1);
				}
				AnalyzeDate.Text = str;
			}

			// 入力データ
			{
				InputDataGrid.Children.Clear();
				AddItem(InputDataGrid, "3D都市モデル", p.InputData.CityModel);
				AddItem(InputDataGrid, "可照時間", p.InputData.KashoData);
				AddItem(InputDataGrid, "平均日照時間", p.InputData.NisshoData);
				if (!string.IsNullOrEmpty(p.InputData.SnowDepthData))
				{
					AddItem(InputDataGrid, "積雪深", p.InputData.SnowDepthData);
				}
				if (p.Target.Land && !string.IsNullOrEmpty(p.InputData.LandData))
				{
					AddItem(InputDataGrid, "土地範囲指定データ", p.InputData.LandData);
				}
				if (p.InputData.UseDemData)
				{
					AddItem(InputDataGrid, "", "※ 地形を考慮したシミュレーションを行う");
				}
				if (p.Target.Land && p.InputData.UseRoadData)
				{
					AddItem(InputDataGrid, "", "※ 道路を除外したシミュレーションを行う");
				}
			}

			// 解析エリア
			AreaListPanel.Visibility = Visibility.Visible;
			ListSelectArea.ItemsSource = AnalyzeParam.Instance.AreaList;

			// 出力設定
			OutputDataGrid.Children.Clear();
			AddItem(OutputDataGrid, "出力フォルダ", p.OutputResultDirectory);

			// 詳細設定
			ParamsGrid.Children.Clear();
			// 発電ポテンシャル推計
			{
				if (p.Target.Build)
				{
					str = $"面積 {p.SolarPotential.Roof.Area} m2未満, 傾き {p.SolarPotential.Roof.SlopeDegree} 度以上\n";
					str += $"方位が {CommonManager.Directions4[p.SolarPotential.Roof.DirDeg.Direction]} かつ傾き {p.SolarPotential.Roof.DirDeg.Degree} 度以上\n";
					str += $"インテリア面の除外： {(p.SolarPotential.Roof.Interior ? "○" : "×")}";
					AddItem(ParamsGrid, "屋根面の解析対象外とする条件", str);

					str = $"傾き {p.SolarPotential.Roof.Area} 度未満の場合、" +
						 $"方位を {CommonManager.Directions4[p.SolarPotential.Roof.CorrectionDirDeg.Direction]} " +
						 $"かつ {p.SolarPotential.Roof.CorrectionDirDeg.Degree} 度に補正";
					AddItem(ParamsGrid, "屋根面の傾斜補正", str);
				}

				if (p.Target.Land)
				{
					str = $"面積 {p.SolarPotential.Land.Area} m2未満, 傾き {p.SolarPotential.Land.SlopeDegree} 度以上";
					AddItem(ParamsGrid, "土地面の解析対象外とする条件", str);

					str = $"方位を {CommonManager.Directions4[p.SolarPotential.Land.CorrectionDirDeg.Direction]} " +
						 $"かつ {p.SolarPotential.Land.CorrectionDirDeg.Degree} 度に補正";
					AddItem(ParamsGrid, "土地面の傾斜補正", str);
				}

				AddItem(ParamsGrid, "メーカー別設置係数", p.SolarPotential.PanelMakerSolarPower);
				AddItem(ParamsGrid, "パネル設置割合", $"{p.SolarPotential.PanelRatio} %");

			}

			// 反射シミュレーション条件
			{
				if (p.Target.Build)
				{
					if (p.Reflection.Roof[ReflectionParams.SlopeConditions.Lower].UseCustomVal)
					{
						str = $"方位を {CommonManager.Directions4[p.Reflection.Roof[ReflectionParams.SlopeConditions.Lower].DirDeg.Direction]} , " +
							$" {p.Reflection.Roof[ReflectionParams.SlopeConditions.Lower].DirDeg.Degree} 度に補正";
						AddItem(ParamsGrid, "屋根面の補正\n3度未満", str);
					}
					else
					{
						AddItem(ParamsGrid, "屋根面の補正\n3度未満", "屋根面と同値");
					}

					if (p.Reflection.Roof[ReflectionParams.SlopeConditions.Upper].UseCustomVal)
					{
						str = $"方位を {CommonManager.Directions4[p.Reflection.Roof[ReflectionParams.SlopeConditions.Upper].DirDeg.Direction]} , " +
							$" {p.Reflection.Roof[ReflectionParams.SlopeConditions.Upper].DirDeg.Degree} 度に補正";
						AddItem(ParamsGrid, "屋根面の補正\n3度以上", str);
					}
					else
					{
						AddItem(ParamsGrid, "屋根面の補正\n3度以上", "屋根面と同値");
					}
				}

				if (p.Target.Land)
				{
					if (p.Reflection.Land[ReflectionParams.SlopeConditions.Lower].UseCustomVal)
					{
						str = $"方位を {CommonManager.Directions4[p.Reflection.Land[ReflectionParams.SlopeConditions.Lower].DirDeg.Direction]} , " +
							$" {p.Reflection.Land[ReflectionParams.SlopeConditions.Lower].DirDeg.Degree} 度に補正";
						AddItem(ParamsGrid, "土地面の補正\n3度未満", str);
					}
					else
					{
						AddItem(ParamsGrid, "土地面の補正\n3度未満", "屋根面と同値");
					}

					if (p.Reflection.Land[ReflectionParams.SlopeConditions.Upper].UseCustomVal)
					{
						str = $"方位を {CommonManager.Directions4[p.Reflection.Roof[ReflectionParams.SlopeConditions.Upper].DirDeg.Direction]} , " +
							$" {p.Reflection.Land[ReflectionParams.SlopeConditions.Upper].DirDeg.Degree} 度に補正";
						AddItem(ParamsGrid, "土地面の補正\n3度以上", str);
					}
					else
					{
						AddItem(ParamsGrid, "土地面の補正\n3度以上", "屋根面と同値");
					}
				}

				AddItem(ParamsGrid, "反射有効範囲", $"{p.Reflection.ReflectionRange} m");

			}

		}

		/// <summary>
		/// 集計パラメータの表示
		/// </summary>
		private void ShowAggregateParams()
		{
			var p = AggregateParam.Instance;

			string str = "";     // 一時格納用

			AnalyzeSettings.Visibility = Visibility.Collapsed;

			// 入力パラメータ
			{
				InputDataGrid.Children.Clear();
				AddItem(InputDataGrid, "解析結果フォルダ", p.AnalyzeResultPath);

				if (p.SelectArea)
				{
					AddItem(InputDataGrid, "集計範囲", "集計範囲を選択");

					//// 集計範囲
					//AreaText.Visibility = Visibility.Visible;
					//AreaText.Text = "■集計範囲";

				}
				else
				{
					AddItem(InputDataGrid, "集計範囲", "全範囲で集計");
					//AreaText.Visibility = Visibility.Collapsed;
				}
			}

			// 解析エリアリストを非表示
			AreaListPanel.Visibility = Visibility.Collapsed;

			// 出力設定
			OutputDataGrid.Children.Clear();
			AddItem(OutputDataGrid, "出力フォルダ", p.OutputResultDirectory);

			// 詳細設定
			ParamsGrid.Children.Clear();

			if (p.Judge.LowerPotential)
			{
				str = "";
				if (p.Judge.Potential) str += $"日射量が {p.Judge.PotentialVal} 未満\n";
				if (p.Judge.PotentialPercent) str += $"日射量が下位 {p.Judge.PotentialPercentVal} ％";
				AddItem(ParamsGrid, "優先度が低い建物の除外条件", str);
			}

			if (p.Judge.BuildStructure)
			{
				str = "";
				if (p.Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.Wood)) str += $"{p.Judge.BuildStructureNames[JudgeParams.BuildStructures.Wood]},";
				if (p.Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.SteelReinforcedConcrete)) str += $"{p.Judge.BuildStructureNames[JudgeParams.BuildStructures.SteelReinforcedConcrete]},";
				if (p.Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.ReinforcedConcrete)) str += $"{p.Judge.BuildStructureNames[JudgeParams.BuildStructures.ReinforcedConcrete]},";
				if (p.Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.Steel)) str += $"{p.Judge.BuildStructureNames[JudgeParams.BuildStructures.Steel]},";
				if (p.Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.LightGaugeSteel)) str += $"{p.Judge.BuildStructureNames[JudgeParams.BuildStructures.LightGaugeSteel]},";
				if (p.Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.MasonryConstruction)) str += $"{p.Judge.BuildStructureNames[JudgeParams.BuildStructures.MasonryConstruction]},";
				if (p.Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.Unknown)) str += $"{p.Judge.BuildStructureNames[JudgeParams.BuildStructures.Unknown]},";
				if (p.Judge.BuildStructureFlag.HasFlag(JudgeParams.BuildStructures.NonWood)) str += $"{p.Judge.BuildStructureNames[JudgeParams.BuildStructures.NonWood]},";
				str = str.Remove(str.Length - 1);
				AddItem(ParamsGrid, "除外する建物構造", str);

				if (p.Judge.BuildFloors)
				{
					str = $" {p.Judge.FloorsBelowVal} 階以下、 {p.Judge.UpperFloorsVal} 階以上の建物";
					AddItem(ParamsGrid, "除外する建物構造", str);
				}

			}

			str = $"津波：{(p.Judge.BelowTsunamiHeight ? "○" : "×")}\n" +
					$"河川浸水：{(p.Judge.BelowFloodDepth ? "○" : "×")}\n" +
					$"土砂災害：{(p.Judge.LandslideWarningArea ? "○" : "×")}\n" +
					$"積雪：{(p.Judge.WeatherData ? "○" : "×")}";
			AddItem(ParamsGrid, "災害リスクによる除外", str);

			if (p.Judge.WeatherData)
			{
				AddItem(ParamsGrid, "気象データ(積雪)による除外", "○");
				AddItem(ParamsGrid, "気象データ", p.Judge.WeatherDataPath);

				str = "";
				if (p.Judge.UseSnowDepth)
				{
					str += $"積雪が {p.Judge.SnowDepthVal} cm以上";
				}
				else
				{
					str += $"積雪荷重が {p.Judge.SnowLoadVal} kgf/m2以上, 単位荷重 {p.Judge.SnowLoadUnitVal} N/m2";
				}
				AddItem(ParamsGrid, "条件", str);
			}

			foreach (JudgeParams.RestrictArea data in p.Judge.RestrictAreas)
			{
				if (!data.Enable) continue;
				str = "";
				str += $"　シェープファイル：{data.ShapePath}\n";
				str += $"　制限する建物の高さ：{data.Height}\n";
				str += $"　制限する方位：{CommonManager.Directions16[data.Direction]}";
				str += $"　座標系：{CommonManager.DatumTypeList[data.Datum]}";

				AddItem(ParamsGrid, "制限範囲による除外", str);
			}

		}

		/// <summary>
		/// 表示項目を作成
		/// </summary>
		private void AddItem(Grid grid, string item, string val)
		{
			grid.RowDefinitions.Add(new RowDefinition());
			int row = grid.RowDefinitions.Count() - 1;

			Brush color = string.IsNullOrEmpty(item) ? null : Brushes.LightGray;

			// 項目名
			TextBlock tbItem = new TextBlock() {
				Text = item,
				TextAlignment = TextAlignment.Center,
				TextWrapping = TextWrapping.Wrap,
				Background = color,
				Margin = new Thickness(3, 1, 3, 1),
				Padding = new Thickness(2),
			};
			grid.Children.Add(tbItem);
			Grid.SetRow(tbItem, row);
			Grid.SetColumn(tbItem, 0);

			// 値
			TextBlock tbVal = new TextBlock() {
				Text = val,
				TextWrapping = TextWrapping.Wrap,
				VerticalAlignment = VerticalAlignment.Center,
			};
			grid.Children.Add(tbVal);
			Grid.SetRow(tbVal, row);
			Grid.SetColumn(tbVal, 1);

		}

	}
}
