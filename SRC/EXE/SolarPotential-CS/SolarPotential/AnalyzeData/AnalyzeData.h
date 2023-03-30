#pragma once
#include "stdafx.h"
#include "../../../../LIB/CommonUtil/CGeoUtil.h"


#include <Windows.h>
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
#include <fstream>
#include <iomanip>

#include <assert.h>
#include <cmath>

#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")


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

#define NAME_SPACE _T("xmlns:core='http://www.opengis.net/citygml/2.0' xmlns:bldg='http://www.opengis.net/citygml/building/2.0' xmlns:gen='http://www.opengis.net/citygml/generics/2.0' xmlns:gml='http://www.opengis.net/gml' xmlns:app='http://www.opengis.net/citygml/appearance/2.0' xmlns:uro='https://www.geospatial.jp/iur/uro/2.0'")
#define DEM_NAME_SPACE _T("xmlns:core='http://www.opengis.net/citygml/2.0' xmlns:dem='http://www.opengis.net/citygml/relief/2.0' xmlns:gml='http://www.opengis.net/gml'")

#define XPATH1 _T("bldg:Building/bldg:lod2Solid")
#define XPATH2 _T("core:CityModel/core:cityObjectMember")
#define XPATH3 _T("bldg:Building")
#define XPATH4 _T("bldg:Building/bldg:boundedBy")
#define XPATH5 _T("bldg:RoofSurface")
#define XPATH6 _T("bldg:RoofSurface/bldg:lod2MultiSurface/gml:MultiSurface/gml:surfaceMember")
#define XPATH7 _T("gml:Polygon")
#define XPATH8 _T("gml:Polygon/gml:exterior/gml:LinearRing")
#define XPATH9 _T("gml:Polygon/gml:exterior/gml:LinearRing/gml:posList")

#define XPATH10 _T("bldg:WallSurface")
#define XPATH11 _T("bldg:WallSurface/bldg:lod2MultiSurface/gml:MultiSurface/gml:surfaceMember")

#define XPATH12 _T("bldg:Building/bldg:lod1Solid/gml:Solid")
#define XPATH13 _T("gml:exterior/gml:CompositeSurface/gml:surfaceMember")

// 建物ID
#define XPATH_stringAttribute1 _T("bldg:Building/gen:stringAttribute[@name='建物ID']")
#define XPATH_stringAttribute2 _T("gen:value")
#define XPATH_stringAttribute1_2 _T("bldg:Building/uro:buildingIDAttribute/uro:BuildingIDAttribute")
#define XPATH_stringAttribute2_2 _T("uro:buildingID")

#define BOUND_XPATH1 _T("core:CityModel/gml:boundedBy/gml:Envelope/gml:lowerCorner")
#define BOUND_XPATH2 _T("core:CityModel/gml:boundedBy/gml:Envelope/gml:upperCorner")


#define DEM_XPATH1 _T("dem:ReliefFeature/dem:reliefComponent/dem:TINRelief/dem:tin/gml:TriangulatedSurface/gml:trianglePatches/gml:Triangle")
#define DEM_XPATH2 _T("gml:exterior/gml:LinearRing/gml:posList")
#define DEM_XPATH3 _T("dem:ReliefFeature/gml:name")

#define TEX_XPATH1 _T("core:CityModel")
#define TEX_XPATH2 _T("core:CityModel/app:appearanceMember/app:Appearance")
#define TEX_XPATH3 _T("core:CityModel/app:appearanceMember")
#define OUTPUTFILE _T("initFile_Coordinates.txt")
#define CANCELFILE _T("cancel.txt")

#define INPUTFILE1 _T("建物毎光害発生時間.csv")         // 予測光害発生時間
#define INPUTFILE2 _T("建物毎年間予測発電量.csv")       // 年間予測日射量・年間予測発電量

// 系番号
int JPZONE;

// 最大緯度経度
CPoint2D maxPosition;
// 最小緯度経度
CPoint2D minPosition;

// 全体jpgサイズ
int jpgWidth = 0;
int jpgHeight = 0;

// 切り出しjpgサイズ
int trimWidth = 0;
int trimHeight = 0;

// jpg初期座標
double tfw_x = 0.0;;
double tfw_y = 0.0;;
double tfw_meshSize = 0.0;


// 1mメッシュ座標
typedef struct meshPositionXY
{
    double leftTopX;                        // 左上X
    double leftTopY;                        // 左上Y
    double leftDownX;                       // 左下X
    double leftDownY;                       // 左下Y
    double rightTopX;                       // 右上X
    double rightTopY;                       // 右上Y
    double rightDownX;                      // 右下X
    double rightDownY;                      // 右下Y

} MESHPOSITION_XY;

// DEM座標
typedef struct demPosition
{
    double lon;		                        // 経度
    double lat;		                        // 緯度
    double ht;		                        // 高さ

} DEMPOSITION;

// DEMメッシュ
typedef struct demList
{
    std::string meshID;		                    // メッシュID
    std::vector<CTriangle> posTriangleList;     // 三角形の座標リスト

} DEMLIST;

// 座標
typedef struct position
{
    double lon;		                        // 経度
    double lat;		                        // 緯度
    double ht;		                        // 高さ


} POSITION;

// 屋根詳細
typedef struct surfaceMembers
{
    std::string polygon;		                    // ポリゴンID
    std::string linearRing;		                // ラインID
    std::vector<CPointBase> posList;               // 座標リスト

} SURFACEMEMBERS;

// 屋根
typedef struct roofSurfaces
{
    std::string roofSurfaceId;		            // 屋根ID
    std::vector<SURFACEMEMBERS> roofSurfaceList; // 屋根詳細リスト
    std::vector<MESHPOSITION_XY> meshPosList;       // 1mメッシュ座標リスト

} ROOFSURFACES;

// 壁
typedef struct wallSurfaces
{
    std::string wallSurfaceId;		            // 壁ID
    std::vector<SURFACEMEMBERS> wallSurfaceList; // 壁詳細リスト

} WALLSURFACES;

// 建物LOD2
typedef struct buildings
{
    std::string building;		                // 建物ID
    std::vector<ROOFSURFACES> roofSurfaceList;   // 屋根リスト
    std::vector<WALLSURFACES> wallSurfaceList;   // 壁リスト

} BUILDINGS;

// 建物LOD1(LOD1しかない建物)
typedef struct buildingsLOD1
{
    std::string building;
    std::vector<WALLSURFACES> wallSurfaceList;   // 壁リスト（全ての面）
}BUILDINGSLOD1;

// 建物、メッシュ座標
typedef struct buildingsInfo
{
    std::vector<BUILDINGS> buildingList;        // 建物リスト
    std::vector<BUILDINGSLOD1> buildingListLOD1; // 建物リスト(LOD1)
    int bbMinX{ 0 }, bbMinY{ 0 };
    int bbMaxX{ 0 }, bbMaxY{ 0 };
} BUILDINGSINFO;

// メッシュ
typedef struct bldgList
{
    std::string meshID;		                    // メッシュID
    std::vector<BUILDINGS> buildingList;        // 建物リスト
    std::vector<BUILDINGSLOD1> buildingListLOD1; // 建物リスト(LOD1)
    // メッシュ座標のMIN,MAX(平面直角座標)
    int bbMinX{ 0 }, bbMinY{ 0 };
    int bbMaxX{ 0 }, bbMaxY{ 0 };
} BLDGLIST;

// LOD2出力項目
typedef struct lod2outList
{
    std::string meshID;		                    // メッシュID
    std::string building;		                // 建物ID
    std::string solarInsolation;                // 年間日射量
    std::string solarPowerGeneration;           // 年間発電量
    std::string lightPollutionTimeSpring;		// 公害発生時間数_春分
    std::string lightPollutionTimeSummer;		// 公害発生時間数_夏至
    std::string lightPollutionTimeWinter;		// 公害発生時間数_冬至

    bool operator<(const lod2outList& right) const {
        return meshID == right.meshID ? building < right.building : meshID < right.meshID;
    }

} LOD2OUTLIST;

// （CSV読み取り用）年間予測日射量、発電量
enum struct yearPrediction
{
    meshID = 0,                 // メッシュID
    building = 1,               // 建物ID
    solarInsolation = 2,        // 日射量
    solarPowerGeneration = 3,   // 発電量
};

// （CSV読み取り用）光害発生時間総数
enum struct lightPollution
{
    meshID = 0,                 // メッシュID
    building = 1,               // 建物ID
    summer = 2,                 // 夏至
    spling = 3,                 // 春分
    winter = 4,                 // 冬至
};

// 建物リストを取得
std::vector<BLDGLIST> allList{};
void* __cdecl GetAllList()
{
    return (void*)(&allList);
}

// DEMリストを取得
std::vector<DEMLIST> allDemList{};
void* __cdecl GetAllDemList()
{
    return (void*)(&allDemList);
}

// エンコーダーの取得
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

// srcで指定するjpegファイルの矩形領域(左上sx,sy 幅 width 高さ height)でトリミングしdtcで指定するjpegファイルに保存
int imgcut(TCHAR* dtc, TCHAR* src, int sx, int sy, int wdith, int height);

class AnalyzeData
{
public:
	AnalyzeData(void);
	~AnalyzeData(void);

};
