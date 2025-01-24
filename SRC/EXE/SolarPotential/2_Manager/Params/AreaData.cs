using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SolarPotential
{
    class AreaData
    {
        /// <summary>
        /// 実行除外フラグ
        /// </summary>
        public bool Exclusion_flg { get; set; } = false;

        /// <summary>
        /// エリアID
        /// </summary>
        public string Id { get; set; } = "";

        /// <summary>
        /// フィーチャID(js管理用)
        /// </summary>
        public int FeatureId { get; set; } = 0;

        /// <summary>
        /// 名称
        /// </summary>
        public string Name { get; set; } = "";

        /// <summary>
        /// 方位(コンボボックス設定)
        /// </summary>
        public List<string> Directions { get; set; } = CommonManager.Directions4;

        /// <summary>
        /// 方位
        /// </summary>
        public int Direction { get; set; } = 0;

        /// <summary>
        /// 傾き
        /// </summary>
        public int Degree { get; set; } = 0;

        /// <summary>
        /// 水面フラグ
        /// </summary>
        public bool Water { get; set; } = false;

        /// <summary>
        /// 説明
        /// </summary>
        public string Explanation { get; set; } = "";

        /// <summary>
        /// 構成点
        /// </summary>
        public List<Point2D> Points { get; set; } = new List<Point2D>();

        /// <summary>
        /// 範囲内の建物を解析するか
        /// </summary>
        public bool AnalyzeBuild { get; set; } = false;

        /// <summary>
        /// 範囲内の土地を解析するか
        /// </summary>
        public bool AnalyzeLand { get; set; } = false;

        /// <summary>
        /// 土地範囲shpの範囲フラグ
        /// </summary>
        public bool IsShpData { get; set; } = false;

        /// <summary>
        /// ID最大長(文字)
        /// </summary>
        public const int MAX_AREA_ID_LEN = 10;

        /// <summary>
        /// 名称最大長
        /// </summary>
        public const int MAX_AREA_NAME_LEN = 100;

        /// <summary>
        /// 構成点の最大数
        /// </summary>
        public const int MAX_POINT_LEN = 256;

        private static AreaData _instance = null;

        public static AreaData Instance
        {
            get
            {
                if (_instance == null)
                {
                    _instance = new AreaData();
                }
                return _instance;
            }
        }
    }

    /// <summary>
    /// 2D座標管理クラス
    /// </summary>
    class Point2D
    {
        /// <summary>
        /// 経度(X)
        /// </summary>
        public double Lon { get; set; }

        /// <summary>
        /// 緯度(Y)
        /// </summary>
        public double Lat { get; set; }
    }

}
