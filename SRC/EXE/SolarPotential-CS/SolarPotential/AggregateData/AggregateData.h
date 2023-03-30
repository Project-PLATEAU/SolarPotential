#pragma once

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーからほとんど使用されていない部分を除外する

// プログラムに必要な追加ヘッダーをここで参照してください
#include "../../../../LIB/CommonUtil/CGeoUtil.h"

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

const std::wstring uroNamespace1 = L"https://www.chisou.go.jp/tiiki/toshisaisei/itoshisaisei/iur/uro/1.5";
const std::wstring uroNamespace2 = L"https://www.geospatial.jp/iur/uro/2.0";

#define NAME_SPACE _T("xmlns:core='http://www.opengis.net/citygml/2.0' xmlns:bldg='http://www.opengis.net/citygml/building/2.0'  xmlns:uro='https://www.chisou.go.jp/tiiki/toshisaisei/itoshisaisei/iur/uro/1.5' xmlns:gen='http://www.opengis.net/citygml/generics/2.0' xmlns:gml='http://www.opengis.net/gml'")
#define NAME_SPACE2 _T("xmlns:core='http://www.opengis.net/citygml/2.0' xmlns:bldg='http://www.opengis.net/citygml/building/2.0'  xmlns:uro='https://www.geospatial.jp/iur/uro/2.0' xmlns:gen='http://www.opengis.net/citygml/generics/2.0' xmlns:gml='http://www.opengis.net/gml'")

#define XPATH1 _T("core:CityModel/core:cityObjectMember/bldg:Building/bldg:boundedBy/bldg:RoofSurface")
#define XPATH2 _T("core:CityModel/core:cityObjectMember")
#define XPATH3 _T("bldg:Building")
#define XPATH4 _T("bldg:Building/bldg:boundedBy")
#define XPATH5 _T("bldg:RoofSurface")
#define XPATH6 _T("bldg:RoofSurface/bldg:lod2MultiSurface/gml:MultiSurface/gml:surfaceMember")
#define XPATH7 _T("gml:Polygon/gml:exterior/gml:LinearRing/gml:posList")

#define XPATH_measureAttribute1 _T("bldg:Building/gen:measureAttribute[@name='年間予測日射量']")
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

#define XPATH_aggregateData1 _T("gen:measureAttribute[@name='年間予測日射量']")
#define XPATH_aggregateData2 _T("gen:value")
#define XPATH_aggregateData3 _T("gen:measureAttribute[@name='年間予測発電量']")
#define XPATH_aggregateData4 _T("gen:measureAttribute[@name='光害発生時間（夏至）']")
#define XPATH_aggregateData5 _T("gen:measureAttribute[@name='光害発生時間（春分）']")
#define XPATH_aggregateData6 _T("gen:measureAttribute[@name='光害発生時間（冬至）']")

// 建物ID
#define XPATH_stringAttribute1 _T("bldg:Building/gen:stringAttribute[@name='建物ID']")
#define XPATH_stringAttribute2 _T("gen:value")
#define XPATH_stringAttribute1_2 _T("bldg:Building/uro:buildingIDAttribute/uro:BuildingIDAttribute")
#define XPATH_stringAttribute2_2 _T("uro:buildingID")

#define CANCELFILE _T("cancel.txt")

const std::string judgeFile = "建物別適地判定結果.csv";
const std::string outputFile = "集計結果.csv";
const std::string extension_csv = ".csv";
const std::string extension_gml = ".gml";
const std::string outputHeader = "範囲内建物数,年間予測日射量総計,年間予測発電量総計,光害を発生させる建物数,光害発生時間総計（夏至）,光害発生時間総計（春分）,光害発生時間総計（冬至）,範囲内優先度5建物数,範囲内優先度4建物数,範囲内優先度3建物数,範囲内優先度2建物数,範囲内優先度1建物数";
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

VC_DLL_EXPORTS int __cdecl AggregateBldgFiles(const char* str, const char* strOut);
VC_DLL_EXPORTS int __cdecl AggregateAllData(const char* str, const char* strOut);
VC_DLL_EXPORTS void __cdecl SetJPZone();

VC_DLL_EXPORTS void* __cdecl GetAllList();

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
	double dSolorRadiation;              // 年間予測日射量(kWh/m2)
	double dElectricGeneration;          // 年間予測発電量(kWh/年)
	double dLightPollutionSummer;        // 光害発生時間(夏至)
	double dLightPollutionSpling;        // 光害発生時間(春分)
	double dLightPollutionWinter;        // 光害発生時間(冬至)

} AGTBUILDING;


// 集計元建物リスト
typedef struct agtBldgList
{
	int meshID;                           // メッシュID
	std::vector<AGTBUILDING> buildingList;// 建物リスト

} AGTBLDGLIST;

// 全体リスト
std::vector<BLDGLIST> allList{};
void* GetAllList()
{
	return (void*)(&allList);
}

class AggregateData
{
public:
	AggregateData(void);
	~AggregateData(void);

};

