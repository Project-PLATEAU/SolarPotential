using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SolarPotential
{
    class GeoUtil
    {
        /// <summary>
        /// 系番号
        /// </summary>
        public int JPZone { get; set; } = 0;

        /// <summary>
        /// 平面直角座標系の原点(単位：秒)
        /// </summary>
        static double[] JPN_ORG_LAT_SEC = {
            0.0,
            118800.0, 118800.0, 129600.0, 118800.0,129600.0, 129600.0,129600.0,
            129600.0, 129600.0, 144000.0, 158400.0, 158400.0,158400.0, 93600.0,
            93600.0,  93600.0,  93600.0,72000.0, 93600.0 };
        static double[] JPN_ORG_LON_SEC = {
            0.0,
            466200.0, 471600.0, 475800.0, 480600.0, 483600.0, 489600.0, 493800.0,
            498600.0, 503400.0, 507000.0, 504900.0, 512100.0, 519300.0, 511200.0,
            459000.0, 446400.0, 471600.0, 489600.0, 554400.0 };

        const double _PI = 3.141592653589793;
        const double _COEF_RAD_TO_DEG = 180.0 / _PI;
        const double _COEF_DEG_TO_RAD = _PI / 180.0;

        /// <summary>
        /// 長半径
        /// </summary>
        const double daa = 6378137;

        /// <summary>
        /// 逆扁平率
        /// </summary>
        const double dF = 298.257222101;

        /// <summary>
        /// 平面直角座標系のY軸上における縮尺係数(UTM座標系の場合→0.9996)
        /// </summary>
        const double dM0 = 0.9999;

        /// <summary>
        /// 平面直角→緯度経度
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        /// <param name="lat"></param>
        /// <param name="lon"></param>
        public void XYToLatLon(double x, double y, out double lat, out double lon)
        {
            // 平面直角座標系原点の緯度及び経度 
            double dLat0 = JPN_ORG_LAT_SEC[JPZone] / 3600.0 * _COEF_DEG_TO_RAD;
            double dLon0 = JPN_ORG_LON_SEC[JPZone] / 3600.0 * _COEF_DEG_TO_RAD;

            double dn = 1.0 / (2 * dF - 1);

            //Sφ0、A
            double[] dA = new double[6];
            dA[0] = 1.0 + Math.Pow(dn, 2) / 4.0 + Math.Pow(dn, 4) / 64.0;
            dA[1] = -3.0 / 2.0 * (dn - Math.Pow(dn, 3) / 8.0 - Math.Pow(dn, 5) / 64.0);
            dA[2] = 15.0 / 16.0 * (Math.Pow(dn, 2) - Math.Pow(dn, 4) / 4);
            dA[3] = -35.0 / 48.0 * (Math.Pow(dn, 3) - 5.0 / 16.0 * Math.Pow(dn, 5));
            dA[4] = 315.0 / 512.0 * Math.Pow(dn, 4);
            dA[5] = -693.0 / 1280.0 * Math.Pow(dn, 5);

            //β
            double[] dBt = new double[6];
            dBt[1] = 1 / 2.0 * dn - 2 / 3.0 * Math.Pow(dn, 2) + 37.0 / 96.0 * Math.Pow(dn, 3) - 1 / 360.0 * Math.Pow(dn, 4) - 81 / 512.0 * Math.Pow(dn, 5);
            dBt[2] = 1 / 48.0 * Math.Pow(dn, 2) + 1 / 15.0 * Math.Pow(dn, 3) - 437 / 1440.0 * Math.Pow(dn, 4) + 46.0 / 105.0 * Math.Pow(dn, 5);
            dBt[3] = 17 / 480.0 * Math.Pow(dn, 3) - 37 / 840.0 * Math.Pow(dn, 4) - 209 / 4480.0 * Math.Pow(dn, 5);
            dBt[4] = 4397 / 161280.0 * Math.Pow(dn, 4) - 11 / 504.0 * Math.Pow(dn, 5);
            dBt[5] = 4583 / 161280.0 * Math.Pow(dn, 5);

            //δ
            double[] dDt = new double[7];
            dDt[1] = 2 * dn - 2.0 / 3.0 * Math.Pow(dn, 2) - 2 * Math.Pow(dn, 3) + 116.0 / 45.0 * Math.Pow(dn, 4)
                + 26.0 / 45.0 * Math.Pow(dn, 5) - 2854.0 / 675.0 * Math.Pow(dn, 6);
            dDt[2] = 7.0 / 3.0 * Math.Pow(dn, 2) - 8.0 / 5.0 * Math.Pow(dn, 3) - 227.0 / 45.0 * Math.Pow(dn, 4)
                + 2704.0 / 315.0 * Math.Pow(dn, 5) + 2323.0 / 945.0 * Math.Pow(dn, 6);
            dDt[3] = 56.0 / 15.0 * Math.Pow(dn, 3) - 136.0 / 35.0 * Math.Pow(dn, 4)
                - 1262.0 / 105.0 * Math.Pow(dn, 5) + 73814.0 / 2835.0 * Math.Pow(dn, 6);
            dDt[4] = 4279.0 / 630.0 * Math.Pow(dn, 4) - 332.0 / 35.0 * Math.Pow(dn, 5) - 399572.0 / 14175.0 * Math.Pow(dn, 6);
            dDt[5] = 4174.0 / 315.0 * Math.Pow(dn, 5) - 144838.0 / 6237.0 * Math.Pow(dn, 6);
            dDt[6] = 601676.0 / 22275.0 * Math.Pow(dn, 6);

            double dAb = dM0 * daa / (1 + dn) * dA[0];
            double dSb0 = 0;
            for (int j = 1; j <= 5; j++)
            {
                dSb0 += dA[j] * Math.Sin(2 * j * dLat0);
            }
            double dSb = dM0 * daa / (1 + dn) * (dA[0] * dLat0 + dSb0);

            //ξ
            double dXi = (y + dSb) / dAb;
            // η
            double dEt = x / dAb;

            //ξ’・η'・σ'・τ'・χ
            double dXi2tmp = 0;
            double dEt2tmp = 0;
            double dSg2tmp = 0;
            double dTu2tmp = 0;
            for (int j = 1; j <= 5; j++)
            {
                dXi2tmp += dBt[j] * Math.Sin(2 * j * dXi) * Math.Cosh(2 * j * dEt);
                dEt2tmp += dBt[j] * Math.Cos(2 * j * dXi) * Math.Sinh(2 * j * dEt);
                dSg2tmp += dBt[j] * Math.Cos(2 * j * dXi) * Math.Cosh(2 * j * dEt);
                dTu2tmp += dBt[j] * Math.Sin(2 * j * dXi) * Math.Sinh(2 * j * dEt);
            }
            double dXi2 = dXi - dXi2tmp;
            double dEt2 = dEt - dEt2tmp;
            double dSg2 = 1 - dSg2tmp;
            double dCi = Math.Asin(Math.Sin(dXi2) / Math.Cosh(dEt2));
            double dLatRad = dCi;
            for (int j = 1; j <= 6; j++)
            {
                dLatRad += dDt[j] * Math.Sin(2 * j * dCi);
            }
            //ラジアン単位の緯度経度
            double dLonRad = dLon0 + Math.Atan(Math.Sinh(dEt2) / Math.Cos(dXi2));

            //度単位に
            lon = dLonRad * _COEF_RAD_TO_DEG;
            lat = dLatRad * _COEF_RAD_TO_DEG;

        }

        /// <summary>
        /// 設定ファイルの系番号を更新する
        /// </summary>
        /// <returns></returns>
        public void UpdateJPZone()
        {
            INIFile iniFile = new INIFile(CommonManager.SystemIniFilePath);
            iniFile.WriteString("CoordinateSystem", "JPZone", $"{JPZone}");
        }

        /// <summary>
        /// 内外判定
        /// </summary>
        /// <param name="point"></param>
        /// <param name="poly"></param>
        public bool IsPointInPolygon(in Point2D pointTarget, in List<Point2D> poly)
        {
            int iCountCrossing = 0;

            Point2D point0 = poly[0];
            bool bFlag0x = (pointTarget.Lon <= point0.Lon);
            bool bFlag0y = (pointTarget.Lat <= point0.Lat);

            // レイの方向は、Ｘプラス方向
            int iCountPoint = poly.Count;
            for (int ui = 1; ui < iCountPoint + 1; ui++)
            {
                Point2D point1 = poly[ui % iCountPoint];    // 最後は始点が入る（多角形データの始点と終点が一致していないデータ対応）
                bool bFlag1x = (pointTarget.Lon <= point1.Lon);
                bool bFlag1y = (pointTarget.Lat <= point1.Lat);
                if (bFlag0y != bFlag1y)
                {   // 線分はレイを横切る可能性あり。
                    if (bFlag0x == bFlag1x)
                    {   // 線分の２端点は対象点に対して両方右か両方左にある
                        if (bFlag0x)
                        {   // 完全に右。⇒線分はレイを横切る
                            iCountCrossing += (bFlag0y ? -1 : 1);   // 上から下にレイを横切るときには、交差回数を１引く、下から上は１足す。
                        }
                    }
                    else
                    {   // レイと交差するかどうか、対象点と同じ高さで、対象点の右で交差するか、左で交差するかを求める。
                        if (pointTarget.Lon <= (point0.Lon + (point1.Lon - point0.Lon) * (pointTarget.Lat - point0.Lat) / (point1.Lat - point0.Lat)))
                        {   // 線分は、対象点と同じ高さで、対象点の右で交差する。⇒線分はレイを横切る
                            iCountCrossing += (bFlag0y ? -1 : 1);   // 上から下にレイを横切るときには、交差回数を１引く、下から上は１足す。
                        }
                    }
                }
                // 次の判定のために、
                point0 = point1;
                bFlag0x = bFlag1x;
                bFlag0y = bFlag1y;
            }

            // クロスカウントがゼロのとき外、ゼロ以外のとき内。
            return (0 != iCountCrossing);

        }

        private GeoUtil()
        {
            INIFile iniFile = new INIFile(CommonManager.SystemIniFilePath);
            JPZone = iniFile.GetInt("CoordinateSystem", "JPZone", 0);
        }

        private static GeoUtil _instance = null;

        public static GeoUtil Instance
        {
            get
            {
                if (_instance == null)
                {
                    _instance = new GeoUtil();
                }
                return _instance;
            }
        }
    }
}
