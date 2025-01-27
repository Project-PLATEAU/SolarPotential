#pragma once

#include <string>
#include <vector>
#include <map>
#include "../../LIB/CommonUtil/CGeoUtil.h"

using namespace std;

#define CANCELFILE "cancel.txt"

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

} DEMMEMBERS;

// DEM
typedef struct demList
{
    std::string lv2meshID;                  // 2次メッシュID
    std::vector<DEMMEMBERS> demMemberList;  // DEM詳細リスト

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

} INTERIOR;

// 面詳細
typedef struct surfaceMembers
{
    std::string polygon;                                    // ポリゴンID
    std::string linearRing;                                 // ラインID
    std::vector<CPointBase> posList;                        // 座標リスト(外周)
    std::vector<INTERIOR> interiorList;                     // 内周ポリゴンリスト

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
    std::vector<ROOFSURFACES> roofSurfaceList;  // 屋根リスト
    std::vector<WALLSURFACES> wallSurfaceList;  // 壁リスト
    //bool analyze{ false };

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

// 3次メッシュ
typedef struct bldgList
{
    std::string meshID;		                    // 3次メッシュID
    std::vector<BUILDINGS> buildingList;        // 建物リスト
    std::vector<BUILDINGSLOD1> buildingListLOD1; // 建物リスト(LOD1)
    // メッシュ座標のMIN,MAX(平面直角座標)
    int bbMinX{ 0 }, bbMinY{ 0 };
    int bbMaxX{ 0 }, bbMaxY{ 0 };
} BLDGLIST;


// 土地面詳細
typedef struct landSurfaceMembers
{
    std::vector<CPointBase> posList;        // 座標リスト(メッシュ)
    //std::vector<CTriangle> triangleList;    // 三角形リスト

} LANDSURFACEMEMBERS;

// 土地面
typedef struct landSurfaces
{
    std::string surfaceID;
    std::vector<LANDSURFACEMEMBERS> landSurfaceList;    // 土地面(メッシュ)の詳細リスト
    std::vector<MESHPOSITION_XY> meshPosList;           // メッシュ座標リスト
    int meshSize{ 5 };                                  // 5m
    int area;                                           // 土地面積(抽出した土地面のピクセル数)

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

} AREADATA;

// 道路
typedef struct tranSurfaces
{
    //std::string tranSurfaceId;                      // 道路面ID
    std::vector<SURFACEMEMBERS> tranSurfaceList;    // 道路詳細リスト

} TRANSURFACES;

// 道路LOD1
typedef struct transLOD1
{
    //std::string road;                           // 道路ID
    std::vector<TRANSURFACES> tranSurfaceList;  // 道路リスト

} TRANSLOD1;

typedef struct transInfo
{
    std::vector<TRANSLOD1> tranListLOD1;        // 道路リスト(LOD1)
    int bbMinX{ 0 }, bbMinY{ 0 };
    int bbMaxX{ 0 }, bbMaxY{ 0 };
} TRANSINFO;

// 道路リスト
typedef struct tranList
{
    std::string meshID;		                    // 3次メッシュID
    std::vector<TRANSLOD1> tranListLOD1;        // 道路リスト(LOD1)
    // メッシュ座標のMIN,MAX(平面直角座標)
    int bbMinX{ 0 }, bbMinY{ 0 };
    int bbMaxX{ 0 }, bbMaxY{ 0 };
} TRANLIST;


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
    areaID = 0,                 // エリアID
    meshID = 1,                 // メッシュID
    building = 2,               // 建物ID
    solarInsolation = 4,        // 日射量
    solarPowerGeneration = 5,   // 発電量
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

// vector<AREADATA>を取得する
extern "C" __declspec(dllimport) void* __cdecl GetAllAreaList();

