#pragma once
#include "stdafx.h"
#include "../../LIB/CommonUtil/CGeoUtil.h"


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

#include <map>

#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")

// 使用する名前空間のKey（必要になったら追加）
std::wstring namespaceKeys[] = {
     L"xmlns:core",
     L"xmlns:bldg",
     L"xmlns:gen",
     L"xmlns:gml",
     L"xmlns:app",
     L"xmlns:uro",
     L"xmlns:dem",
     L"xmlns:tran",

};

// CityGMLのバージョン
enum class eCityGMLVersion
{
    VERSION_1,          // v1
    VERSION_2,          // v2
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

#define XPATH14 _T("gml:Polygon/gml:interior")
#define XPATH15 _T("gml:LinearRing")
#define XPATH16 _T("gml:LinearRing/gml:posList")

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

#define TRAN_XPATH1 _T("tran:Road/tran:lod1MultiSurface/gml:MultiSurface")
#define TRAN_XPATH2 _T("gml:surfaceMember")

#define TEX_XPATH1 _T("core:CityModel")
#define TEX_XPATH2 _T("core:CityModel/app:appearanceMember/app:Appearance")
#define TEX_XPATH3 _T("core:CityModel/app:appearanceMember")
#define OUTPUTFILE _T("initFile_Coordinates.txt")
#define CANCELFILE _T("cancel.txt")

#define INPUTFILE1 _T("建物ごと光害発生時間.csv")         // 予測光害発生時間
#define INPUTFILE2 _T("建物ごと予測発電量.csv")           // 予測日射量・予測発電量ファイル名

#define IMG_NODATA -9999

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

// DEM詳細
typedef struct demMembers
{
    std::string lv3meshID;                      // 3次メッシュID
    std::vector<CTriangle> posTriangleList;     // 三角形の座標リスト
    // 3次メッシュ範囲(三角形が存在する範囲)のバウンディング
    double bbMinX{ 0.0 }, bbMinY{ 0.0 };
    double bbMaxX{ 0.0 }, bbMaxY{ 0.0 };

    ~demMembers()
    {
        posTriangleList.clear();
    }
} DEMMEMBERS;

// DEM
typedef struct demList
{
    std::string lv2meshID;                  // 2次メッシュID
    std::vector<DEMMEMBERS> demMemberList;  // DEM詳細リスト

    ~demList()
    {
        demMemberList.clear();
    }
} DEMLIST;

// 座標
typedef struct position
{
    double lon;		                        // 経度
    double lat;		                        // 緯度
    double ht;		                        // 高さ


} POSITION;

// interior
typedef struct interior
{
    std::string linearRing;                 // ラインID
    std::vector<CPointBase> posList;        // 座標リスト

    ~interior()
    {
        posList.clear();
    }
} INTERIOR;

// 面詳細
typedef struct surfaceMembers
{
    std::string polygon;                                    // ポリゴンID
    std::string linearRing;                                 // ラインID
    std::vector<CPointBase> posList;                        // 座標リスト(外周)
    std::vector<INTERIOR> interiorList;                     // 内周ポリゴンリスト

    ~surfaceMembers()
    {
        posList.clear();
        interiorList.clear();
    }
} SURFACEMEMBERS;

// 屋根
typedef struct roofSurfaces
{
    std::string roofSurfaceId;		            // 屋根ID
    std::vector<SURFACEMEMBERS> roofSurfaceList; // 屋根詳細リスト
    std::vector<MESHPOSITION_XY> meshPosList;       // 1mメッシュ座標リスト

    ~roofSurfaces()
    {
        roofSurfaceList.clear();
        meshPosList.clear();
    }
} ROOFSURFACES;

// 壁
typedef struct wallSurfaces
{
    std::string wallSurfaceId;		            // 壁ID
    std::vector<SURFACEMEMBERS> wallSurfaceList; // 壁詳細リスト

    ~wallSurfaces()
    {
        wallSurfaceList.clear();
    }
} WALLSURFACES;

// 建物LOD2
typedef struct buildings
{
    std::string building;		                // 建物ID
    std::vector<ROOFSURFACES> roofSurfaceList;  // 屋根リスト
    std::vector<WALLSURFACES> wallSurfaceList;  // 壁リスト

    ~buildings()
    {
        roofSurfaceList.clear();
        wallSurfaceList.clear();
    }
} BUILDINGS;

// 建物LOD1(LOD1しかない建物)
typedef struct buildingsLOD1
{
    std::string building;
    std::vector<WALLSURFACES> wallSurfaceList;   // 壁リスト（全ての面）

    ~buildingsLOD1()
    {
        wallSurfaceList.clear();
    }
}BUILDINGSLOD1;

// 建物、メッシュ座標
typedef struct buildingsInfo
{
    std::vector<BUILDINGS> buildingList;        // 建物リスト
    std::vector<BUILDINGSLOD1> buildingListLOD1; // 建物リスト(LOD1)
    int bbMinX{ 0 }, bbMinY{ 0 };
    int bbMaxX{ 0 }, bbMaxY{ 0 };

    ~buildingsInfo()
    {
        buildingList.clear();
        buildingListLOD1.clear();
    }
} BUILDINGSINFO;

// 3次メッシュ
typedef struct bldgList
{
    std::string meshID;		                    // 3次メッシュID
    std::vector<BUILDINGS> buildingList;        // 建物リスト
    std::vector<BUILDINGSLOD1> buildingListLOD1; // 建物リスト(LOD1)
    // メッシュ座標のMIN,MAX(平面直角座標)
    int bbMinX{ 0 }, bbMinY{ 0 };
    int bbMaxX{ 0 }, bbMaxY{ 0 };

    ~bldgList()
    {
        buildingList.clear();
        buildingListLOD1.clear();
    }

} BLDGLIST;


// 土地面詳細
typedef struct landSurfaceMembers
{
    std::vector<CPointBase> posList;        // 座標リスト(輪郭)
    //std::vector<CTriangle> triangleList;    // 三角形リスト

    ~landSurfaceMembers()
    {
        posList.clear();
    }
} LANDSURFACEMEMBERS;

// 土地面
typedef struct landSurfaces
{
    std::string surfaceID;
    std::vector<LANDSURFACEMEMBERS> landSurfaceList;    // 土地面の詳細リスト(メッシュ)
    std::vector<MESHPOSITION_XY> meshPosList;           // メッシュ座標リスト
    int meshSize{ 5 };                                  // 5m or 10m
    int area;                                           // 土地面積(抽出した土地面のピクセル数)

    ~landSurfaces()
    {
        landSurfaceList.clear();
        meshPosList.clear();
    }
} LANDSURFACES;

// 3次メッシュIDと建物リストのマップ
typedef std::map<std::string, std::vector<BUILDINGS*>> BuildingsMap;

// エリア
typedef struct areaData
{
    std::string areaID;                         // エリアID
    std::string areaName;                       // エリア名称
    std::vector<CPoint2D> pos2dList;            // エリア構成点リスト
    int direction;                              // エリア内の補正方位
    double degree;                              // エリア内の補正傾斜角
    std::string areaExplanation;                // エリア説明
    bool isWater;                               // 水部フラグ

    // エリア内の各データ
    std::vector<BLDGLIST*> neighborBldgList;    // 解析エリア周辺の3次メッシュ建物リスト
    std::vector<DEMMEMBERS*> neighborDemList;   // 解析エリア周辺の3次メッシュDEMリスト
    BuildingsMap targetBuildings;               // 解析対象の建物リスト(key:3次メッシュID)
    LANDSURFACES landSurface;                   // 土地データ
    std::vector<CPointBase> pointMemData;       // 抽出した土地面のポイントデータ
    double bbMinX{ DBL_MAX }, bbMinY{ DBL_MAX };
    double bbMaxX{ -DBL_MAX }, bbMaxY{ -DBL_MAX };

    bool analyzeBuild{ false };                 // 範囲内建物の解析対象フラグ
    bool analyzeLand{ false };                  // 範囲内土地の解析対象フラグ

    std::vector<std::string> gmlFileList;       // 範囲内のCityGMLファイルリスト

    ~areaData()
    {
        neighborBldgList.clear();
        neighborDemList.clear();
        targetBuildings.clear();
        pointMemData.clear();
    }

    bool operator == (const areaData& a) const {
        return areaID == a.areaID;
    }

} AREADATA;


// 道路
typedef struct tranSurfaces
{
    //std::string tranSurfaceId;                      // 道路面ID
    std::vector<SURFACEMEMBERS> tranSurfaceList;    // 道路詳細リスト

    ~tranSurfaces()
    {
        tranSurfaceList.clear();
    }
} TRANSURFACES;

// 道路LOD1
typedef struct transLOD1
{
    //std::string road;                           // 道路ID
    std::vector<TRANSURFACES> tranSurfaceList;  // 道路リスト

    ~transLOD1()
    {
        tranSurfaceList.clear();
    }
} TRANSLOD1;

typedef struct transInfo
{
    std::vector<TRANSLOD1> tranListLOD1;        // 道路リスト(LOD1)
    int bbMinX{ 0 }, bbMinY{ 0 };
    int bbMaxX{ 0 }, bbMaxY{ 0 };

    ~transInfo()
    {
        tranListLOD1.clear();
    }
} TRANSINFO;

// 道路リスト
typedef struct tranList
{
    std::string meshID;		                    // 3次メッシュID
    std::vector<TRANSLOD1> tranListLOD1;        // 道路リスト(LOD1)
    // メッシュ座標のMIN,MAX(平面直角座標)
    int bbMinX{ 0 }, bbMinY{ 0 };
    int bbMaxX{ 0 }, bbMaxY{ 0 };

    ~tranList()
    {
        tranListLOD1.clear();
    }
} TRANLIST;


// LOD2出力項目
typedef struct lod2outList
{
    std::string areaID;		                    // エリアID
    std::string meshID;		                    // メッシュID
    std::string building;		                // 建物ID
    std::string solarInsolation;                // 年間日射量
    std::string solarPowerGeneration;           // 年間発電量
    std::string lightPollutionTimeSpring;		// 公害発生時間数_春分
    std::string lightPollutionTimeSummer;		// 公害発生時間数_夏至
    std::string lightPollutionTimeWinter;		// 公害発生時間数_冬至
    std::string lightPollutionTimeOneDay;		// 公害発生時間数_指定日(or 指定月の代表日)


    bool operator<(const lod2outList& right) const {
        return meshID == right.meshID ? building < right.building : meshID < right.meshID;
    }

} LOD2OUTLIST;

// （CSV読み取り用）年間予測日射量、発電量
enum struct yearPrediction
{
    areaID = 0,                 // エリアID
    meshID = 1,                 // メッシュID
    building = 2,               // 建物ID
    solarInsolation = 4,        // 日射量
    solarPowerGeneration = 5,   // 発電量
};

// （CSV読み取り用）光害発生時間総数
enum struct lightPollution
{
    areaID = 0,                 // エリアID
    meshID = 1,                 // メッシュID
    building = 2,               // 建物ID
    summer = 3,                 // 夏至
    spling = 4,                 // 春分
    winter = 5,                 // 冬至
    oneday = 6,                 // 指定日(or 指定月の代表日)
};

// 解析範囲リストを取得
std::vector<AREADATA> allAreaList{};
void* __cdecl GetAllAreaList()
{
    return (void*)(&allAreaList);
}

// 道路リストを取得
std::vector<TRANLIST> allTranList{};
void* __cdecl GetAllTranList()
{
    return (void*)(&allTranList);
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
