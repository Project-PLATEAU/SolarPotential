#pragma once

#include "../../../../LIB/CommonUtil/CGeoUtil.h"

#include <string>
#include <vector>
using namespace std;

#define CANCELFILE "cancel.txt"

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


// vector<BLDGLIST>を取得する
extern "C" __declspec(dllimport) void* __cdecl GetAllList();