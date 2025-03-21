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
using System.Xml;
using System.Xml.Linq;

namespace SolarPotential
{
	/// <summary>
	/// Page1.xaml の相互作用ロジック
	/// </summary>
	public partial class PageAnalyzeDataIO : Page
	{
		/// <summary>
		/// 入力不正コントロールリスト
		/// </summary>
		List<object> ErrorControlList = new List<object>();

		public PageAnalyzeDataIO()
		{
			InitializeComponent();

			SetBindings();

			// Events
			Loaded += PageAnalyzeDataIO_Loaded;

			// 解析期間イベント設定
			AnalyzeDateControlEvents();

			LandShapePath.TextChanged += (s, e) =>
			{
				if (AnalyzeParam.Instance.InputData.LandData != LandShapePath.Text)
				{
					AnalyzeParam.Instance.InputData.ReadLandData = false;
				}
			};

			CheckBuilding.Unchecked += (s, e) =>
			{
				var findArea = AnalyzeParam.Instance.AreaList.Where(x => x.Id == AnalyzeParam.ALLAREA_ID).FirstOrDefault();
				if (findArea != null)
				{
					AnalyzeParam.Instance.AreaList.Remove(findArea);
				}
			};

			CheckLand.Unchecked += (s, e) =>
			{
				// shpから取得したエリアを削除する
				var areaList = AnalyzeParam.Instance.AreaList.Where(x => x.IsShpData).ToList();
				foreach (var area in areaList)
				{
					AnalyzeParam.Instance.AreaList.Remove(area);
				}
				AnalyzeParam.Instance.InputData.ReadLandData = false;
			};

		}

		/// <summary>
		/// 入力値チェック
		/// </summary>
		/// <returns></returns>
		public bool CheckParams()
		{
			foreach (var ctrl in ErrorControlList)
			{
				(ctrl as Control).Background = null;
			}
			ErrorControlList.Clear();

			AnalyzeInputData inputData = AnalyzeParam.Instance.InputData;
			if (string.IsNullOrEmpty(inputData.CityModel) || !citygmlExistsAtPath())
			{
				LogManager.Instance.OutputLogMessage($"!!入力エラー!! 3D都市モデルフォルダ：{inputData.CityModel}", LogManager.LogType.INFO);
				ErrorControlList.Add(CityModelPath);
			}
			if (string.IsNullOrEmpty(inputData.KashoData) || !System.IO.File.Exists(inputData.KashoData))
			{
				LogManager.Instance.OutputLogMessage($"!!入力エラー!! 可照時間データ：{inputData.KashoData}", LogManager.LogType.INFO);
				ErrorControlList.Add(KashoDataPath);
			}
			if (string.IsNullOrEmpty(inputData.NisshoData) || !System.IO.File.Exists(inputData.NisshoData))
			{
				LogManager.Instance.OutputLogMessage($"!!入力エラー!! 日照時間データ：{inputData.NisshoData}", LogManager.LogType.INFO);
				ErrorControlList.Add(NisshoDataPath);
			}
			if (!string.IsNullOrEmpty(inputData.SnowDepthData) && !System.IO.File.Exists(inputData.SnowDepthData))
			{
				LogManager.Instance.OutputLogMessage($"!!入力エラー!! 積雪深データ：{inputData.SnowDepthData}", LogManager.LogType.INFO);
				ErrorControlList.Add(SnowDepthPath);
			}
			if (!string.IsNullOrEmpty(inputData.LandData) && !System.IO.File.Exists(inputData.LandData))
			{
				LogManager.Instance.OutputLogMessage($"!!入力エラー!! 土地範囲指定データ：{inputData.LandData}", LogManager.LogType.INFO);
				ErrorControlList.Add(LandShapePath);
			}
			if (string.IsNullOrEmpty(AnalyzeParam.Instance.OutputResultDirectory))
			{
				LogManager.Instance.OutputLogMessage($"!!入力エラー!! 解析結果出力フォルダ：{AnalyzeParam.Instance.OutputResultDirectory}", LogManager.LogType.INFO);
				ErrorControlList.Add(OutputDirPath);
			}

			foreach (var ctrl in ErrorControlList)
			{
				(ctrl as Control).Background = Brushes.Pink;
			}

			if (ErrorControlList.Count() > 0)
			{
				MessageBox.Show("入力内容に不備があります。", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Warning);
				return false;
			}

			// 解析期間を設定
			AnalyzeParam.Instance.TargetDate = AnalyzeParam.DateType.None;
			if (OneMonthRadio.IsChecked == true) AnalyzeParam.Instance.TargetDate = AnalyzeParam.DateType.OneMonth;
			if (OneDayRadio.IsChecked == true)
			{
				if (DateRadio.IsChecked == true) AnalyzeParam.Instance.TargetDate = AnalyzeParam.DateType.OneDay;
				if (SummerCheckBox.IsChecked == true) AnalyzeParam.Instance.TargetDate |= AnalyzeParam.DateType.Summer;
				if (WinterCheckBox.IsChecked == true) AnalyzeParam.Instance.TargetDate |= AnalyzeParam.DateType.Winter;
			}
			if (AllDaysRadio.IsChecked == true) AnalyzeParam.Instance.TargetDate = AnalyzeParam.DateType.Year;

			if (AnalyzeParam.Instance.TargetDate == AnalyzeParam.DateType.None)
			{
				MessageBox.Show("解析期間が設定されていません。", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Warning);
				return false;
			}
			else if (AnalyzeParam.Instance.TargetDate.HasFlag(AnalyzeParam.DateType.OneMonth) && AnalyzeParam.Instance.Month == 0)
			{
				MessageBox.Show("月が設定されていません。", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Warning);
				return false;
			}
			else if (AnalyzeParam.Instance.TargetDate.HasFlag(AnalyzeParam.DateType.OneDay))
			{
				if (AnalyzeParam.Instance.Month == 0)
				{
					MessageBox.Show("月が設定されていません。", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Warning);
					return false;
				}
				if (AnalyzeParam.Instance.Day == 0)
				{
					MessageBox.Show("日が設定されていません。", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Warning);
					return false;
				}
			}

			// 解析対象チェック
			if (!AnalyzeParam.Instance.Target.Build && !AnalyzeParam.Instance.Target.Land)
			{
				MessageBox.Show("解析対象が選択されていません。", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Warning);
				return false;
			}
			else
			{
				// 解析対象を更新
				foreach (var area in AnalyzeParam.Instance.AreaList)
				{
					area.AnalyzeBuild = AnalyzeParam.Instance.Target.Build && !area.IsShpData && !area.Water;
					area.AnalyzeLand = AnalyzeParam.Instance.Target.Land && !area.Id.Equals(AnalyzeParam.ALLAREA_ID);
				}
			}

			if (!AnalyzeParam.Instance.Target.SolarPotential && !AnalyzeParam.Instance.Target.Reflection)
			{
				MessageBox.Show("解析内容が選択されていません。", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Warning);
				return false;
			}

			return true;

		}

		/// <summary>
		/// 入力した3D都市モデルデータに必要なCityGMLが存在するかどうか
		/// </summary>
		/// <returns></returns>
		private bool citygmlExistsAtPath()
		{
			string udxDir = System.IO.Path.Combine(AnalyzeParam.Instance.InputData.CityModel, "udx");
			if (!System.IO.Directory.Exists(udxDir)) return false;

			// 建物モデルは必須
			bool ret = CommonManager.IsExtensionExists_Directory(udxDir + "\\bldg", ".gml");

			if (AnalyzeParam.Instance.Target.Land
				|| AnalyzeParam.Instance.InputData.UseDemData)
			{
				ret &= CommonManager.IsExtensionExists_Directory(udxDir + "\\dem", ".gml");
			}

			if (AnalyzeParam.Instance.Target.Land
				&& AnalyzeParam.Instance.InputData.UseRoadData)
			{
				ret &= CommonManager.IsExtensionExists_Directory(udxDir + "\\tran", ".gml");
			}

			return ret;
		}

		/// <summary>
		/// 土地範囲指定ファイルを読み込む
		/// </summary>
		public void ReadLandShp()
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			if (AnalyzeParam.Instance.InputData.ReadLandData) return;

			// 元のshpから読み込んだ範囲があれば削除する
			var delAreaList = AnalyzeParam.Instance.AreaList.Where(x => x.IsShpData).ToList();
			foreach (var area in delAreaList)
			{
				AnalyzeParam.Instance.AreaList.Remove(area);
			}

			// 対象外かどうかの判定
			if (!AnalyzeParam.Instance.Target.Land) return;
			if (string.IsNullOrEmpty(AnalyzeParam.Instance.InputData.LandData)) return;

			string file = AnalyzeParam.Instance.InputData.LandData;
			if (!File.Exists(file)) return;

			var layer = DotSpatial.Data.DataManager.DefaultDataManager.OpenFile(file) as DotSpatial.Data.FeatureSet;
			foreach (var feature in layer.Features)
			{
				int id = AnalyzeParam.Instance.LastAreaIdNum + 1;
				AreaData area = new AreaData();
				area.Id = $"A{string.Format("{0:D3}", id)}";
				area.FeatureId = id;
				area.Name = "";
				area.Directions = CommonManager.Directions4;
				area.Direction = 2; // 南向き
				area.Water = false;
				area.Degree = 15;
				area.AnalyzeBuild = false;
				area.AnalyzeLand = true;
				area.IsShpData = true;

				foreach (var coord in feature.Geometry.Coordinates)
				{
					Point2D pt = new Point2D();

					switch (AnalyzeParam.Instance.InputData.LandDataDatum)
					{
						case (int)CommonManager.DatumTypes.LatLon:
							pt.Lat = coord.Y; pt.Lon = coord.X;
							break;
						case (int)CommonManager.DatumTypes.XY:
							// 緯度経度に変換
							GeoUtil.Instance.XYToLatLon(coord.X, coord.Y, out double lat, out double lon);
							pt.Lat = lat; pt.Lon = lon;
							break;
						default:
							MessageBox.Show("土地指定シェープファイルの読み込みに失敗しました。", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Error);
							return;
					}

					area.Points.Add(pt);
				}

				AnalyzeParam.Instance.AreaList.Add(area);
				AnalyzeParam.Instance.LastAreaIdNum = id;
			}

			AnalyzeParam.Instance.InputData.ReadLandData = true;
		}

		/// <summary>
		/// 3D都市モデルを読み込んで表示範囲を設定
		/// </summary>
		/// <param name="minLat"></param>
		/// <param name="maxLat"></param>
		/// <param name="minLon"></param>
		/// <param name="maxLon"></param>
		public void InitMapRangeFromMetadata()
		{
			string metadataDir = System.IO.Path.Combine(AnalyzeParam.Instance.InputData.CityModel, "metadata");
			if (Directory.Exists(metadataDir))
			{
				string[] files = Directory.GetFiles(metadataDir, "*.xml");
				// 先頭のファイルを読み込む
				foreach (var file in files)
				{
					XDocument xmlDoc = XDocument.Load(file);
					XNamespace ns = "http://zgate.gsi.go.jp/ch/jmp/";

					var bbox = xmlDoc.Root
								.Element(ns + "identificationInfo")
								.Element(ns + "MD_DataIdentification")
								.Element(ns + "extent")
								.Element(ns + "geographicElement")
								.Element(ns + "EX_GeographicBoundingBox");

					if (bbox != null)
					{
						double.TryParse(bbox.Element(ns + "westBoundLongitude").Value, out double minLon);
						double.TryParse(bbox.Element(ns + "eastBoundLongitude").Value, out double maxLon);
						double.TryParse(bbox.Element(ns + "southBoundLatitude").Value, out double minLat);
						double.TryParse(bbox.Element(ns + "northBoundLatitude").Value, out double maxLat);
						AnalyzeParam.Instance.InputData.MinPos.Lon = minLon; AnalyzeParam.Instance.InputData.MinPos.Lat = minLat;
						AnalyzeParam.Instance.InputData.MaxPos.Lon = maxLon; AnalyzeParam.Instance.InputData.MaxPos.Lat = maxLat;
						break;
					}
				}
			}
			else
			{
				// メタデータから読み取れないときはbldgフォルダ内のファイルから最大最小値を取得する
				string bldgDir = System.IO.Path.Combine(AnalyzeParam.Instance.InputData.CityModel, "udx/bldg");
				if (Directory.Exists(bldgDir))
				{
					double tmpMinLat = double.MaxValue, tmpMinLon = double.MaxValue;
					double tmpMaxLat = -double.MaxValue, tmpMaxLon = -double.MaxValue;

					string[] files = Directory.GetFiles(bldgDir, "*.gml");
					foreach (var file in files)
					{
						bool findLower = false, findUpper = false;

						using (XmlReader reader = XmlReader.Create(file))
						{
							// バウンディング設定は最初の数行にある想定で1行ずつ読み込む
							while (reader.Read())
							{
								if (findLower && findUpper) break;
								if (reader.NodeType != XmlNodeType.Element) continue;

								if (reader.Name.Contains("lowerCorner"))
								{
									reader.Read();  // 次の行を取得
									var splitLower = reader.Value.Split(' ');
									if (splitLower.Count() == 3)
									{
										double.TryParse(splitLower[0], out double lat);
										double.TryParse(splitLower[1], out double lon);
										tmpMinLat = Math.Min(lat, tmpMinLat);
										tmpMinLon = Math.Min(lon, tmpMinLon);
										findLower = true;
									}
								}
								else if (reader.Name.Contains("upperCorner"))
								{
									reader.Read();  // 次の行を取得
									var splitUpper = reader.Value.Split(' ');
									if (splitUpper.Count() == 3)
									{
										double.TryParse(splitUpper[0], out double lat);
										double.TryParse(splitUpper[1], out double lon);
										tmpMaxLat = Math.Max(lat, tmpMaxLat);
										tmpMaxLon = Math.Max(lon, tmpMaxLon);
										findUpper = true;
									}
								}
							}
						}
					}

					AnalyzeParam.Instance.InputData.MinPos.Lon = tmpMinLon; AnalyzeParam.Instance.InputData.MinPos.Lat = tmpMinLat;
					AnalyzeParam.Instance.InputData.MaxPos.Lon = tmpMaxLon; AnalyzeParam.Instance.InputData.MaxPos.Lat = tmpMaxLat;
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

				var binding = new Binding(nameof(AnalyzeParam.Instance.InputData.CityModel))
				{
					Source = AnalyzeParam.Instance.InputData,
					Mode = BindingMode.TwoWay,
				};
				CityModelPath.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.Instance.InputData.KashoData))
				{
					Source = AnalyzeParam.Instance.InputData,
					Mode = BindingMode.TwoWay,
				};
				KashoDataPath.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.Instance.InputData.NisshoData))
				{
					Source = AnalyzeParam.Instance.InputData,
					Mode = BindingMode.TwoWay,
				};
				NisshoDataPath.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.Instance.InputData.SnowDepthData))
				{
					Source = AnalyzeParam.Instance.InputData,
					Mode = BindingMode.TwoWay,
				};
				SnowDepthPath.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.Instance.InputData.UseDemData))
				{
					Source = AnalyzeParam.Instance.InputData,
					Mode = BindingMode.TwoWay,
				};
				CheckUseDem.SetBinding(CheckBox.IsCheckedProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.Instance.InputData.LandData))
				{
					Source = AnalyzeParam.Instance.InputData,
					Mode = BindingMode.TwoWay,
				};
				LandShapePath.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.Instance.InputData.UseRoadData))
				{
					Source = AnalyzeParam.Instance.InputData,
					Mode = BindingMode.TwoWay,
				};
				CheckUseRoad.SetBinding(CheckBox.IsCheckedProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.OutputResultDirectory))
				{
					Source = AnalyzeParam.Instance,
					Mode = BindingMode.TwoWay,
				};
				OutputDirPath.SetBinding(TextBox.TextProperty, binding);


				binding = new Binding(nameof(AnalyzeParam.Instance.Target.SolarPotential))
				{
					Source = AnalyzeParam.Instance.Target,
					Mode = BindingMode.TwoWay,
				};
				CheckSolarPotential.SetBinding(CheckBox.IsCheckedProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.Instance.Target.Reflection))
				{
					Source = AnalyzeParam.Instance.Target,
					Mode = BindingMode.TwoWay,
				};
				CheckReflection.SetBinding(CheckBox.IsCheckedProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.Instance.Target.Build))
				{
					Source = AnalyzeParam.Instance.Target,
					Mode = BindingMode.TwoWay,
				};
				CheckBuilding.SetBinding(CheckBox.IsCheckedProperty, binding);

				binding = new Binding(nameof(AnalyzeParam.Instance.Target.Land))
				{
					Source = AnalyzeParam.Instance.Target,
					Mode = BindingMode.TwoWay,
				};
				CheckLand.SetBinding(CheckBox.IsCheckedProperty, binding);

			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}
		}

		/// <summary>
		/// パラメータを更新する(Binding非対応のパラメータ)
		/// </summary>
		public void UpdateParams()
		{
			AnalyzeParam param = AnalyzeParam.Instance;

			int month = AnalyzeParam.Instance.Month;
			int day = AnalyzeParam.Instance.Day;

			// 月コンボボックス
			TargetMonthComboBox.Items.Clear();
			for (int i = 0; i < 12; i++)
			{
				TargetMonthComboBox.Items.Add($"{i + 1}月");
			}

			// 解析期間
			if (param.TargetDate == AnalyzeParam.DateType.None)
			{   // 指定なし
				OneMonthRadio.IsChecked = true;
				TargetMonthComboBox.SelectedIndex = 0;
			}
			else if (param.TargetDate.HasFlag(AnalyzeParam.DateType.OneMonth))
			{   // 指定月
				OneMonthRadio.IsChecked = true;
				TargetMonthComboBox.SelectedIndex = month - 1;
			}
			else if (param.TargetDate.HasFlag(AnalyzeParam.DateType.OneDay))
			{   // 指定日
				OneDayRadio.IsChecked = true;
				DateRadio.IsChecked = true;
				TargetMonthComboBox.SelectedIndex = month - 1;
				TargetDayComboBox.SelectedIndex = day - 1;
			}
			else if (param.TargetDate.HasFlag(AnalyzeParam.DateType.Year))
			{	// 年間
				AllDaysRadio.IsChecked = true;
			}
			else
			{	// 夏至 or 冬至
				OneDayRadio.IsChecked = true;
				SummerCheckBox.IsChecked = param.TargetDate.HasFlag(AnalyzeParam.DateType.Summer);
				WinterCheckBox.IsChecked = param.TargetDate.HasFlag(AnalyzeParam.DateType.Winter);
			}

			DatumComboBox.SelectedIndex = AnalyzeParam.Instance.InputData.LandDataDatum;
		}

		private void PageAnalyzeDataIO_Loaded(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			AnalyzeParam param = AnalyzeParam.Instance;

			// リンク設定
			KashoDataLink.NavigateUri = new Uri("https://eco.mtk.nao.ac.jp/cgi-bin/koyomi/koyomix.cgi");
			NisshoDataLink.NavigateUri = new Uri("https://www.data.jma.go.jp/gmd/risk/obsdl/index.php");
			SnowDepthLink.NavigateUri = new Uri("https://appww2.infoc.nedo.go.jp/appww/metpv_map.html");

			// ツールチップ設定
			CityModelToolTip.Content = "3D都市モデルデータのフォルダを指定します。\n例：14130_kawasaki-shi_city_2022_citygml_3_op";
			KashoDataToolTip.Content = "国立天文台 こよみの計算Webページから取得した可照時間のCSVファイルを指定します。\n"
										+ "解析したい都市の「1日おきに1年間」のデータを使用します。";
			NisshoDataToolTip.Content = "気象庁の過去の気象データから取得した平均日照時間のCSVファイルを指定します。";
			SnowDepthDataToolTip.Content = "NEDOの日射量データベース閲覧システムから取得したCSVファイルを指定します。";
			LandDataToolTip.Content = "土地範囲を指定するシェープファイルを指定します。";

			// 土地SHPの座標系コンボボックス
			DatumComboBox.ItemsSource = CommonManager.DatumTypeList;

			UpdateParams();
		}

		/// <summary>
		/// 解析期間関連のイベント処理
		/// </summary>
		/// <returns></returns>
		private void AnalyzeDateControlEvents()
		{
			// 指定月
			OneMonthRadio.Checked += (sender, e) => {
				TargetMonthComboBox.IsEnabled = true;
				TargetDayComboBox.IsEnabled = false;
				TargetMonthComboBox.SelectedIndex = AnalyzeParam.Instance.Month - 1;
			};

			// 指定日
			OneDayRadio.Unchecked += (sender, e) => {
				DateRadio.IsChecked = false;
				SummerCheckBox.IsChecked = false;
				WinterCheckBox.IsChecked = false;
			};
			DateRadio.Checked += (sender, e) => {
				SummerCheckBox.IsChecked = false;
				WinterCheckBox.IsChecked = false;
				TargetMonthComboBox.IsEnabled = true;
				TargetDayComboBox.IsEnabled = true;
			};
			SummerCheckBox.Checked += (sender, e) => {
				DateRadio.IsChecked = false;
				TargetMonthComboBox.IsEnabled = false;
				TargetDayComboBox.IsEnabled = false;
			};
			WinterCheckBox.Checked += (sender, e) => {
				DateRadio.IsChecked = false;
				TargetMonthComboBox.IsEnabled = false;
				TargetDayComboBox.IsEnabled = false;
			};

			// 年間
			AllDaysRadio.Checked += (sender, e) => {
				TargetMonthComboBox.IsEnabled = false;
				TargetDayComboBox.IsEnabled = false;
			};

			TargetMonthComboBox.IsEnabledChanged += (sender, e) => {
				if ((bool)e.NewValue)
					TargetMonthComboBox.SelectedIndex = AnalyzeParam.Instance.Month - 1;
				else
					TargetMonthComboBox.SelectedIndex = -1;
			};
			TargetMonthComboBox.SelectionChanged += (sender, e) =>
			{
				AnalyzeParam.Instance.Month = TargetMonthComboBox.SelectedIndex + 1;
				if (AnalyzeParam.Instance.Month > 0)
				{
					int dayCount = DateTime.DaysInMonth(2025, AnalyzeParam.Instance.Month);   // うるう年は対象外のため固定年で取得
					TargetDayComboBox.Items.Clear();
					for (int i = 0; i < dayCount; i++)
					{
						TargetDayComboBox.Items.Add($"{i + 1}日");
					}
				}
			};

			TargetDayComboBox.IsEnabledChanged += (sender, e) => {
				if ((bool)e.NewValue)
					TargetDayComboBox.SelectedIndex = AnalyzeParam.Instance.Day - 1;
				else
					TargetDayComboBox.SelectedIndex = -1;
			};
			TargetDayComboBox.SelectionChanged += (sender, e) => {
				AnalyzeParam.Instance.Day = TargetDayComboBox.SelectedIndex + 1;
			};

		}


		private void SelectCityModelButton_Click(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			string resultDir = CityModelPath.Text;
			CommonManager.Instance.ShowFolderBrowserDialog(out string dir, "3D都市モデルを選択します。", resultDir);
			if (string.IsNullOrEmpty(dir)) return;
			AnalyzeParam.Instance.InputData.CityModel = dir;
			LogManager.Instance.OutputLogMessage($"3D都市モデルフォルダ：{dir}", LogManager.LogType.INFO);

		}

		private void SelectKashoDataButton_Click(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			string file = KashoDataPath.Text;
			CommonManager.Instance.ShowSelectFileDialog(out string path, "可照時間データを選択します。", file, CommonManager.EXT_FILTER_CSV);
			if (string.IsNullOrEmpty(path)) return;
			AnalyzeParam.Instance.InputData.KashoData = path;
			LogManager.Instance.OutputLogMessage($"可照時間データ：{path}", LogManager.LogType.INFO);
		}

		private void SelectNisshoDataButton_Click(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			string file = NisshoDataPath.Text;
			CommonManager.Instance.ShowSelectFileDialog(out string path, "日照時間データを選択します。", file, CommonManager.EXT_FILTER_CSV);
			if (string.IsNullOrEmpty(path)) return;
			AnalyzeParam.Instance.InputData.NisshoData = path;
			LogManager.Instance.OutputLogMessage($"日照時間データ：{path}", LogManager.LogType.INFO);
		}

		private void SelectSnowDepthButton_Click(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			string file = SnowDepthPath.Text;
			CommonManager.Instance.ShowSelectFileDialog(out string path, "積雪深データを選択します。", file, CommonManager.EXT_FILTER_CSV);
			if (string.IsNullOrEmpty(path)) return;
			AnalyzeParam.Instance.InputData.SnowDepthData = path;
			LogManager.Instance.OutputLogMessage($"積雪深データ：{path}", LogManager.LogType.INFO);
		}

		private void SelectLandShapeButton_Click(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			string file = LandShapePath.Text;
			CommonManager.Instance.ShowSelectFileDialog(out string path, "空地のシェープファイルを選択します。", file, CommonManager.EXT_FILTER_SHP);
			if (string.IsNullOrEmpty(path)) return;
			if (!AnalyzeParam.Instance.InputData.LandData.Equals(path))
			{
				AnalyzeParam.Instance.InputData.LandData = path;
				AnalyzeParam.Instance.InputData.ReadLandData = false;
			}
			LogManager.Instance.OutputLogMessage($"空地のシェープファイル：{path}", LogManager.LogType.INFO);
		}

		private void SelectOutputDirButton_Click(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			string resultDir = OutputDirPath.Text;
			CommonManager.Instance.ShowFolderBrowserDialog(out string dir, "解析結果出力フォルダを選択します。", resultDir);
			if (string.IsNullOrEmpty(dir)) return;
			AnalyzeParam.Instance.OutputResultDirectory = dir;
			LogManager.Instance.OutputLogMessage($"解析結果出力フォルダ：{dir}", LogManager.LogType.INFO);
		}

		private void Hyperlink_RequestNavigate(object sender, RequestNavigateEventArgs e)
		{
			try
			{
				System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo(e.Uri.AbsoluteUri));
				e.Handled = true;
			}
			catch (Exception ex)
			{
				LogManager.Instance.ExceptionLog(ex);
			}
		}

		private void DatumComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
			if (DatumComboBox.SelectedItem == null) return;
			if (AnalyzeParam.Instance.InputData.LandDataDatum != DatumComboBox.SelectedIndex)
			{
				AnalyzeParam.Instance.InputData.LandDataDatum = DatumComboBox.SelectedIndex;
				AnalyzeParam.Instance.InputData.ReadLandData = false;
			}
		}

	}
}
