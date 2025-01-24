#pragma once

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーからほとんど使用されていない部分を除外する

// プログラムに必要な追加ヘッダーをここで参照してください
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "../../LIB/CommonUtil/CPoint2DPolygon.h"

#include <Windows.h>
#include <string.h>

#include <stdexcept>
#include <iostream>
#include <vector>
#include <filesystem>
#include <string>
#include <tchar.h>


#include <algorithm>
#include <iterator> 

#import "msxml6.dll" named_guids raw_interfaces_only
#include <atlbase.h>	// CComVariant, CComBSTR
#include <conio.h>
#include <iomanip>
#include <fstream>
#include <sstream>


// 使用する名前空間のKey（必要になったら追加）
std::wstring namespaceKeys[] = {
     L"xmlns:core",
     L"xmlns:bldg",
     L"xmlns:gen",
     L"xmlns:gml",
     L"xmlns:uro",
     L"xmlns:wtr",
     L"xmlns:urf",

};

// CityGMLのバージョン
enum class eCityGMLVersion
{
    VERSION_1,          // 1: 2021年度版
    VERSION_2,          // 2: 2022年度版
    End
};
// operator ++
eCityGMLVersion& operator ++ (eCityGMLVersion& ver)
{
    if (ver == eCityGMLVersion::End) {
        throw std::out_of_range("for eCityGMLVersion& operator ++ (eCityGMLVersion&)");
    }
    ver = eCityGMLVersion(static_cast<std::underlying_type<eCityGMLVersion>::type>(ver) + 1);
    return ver;
}

#define XPATH1 _T("core:CityModel/core:cityObjectMember/bldg:Building/bldg:boundedBy/bldg:RoofSurface")
#define XPATH2 _T("core:CityModel/core:cityObjectMember")
#define XPATH3 _T("bldg:Building")
#define XPATH4 _T("bldg:Building/bldg:boundedBy")
#define XPATH5 _T("bldg:RoofSurface")
#define XPATH6 _T("bldg:RoofSurface/bldg:lod2MultiSurface/gml:MultiSurface/gml:surfaceMember")
#define XPATH7 _T("gml:Polygon/gml:exterior/gml:LinearRing/gml:posList")

#define XPATH_measureAttribute1 _T("bldg:Building/gen:measureAttribute[@name='予測日射量']")
#define XPATH_measureAttribute2 _T("gen:value")
#define XPATH_measuredHeight1 _T("bldg:Building/bldg:measuredHeight")

// 2021
#define XPATH_genericAttributeSet1 _T("bldg:Building/gen:genericAttributeSet[contains(@name,'洪水浸水')]")
#define XPATH_genericAttributeSet2 _T("gen:measureAttribute[@name='浸水深']")
#define XPATH_genericAttributeSet3 _T("gen:value")
#define XPATH_genericAttributeSet4 _T("bldg:Building/gen:genericAttributeSet[contains(@name,'津波浸水')]")
#define XPATH_genericAttributeSet5 _T("bldg:Building/gen:genericAttributeSet[@name='土砂災害警戒区域']")
// 2022
#define XPATH_genericAttributeSet1_2 _T("bldg:Building/uro:buildingDisasterRiskAttribute/uro:BuildingRiverFloodingRiskAttribute") // 洪水浸水
#define XPATH_genericAttributeSet2_2 _T("uro:depth")
#define XPATH_genericAttributeSet3_2 _T("bldg:Building/uro:buildingDisasterRiskAttribute/uro:BuildingTsunamiRiskAttribute") // 津波浸水
#define XPATH_genericAttributeSet4_2 _T("bldg:Building/uro:buildingDisasterRiskAttribute/uro:BuildingLandSlideRiskAttribute") // 土砂災害
#define XPATH_genericAttributeSet5_2 _T("uro:description")

// 2021
#define XPATH_buildingStructureType _T("bldg:Building/uro:buildingDetails/uro:BuildingDetails/uro:buildingStructureType")
// 2022
#define XPATH_buildingStructureType_2 _T("bldg:Building/uro:buildingDetailAttribute/uro:BuildingDetailAttribute/uro:buildingStructureType")

#define XPATH_extendedAttribute1 _T("bldg:Building/uro:extendedAttribute")
#define XPATH_extendedAttribute2 _T("uro:KeyValuePair/uro:key")
#define XPATH_extendedAttribute3 _T("uro:KeyValuePair/uro:codeValue")

#define XPATH_aggregateData1 _T("gen:measureAttribute[@name='予測日射量']")
#define XPATH_aggregateData2 _T("gen:value")
#define XPATH_aggregateData3 _T("gen:measureAttribute[@name='予測発電量']")
#define XPATH_aggregateData4 _T("gen:measureAttribute[@name='光害発生時間（夏至）']")
#define XPATH_aggregateData5 _T("gen:measureAttribute[@name='光害発生時間（春分）']")
#define XPATH_aggregateData6 _T("gen:measureAttribute[@name='光害発生時間（冬至）']")
#define XPATH_aggregateData7 _T("gen:measureAttribute[@name='光害発生時間（指定日）']")

// 建物ID
#define XPATH_stringAttribute1 _T("bldg:Building/gen:stringAttribute[@name='建物ID']")
#define XPATH_stringAttribute2 _T("gen:value")
#define XPATH_stringAttribute1_2 _T("bldg:Building/uro:buildingIDAttribute/uro:BuildingIDAttribute")
#define XPATH_stringAttribute2_2 _T("uro:buildingID")

#define BOUND_XPATH1 _T("core:CityModel/gml:boundedBy/gml:Envelope/gml:lowerCorner")
#define BOUND_XPATH2 _T("core:CityModel/gml:boundedBy/gml:Envelope/gml:upperCorner")

// 災害リスク(浸水)
#define WTR_XPATH1 _T("wtr:WaterBody")
#define WTR_XPATH2 _T("wtr:lod1MultiSurface/gml:MultiSurface/gml:surfaceMember")
// 災害リスク(土砂災害)
#define LSLD_XPATH1 _T("urf:SedimentDisasterProneArea")
#define LSLD_XPATH2 _T("urf:lod1MultiSurface/gml:MultiSurface/gml:surfaceMember")

#define CANCELFILE _T("cancel.txt")

const std::string landAnalyzeFile = "土地ごと予測発電量.csv";
const std::string judgeFile = "建物別適地判定結果.csv";
const std::string outputFile = "集計結果.csv";
const std::string extension_csv = ".csv";
const std::string extension_gml = ".gml";
const std::string outputHeader = "エリアID,範囲内建物数,予測日射量総計(kWh/m2),予測発電量総計(kWh),光害を発生させる建物数,光害発生時間総計（夏至）,光害発生時間総計（春分）,光害発生時間総計（冬至）,光害発生時間総計（指定日）,範囲内優先度5建物数,範囲内優先度4建物数,範囲内優先度3建物数,範囲内優先度2建物数,範囲内優先度1建物数";
const int priorityLevel1 = 1;
const int priorityLevel2 = 2;
const int priorityLevel3 = 3;
const int priorityLevel4 = 4;
const int priorityLevel5 = 5;

// 系番号
int JPZONE;


// エクスポートとインポートの切り替え
#ifdef AGGREGATEDATA_EXPORTS
#define VC_DLL_EXPORTS extern "C" __declspec(dllexport)
#else
#define VC_DLL_EXPORTS extern "C" __declspec(dllimport)
#endif


// 解析エリア(C#受け渡し用)
typedef struct AnalyzeAreaData
{
    char strAreaId[10];
    char strAreaName[100];
    int nPointCount;
    double pPointArray[256];
};

VC_DLL_EXPORTS int __cdecl AggregateBldgFiles(const char* str, const char* strOut);
VC_DLL_EXPORTS int __cdecl AggregateLandFiles(const char* str, const char* strOut);
VC_DLL_EXPORTS int __cdecl AggregateAllData(const char* str, const char* strOut);
VC_DLL_EXPORTS void __cdecl Initialize();
VC_DLL_EXPORTS void __cdecl DllDispose();
VC_DLL_EXPORTS int __cdecl AnalyzeHazardRisk(const char* str, const char* strOut, bool fldrisk, bool tnmrisk, bool lsldrisk);
VC_DLL_EXPORTS void __cdecl AddAnalyzeAreaData(AnalyzeAreaData* p);

VC_DLL_EXPORTS void* __cdecl GetAllList();
VC_DLL_EXPORTS void* __cdecl GetAllAreaList();

// 適地判定結果
typedef struct judgeSuitablePlace
{
    std::string strBuildingId;           // 建物ID
    int priority;						 // 優先度
    std::string condition_1_1_1;		 // 判定条件_1_1_1
    std::string condition_1_1_2;		 // 判定条件_1_1_2
    std::string condition_1_2;			 // 判定条件_1_2
    std::string condition_1_3;			 // 判定条件_1_3
    std::string condition_2_1;			 // 判定条件_2_1
    std::string condition_2_2;			 // 判定条件_2_2
    std::string condition_2_3;			 // 判定条件_2_3
    std::string condition_2_4;			 // 判定条件_2_4
    std::string condition_3_1;			 // 判定条件_3_1
    std::string condition_3_2;			 // 判定条件_3_2
    std::string condition_3_3;			 // 判定条件_3_3

} JUDGESUITABLEPLACE;

// 適地判定リスト
typedef struct judgeList
{
    int meshID;                          // メッシュID
    std::vector<JUDGESUITABLEPLACE> judgeSuitablePlaceList;  // 適地判定結果リスト

} JUDGELIST;

// 屋根詳細
typedef struct surfaceMembers
{
    std::vector<CPointBase> posList;               // 座標リスト
} SURFACEMEMBERS;

// 屋根
typedef struct roofSurfaces
{
    std::string roofSurfaceId;                     // 屋根ID
    std::vector<SURFACEMEMBERS> roofSurfaceList;   // 屋根詳細リスト
} ROOFSURFACES;

// 建物
typedef struct building
{

    std::string strBuildingId;           // 建物ID
    double dBuildingHeight;		         // 高さ
    std::vector<ROOFSURFACES> roofSurfaceList;   // 屋根リスト
    double dSolorRadiation;              // 年間予測日射量(kWh/m2)
    int iBldStructureType;               // 構造種別
    double dFloodDepth;                  // 洪水浸水想定の浸水深(メートル)
    double dTsunamiHeight;               // 津波浸水想定(メートル)
    bool bLandslideArea;                 // 土砂災害警戒区域(タグがあればtrue)
    int iBldStructureType2;              // 都市ごとの独自区分に基づく建築構造の種類
    int iFloorType;                      // 都市ごとの独自区分に基づく地上階数の範囲

} BUILDING;


// 建物リスト
typedef struct bldgList
{
    int meshID;                          // メッシュID
    std::vector<BUILDING> buildingList;  // 建物リスト

} BLDGLIST;

// 集計元建物情報
typedef struct agtBuilding
{
    std::string strBuildingId;           // 建物ID
    double dSolorRadiation;              // 予測日射量(kWh/m2)
    double dElectricGeneration;          // 予測発電量(kWh)
    double dLightPollutionSummer;        // 光害発生時間(夏至)
    double dLightPollutionSpling;        // 光害発生時間(春分)
    double dLightPollutionWinter;        // 光害発生時間(冬至)
    double dLightPollutionOneDay;        // 光害発生時間(指定日)

} AGTBUILDING;

// 土地
typedef struct landsurface
{
    double dLandHeight;                  // 高さ
    double dSolorRadiation;              // 予測日射量(kWh/m2)
    double dFloodDepth;                  // 洪水浸水想定の浸水深(メートル)
    double dTsunamiHeight;               // 津波浸水想定(メートル)
    bool bLandslideArea;                 // 土砂災害リスク区域(範囲内true)

} LANDSURFACE;

// エリア
typedef struct areaData
{
    std::string areaID;                         // エリアID
    std::string areaName;                       // エリア名称
    std::vector<CPoint2D> pos2dList;            // エリア構成点リスト
    std::vector<CPoint2DPolygon> polygons;      // 分割した凸ポリゴンリスト

    // エリア内の各データ
    std::vector<BLDGLIST> buildList;            // 解析エリア周辺の建物リスト
    LANDSURFACE landData;                       // 土地データ

} AREADATA;

// 集計元建物リスト
typedef struct agtBldgList
{
    int meshID;                           // メッシュID
    std::vector<AGTBUILDING> buildingList;// 建物リスト

} AGTBLDGLIST;


// 集計元土地情報
typedef struct agtLand
{
    std::string strLandId;               // エリアID
    double dSolorRadiation;              // 予測日射量(kWh/m2)
    double dElectricGeneration;          // 予測発電量(kWh)

} AGTLAND;


// 座標
typedef struct position
{
    double lon;                         // 経度
    double lat;                         // 緯度
    double ht;                          // 高さ

} POSITION;

// 災害リスク(洪水浸水・津波)モデル共通 LOD1データ
typedef struct hazardRiskLOD1
{
    std::vector<SURFACEMEMBERS> wtrSurfaceList;    // 面詳細リスト

} HAZARDRISKLOD1;

typedef struct fldRisk
{
    std::string meshID;                         // メッシュID
    std::string scale;                          // 規模
    std::vector<HAZARDRISKLOD1> fldListLOD1;    // 浸水面リスト(LOD1)
    // 最大緯度経度
    POSITION upperCorner;
    // 最小緯度経度
    POSITION lowerCorner;
} FLDRISK;

typedef struct fldRisks
{
    std::string type;                           // 洪水浸水想定区域フォルダ名
    std::string description;                    // 洪水浸水想定区域図フォルダ名(水系名、指定河川名)
    std::vector<FLDRISK> vecFldRisk;            // 洪水浸水想定データリスト

} FLDRISKLIST;

// 災害リスク(津波)
typedef struct tnmRisk
{
    std::string meshID;                         // メッシュID
    std::vector<HAZARDRISKLOD1> tnmRiskLOD1;    // 浸水面リスト(LOD1)
    // 最大緯度経度
    POSITION upperCorner;
    // 最小緯度経度
    POSITION lowerCorner;
} TNMRISK;

typedef struct tnmRisks
{
    std::string description;                    // 津波浸水想定フォルダ名
    std::vector<TNMRISK> vecTnmRisk;            // 津波浸水想定データリスト

} TNMRISKLIST;

// 災害リスク(土砂災害)
typedef struct lsldRisk
{
    std::string meshID;		                    // 3次メッシュID
    std::vector<HAZARDRISKLOD1> lsldRiskLOD1;   // 災害モデルリスト(LOD1)
    // 最大緯度経度
    POSITION upperCorner;
    // 最小緯度経度
    POSITION lowerCorner;
} LSLDRISK;

// 災害リスク
typedef struct hazardRisk
{
    std::vector<FLDRISKLIST> fldRisks;          // 洪水浸水想定データリスト
    std::vector<TNMRISKLIST> tnmRisks;          // 津波浸水想定データリスト
    std::vector<LSLDRISK> lsldRisks;            // 土砂災害データリスト

} HAZARDRISK;

// 災害リスクデータ
HAZARDRISK hazardRiskData{};

// 全体リスト
std::vector<BLDGLIST> allList{};
void* GetAllList()
{
    return (void*)(&allList);
}

// 範囲リストを取得
std::vector<AREADATA> allAreaList{};
void* __cdecl GetAllAreaList()
{
    return (void*)(&allAreaList);
}

class AggregateData
{
public:
    AggregateData(void);
    ~AggregateData(void);

};

