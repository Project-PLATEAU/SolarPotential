using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SolarPotential.ResultData
{
    class SystemData
    {
        /// <summary>
        /// システムデータ - フォルダ
        /// </summary>
        public static readonly string SYSTEM_DIRNAME = "system";

        /// <summary>
        /// システムデータ - 座標ファイル
        /// </summary>
        public static readonly string FILE_RANGE_COORDINATES = $"{SYSTEM_DIRNAME}/initFile_Coordinates.txt";

    }

    class ParamData
    {
        /// <summary>
        /// パラメータ出力 - フォルダ
        /// </summary>
        public static readonly string PARAM_DIRNAME = "実行パラメータ";

        /// <summary>
        /// パラメータ出力 - 解析パラメータファイル名
        /// </summary>
        public static readonly string ANALYZE_PARAM_FILENAME = "analyze.param";

        /// <summary>
        /// パラメータ出力 - 集計パラメータファイル名
        /// </summary>
        public static readonly string AGGREGATE_PARAM_FILENAME = "aggregate.param";

        /// <summary>
        /// パラメータ出力 - 解析パラメータファイル
        /// </summary>
        public static readonly string FILE_ANALYZE_PARAM = $"{PARAM_DIRNAME}/{ANALYZE_PARAM_FILENAME}";

        /// <summary>
        /// パラメータ出力 - 集計パラメータファイル
        /// </summary>
        public static readonly string FILE_AGGREGATE_PARAM = $"{PARAM_DIRNAME}/{AGGREGATE_PARAM_FILENAME}";

        /// <summary>
        /// 実行パラメータログファイル
        /// </summary>
        public static readonly string PARAM_LOG_FILENAME = $"実行パラメータログ.txt";
    }

    class AnalyzeResult
    {
        /// <summary>
        /// 解析 - 解析エリアデータ出力フォルダ
        /// </summary>
        public static readonly string AREADATA_NAME = "解析エリア";

        /// <summary>
        /// 解析 - シミュレーション結果出力フォルダ（日射量推計・反射シミュレーション）
        /// </summary>
        public static readonly string SIM_DIRNAME = "シミュレーション結果";

        /// <summary>
        /// 解析 - シミュレーション結果出力フォルダ（日射量推計・反射シミュレーション）
        /// </summary>
        public static readonly string BUILD_DIRNAME = $"{SIM_DIRNAME}/建物";

        /// <summary>
        /// 解析 - シミュレーション結果出力フォルダ（日射量推計・反射シミュレーション）
        /// </summary>
        public static readonly string LAND_DIRNAME = $"{SIM_DIRNAME}/土地";

        /// <summary>
        /// 解析 - CityGML出力フォルダ
        /// </summary>
        public static readonly string CITYGML_DIRNAME = "citygml/bldg";

        /// <summary>
        /// 解析日時ごとの出力フォルダ名を取得
        /// </summary>
        /// <param name="type"></param>
        /// <returns></returns>
        public static string GetResultDateDirName(AnalyzeParam.DateType type)
        {
            string result = "";
            switch (type)
            {
                case AnalyzeParam.DateType.OneMonth:
                    result = $"{ResultDataManager.RESULT_DIRNAME}_{AnalyzeParam.Instance.Month}月";
                    break;
                case AnalyzeParam.DateType.OneDay:
                    result = $"{ResultDataManager.RESULT_DIRNAME}_{AnalyzeParam.Instance.Month}月{AnalyzeParam.Instance.Day}日";
                    break;
                case AnalyzeParam.DateType.Summer:
                    result = $"{ResultDataManager.RESULT_DIRNAME}_夏至";
                    break;
                case AnalyzeParam.DateType.Winter:
                    result = $"{ResultDataManager.RESULT_DIRNAME}_冬至";
                    break;
                case AnalyzeParam.DateType.Year:
                    result = $"{ResultDataManager.RESULT_DIRNAME}_年間";
                    break;
                default:
                    break;

            }

            return result;
        }
    }

    class AggregateResult
    {
        /// <summary>
        /// 集計 - 集計範囲データ出力フォルダ
        /// </summary>
        public static readonly string RANGEDATA_NAME = "集計範囲";

        /// <summary>
        /// 集計 - 適地判定出力フォルダ
        /// </summary>
        public static readonly string JUDGE_DIRNAME = "適地判定";

        /// <summary>
        /// 集計 - 適地判定出力フォルダ
        /// </summary>
        public static readonly string AGGREGATE_DIRNAME = "集計";
    }

    class ResultDataManager
    {
        /// <summary>
        /// 共通 - 実行結果フォルダ名
        /// </summary>
        public static readonly string RESULT_DIRNAME = "実行結果";


        private ResultDataManager()
        {

        }

        private static ResultDataManager _instance = null;

        public static ResultDataManager Instance
        {
            get
            {
                if (_instance == null)
                {
                    _instance = new ResultDataManager();
                }
                return _instance;
            }
        }
    }
}
