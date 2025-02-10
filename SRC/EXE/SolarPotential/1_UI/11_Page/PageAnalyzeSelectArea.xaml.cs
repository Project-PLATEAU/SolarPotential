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
using System.Xml.Linq;
using DotSpatial.Projections;
using System.Windows.Threading;

namespace SolarPotential
{
    /// <summary>
    /// Page1.xaml の相互作用ロジック
    /// </summary>
    public partial class PageAnalyzeSelectArea : Page
    {
        /// <summary>
        /// 地図表示フラグ
        /// </summary>
        bool IsDispMap = false;

        /// <summary>
        /// 選択中の範囲ID
        /// </summary>
        int SelectId = -1;

        /// <summary>
        /// ユーザー指定シェープデータ情報
        /// </summary>
        class UserShpInfo
        {
            public string shpFile { get; set; } = "";
            public List<string> coordinates { get; set; } = new List<string>();
            public CommonManager.DatumTypes type { get; set; } = CommonManager.DatumTypes.None;

        }

        /// <summary>
        /// ユーザー指定シェープデータ
        /// </summary>
        UserShpInfo[] UserShpData = new UserShpInfo[3];

        public PageAnalyzeSelectArea()
        {
            InitializeComponent();

            SetBindings();

            // スクリプトエラー抑止
            var axIWebBrowser2 = typeof(WebBrowser).GetProperty("AxIWebBrowser2", BindingFlags.Instance | BindingFlags.NonPublic);
            var comObj = axIWebBrowser2.GetValue(WebMapArea, null);
            comObj.GetType().InvokeMember("Silent", BindingFlags.SetProperty, null, comObj, new object[] { true });

            // Events
            Loaded += PageAnalyzeSelectArea_Loaded;
            //WebMapArea.LoadCompleted += (ss, ee) => { MapDisp(); };

            // ユーザー指定SHP初期化
            for (int i = 0; i < 3; i++)
                UserShpData[i] = new UserShpInfo();

        }

        #region 地図表示

        /// <summary>
        /// 地図表示エリアを初期化
        /// </summary>
        /// <returns></returns>
        private void WebMapArea_Init()
        {
            try
            {
                LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

                // 初期化
                _3_Class.ExternalMapObj extMapObj = new _3_Class.ExternalMapObj();
                WebMapArea.ObjectForScripting = extMapObj;

                extMapObj.MoveCenter += new _3_Class.ExternalMapObj.MoveCenterEventHandler(this.MoveCenter);
                extMapObj.AddArea += new _3_Class.ExternalMapObj.AddAreaEventHandler(this.AddArea);
                extMapObj.EditArea += new _3_Class.ExternalMapObj.EditAreaEventHandler(this.EditArea);
                extMapObj.DeleteArea += new _3_Class.ExternalMapObj.DeleteAreaEventHandler(this.DeleteArea);

                var cls_hml = new _3_Class.ClassAnalyzeHTML();
                WebMapArea.NavigateToString(cls_hml.GetHTMLText());

                IsDispMap = false;
                MapShowButton.IsEnabled = true;
            }
            catch (Exception ex)
            {
                CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
                LogManager.Instance.ExceptionLog(ex);
            }
        }

        /// <summary>
        /// 地図を表示
        /// </summary>
        public void MapDisp()
        {
            try
            {
                LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

                // 3D都市モデルデータから表示範囲を取得
                object[] array = new object[4];
                array[0] = AnalyzeParam.Instance.InputData.MaxPos.Lat;  // 最大緯度
                array[1] = AnalyzeParam.Instance.InputData.MinPos.Lat;  // 最小緯度
                array[2] = AnalyzeParam.Instance.InputData.MinPos.Lon;  // 最小経度
                array[3] = AnalyzeParam.Instance.InputData.MaxPos.Lon;  // 最大経度

                if (CommonManager.CheckEnableDispMap())
                {
                    WebMapArea.InvokeScript("MapDisp", array);
                    IsDispMap = true;
                }
                else
                {
                    MessageBox.Show("ネットワーク接続エラーのため、地図を表示できませんでした。", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Warning);
                    IsDispMap = false;
                }

                MapShowButton.IsEnabled = false;
            }
            catch (Exception ex)
            {
                CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
                LogManager.Instance.ExceptionLog(ex);
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
        /// 範囲を追加
        /// </summary>
        /// <param name="id"></param>
        /// <param name="coordinates"></param>
        public void AddArea(int id, string coordinates)
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
            LogManager.Instance.OutputLogMessage($"id = {id}, coordinates = {coordinates}", LogManager.LogType.DEBUG);

            AreaData area = new AreaData();
            area.Id = $"A{string.Format("{0:D3}", id)}";
            area.FeatureId = id;
            area.Name = "";
            area.Directions = CommonManager.Directions4;
            area.Direction = 2; // 南向き
            area.Water = false;
            area.AnalyzeBuild = AnalyzeParam.Instance.Target.Build;
            area.AnalyzeLand = AnalyzeParam.Instance.Target.Land;
            area.Degree = 15;
            if (!string.IsNullOrEmpty(coordinates))
            {
                string[] coords = coordinates.Split(',');
                for (int n = 0; n < coords.Count() - 1; n += 2)
                {
                    double.TryParse(coords[n], out double lon);
                    double.TryParse(coords[n+1], out double lat);
                    Point2D pt = new Point2D { Lat = lat, Lon = lon };
                    area.Points.Add(pt);
                }

            }
            AnalyzeParam.Instance.AreaList.Add(area);
            AnalyzeParam.Instance.LastAreaIdNum = id;
        }

        /// <summary>
        /// 範囲を編集
        /// </summary>
        /// <param name="id"></param>
        /// <param name="coordinates"></param>
        public void EditArea(int id, string coordinates)
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
            LogManager.Instance.OutputLogMessage($"id = {id}, coordinates = {coordinates}", LogManager.LogType.DEBUG);

            var area = AnalyzeParam.Instance.AreaList.Where(x => x.FeatureId == id).FirstOrDefault();
            if (area != null)
            {
                if (!string.IsNullOrEmpty(coordinates))
                {
                    area.Points.Clear();

                    string[] coords = coordinates.Split(',');
                    for (int n = 0; n < coords.Count() - 1; n += 2)
                    {
                        double.TryParse(coords[n], out double lon);
                        double.TryParse(coords[n + 1], out double lat);
                        Point2D pt = new Point2D { Lat = lat, Lon = lon };
                        area.Points.Add(pt);
                    }
                }
            }
        }

        /// <summary>
        /// 範囲を削除
        /// </summary>
        /// <param name="id"></param>
        public void DeleteArea(int id)
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
            LogManager.Instance.OutputLogMessage($"id = {id}", LogManager.LogType.DEBUG);

            var area = AnalyzeParam.Instance.AreaList.Where(x => x.FeatureId == id).FirstOrDefault();
            if (area != null)
            {
                AnalyzeParam.Instance.AreaList.Remove(area);
            }

            if (AnalyzeParam.Instance.AreaList.Count() == 0)
            {
                // IDを初期値に戻す
                AnalyzeParam.Instance.LastAreaIdNum = 0;
                WebMapArea.InvokeScript("InitId");
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
        /// 地図を画像に保存
        /// </summary>
        public void SaveMapImage()
        {
            if (!IsDispMap) return;
            if (AnalyzeParam.Instance.AreaList.Count() == 0) return;

            if (!AnalyzeParam.Instance.Target.Land &&
                AnalyzeParam.Instance.AreaList.Where(x => x.Id.Equals(AnalyzeParam.ALLAREA_ID)).FirstOrDefault() != null)
            {
                object[] array = new object[4];
                array[0] = AnalyzeParam.Instance.InputData.MaxPos.Lat;  // 最大緯度
                array[1] = AnalyzeParam.Instance.InputData.MinPos.Lat;  // 最小緯度
                array[2] = AnalyzeParam.Instance.InputData.MinPos.Lon;  // 最小経度
                array[3] = AnalyzeParam.Instance.InputData.MaxPos.Lon;  // 最大経度
                WebMapArea.InvokeScript("FitEntireArea", array);
            }
            else
            {
                // 範囲に合わせて表示する
                WebMapArea.InvokeScript("FitArea");
            }
            DoEvents();     // 画面を強制的に再描画

            WebMapArea.InvokeScript("MapCaptureBefore");

            Image img = new Image();
            img.Width = (int)WebMapArea.ActualWidth; img.Height = (int)WebMapArea.ActualHeight;
            img.Source = new DrawingImage(VisualTreeHelper.GetDrawing(WebMapArea));

            string jpgFile = $"{ResultData.AnalyzeResult.AREADATA_NAME}.jpg";
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

            // KMLファイル出力
            string kmlFile = $"{ResultData.AnalyzeResult.AREADATA_NAME}.kml";
            CommonManager.AreaKmlTempPath = System.IO.Path.Combine(tempDir, kmlFile);
            List<CommonManager.KMLData> kmlDataList = new List<CommonManager.KMLData>();
            foreach( var area in AnalyzeParam.Instance.AreaList)
            {
                if (area.Exclusion_flg) continue;

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
        /// ユーザー指定データを読み込む
        /// </summary>
        /// <param name="file">シェープファイル</param>
        /// <param name="strcoord">取得したXY座標列(x0,y0,x1,y1,...)</param>
        private bool ReadUserShpData(string file, CommonManager.DatumTypes type, out List<string> coordList)
        {
            coordList = new List<string>();

            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);
            if (!File.Exists(file)) return false;

            if (type == CommonManager.DatumTypes.None) return false;

            var layer = DotSpatial.Data.DataManager.DefaultDataManager.OpenFile(file) as DotSpatial.Data.FeatureSet;
            foreach (var feature in layer.Features)
            {
                string strcoord = "";
                foreach (var coord in feature.Geometry.Coordinates)
                {
                    switch (type)
                    {
                        case CommonManager.DatumTypes.LatLon:
                            strcoord += $"{coord.X},{coord.Y},";
                            break;
                        case CommonManager.DatumTypes.XY:
                            // 緯度経度に変換
                            GeoUtil.Instance.XYToLatLon(coord.X, coord.Y, out double lat, out double lon);
                            strcoord += $"{lon},{lat},";
                            break;
                        default:
                            MessageBox.Show("指定したシェープファイルの読み込みに失敗しました。", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Error);
                            return false;
                    }


                }
                strcoord = strcoord.TrimEnd(',');
                coordList.Add(strcoord);
            }

            return true;
        }

        /// <summary>
        /// ユーザー指定シェープファイルを表示する
        /// </summary>
        /// <param name="num"></param>
        private void ShowUserShp(int num)
        {
            if (!IsDispMap) return;
            if (string.IsNullOrEmpty(UserShpData[num - 1].shpFile)) return;

            foreach (var coord in UserShpData[num - 1].coordinates)
            {
                object[] array = new object[2];
                array[0] = coord;
                array[1] = num;
                WebMapArea.InvokeScript("feature_add_usershp", array);
            }
        }

        /// <summary>
        /// ユーザー指定シェープファイルを非表示にする
        /// </summary>
        /// <param name="num"></param>
        private void HideUserShp(int num)
        {
            if (!IsDispMap) return;

            object[] array = new object[1];
            array[0] = num;
            WebMapArea.InvokeScript("feature_delete_usershp", array);
        }

        /// <summary>
        /// 地図の範囲を再描画する
        /// </summary>
        private void RedrawArea()
        {
            if (!IsDispMap) return;

            // 範囲を削除
            WebMapArea.InvokeScript("feature_deleteAll");

            // 再描画
            foreach (var area in AnalyzeParam.Instance.AreaList)
            {
                if (area.Id.Equals(AnalyzeParam.ALLAREA_ID)) continue;
                if (area.Exclusion_flg) continue;

                string coord = "";
                foreach (var pt in area.Points)
                {
                    coord += $"{pt.Lon},{pt.Lat},";
                }
                coord = coord.TrimEnd(',');
                object[] array = new object[2];
                array[0] = coord;
                array[1] = area.FeatureId;
                WebMapArea.InvokeScript("feature_draw", array);
            }
        }

        /// <summary>
        /// 入力値チェック
        /// </summary>
        /// <returns></returns>
        public bool CheckParams()
        {
            if (AnalyzeParam.Instance.Target.Land)
            {
                var areaList = AnalyzeParam.Instance.AreaList.Where(x => x.Id != AnalyzeParam.ALLAREA_ID).ToList();
                if (areaList == null || areaList.Count() == 0)
                {
                    if (string.IsNullOrEmpty(AnalyzeParam.Instance.InputData.LandData))
                    {
                        // 土地の解析が有効＋土地データ選択なし
                        var result = MessageBox.Show("解析する土地の範囲が選択されていません。\n" +
                                    "土地範囲指定データを入力する場合は「はい」、\n地図上から範囲を追加する場合は「いいえ」を押してください。", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.YesNoCancel, MessageBoxImage.Warning);
                        if (result == MessageBoxResult.Yes)
                        {
                            return true;
                        }
                        else
                        {
                            return false;
                        }
                    }
                }
            }

            if (AnalyzeParam.Instance.Target.Build)
            {
                var areaList = AnalyzeParam.Instance.AreaList.Where(x => !x.IsShpData && !x.Exclusion_flg).ToList();
                if (areaList == null || areaList.Count() == 0)
                {
                    var result = MessageBox.Show("入力した3D都市モデルのLOD2整備範囲全域を対象に解析します。\n" +
                        "処理に数日かかる可能性があります。\n解析エリアを選択せずに、続けてよろしいですか？", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.YesNo, MessageBoxImage.Question);
                    if (result == MessageBoxResult.No)
                    {
                        return false;
                    }
                    else
                    {
                        // 全域を解析エリアとして追加
                        AreaData area = new AreaData();
                        area.Id = AnalyzeParam.ALLAREA_ID;
                        area.Name = AnalyzeParam.ALLAREA_NAME;
                        area.Directions = CommonManager.Directions4;
                        area.Direction = 2; // 南向き
                        area.Water = false;
                        area.AnalyzeBuild = true;
                        area.AnalyzeLand = false;   // 全域は土地解析しない
                        area.Degree = 15;
                        var MinPos = AnalyzeParam.Instance.InputData.MinPos;
                        var MaxPos = AnalyzeParam.Instance.InputData.MaxPos;
                        area.Points.Add(new Point2D { Lat = MinPos.Lat, Lon = MinPos.Lon });  // 左下
                        area.Points.Add(new Point2D { Lat = MinPos.Lat, Lon = MaxPos.Lon });  // 右下
                        area.Points.Add(new Point2D { Lat = MaxPos.Lat, Lon = MaxPos.Lon });  // 右上
                        area.Points.Add(new Point2D { Lat = MaxPos.Lat, Lon = MinPos.Lon });  // 左上
                        AnalyzeParam.Instance.AreaList.Add(area);
                    }
                }
            }

            // 範囲が重なっていないかチェック
            var targetList = AnalyzeParam.Instance.AreaList.Where(x => !x.Exclusion_flg && !x.Id.Equals(AnalyzeParam.ALLAREA_ID)).ToList();
            var tmpList = targetList.ToList();
            foreach (var targetArea in targetList)
            {
                tmpList.Remove(targetArea);

                foreach (var area in tmpList)
                {
                    foreach (var pt in area.Points)
                    {
                        if (GeoUtil.Instance.IsPointInPolygon(pt, targetArea.Points))
                        {
                            // 範囲が重なっている
                            MessageBox.Show("指定したエリアが重なっている箇所があります。", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Warning);
                            return false;
                        }
                    }
                }
            }

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

                var binding = new Binding(nameof(AnalyzeParam.Instance.AreaList))
                {
                    Source = AnalyzeParam.Instance,
                    Mode = BindingMode.OneWay,
                };
                ListSelectArea.SetBinding(ListView.ItemsSourceProperty, binding);

                binding = new Binding(nameof(AnalyzeParam.Instance.AreaOutputImageRange))
                {
                    Source = AnalyzeParam.Instance,
                    Mode = BindingMode.TwoWay,
                };
                OutputImageRange.SetBinding(TextBox.TextProperty, binding);

            }
            catch (Exception ex)
            {
                CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
                LogManager.Instance.ExceptionLog(ex);
            }
        }

        private void PageAnalyzeSelectArea_Loaded(object sender, RoutedEventArgs e)
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            // 座標系コンボボックス
            DatumComboBox1.ItemsSource = CommonManager.DatumTypeList;
            DatumComboBox2.ItemsSource = CommonManager.DatumTypeList;
            DatumComboBox3.ItemsSource = CommonManager.DatumTypeList;

            // 地図の初期化
            WebMapArea_Init();

            // 全域がある場合は一旦エリアリストから削除
            var area = AnalyzeParam.Instance.AreaList.Where(x => x.Id == AnalyzeParam.ALLAREA_ID).FirstOrDefault();
            if (area != null)
            {
                AnalyzeParam.Instance.AreaList.Remove(area);
            }

            SelectId = -1;
        }

        private void MapShowButton_Click(object sender, RoutedEventArgs e)
        {
            // 地図を表示
            MapDisp();

            try
            {
                // 選択済の範囲があれば描画する
                foreach (var area in AnalyzeParam.Instance.AreaList)
                {
                    if (area.Id.Equals(AnalyzeParam.ALLAREA_ID)) continue;
                    if (area.Exclusion_flg) continue;

                    string coord = "";
                    foreach( var pt in area.Points)
                    {
                        coord += $"{pt.Lon},{pt.Lat},";
                    }
                    coord = coord.TrimEnd(',');
                    object[] array = new object[2];
                    array[0] = coord;
                    array[1] = area.FeatureId;
                    WebMapArea.InvokeScript("feature_draw", array);
                    WebMapArea.InvokeScript("FitArea");
                }
                WebMapArea.InvokeScript("UpdateId", new object[] { AnalyzeParam.Instance.LastAreaIdNum });

                // 補助情報を表示
                for (int i = 1; i <= 3; i++ )
                {
                    CheckBox checkBox = this.FindName($"SelectDataCheckBox{i}") as CheckBox;
                    if (checkBox.IsChecked == true)
                    {
                        ShowUserShp(i);
                    }
                }

            }
            catch (Exception ex)
            {
                CommonManager.Instance.ShowExceptionMessageBox(ex.Message);
                LogManager.Instance.ExceptionLog(ex);
            }

        }

        private void ShowUserData_Checked(object sender, RoutedEventArgs e)
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            CheckBox checkBox = e.Source as CheckBox;
            if (checkBox.IsChecked == true)
            {
                string ctrlName = checkBox.Tag.ToString();
                int num = int.Parse(ctrlName.Substring(ctrlName.Length - 1));

                TextBox tb = this.FindName(ctrlName) as TextBox;
                string file = tb.Text;

                if (!file.Equals(UserShpData[num - 1].shpFile))
                {
                    ComboBox cb = this.FindName($"DatumComboBox{num}") as ComboBox;
                    var type = (CommonManager.DatumTypes)cb.SelectedIndex;

                    if (ReadUserShpData(file, type, out List<string> coordinates))
                    {
                        UserShpData[num - 1].shpFile = file;
                        UserShpData[num - 1].coordinates = coordinates;

                        HideUserShp(num);
                    }
                }

                ShowUserShp(num);

            }
        }

        private void ShowUserData_Unchecked(object sender, RoutedEventArgs e)
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            CheckBox checkBox = e.Source as CheckBox;
            if (checkBox.IsChecked == false)
            {
                string ctrlName = checkBox.Tag.ToString();
                int num = int.Parse(ctrlName.Substring(ctrlName.Length - 1));
                HideUserShp(num);
            }
        }

        private void SelectDataPath_LostFocus(object sender, RoutedEventArgs e)
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            TextBox tb = e.Source as TextBox;
            string file = tb.Text;
            if (string.IsNullOrEmpty(file)) return;
            if (!File.Exists(file))
            {
                MessageBox.Show("指定したシェープファイルが見つかりませんでした。", CommonManager.TEXT_SYSTEM_CAPTION, MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            string ctrlName = tb.Name;
            int num = int.Parse(ctrlName.Substring(ctrlName.Length - 1));

            if (string.IsNullOrEmpty(file) || !file.Equals(UserShpData[num - 1].shpFile))
            {
                ComboBox cb = this.FindName($"DatumComboBox{num}") as ComboBox;
                var type = (CommonManager.DatumTypes)cb.SelectedIndex;

                if (ReadUserShpData(file, type, out List<string> coordinates))
                {
                    UserShpData[num - 1].shpFile = file;
                    UserShpData[num - 1].coordinates = coordinates;

                    HideUserShp(num);
                }
            }
            CheckBox checkBox = this.FindName($"SelectDataCheckBox{num}") as CheckBox;
            if (checkBox.IsChecked == true)
            {
                ShowUserShp(num);
            }

        }

        private void SelectDataButton_Click(object sender, RoutedEventArgs e)
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            Button btn = e.Source as Button;
            string ctrlName = btn.Name;
            int num = int.Parse(ctrlName.Substring(ctrlName.Length - 1));

            string file = UserShpData[num - 1].shpFile;
            CommonManager.Instance.ShowSelectFileDialog(out string selectPath, "シェープファイルを選択します。", file, CommonManager.EXT_FILTER_SHP);
            if (string.IsNullOrEmpty(selectPath)) return;

            // テキストボックスに反映
            TextBox tb = this.FindName($"SelectDataPath{num}") as TextBox;
            tb.Text = selectPath;
            LogManager.Instance.OutputLogMessage($"選択したシェープファイル：{selectPath}", LogManager.LogType.INFO);

            if (string.IsNullOrEmpty(file) || !file.Equals(selectPath))
            {
                ComboBox cb = this.FindName($"DatumComboBox{num}") as ComboBox;
                var type = (CommonManager.DatumTypes)cb.SelectedIndex;

                if (ReadUserShpData(selectPath, type, out List<string> coordinates))
                {
                    UserShpData[num - 1].shpFile = selectPath;
                    UserShpData[num - 1].coordinates = coordinates;

                    HideUserShp(num);
                }
            }

            CheckBox checkBox = this.FindName($"SelectDataCheckBox{num}") as CheckBox;
            if (checkBox.IsChecked == true)
            {
                ShowUserShp(num);
            }

        }

        private void ListSelectArea_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (!IsDispMap) return;

            var item = ListSelectArea.SelectedItem;
            if (item == null) return;

            AreaData area = item as AreaData;
            if (SelectId != area.FeatureId)
            {
                // 前のエリアが除外対象の場合は再描画する
                var preArea = AnalyzeParam.Instance.AreaList.Where(x => x.FeatureId == SelectId).FirstOrDefault();
                if (preArea != null && preArea.Exclusion_flg)
                {
                    RedrawArea();
                }

                // 選択範囲が除外対象になっている場合は図形を表示する
                if (area.Exclusion_flg)
                {
                    string coord = "";
                    foreach (var pt in area.Points)
                    {
                        coord += $"{pt.Lon},{pt.Lat},";
                    }
                    coord = coord.TrimEnd(',');
                    object[] array1 = new object[2];
                    array1[0] = coord;
                    array1[1] = area.FeatureId;
                    WebMapArea.InvokeScript("feature_draw", array1);
                }

                SelectId = area.FeatureId;

                // 範囲を選択
                object[] array = new object[1];
                array[0] = SelectId;
                WebMapArea.InvokeScript("feature_select", array);
            }
        }

        private void DatumComboBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            LogManager.Instance.MethodStartLog(GetType().Name, System.Reflection.MethodBase.GetCurrentMethod().Name);

            ComboBox cb = e.Source as ComboBox;
            var type = (CommonManager.DatumTypes)cb.SelectedIndex;
            if (type == CommonManager.DatumTypes.None) return;

            string ctrlName = cb.Tag.ToString(); ;
            int num = int.Parse(ctrlName.Substring(ctrlName.Length - 1));

            TextBox tb = this.FindName(ctrlName) as TextBox;
            string file = tb.Text;

            if (!type.Equals(UserShpData[num - 1].type))
            {
                if (ReadUserShpData(file, type, out List<string> coordinates))
                {
                    UserShpData[num - 1].shpFile = file;
                    UserShpData[num - 1].coordinates = coordinates;

                    HideUserShp(num);
                }
            }

            CheckBox checkBox = this.FindName($"SelectDataCheckBox{num}") as CheckBox;
            if (checkBox.IsChecked == true)
            {
                ShowUserShp(num);
            }

        }

        private void ExclusionCheckBox_ChangeChecked(object sender, RoutedEventArgs e)
        {
            // 再描画する
            RedrawArea();
        }
    }

}
