#pragma once

#include <string>
#include <vector>
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

// vector<BLDGLIST>を取得する
extern "C" __declspec(dllimport) void* __cdecl GetAllList();

// DEMリストを取得
extern "C" __declspec(dllimport) void* __cdecl GetAllDemList();
