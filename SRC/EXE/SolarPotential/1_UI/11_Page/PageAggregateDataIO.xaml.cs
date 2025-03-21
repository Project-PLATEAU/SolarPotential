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
using System.Reflection;
using System.Windows.Threading;

namespace SolarPotential
{
	/// <summary>
	/// Page1.xaml の相互作用ロジック
	/// </summary>
	public partial class PageAggregateDataIO : Page
	{
		/// <summary>
		/// 入力不正コントロールリスト
		/// </summary>
		List<object> ErrorControlList = new List<object>();

		/// <summary>
		/// 最大緯度
		/// </summary>
		private double dbl_topMax = 0;

		/// <summary>
		/// 最小緯度
		/// </summary>
		private double dbl_bottomMax = 0;

		/// <summary>
		/// 最小経度
		/// </summary>
		private double dbl_leftMax = 0;

		/// <summary>
		/// 最大経度
		/// </summary>
		private double dbl_rightMax = 0;


		public PageAggregateDataIO()
		{
			InitializeComponent();

			SetBindings();

			// スクリプトエラー抑止
			var axIWebBrowser2 = typeof(WebBrowser).GetProperty("AxIWebBrowser2", BindingFlags.Instance | BindingFlags.NonPublic);
			var comObj = axIWebBrowser2.GetValue(WebMapArea, null);
			comObj.GetType().InvokeMember("Silent", BindingFlags.SetProperty, null, comObj, new object[] { true });

			Loaded += PageAggregateDataIO_Loaded;

			AnalyzeResultPath.LostFocus += AnalyzeResultPathTextBox_LostFocus;
			AllAreaRadio.Checked += AllAreaRadio_Checked;
			SelectAreaRadio.Checked += SelectAreaRadio_Checked;

		}

		#region 地図表示

		/// <summary>
		/// 地図を表示
		/// </summary>
		/// <returns></returns>
		private void WebMapArea_Init()
		{
			try
			{
				LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

				_3_Class.ExternalMapObj extMapObj = new _3_Class.ExternalMapObj();
				WebMapArea.ObjectForScripting = extMapObj;

				extMapObj.MoveCenter += new _3_Class.ExternalMapObj.MoveCenterEventHandler(this.MoveCenter);
				extMapObj.PickStart += new _3_Class.ExternalMapObj.PickStartEventHandler(this.PickStart);

				var cls_hml = new _3_Class.ClassHTML();
				SetDefaultCoordinates(AggregateParam.Instance.AnalyzeResultPath);
				WebMapArea.NavigateToString(cls_hml.GetHTMLText());

				if (!CommonManager.CheckEnableDispMap())
				{
					MessageBox.Show("ネットワーク接続エラーのため、地図を表示できませんでした。", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Warning);
					AreaSettings.IsEnabled = false;
				}
			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}

		}

		/// <summary>
		/// 解析結果フォルダ選択時
		/// </summary>
		/// <param name="entire">全体表示ボタン押下</param>
		public void MapDataSelection(bool entire = false)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			try
			{
				if (string.IsNullOrEmpty(AggregateParam.Instance.AnalyzeResultPath)) return;

				if (Directory.Exists(AggregateParam.Instance.AnalyzeResultPath))
				{
					if (!entire)
					{
						// 解析パラメータファイルパス
						string paramFile = System.IO.Path.Combine(AggregateParam.Instance.AnalyzeResultPath, ResultData.ParamData.FILE_ANALYZE_PARAM);
						// 解析エリアを読み込む
						ParamFileManager.Instance.ReadAreaDataParams(paramFile);
					}

					// 解析結果データより得られる予定の座標を指定（指定フォルダを引数にセット）
					if (SetDefaultCoordinates(AggregateParam.Instance.AnalyzeResultPath))
					{
						object[] array = new object[4];
						array[0] = dbl_topMax;          // 最大緯度（top）
						array[1] = dbl_bottomMax;       // 最小経度（bottom）
						array[2] = dbl_leftMax;         // 最小経度（left）
						array[3] = dbl_rightMax;        // 最大経度（right）

						if (CommonManager.CheckEnableDispMap())
						{
							SelectAreaRadio.IsEnabled = true;
							WebMapArea.InvokeScript("MapDisp", array);

							// 解析エリア設定範囲を描画
							DrawAnalyzeArea(AggregateParam.Instance.AnalyzeResultPath);
						}

						// 解析エリアの範囲に合わせて表示
						WebMapArea.InvokeScript("FitArea");

						// 全体表示の場合は以下の処理は実施しない
						if (entire) return;

						// 初期状態：全範囲選択時は最大値をセットする
						if (AllAreaRadio.IsChecked == true)
						{
							AggregateParam.Instance.MaxLat = dbl_topMax.ToString();
							AggregateParam.Instance.MinLat = dbl_bottomMax.ToString();
							AggregateParam.Instance.MaxLon = dbl_rightMax.ToString();
							AggregateParam.Instance.MinLon = dbl_leftMax.ToString();
						}
						else
						{
							UpdateCoordinate();
						}

					}
					else
					{
						// 地図を初期化
						var cls_hml = new _3_Class.ClassHTML();
						WebMapArea.NavigateToString(cls_hml.GetHTMLText());
						// 全範囲で集計を選択させる
						AllAreaRadio.IsChecked = true;
						SelectAreaRadio.IsEnabled = false;

						AggregateParam.Instance.MaxLat = "";
						AggregateParam.Instance.MinLat = "";
						AggregateParam.Instance.MaxLon = "";
						AggregateParam.Instance.MinLon = "";
						dbl_topMax = 0;
						dbl_bottomMax = 0;
						dbl_rightMax = 0;
						dbl_leftMax = 0;
					}
				}
				else
				{
					// フォルダ指定がない場合は範囲指定は無効
					SelectAreaRadio.IsEnabled = false;
				}
			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}
		}

		/// <summary>
		/// 指定の解析結果フォルダから最大緯度経度の座標値を取得
		/// </summary>
		/// <param name="dir">解析結果データフォルダ</param>
		public bool SetDefaultCoordinates(string dir)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			if (!Directory.Exists(dir)) return false;

			// 対象地域の座標とズーム最大値を取得しておく（ファイル読み込み）
			var fileName = System.IO.Path.Combine(dir, ResultData.SystemData.FILE_RANGE_COORDINATES);

			if (string.IsNullOrEmpty(dir))
			{
				// 指定なしの場合は後続処理しない
				return false;
			}

			//読み込むテキストを保存する変数
			var maxs = new List<string>();

			try
			{
				// 最大最小緯度経度、ズームファイルをオープンする
				using (StreamReader sr = new StreamReader(fileName, Encoding.GetEncoding("Shift_JIS")))
				{
					while (0 <= sr.Peek())
					{
						maxs.Add(sr.ReadLine());
					}
				}

				// 取得した保存内容をコントロールにセット
				for (int i = 0; i < maxs.Count; i++)
				{
					if (maxs[i].Contains("top"))
					{
						//「:」で分割して値セット
						var items = maxs[i].Split(':');
						dbl_topMax = double.Parse(items[1]);

					}
					else if (maxs[i].Contains("bottom"))
					{
						//「:」で分割して値セット
						var items = maxs[i].Split(':');
						dbl_bottomMax = double.Parse(items[1]);

					}
					else if (maxs[i].Contains("left"))
					{
						//「:」で分割して値セット
						var items = maxs[i].Split(':');
						dbl_leftMax = double.Parse(items[1]);
					}
					else if (maxs[i].Contains("right"))
					{
						//「:」で分割して値セット
						var items = maxs[i].Split(':');
						dbl_rightMax = double.Parse(items[1]);
					}
				}
				return true;

			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
				return false;
			}

		}

		/// <summary>
		/// 解析エリア設定範囲を描画する
		/// </summary>
		/// <param name=""></param>
		private void DrawAnalyzeArea(string dir)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			// 解析エリアを描画する
			foreach (var area in AnalyzeParam.Instance.AreaList)
			{
				if (area.Exclusion_flg) continue;

				string coord = "";
				foreach (var pt in area.Points)
				{
					coord += $"{pt.Lon},{pt.Lat},";
				}
				coord = coord.TrimEnd(',');
				object[] array = new object[1];
				array[0] = coord;
				WebMapArea.InvokeScript("feature_draw", array);
			}

		}

		/// <summary>
		/// 画面中心に移動
		/// </summary>
		/// <param name="lat"></param>
		/// <param name="lon"></param>
		/// <param name="zoom"></param>
		public void MoveCenter(double lat, double lon, int zoom)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			object[] array = new object[3];
			array[0] = lat;
			array[1] = lon;
			array[2] = zoom;
			WebMapArea.InvokeScript("MoveCenter", array);
		}

		/// <summary>
		/// 画面座標を取得
		/// </summary>
		/// <param name="top"></param>
		/// <param name="bottom"></param>
		/// <param name="left"></param>
		/// <param name="right"></param>
		public void PickStart(double top, double bottom, double left, double right)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			// コントロールに反映
			AggregateParam.Instance.MaxLat = top.ToString();
			AggregateParam.Instance.MinLat = bottom.ToString();
			AggregateParam.Instance.MinLon = left.ToString();
			AggregateParam.Instance.MaxLon = right.ToString();
		}

		private void UpdateCoordinate()
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			// 入力範囲チェック（4点の座標位置を全てチェックしてOKな場合は描画）
			double dbl_Max_Lat = double.Parse(AggregateParam.Instance.MaxLat); // top
			double dbl_Min_Lat = double.Parse(AggregateParam.Instance.MinLat); // bottom
			double dbl_Min_Lon = double.Parse(AggregateParam.Instance.MinLon); // left
			double dbl_Max_Lon = double.Parse(AggregateParam.Instance.MaxLon); // right

			// 選択範囲判定（4点の座標位置を全てチェック）
			if (dbl_topMax < dbl_Max_Lat ||
				dbl_bottomMax > dbl_Min_Lat ||
				dbl_leftMax > dbl_Min_Lon ||
				dbl_rightMax < dbl_Max_Lon)
			{
				// NGの場合：選択範囲外エラー
				CommonManager.Instance.ShowExceptionMessageBox("指定した座標は入力データの範囲外です。");
				LogManager.Instance.OutputLogMessage("指定した座標は入力データの範囲外です。", LogManager.LogType.ERROR);
			}
			else
			{
				object[] array = new object[4];
				array[0] = AggregateParam.Instance.MaxLat;    // 最大緯度（top）
				array[1] = AggregateParam.Instance.MinLat;    // 最小経度（bottom）
				array[2] = AggregateParam.Instance.MinLon;    // 最小経度（left）
				array[3] = AggregateParam.Instance.MaxLon;    // 最大経度（right）

				// OKの場合：入力された座標値で描画を更新
				WebMapArea.InvokeScript("feature_update", array);

			}
		}

		private void DoEvents()
		{
			DispatcherFrame frame = new DispatcherFrame();
			var callback = new DispatcherOperationCallback(obj =>
			{
				(obj as DispatcherFrame).Continue = false;
				return null;
			});
			Dispatcher.CurrentDispatcher.BeginInvoke(DispatcherPriority.Background, callback, frame);
			Dispatcher.PushFrame(frame);
		}

		/// <summary>
		/// 選択範囲を保存
		/// </summary>
		public void SaveRange()
		{
			// 選択範囲の画像出力

			// 選択した範囲に合わせて表示する
			WebMapArea.InvokeScript("FitArea");
            DoEvents();     // 画面を強制的に再描画

			// キャプチャ前に不要なコントロールを削除する
			WebMapArea.InvokeScript("MapCaptureBefore");

			Image img = new Image();
			img.Width = (int)WebMapArea.ActualWidth; img.Height = (int)WebMapArea.ActualHeight;
			img.Source = new DrawingImage(VisualTreeHelper.GetDrawing(WebMapArea));

			string jpgFile = $"{ResultData.AggregateResult.RANGEDATA_NAME}.jpg";

			string tempDir = System.IO.Path.GetTempPath();
			CommonManager.AreaImageTempPath = System.IO.Path.Combine(tempDir, jpgFile);
			using (FileStream fs = new FileStream(CommonManager.AreaImageTempPath, FileMode.Create))
			{
				var vis = new DrawingVisual();
				DrawingContext dc = vis.RenderOpen();
				dc.DrawImage(img.Source, new Rect(new Size(img.Width, img.Height)));
				dc.Close();

				var rtb = new RenderTargetBitmap(
					(int)img.Width, (int)img.Height, 96d, 96d, PixelFormats.Default
				);
				rtb.Render(vis);

				var enc = new JpegBitmapEncoder();
				enc.Frames.Add(BitmapFrame.Create(rtb));
				enc.Save(fs);
			}

			// キャプチャ後に必要なコントロールを元に戻す
			WebMapArea.InvokeScript("MapAddControl");


			// KMLファイル出力
			string kmlFile = $"{ResultData.AggregateResult.RANGEDATA_NAME}.kml";
			CommonManager.AreaKmlTempPath = System.IO.Path.Combine(tempDir, kmlFile);

			List<CommonManager.KMLData> kmlDataList = new List<CommonManager.KMLData>();

			// 範囲選択時は選択範囲を出力
			if (SelectAreaRadio.IsChecked == true)
			{
				// 集計範囲を出力
				CommonManager.KMLData rangeKmlData;
				rangeKmlData.Id = DateTime.Now.ToString("yyyyMMddHHmmss");
				rangeKmlData.Name = rangeKmlData.Id;
				List<Point2D> points = new List<Point2D>();
				points.Add(new Point2D { Lat = double.Parse(AggregateParam.Instance.MaxLat), Lon = double.Parse(AggregateParam.Instance.MinLon) });
				points.Add(new Point2D { Lat = double.Parse(AggregateParam.Instance.MaxLat), Lon = double.Parse(AggregateParam.Instance.MaxLon) });
				points.Add(new Point2D { Lat = double.Parse(AggregateParam.Instance.MinLat), Lon = double.Parse(AggregateParam.Instance.MaxLon) });
				points.Add(new Point2D { Lat = double.Parse(AggregateParam.Instance.MinLat), Lon = double.Parse(AggregateParam.Instance.MinLon) });
				rangeKmlData.Points = points;
				kmlDataList.Add(rangeKmlData);
			}

			// 処理対象の解析エリアを出力
			foreach (var area in AggregateParam.Instance.AnalyzeAreaList)
			{
				CommonManager.KMLData kmlData;
				kmlData.Id = area.Id;
				kmlData.Name = string.IsNullOrEmpty(area.Name) ? kmlData.Id : area.Name;
				kmlData.Points = area.Points;
				kmlDataList.Add(kmlData);
			}
			CommonManager.Instance.WriteKMLFile(kmlDataList, CommonManager.AreaKmlTempPath);
		}

		#endregion


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

			AggregateParam param = AggregateParam.Instance;
			if (string.IsNullOrEmpty(param.AnalyzeResultPath) || !Directory.Exists(param.AnalyzeResultPath))
			{
				ErrorControlList.Add(AnalyzeResultPath);
			}

			if (string.IsNullOrEmpty(param.OutputResultDirectory))
			{
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

			// 範囲指定時、範囲内に解析結果が無い場合はエラー
			if (SelectAreaRadio.IsChecked == true)
			{
				AggregateParam.Instance.AnalyzeAreaList.Clear();

				List<Point2D> range = new List<Point2D>();
				range.Add(new Point2D { Lat = double.Parse(AggregateParam.Instance.MaxLat), Lon = double.Parse(AggregateParam.Instance.MinLon) });
				range.Add(new Point2D { Lat = double.Parse(AggregateParam.Instance.MaxLat), Lon = double.Parse(AggregateParam.Instance.MaxLon) });
				range.Add(new Point2D { Lat = double.Parse(AggregateParam.Instance.MinLat), Lon = double.Parse(AggregateParam.Instance.MaxLon) });
				range.Add(new Point2D { Lat = double.Parse(AggregateParam.Instance.MinLat), Lon = double.Parse(AggregateParam.Instance.MinLon) });

				foreach (var area in AnalyzeParam.Instance.AreaList)
				{
					if (area.Exclusion_flg) continue;

					bool inside = false;

					// 範囲内に解析エリアがあるか
					foreach (var pt in area.Points)
					{
						if (GeoUtil.Instance.IsPointInPolygon(pt, range))
						{
							inside = true;
							break;
						}
					}
					if (inside)
					{
						AggregateParam.Instance.AnalyzeAreaList.Add(area);
						continue;
					}

					// 解析エリア内に範囲があるか
					foreach (var pt in range)
					{
						if (GeoUtil.Instance.IsPointInPolygon(pt, area.Points))
						{
							inside = true;
							break;
						}
					}
					if (inside)
					{
						AggregateParam.Instance.AnalyzeAreaList.Add(area);
					}
				}
			}
			else
			{
				AggregateParam.Instance.AnalyzeAreaList = AnalyzeParam.Instance.AreaList.Where(x => !x.Exclusion_flg).ToList();
			}

			if (AggregateParam.Instance.AnalyzeAreaList.Count() == 0)
			{
				MessageBox.Show("集計範囲内に解析結果がありません。\n選択範囲を修正してください。", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Warning);
				return false;

			}

			// CityGMLフォルダチェック(指定しなくてもエラーにはならない)
			if (!citygmlDirExists())
			{
				var result = MessageBox.Show("解析時に使用した3D都市モデルフォルダが見つかりませんでした。\n" +
											"災害リスク等による適地判定を行う場合は3D都市モデルフォルダを指定してください。\n" +
											"フォルダを指定しますか？",
											"確認", MessageBoxButton.YesNo, MessageBoxImage.Warning);
				if (result == MessageBoxResult.Yes)
				{
					CommonManager.Instance.ShowFolderBrowserDialog(out string dir, "", param.AnalyzeResultPath);
					if (!string.IsNullOrEmpty(dir))
					{
						AggregateParam.Instance.CityModelPath = dir;
					}
				}
			}

			return true;

		}

		/// <summary>
		/// 入力した3D都市モデルデータに必要なCityGMLが存在するかどうか
		/// </summary>
		/// <returns></returns>
		private bool citygmlDirExists()
		{
			// 解析時のパラメータからCityGMLフォルダパスを取得
			string paramFile = System.IO.Path.Combine(AggregateParam.Instance.AnalyzeResultPath, ResultData.ParamData.FILE_ANALYZE_PARAM);
			INIFile iniFile = new INIFile(paramFile);
			AggregateParam.Instance.CityModelPath = iniFile.GetString("InputData", "CityModel", "");

			if (!Directory.Exists(AggregateParam.Instance.CityModelPath)) return false;

			string udxDir = System.IO.Path.Combine(AggregateParam.Instance.CityModelPath, "udx");
			if (!System.IO.Directory.Exists(udxDir)) return false;

			return true;
		}

		/// <summary>
		/// バインディング定義
		/// </summary>
		private void SetBindings()
		{
			try
			{
				LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

				var binding = new Binding(nameof(AggregateParam.Instance.AnalyzeResultPath))
				{
					Source = AggregateParam.Instance,
					Mode = BindingMode.TwoWay,
					UpdateSourceTrigger = UpdateSourceTrigger.LostFocus
				};
				AnalyzeResultPath.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AggregateParam.Instance.MaxLat))
				{
					Source = AggregateParam.Instance,
					Mode = BindingMode.TwoWay,
				};
				TextMaxLat.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AggregateParam.Instance.MinLat))
				{
					Source = AggregateParam.Instance,
					Mode = BindingMode.TwoWay,
				};
				TextMinLat.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AggregateParam.Instance.MaxLon))
				{
					Source = AggregateParam.Instance,
					Mode = BindingMode.TwoWay,
				};
				TextMaxLon.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AggregateParam.Instance.MinLon))
				{
					Source = AggregateParam.Instance,
					Mode = BindingMode.TwoWay,
				};
				TextMinLon.SetBinding(TextBox.TextProperty, binding);

				binding = new Binding(nameof(AggregateParam.Instance.OutputResultDirectory))
				{
					Source = AggregateParam.Instance,
					Mode = BindingMode.TwoWay,
					UpdateSourceTrigger = UpdateSourceTrigger.LostFocus
				};
				OutputDirPath.SetBinding(TextBox.TextProperty, binding);

			}
			catch (Exception ex)
			{
				CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
				LogManager.Instance.ExceptionLog(ex);
			}
		}

		private void PageAggregateDataIO_Loaded(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			// ツールチップ設定
			AnalyzeResultToolTip.Content = "解析結果フォルダを指定します。\n「解析_yyyymmddHHMM（日付）」のフォルダを指定してください。";

			if (Directory.Exists(AggregateParam.Instance.AnalyzeResultPath))
			{
				// 解析パラメータファイルパス
				string paramFile = System.IO.Path.Combine(AggregateParam.Instance.AnalyzeResultPath, ResultData.ParamData.FILE_ANALYZE_PARAM);
				// 解析エリアを読み込む
				ParamFileManager.Instance.ReadAreaDataParams(paramFile);

				AreaSettings.IsEnabled = true;
			}
			else
			{
				AreaSettings.IsEnabled = false;
			}

			WebMapArea_Init();
		}

		private void AnalyzeResultPathTextBox_LostFocus(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			if (!string.IsNullOrEmpty(AggregateParam.Instance.AnalyzeResultPath))
			{
				AreaSettings.IsEnabled = true;
				MapDataSelection();
			}
		}

		private void SelectAnalyzeResultButton_Click(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			string resultDir = AnalyzeResultPath.Text;
			CommonManager.Instance.ShowFolderBrowserDialog(out string dir, "", resultDir);
			if (string.IsNullOrEmpty(dir)) return;

			AggregateParam.Instance.AnalyzeResultPath = dir;

			if (!string.IsNullOrEmpty(AggregateParam.Instance.AnalyzeResultPath))
			{
				AreaSettings.IsEnabled = true;
				MapDataSelection();
			}
		}

		private void SelectAreaRadio_Checked(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			AggregateParam.Instance.SelectArea = true;

			object[] array = new object[1];
			array[0] = int.Parse(SelectAreaRadio.Uid);
			WebMapArea.InvokeScript("ChangeExtent", array);
		}

		private void AllAreaRadio_Checked(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			if (!string.IsNullOrEmpty(AnalyzeResultPath.Text))
			{
				AggregateParam.Instance.MaxLat = dbl_topMax.ToString();
				AggregateParam.Instance.MinLat = dbl_bottomMax.ToString();
				AggregateParam.Instance.MaxLon = dbl_rightMax.ToString();
				AggregateParam.Instance.MinLon = dbl_leftMax.ToString();
			}

			AggregateParam.Instance.SelectArea = false;

			object[] array = new object[1];
			array[0] = int.Parse(AllAreaRadio.Uid);
			WebMapArea.InvokeScript("ChangeExtent", array);
		}

		/// <summary>
		/// 更新ボタン
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void UpdateCoordinateButton_Click(object sender, EventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			UpdateCoordinate();
		}

		private void SelectOutputDirButton_Click(object sender, RoutedEventArgs e)
		{
			LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

			string resultDir = OutputDirPath.Text;
			CommonManager.Instance.ShowFolderBrowserDialog(out string dir, "", resultDir);
			if (string.IsNullOrEmpty(dir)) return;
			AggregateParam.Instance.OutputResultDirectory = dir;

		}

		private void ViewEntireButton_Click(object sender, RoutedEventArgs e)
		{
			MapDataSelection(true);
		}

	}
}
